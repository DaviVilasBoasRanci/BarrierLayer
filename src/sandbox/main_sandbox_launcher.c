#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sched.h>
#include <sys/mount.h>
#include <linux/limits.h>
#include <signal.h> // For kill()
#include "sandbox_core.h"

#define CHILD_STACK_SIZE (1024 * 1024)
#define MAX_BINDS 16

struct launcher_config {
    int verbose;
    char *target_executable;
    char **target_args;
    char *bind_mounts[MAX_BINDS];
    int num_binds;
};

static int child_function(void *arg) {
    struct launcher_config *config = (struct launcher_config *)arg;

    if (config->verbose) {
        printf("CHILD: Process started inside namespaces.\n");
    }

    // Set a new hostname for the UTS namespace
    if (sethostname("barrier-sandbox", 15) != 0) {
        perror("CHILD: sethostname");
        exit(1);
    }

    // --- Filesystem Setup ---
    if (config->verbose) printf("CHILD: 1. Setting up private mount propagation...\n");
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0) {
        perror("CHILD: mount MS_PRIVATE");
        exit(1);
    }
    if (config->verbose) printf("CHILD:    ...private mount propagation set.\n");

    if (config->verbose) printf("CHILD: 2. Setting up OverlayFS...\n");
    const char* lowerdir = "/home/davivbrdev/BarrierLayer/barrierlayer_sandbox";
    const char* upperdir = "/home/davivbrdev/BarrierLayer/overlay_upper";
    const char* workdir  = "/home/davivbrdev/BarrierLayer/overlay_work";
    const char* mergedir = "/home/davivbrdev/BarrierLayer/overlay_merged";
    char mount_opts[PATH_MAX];
    snprintf(mount_opts, sizeof(mount_opts), "lowerdir=%s,upperdir=%s,workdir=%s", lowerdir, upperdir, workdir);
    if (config->verbose) printf("CHILD:    Mounting overlay with options: %s\n", mount_opts);
    if (mount("overlay", mergedir, "overlay", 0, mount_opts) != 0) {
        perror("CHILD: mount overlayfs");
        exit(1);
    }
    if (config->verbose) printf("CHILD:    ...OverlayFS mounted on %s.\n", mergedir);

    if (config->verbose) printf("CHILD: 3. Setting up Bind Mounts...\n");
    for (int i = 0; i < config->num_binds; i++) {
        char* spec = strdup(config->bind_mounts[i]);
        char* host_path = strtok(spec, ":");
        char* sandbox_path_rel = strtok(NULL, ":");
        if (host_path && sandbox_path_rel) {
            char sandbox_path_abs[PATH_MAX];
            snprintf(sandbox_path_abs, sizeof(sandbox_path_abs), "%s%s", mergedir, sandbox_path_rel);
            if (config->verbose) printf("CHILD:    Binding host '%s' to sandbox '%s'...\n", host_path, sandbox_path_abs);
            // Create directory if it doesn't exist
            if (mkdir(sandbox_path_abs, 0755) != 0 && errno != EEXIST) {
                perror("CHILD: mkdir for bind mount");
            } else if (mount(host_path, sandbox_path_abs, NULL, MS_BIND | MS_REC, NULL) != 0) {
                fprintf(stderr, "CHILD: Failed to bind mount %s to %s\n", host_path, sandbox_path_abs);
                perror("CHILD: mount bind");
            }
        }
        free(spec);
    }
    if (config->verbose) printf("CHILD:    ...bind mounts configured.\n");

    if (config->verbose) printf("CHILD: 4. Chrooting into merged directory...\n");
    if (chroot(mergedir) != 0) {
        perror("CHILD: chroot to merged dir");
        umount(mergedir);
        exit(1);
    }
    if (chdir("/") != 0) {
        perror("CHILD: chdir to new root");
        exit(1);
    }
    if (config->verbose) printf("CHILD:    ...chroot successful.\n");

    if (config->verbose) printf("CHILD: 5. Mounting pseudo-filesystems...\n");
    if (mkdir("proc", 0555) != 0 && errno != EEXIST) { perror("CHILD: mkdir /proc"); exit(1); }
    if (mount("proc", "proc", "proc", 0, NULL) != 0) { perror("CHILD: mount /proc"); exit(1); }
    if (config->verbose) printf("CHILD:    .../proc mounted.\n");
    // --- End of Filesystem Setup ---

    if (initialize_sandbox() != 0) {
        fprintf(stderr, "Sandbox core initialization failed in child.\n");
        exit(1);
    }

    clearenv();
    setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
    setenv("TERM", "xterm-256color", 1);
    setenv("BARRIERLAYER_ACTIVE", "1", 1);

    if (config->verbose) {
        printf("CHILD: Executing target: %s\n", config->target_executable);
    }
    execvp(config->target_executable, config->target_args);
    fprintf(stderr, "CHILD: execvp failed for %s: %s\n", config->target_executable, strerror(errno));
    exit(1);
}

static int execute_target(struct launcher_config *config) {
    printf("[LAUNCHER-DEBUG] Starting sandbox setup...\n");

    printf("[LAUNCHER-DEBUG] 1. Cloning child process with new namespaces...\n");
    char *child_stack = malloc(CHILD_STACK_SIZE);
    if (!child_stack) { perror("malloc"); return -1; }
    char *child_stack_top = child_stack + CHILD_STACK_SIZE;

    // Add CLONE_NEWUSER, CLONE_NEWUTS, CLONE_NEWIPC, CLONE_NEWCGROUP
    int flags = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWUSER | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWCGROUP | SIGCHLD;
    pid_t child_pid = clone(child_function, child_stack_top, flags, config);

    if (child_pid == -1) {
        perror("clone");
        free(child_stack);
        return -1;
    }

    if (config->verbose) {
        printf("LAUNCHER: Child process created with PID %d\n", child_pid);
        printf("LAUNCHER: Configuring UID/GID maps...\n");
    }

    // --- UID/GID Mapping ---
    char map_path[PATH_MAX];
    char map_content[128];
    int fd;

    // Map UID
    snprintf(map_path, sizeof(map_path), "/proc/%d/uid_map", child_pid);
    snprintf(map_content, sizeof(map_content), "0 %d 1", getuid());
    fd = open(map_path, O_WRONLY);
    if (fd < 0) {
        perror("open uid_map");
        kill(child_pid, SIGKILL);
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    if (write(fd, map_content, strlen(map_content)) != strlen(map_content)) {
        perror("write uid_map");
        close(fd);
        kill(child_pid, SIGKILL);
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    close(fd);

    // Deny setgroups
    snprintf(map_path, sizeof(map_path), "/proc/%d/setgroups", child_pid);
    fd = open(map_path, O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "deny", 4) != 4) {
            perror("write setgroups");
            close(fd);
            kill(child_pid, SIGKILL);
            waitpid(child_pid, NULL, 0);
            free(child_stack);
            return -1;
        }
        close(fd);
    }

    // Map GID
    snprintf(map_path, sizeof(map_path), "/proc/%d/gid_map", child_pid);
    snprintf(map_content, sizeof(map_content), "0 %d 1", getgid());
    fd = open(map_path, O_WRONLY);
    if (fd < 0) {
        perror("open gid_map");
        kill(child_pid, SIGKILL);
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    if (write(fd, map_content, strlen(map_content)) != strlen(map_content)) {
        perror("write gid_map");
        close(fd);
        kill(child_pid, SIGKILL);
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    close(fd);
    // --- End of UID/GID Mapping ---

    if (config->verbose) {
        printf("LAUNCHER: UID/GID maps configured.\n");
    }

    printf("[LAUNCHER-DEBUG] 2. Waiting for child process to exit...\n");
    int status;
    waitpid(child_pid, &status, 0);
    free(child_stack);
    printf("[LAUNCHER-DEBUG]    ...child process exited.\n");

    return WEXITSTATUS(status);
}


static int parse_arguments(int argc, char **argv, struct launcher_config *config) {
    memset(config, 0, sizeof(struct launcher_config));
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            config->verbose = 1;
            i++;
        } else if (strcmp(argv[i], "--bind") == 0) {
            if (i + 1 < argc && config->num_binds < MAX_BINDS) {
                config->bind_mounts[config->num_binds++] = argv[i + 1];
                i += 2;
            } else {
                fprintf(stderr, "--bind requires an argument or max binds reached\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else {
            break;
        }
    }

    if (i >= argc) {
        fprintf(stderr, "Error: No target executable specified.\n");
        return -1;
    }
    config->target_executable = argv[i];
    config->target_args = &argv[i];
    return 0;
}

int main(int argc, char **argv) {
    struct launcher_config config;
    printf("BarrierLayer Sandbox Launcher (Bind Mount Version)\n");
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [--verbose] [--bind /host:/sandbox]... -- <executable> [args...]\n", argv[0]);
        return 1;
    }
    if (parse_arguments(argc, argv, &config) != 0) { return 1; }
    int exit_code = execute_target(&config);
    if (config.verbose) { printf("LAUNCHER: Child process exited with code: %d\n", exit_code); }
    return exit_code;
}

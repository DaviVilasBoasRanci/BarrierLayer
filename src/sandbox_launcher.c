#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <sched.h>
#include <sys/mount.h>
#include <libgen.h>

#define MAX_ARGS 256
#define CHILD_STACK_SIZE (1024 * 1024) // 1MB

extern char** environ;

struct launcher_config {
    int verbose;
    int enable_sandbox;
    char *target_executable;
    char **target_args;
    char **environment;
    char *working_directory;
    // This field is no longer used from the old implementation, but might be useful later.
    char *sandboxed_executable_path;
    int debug_fd; // NEW: File descriptor for debug logging
};

// Forward declaration from sandbox_core.c
int initialize_sandbox(void);

// The function that the cloned child will execute.
static int child_function(void *arg) {
    struct launcher_config *config = (struct launcher_config *)arg;

    if (config->enable_sandbox) {
        if (initialize_sandbox() != 0) {
            fprintf(stderr, "Erro fatal ao inicializar a sandbox no processo filho.\n");
            exit(1);
        }
    }

    // Set LD_LIBRARY_PATH for the sandboxed process
    if (setenv("LD_LIBRARY_PATH", "/lib", 1) != 0) { // Set to /lib within the chroot
        perror("setenv LD_LIBRARY_PATH failed");
        exit(1);
    }

    if (config->working_directory) {
        if (chdir(config->working_directory) != 0) {
            perror("Falha ao mudar o diretório de trabalho no filho");
            exit(1);
        }
    }

    execve(config->target_executable, config->target_args, config->environment);
    perror("execve no processo filho falhou");
    exit(1);
}

// The new function that orchestrates the creation of the process with namespaces
static int execute_target(struct launcher_config *config) {
    char *child_stack = malloc(CHILD_STACK_SIZE);
    if (!child_stack) {
        perror("malloc para stack do filho");
        return -1;
    }
    char *child_stack_top = child_stack + CHILD_STACK_SIZE;

    pid_t child_pid;
    int flags = CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWNET | SIGCHLD;

    child_pid = clone(child_function, child_stack_top, flags, config);

    if (child_pid == -1) {
        perror("clone");
        free(child_stack);
        return -1;
    }

    if (config->verbose) {
        printf("LAUNCHER: Processo filho criado com PID %d\n", child_pid);
        printf("LAUNCHER: Configurando mapeamentos de UID/GID...\n");
    }

    char map_path[PATH_MAX];
    char map_content[128];
    int fd;

    snprintf(map_path, sizeof(map_path), "/proc/%d/uid_map", child_pid);
    snprintf(map_content, sizeof(map_content), "0 %d 1", getuid());
    fd = open(map_path, O_WRONLY);
    if (fd < 0) {
        perror("open uid_map");
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    if (write(fd, map_content, strlen(map_content)) != strlen(map_content)) {
        perror("write uid_map");
        close(fd);
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    close(fd);

    snprintf(map_path, sizeof(map_path), "/proc/%d/setgroups", child_pid);
    fd = open(map_path, O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "deny", 4) != 4) {
            perror("write setgroups");
            close(fd);
            waitpid(child_pid, NULL, 0);
            free(child_stack);
            return -1;
        }
        close(fd);
    }

    snprintf(map_path, sizeof(map_path), "/proc/%d/gid_map", child_pid);
    snprintf(map_content, sizeof(map_content), "0 %d 1", getgid());
    fd = open(map_path, O_WRONLY);
    if (fd < 0) {
        perror("open gid_map");
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    if (write(fd, map_content, strlen(map_content)) != strlen(map_content)) {
        perror("write gid_map");
        close(fd);
        waitpid(child_pid, NULL, 0);
        free(child_stack);
        return -1;
    }
    close(fd);
    
    if (config->verbose) {
        printf("LAUNCHER: Mapeamentos concluídos. Aguardando processo filho...\n");
    }

    int status;
    waitpid(child_pid, &status, 0);
    free(child_stack);
    return WEXITSTATUS(status);
}

static int parse_arguments(int argc, char **argv, struct launcher_config *config) {
    int i;
    int target_start = -1;

    memset(config, 0, sizeof(struct launcher_config));

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            target_start = i + 1;
            break;
        }
        
        if (strcmp(argv[i], "--enable-sandbox") == 0) {
            config->enable_sandbox = 1;
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            config->verbose = 1;
        } else if (strcmp(argv[i], "--working-dir") == 0 || strcmp(argv[i], "-w") == 0) {
            if (i + 1 < argc) {
                config->working_directory = argv[++i];
            } else {
                fprintf(stderr, "Erro: --working-dir requer um argumento\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Uso: %s [opções] [--] <executável> [argumentos...]\n", argv[0]);
            printf("  --enable-sandbox     Ativar sandbox de filesystem (experimental)\n");
            printf("  --verbose, -v        Modo verboso\n");
            printf("  --working-dir, -w    Diretório de trabalho\n");
            printf("  --help, -h           Mostrar esta ajuda\n");
            return 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Erro: opção desconhecida: %s\n", argv[i]);
            return -1;
        } 
        else {
            target_start = i;
            break;
        }
    }

    if (target_start == -1) {
        fprintf(stderr, "Erro: Executável alvo não especificado\n");
        return -1;
    }
    
    if (target_start >= argc) {
        fprintf(stderr, "Erro: Executável alvo não especificado após '--'\n");
        return -1;
    }

    config->target_executable = argv[target_start];
    config->target_args = &argv[target_start];
    config->environment = environ;

    return 0;
}


int main(int argc, char **argv) {
    struct launcher_config config;
    int ret;

    printf("BarrierLayer Sandbox Launcher\n");

    ret = parse_arguments(argc, argv, &config);
    if (ret != 0) return ret > 0 ? 0 : 1;

    if (access(config.target_executable, X_OK) != 0) {
        fprintf(stderr, "Erro: Executável '%s' não encontrado ou não executável\n", config.target_executable);
        return 1;
    }

    ret = execute_target(&config);

    if (config.verbose) {
        printf("Programa alvo terminou com código de saída: %d\n", ret);
    }

    return ret;
}
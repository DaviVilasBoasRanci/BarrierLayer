#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <seccomp.h>
#include <linux/seccomp.h>
#include <unistd.h>
#include <sys/mount.h>
#include <stdio.h> // Para perror
#include <stdlib.h> // For system()
#include "../include/barrierlayer.h"
#include "../include/logger.h"
#include "../include/path_utils.h" // NEW: Explicitly include path_utils.h
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>

// Define SYS_pivot_root if not available
#ifndef SYS_pivot_root
#define SYS_pivot_root 153 // For x86-64 architecture
#endif

// Define pivot_root if not available (older kernels)
#ifndef pivot_root
#define pivot_root(new_root, put_old) syscall(SYS_pivot_root, new_root, put_old)
#endif

// Protótipos de funções
static void setup_sandbox_rules(void);
static void setup_isolated_memory(void);
static void setup_filesystem_isolation(void);
static void setup_sandbox_protection(void);
static void setup_resource_limits(void);
static void setup_network_isolation(void);
static void setup_memory_protection(void);
static void setup_signal_handlers(void);
static void setup_checkpoints(void);
static void setup_rollback_mechanism(void);
static void save_current_state(void);
static void recover_from_violation(void);
static void monitor_resource_usage(void);
static void monitor_network_activity(void);
static void monitor_memory_operations(void);

// Estrutura do sandbox
typedef struct {
    scmp_filter_ctx ctx;
    uint32_t flags;
    void* isolated_memory;
    size_t memory_size;
} SANDBOX_CONTEXT;

static SANDBOX_CONTEXT sandbox = {0};

// Inicialização do sandbox
int initialize_sandbox(void) {
    dprintf(STDOUT_FILENO, "DEBUG: Initializing sandbox - Direct dprintf to stdout.\n");
    setup_filesystem_isolation(); // Call the filesystem isolation setup
    setup_sandbox_protection(); // Call the sandbox protection setup
    return 0;
}




// Configuração de regras do sandbox
void setup_sandbox_rules(void) {
    // Bloqueia syscalls perigosas
    seccomp_rule_add(sandbox.ctx, SCMP_ACT_KILL, SCMP_SYS(ptrace), 0);
    seccomp_rule_add(sandbox.ctx, SCMP_ACT_KILL, SCMP_SYS(process_vm_writev), 0);
    seccomp_rule_add(sandbox.ctx, SCMP_ACT_KILL, SCMP_SYS(process_vm_readv), 0);
    
    // Permite syscalls necessárias
    seccomp_rule_add(sandbox.ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(sandbox.ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(sandbox.ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
}

// Configuração de memória isolada
void setup_isolated_memory(void) {
    // Aloca memória isolada
    sandbox.memory_size = 64 * 1024 * 1024; // 64MB
    sandbox.isolated_memory = mmap(
        NULL,
        sandbox.memory_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    
    if (sandbox.isolated_memory == MAP_FAILED) {
        logger_log(get_log_path(), "Falha ao alocar memória isolada");
        return;
    }
    
    // Configura proteções de memória
    mprotect(sandbox.isolated_memory, sandbox.memory_size, PROT_READ | PROT_WRITE);
}

// Isola o sistema de arquivos (executado DENTRO dos namespaces criados pelo launcher)
static void setup_filesystem_isolation(void) {
    logger_log(get_log_path(), "SANDBOX_CORE: Setting up filesystem isolation.");

    const char* new_root = "/home/davivbrdev/BarrierLayer/fake_c_drive";
    const char* put_old_dir = "old_root"; // Relative to new_root

    char old_root_path[PATH_MAX];
    snprintf(old_root_path, PATH_MAX, "%s/%s", new_root, put_old_dir);

    // Create the directory for the old root
    if (mkdir(old_root_path, 0755) == -1 && errno != EEXIST) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to create old_root directory.");
        perror("mkdir old_root");
        return;
    }

    dprintf(STDOUT_FILENO, "DEBUG: Attempting to bind-mount new_root: %s\n", new_root);
    // Make the new_root a mount point by bind-mounting it to itself
    if (mount(new_root, new_root, NULL, MS_BIND, NULL) == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to bind-mount new_root.");
        perror("mount new_root bind");
        dprintf(STDOUT_FILENO, "ERROR: Bind-mount failed with errno %d\n", errno);
        return;
    }
    dprintf(STDOUT_FILENO, "DEBUG: Bind-mount successful.\n");

    dprintf(STDOUT_FILENO, "DEBUG: Attempting to make new_root private.\n");
    // Make the new_root a private mount point
    // This is crucial for pivot_root to work correctly and to ensure that changes
    // within the sandbox's filesystem don't affect the host.
    if (mount(NULL, new_root, NULL, MS_PRIVATE, NULL) == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to make new_root a private mount.");
        perror("mount new_root private");
        dprintf(STDOUT_FILENO, "ERROR: Make private failed with errno %d\n", errno);
        return;
    }
    dprintf(STDOUT_FILENO, "DEBUG: New_root made private.\n");

    dprintf(STDOUT_FILENO, "DEBUG: Attempting pivot_root with new_root: %s, old_root_path: %s\n", new_root, old_root_path);
    dprintf(STDOUT_FILENO, "DEBUG: Changing directory to new_root: %s\n", new_root);
    if (chdir(new_root) == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to chdir to new_root before pivot_root.");
        perror("chdir new_root");
        dprintf(STDOUT_FILENO, "ERROR: chdir to new_root failed with errno %d\n", errno);
        return;
    }
    dprintf(STDOUT_FILENO, "DEBUG: Changed directory to new_root.\n");

    // Perform pivot_root
    // Use "." for new_root and "old_root" for put_old_dir as we are already in new_root
    if (pivot_root(".", "old_root") == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to pivot_root.");
        perror("pivot_root");
        dprintf(STDOUT_FILENO, "ERROR: pivot_root failed with errno %d\n", errno);
        return;
    }
    dprintf(STDOUT_FILENO, "DEBUG: pivot_root successful.\n");

    // Unmount the old root
    // After pivot_root, the old root is accessible at old_root_path within the new root.
    // The path is now relative to the new root.
    if (umount2("old_root", MNT_DETACH) == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to unmount old_root.");
        perror("umount2 old_root");
        dprintf(STDOUT_FILENO, "ERROR: umount2 old_root failed with errno %d\n", errno);
        // Continue, as failing to unmount might not be critical for sandbox functionality
    }
    dprintf(STDOUT_FILENO, "DEBUG: Old root unmounted.\n");

    // Change current directory to the new root's root (which is now ".")
    if (chdir("/") == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to change directory to new root's root.");
        perror("chdir /");
        dprintf(STDOUT_FILENO, "ERROR: chdir / failed with errno %d\n", errno);
        return;
    }
    dprintf(STDOUT_FILENO, "DEBUG: Changed directory to new root's root.\n");

    logger_log(get_log_path(), "SANDBOX_CORE: Filesystem isolation setup complete.");

    // Bind mount essential system directories (optional but recommended for functionality)
    // These mounts should be done *after* pivot_root and *within* the new root.
    // /proc
    if (mkdir("/proc", 0755) == -1 && errno != EEXIST) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to create /proc directory in new root.");
        perror("mkdir /proc");
    } else if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to mount /proc.");
        perror("mount /proc");
    }

    // /sys
    if (mkdir("/sys", 0755) == -1 && errno != EEXIST) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to create /sys directory in new root.");
        perror("mkdir /sys");
    } else if (mount("sysfs", "/sys", "sysfs", 0, NULL) == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to mount /sys.");
        perror("mount /sys");
    }

    // /dev
    if (mkdir("/dev", 0755) == -1 && errno != EEXIST) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to create /dev directory in new root.");
        perror("mkdir /dev");
    } else if (mount("tmpfs", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, "mode=0755") == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to mount /dev tmpfs.");
        perror("mount /dev tmpfs");
    } else {
        // Create essential device nodes if /dev is tmpfs
        // This is a simplified example; a full /dev setup is complex.
        // For a real sandbox, consider bind-mounting the host's /dev or using a devtmpfs with mknod.
        // For now, we'll assume the launcher handles more complex /dev setup or the app doesn't need it.
    }
}

// Sistema de proteção do sandbox
void setup_sandbox_protection(void) {
    // Configura limites de recursos
    setup_resource_limits();
    
    // Configura isolamento de rede
    setup_network_isolation();
    
    // Configura proteções de memória
    setup_memory_protection();
}

// Sistema de recuperação
void setup_recovery_system(void) {
    // Configura handlers de sinais
    setup_signal_handlers();
    
    // Configura sistema de checkpoint
    setup_checkpoints();
    
    // Configura sistema de rollback
    setup_rollback_mechanism();
}

// Handlers de recuperação
void handle_sandbox_violation(int signal) {
    logger_log(get_log_path(), "Violação detectada no sandbox");
    
    // Salva estado atual
    save_current_state();
    
    // Tenta recuperar
    recover_from_violation();
}

// Implementação das funções auxiliares
static void setup_resource_limits(void) {
    struct rlimit limit;
    limit.rlim_cur = limit.rlim_max = 100 * 1024 * 1024; // 100MB
    setrlimit(RLIMIT_AS, &limit);
}

static void setup_network_isolation(void) {
    logger_log(get_log_path(), "SANDBOX_CORE: Setting up network isolation.");

    // Bring up the loopback interface
    if (system("ip link set lo up") == -1) {
        logger_log(get_log_path(), "SANDBOX_CORE: Failed to bring up loopback interface.");
        perror("ip link set lo up");
    } else {
        logger_log(get_log_path(), "SANDBOX_CORE: Loopback interface brought up.");
    }

    // TODO: Implement more advanced network isolation (e.g., iptables rules, veth pairs)
}

static void setup_memory_protection(void) {
    // TODO: Implementar proteção de memória
}

static void setup_signal_handlers(void) {
    // TODO: Implementar handlers de sinais
}

static void setup_checkpoints(void) {
    // TODO: Implementar sistema de checkpoints
}

static void setup_rollback_mechanism(void) {
    // TODO: Implementar mecanismo de rollback
}

static void save_current_state(void) {
    // TODO: Implementar salvamento de estado
}

static void recover_from_violation(void) {
    // TODO: Implementar recuperação de violações
}

static void monitor_resource_usage(void) {
    // TODO: Implementar monitoramento de recursos
}

static void monitor_network_activity(void) {
    // TODO: Implementar monitoramento de rede
}

static void monitor_memory_operations(void) {
    // TODO: Implementar monitoramento de memória
}

// Sistema de monitoramento
void monitor_sandbox_activity(void) {
    // Monitora uso de recursos
    monitor_resource_usage();
    
    // Monitora atividade de rede
    monitor_network_activity();
    
    // Monitora operações de memória
    monitor_memory_operations();
}

// Limpeza do sandbox
void cleanup_sandbox(void) {
    // Libera recursos
    if (sandbox.isolated_memory) {
        munmap(sandbox.isolated_memory, sandbox.memory_size);
    }
    
    // Limpa contexto do seccomp
    if (sandbox.ctx) {
        seccomp_release(sandbox.ctx);
    }
    
    // Reseta estado
    memset(&sandbox, 0, sizeof(SANDBOX_CONTEXT));
}

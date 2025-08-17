#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <sched.h>
#include <sys/mount.h>
#include <libgen.h> // Para basename

#define KERNEL_MODULE_PATH "/dev/sysinfo"
#define KERNEL_MODULE_NAME "barrierlayer_kernel_advanced"
#define FAKE_ROOT_PATH "/tmp/barrier_root"
#define MAX_ARGS 256
#define MAX_ENV_VARS 256

extern char** environ;

// ... (structs e defines do IOCTL permanecem os mesmos) ...
struct stealth_status {
    int stealth_mode;
    int anti_debug;
    int hidden_processes;
    int active_hooks;
};

#define STEALTH_IOCTL_MAGIC 'S'
#define STEALTH_HIDE_PID    _IOW(STEALTH_IOCTL_MAGIC, 1, pid_t)
#define STEALTH_UNHIDE_PID  _IOW(STEALTH_IOCTL_MAGIC, 2, pid_t)
#define STEALTH_SET_MODE    _IOW(STEALTH_IOCTL_MAGIC, 3, int)
#define STEALTH_GET_STATUS  _IOR(STEALTH_IOCTL_MAGIC, 4, struct stealth_status)

// Configurações do launcher
struct launcher_config {
    int stealth_mode;
    int hide_process;
    int anti_debug;
    int kernel_mode;
    int verbose;
    int enable_sandbox;
    char *target_executable;
    char *sandboxed_executable_path; // Novo campo para o caminho dentro da sandbox
    char *working_directory;
    char **target_args;
    char **environment;
};

static int setup_filesystem_sandbox(struct launcher_config *config);
static int copy_file(const char* src, const char* dest);

// ... (check_kernel_module, load_kernel_module, communicate_with_kernel permanecem os mesmos) ...
static int check_kernel_module(void) {
    FILE *modules;
    char line[256];
    int found = 0;
    modules = fopen("/proc/modules", "r");
    if (!modules) return 0;
    while (fgets(line, sizeof(line), modules)) {
        if (strstr(line, KERNEL_MODULE_NAME)) {
            found = 1;
            break;
        }
    }
    fclose(modules);
    return found;
}

static int load_kernel_module(int verbose) {
    char command[512];
    int ret;
    if (verbose) printf("Carregando módulo do kernel avançado...\n");
    snprintf(command, sizeof(command), "sudo insmod /lib/modules/$(uname -r)/extra/%s.ko", KERNEL_MODULE_NAME);
    ret = system(command);
    if (ret != 0) {
        if (verbose) printf("Tentando carregar do diretório local...\n");
        snprintf(command, sizeof(command), "sudo insmod ./kernel/%s.ko", KERNEL_MODULE_NAME);
        ret = system(command);
    }
    return ret;
}

static int communicate_with_kernel(int cmd, void *data) {
    int fd = open(KERNEL_MODULE_PATH, O_RDWR);
    if (fd < 0) return -1;
    int ret = ioctl(fd, cmd, data);
    close(fd);
    return ret;
}

// Função para executar o programa alvo
static int execute_target(struct launcher_config *config) {
    pid_t child_pid;
    int status;
    
    if (config->verbose) {
        printf("Executando: %s\n", config->target_executable);
    }
    
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (child_pid == 0) {
        // --- PROCESSO FILHO ---

        if (config->enable_sandbox) {
            if (setup_filesystem_sandbox(config) != 0) {
                fprintf(stderr, "Erro fatal ao configurar a sandbox. Abortando.\n");
                exit(1);
            }
        }
        
        if (config->working_directory) {
            if (chdir(config->working_directory) != 0) {
                perror("chdir");
                exit(1);
            }
        }
        
        char** exec_args = config->target_args;
        char* exec_path = config->target_executable;

        if (config->enable_sandbox) {
            exec_path = config->sandboxed_executable_path;
            exec_args[0] = exec_path;
        }

        execve(exec_path, exec_args, config->environment);
        perror("execve");
        exit(1);
    } else {
        // --- PROCESSO PAI ---
        if (config->hide_process && config->kernel_mode) {
            sleep(1);
            communicate_with_kernel(STEALTH_HIDE_PID, &child_pid);
        }
        
        waitpid(child_pid, &status, 0);
        
        if (config->hide_process && config->kernel_mode) {
            communicate_with_kernel(STEALTH_UNHIDE_PID, &child_pid);
        }
        
        return WEXITSTATUS(status);
    }
}

// ... (parse_arguments e check_privileges permanecem os mesmos, mas precisam ser lidos novamente para garantir)
static int parse_arguments(int argc, char **argv, struct launcher_config *config) {
    int i;
    int target_start = -1;
    
    // Inicializar configuração
    memset(config, 0, sizeof(struct launcher_config));
    config->stealth_mode = 1;
    config->hide_process = 1;
    config->anti_debug = 1;
    config->kernel_mode = 1;
    config->enable_sandbox = 0; // Sandbox desativado por padrão
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-stealth") == 0) {
            config->stealth_mode = 0;
        } else if (strcmp(argv[i], "--no-hide") == 0) {
            config->hide_process = 0;
        } else if (strcmp(argv[i], "--no-anti-debug") == 0) {
            config->anti_debug = 0;
        } else if (strcmp(argv[i], "--no-kernel") == 0) {
            config->kernel_mode = 0;
        } else if (strcmp(argv[i], "--enable-sandbox") == 0) {
            config->enable_sandbox = 1;
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            config->verbose = 1;
        } else if (strcmp(argv[i], "--working-dir") == 0 || strcmp(argv[i], "-w") == 0) {
            if (i + 1 < argc) {
                config->working_directory = argv[++i];
            } else {
                printf("Erro: --working-dir requer um argumento\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Uso: %s [opções] <executável> [argumentos...]\n", argv[0]);
            printf("\nOpções:\n");
            printf("  --no-stealth         Desativar modo furtivo\n");
            printf("  --no-hide            Não ocultar processo\n");
            printf("  --no-anti-debug      Desativar anti-debug\n");
            printf("  --no-kernel          Não usar módulo do kernel\n");
            printf("  --enable-sandbox     Ativar sandbox de filesystem (experimental)\n");
            printf("  --verbose, -v        Modo verboso\n");
            printf("  --working-dir, -w    Diretório de trabalho\n");
            printf("  --help, -h           Mostrar esta ajuda\n");
            return 1;
        } else {
            target_start = i;
            break;
        }
    }
    
    if (target_start == -1) {
        printf("Erro: Executável alvo não especificado\n");
        return -1;
    }
    
    config->target_executable = argv[target_start];
    config->target_args = &argv[target_start];
    config->environment = environ;
    
    return 0;
}

static int check_privileges(void) {
    if (geteuid() != 0) {
        printf("ERRO: Esta funcionalidade (sandbox) requer privilégios de root.\n");
        return 0;
    }
    return 1;
}

// Função principal
int main(int argc, char **argv) {
    struct launcher_config config;
    int ret;
    
    printf("BarrierLayer Stealth Launcher v3.0\n");
    printf("===================================\n\n");
    
    ret = parse_arguments(argc, argv, &config);
    if (ret != 0) return ret > 0 ? 0 : 1;

    if (config.enable_sandbox && !check_privileges()) {
        return 1;
    }
    
    if (access(config.target_executable, X_OK) != 0) {
        printf("Erro: Executável '%s' não encontrado ou não executável\n", config.target_executable);
        return 1;
    }
    
    if (config.kernel_mode) {
        if (!check_kernel_module()) {
            if (load_kernel_module(config.verbose) != 0) {
                printf("Aviso: Não foi possível carregar o módulo do kernel, continuando sem...\n");
                config.kernel_mode = 0;
            }
        } else if (config.verbose) {
            printf("Módulo do kernel já está carregado\n");
        }
    }
    
    ret = execute_target(&config);
    
    if (config.verbose) {
        printf("Programa alvo terminou com código de saída: %d\n", ret);
    }
    
    return ret;
}

// Implementação da função da sandbox
static int setup_filesystem_sandbox(struct launcher_config *config) {
    if (config->verbose) {
        printf("SANDBOX: Configurando sandbox de sistema de arquivos...\n");
    }

    if (unshare(CLONE_NEWNS) != 0) {
        perror("unshare(CLONE_NEWNS)");
        return -1;
    }

    if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) != 0) {
        perror("mount MS_PRIVATE");
        return -1;
    }

    mkdir(FAKE_ROOT_PATH, 0755);
    const char* dirs_to_create[] = {
        "/C", "/C/windows", "/C/windows/system32", "/C/users", "/C/users/default", "/C/program_files", "/proc", "/C/game", NULL
    };
    for (int i = 0; dirs_to_create[i]; i++) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s%s", FAKE_ROOT_PATH, dirs_to_create[i]);
        mkdir(path, 0755);
    }

    // Copiar o executável para dentro da sandbox
    char* exec_basename = basename(config->target_executable);
    char dest_path[PATH_MAX];
    snprintf(dest_path, sizeof(dest_path), "%s/C/game/%s", FAKE_ROOT_PATH, exec_basename);
    if (copy_file(config->target_executable, dest_path) != 0) {
        fprintf(stderr, "SANDBOX: Falha ao copiar o executável para a sandbox\n");
        return -1;
    }
    config->sandboxed_executable_path = strdup(dest_path + strlen(FAKE_ROOT_PATH)); // Caminho relativo à nova raiz

    char proc_path[PATH_MAX];
    snprintf(proc_path, sizeof(proc_path), "%s/proc", FAKE_ROOT_PATH);
    if (mount("proc", proc_path, "proc", 0, NULL) != 0) {
        perror("mount proc");
        return -1;
    }

    if (chroot(FAKE_ROOT_PATH) != 0) {
        perror("chroot");
        return -1;
    }

    if (chdir("/") != 0) {
        perror("chdir to new root");
        return -1;
    }

    if (config->verbose) {
        printf("SANDBOX: Estrutura de diretórios do Windows criada e executável copiado.\n");
    }

    return 0;
}

static int copy_file(const char* src_path, const char* dest_path) {
    FILE *src, *dest;
    char buffer[4096];
    size_t n;

    src = fopen(src_path, "rb");
    if (!src) return -1;

    dest = fopen(dest_path, "wb");
    if (!dest) {
        fclose(src);
        return -1;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, n, dest) != n) {
            fclose(src);
            fclose(dest);
            return -1;
        }
    }

    fclose(src);
    fclose(dest);
    
    struct stat st;
    stat(src_path, &st);
    chmod(dest_path, st.st_mode);

    return 0;
}


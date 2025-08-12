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

#define KERNEL_MODULE_PATH "/dev/sysinfo"
#define KERNEL_MODULE_NAME "barrierlayer_kernel_advanced"
#define MAX_ARGS 256
#define MAX_ENV_VARS 256

// Comandos para comunicação com o módulo do kernel
#define STEALTH_IOCTL_MAGIC 'S'
#define STEALTH_HIDE_PID    _IOW(STEALTH_IOCTL_MAGIC, 1, pid_t)
#define STEALTH_UNHIDE_PID  _IOW(STEALTH_IOCTL_MAGIC, 2, pid_t)
#define STEALTH_SET_MODE    _IOW(STEALTH_IOCTL_MAGIC, 3, int)
#define STEALTH_GET_STATUS  _IOR(STEALTH_IOCTL_MAGIC, 4, struct stealth_status)

struct stealth_status {
    int stealth_mode;
    int anti_debug;
    int hidden_processes;
    int active_hooks;
};

// Configurações do launcher
struct launcher_config {
    int stealth_mode;
    int hide_process;
    int anti_debug;
    int kernel_mode;
    int verbose;
    char *target_executable;
    char *working_directory;
    char **target_args;
    char **environment;
};

// Função para verificar se o módulo do kernel está carregado
static int check_kernel_module(void) {
    FILE *modules;
    char line[256];
    int found = 0;
    
    modules = fopen("/proc/modules", "r");
    if (!modules) {
        return 0;
    }
    
    while (fgets(line, sizeof(line), modules)) {
        if (strstr(line, KERNEL_MODULE_NAME)) {
            found = 1;
            break;
        }
    }
    
    fclose(modules);
    return found;
}

// Função para carregar o módulo do kernel
static int load_kernel_module(int verbose) {
    char command[512];
    int ret;
    
    if (verbose) {
        printf("Carregando módulo do kernel avançado...\n");
    }
    
    snprintf(command, sizeof(command), "insmod /lib/modules/$(uname -r)/extra/%s.ko", KERNEL_MODULE_NAME);
    ret = system(command);
    
    if (ret != 0) {
        if (verbose) {
            printf("Tentando carregar do diretório local...\n");
        }
        snprintf(command, sizeof(command), "insmod ./kernel/%s.ko", KERNEL_MODULE_NAME);
        ret = system(command);
    }
    
    return ret;
}

// Função para comunicar com o módulo do kernel
static int communicate_with_kernel(int cmd, void *data) {
    int fd;
    int ret;
    
    fd = open(KERNEL_MODULE_PATH, O_RDWR);
    if (fd < 0) {
        return -1;
    }
    
    ret = ioctl(fd, cmd, data);
    close(fd);
    
    return ret;
}

// Função para limpar variáveis de ambiente sensíveis
static void clean_environment(char **envp) {
    // Explicitly unset LD_PRELOAD to prevent interference
    unsetenv("LD_PRELOAD");

    int i;
    const char *sensitive_vars[] = {
        "LD_PRELOAD", // Keep in list for other potential handling, though unsetenv is primary for this
        "LD_LIBRARY_PATH",
        "BARRIERLAYER_",
        "HOOK_",
        "INJECT_",
        "CHEAT_",
        "BYPASS_",
        NULL
    };
    
    for (i = 0; envp[i]; i++) {
        int j;
        for (j = 0; sensitive_vars[j]; j++) {
            if (strncmp(envp[i], sensitive_vars[j], strlen(sensitive_vars[j])) == 0) {
                // Substituir por variável inócua
                char *equals = strchr(envp[i], '=');
                if (equals) {
                    snprintf(envp[i], equals - envp[i] + 1, "SYSTEM_INFO");
                    strcpy(equals + 1, "enabled");
                }
                break;
            }
        }
    }
}

// Função para mascarar argumentos da linha de comando
static void mask_arguments(int argc, char **argv) {
    int i;
    const char *sensitive_patterns[] = {
        "barrierlayer",
        "hook",
        "inject",
        "cheat",
        "bypass",
        NULL
    };
    
    for (i = 0; i < argc; i++) {
        int j;
        for (j = 0; sensitive_patterns[j]; j++) {
            char *pos = strstr(argv[i], sensitive_patterns[j]);
            if (pos) {
                // Substituir por string genérica
                memset(pos, 'x', strlen(sensitive_patterns[j]));
            }
        }
    }
}

// Função para configurar modo furtivo
static int setup_stealth_mode(struct launcher_config *config) {
    struct stealth_status status;
    int stealth_flags = 0;
    
    if (config->verbose) {
        printf("Configurando modo furtivo...\n");
    }
    
    // Ativar modo furtivo no kernel
    if (config->stealth_mode) {
        stealth_flags |= 1;
    }
    if (config->anti_debug) {
        stealth_flags |= 2;
    }
    
    if (communicate_with_kernel(STEALTH_SET_MODE, &stealth_flags) != 0) {
        if (config->verbose) {
            printf("Aviso: Não foi possível comunicar com o módulo do kernel\n");
        }
        return -1;
    }
    
    // Verificar status
    if (communicate_with_kernel(STEALTH_GET_STATUS, &status) == 0) {
        if (config->verbose) {
            printf("Status do módulo:\n");
            printf("  Modo furtivo: %s\n", status.stealth_mode ? "ativo" : "inativo");
            printf("  Anti-debug: %s\n", status.anti_debug ? "ativo" : "inativo");
            printf("  Processos ocultos: %d\n", status.hidden_processes);
            printf("  Hooks ativos: %d\n", status.active_hooks);
        }
    }
    
    return 0;
}

// Função para ocultar processo no kernel
static int hide_process_in_kernel(pid_t pid, int verbose) {
    if (communicate_with_kernel(STEALTH_HIDE_PID, &pid) != 0) {
        if (verbose) {
            printf("Aviso: Não foi possível ocultar processo %d no kernel\n", pid);
        }
        return -1;
    }
    
    if (verbose) {
        printf("Processo %d ocultado no kernel\n", pid);
    }
    
    return 0;
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
        // Processo filho
        
        // Mudar diretório de trabalho se especificado
        if (config->working_directory) {
            if (chdir(config->working_directory) != 0) {
                perror("chdir");
                exit(1);
            }
        }
        
        // Limpar ambiente
        clean_environment(config->environment);
        
        // Mascarar argumentos
        int argc = 0;
        while (config->target_args[argc]) argc++;
        mask_arguments(argc, config->target_args);
        
        // Executar programa alvo
        execve(config->target_executable, config->target_args, config->environment);
        perror("execve");
        exit(1);
    } else {
        // Processo pai
        
        // Ocultar processo filho se solicitado
        if (config->hide_process && config->kernel_mode) {
            sleep(1); // Aguardar processo inicializar
            hide_process_in_kernel(child_pid, config->verbose);
        }
        
        // Aguardar processo filho
        waitpid(child_pid, &status, 0);
        
        // Desocultar processo
        if (config->hide_process && config->kernel_mode) {
            communicate_with_kernel(STEALTH_UNHIDE_PID, &child_pid);
        }
        
        return WEXITSTATUS(status);
    }
}

// Função para analisar argumentos da linha de comando
static int parse_arguments(int argc, char **argv, struct launcher_config *config) {
    int i;
    int target_start = -1;
    
    // Inicializar configuração
    memset(config, 0, sizeof(struct launcher_config));
    config->stealth_mode = 1;
    config->hide_process = 1;
    config->anti_debug = 1;
    config->kernel_mode = 1;
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-stealth") == 0) {
            config->stealth_mode = 0;
        } else if (strcmp(argv[i], "--no-hide") == 0) {
            config->hide_process = 0;
        } else if (strcmp(argv[i], "--no-anti-debug") == 0) {
            config->anti_debug = 0;
        } else if (strcmp(argv[i], "--no-kernel") == 0) {
            config->kernel_mode = 0;
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
            printf("  --no-stealth      Desativar modo furtivo\n");
            printf("  --no-hide         Não ocultar processo\n");
            printf("  --no-anti-debug   Desativar anti-debug\n");
            printf("  --no-kernel       Não usar módulo do kernel\n");
            printf("  --verbose, -v     Modo verboso\n");
            printf("  --working-dir, -w Diretório de trabalho\n");
            printf("  --help, -h        Mostrar esta ajuda\n");
            return 1;
        } else {
            // Primeiro argumento não-opção é o executável alvo
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

// Função para verificar privilégios
static int check_privileges(void) {
    if (geteuid() != 0) {
        printf("Aviso: Executando sem privilégios de root.\n");
        printf("Algumas funcionalidades podem não estar disponíveis.\n");
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
    
    // Analisar argumentos
    ret = parse_arguments(argc, argv, &config);
    if (ret != 0) {
        return ret;
    }
    
    // Verificar privilégios
    check_privileges();
    
    // Verificar se o executável alvo existe
    if (access(config.target_executable, X_OK) != 0) {
        printf("Erro: Executável '%s' não encontrado ou não executável\n", config.target_executable);
        return 1;
    }
    
    // Carregar módulo do kernel se necessário
    if (config.kernel_mode) {
        if (!check_kernel_module()) {
            if (config.verbose) {
                printf("Módulo do kernel não encontrado, tentando carregar...\n");
            }
            
            if (load_kernel_module(config.verbose) != 0) {
                printf("Aviso: Não foi possível carregar o módulo do kernel\n");
                printf("Continuando sem funcionalidades kernel-mode...\n");
                config.kernel_mode = 0;
            }
        } else {
            if (config.verbose) {
                printf("Módulo do kernel já está carregado\n");
            }
        }
    }
    
    // Configurar modo furtivo
    if (config.kernel_mode) {
        setup_stealth_mode(&config);
    }
    
    // Executar programa alvo
    ret = execute_target(&config);
    
    if (config.verbose) {
        printf("Programa alvo terminou com código de saída: %d\n", ret);
    }
    
    return ret;
}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "../include/proton_support.h"
#include "../include/ultra_logger.h"
#include "../include/config.h"

// Estrutura para informações do Proton
static struct {
    char proton_path[1024];
    char proton_version[256];
    char steam_app_id[32];
    char steam_user_id[32];
    char prefix_path[1024];
    int initialized;
} proton_info = {0};

// Detectar instalação do Proton
int detect_proton_installation(void) {
    ULTRA_TRACE("PROTON", "Detecting Proton installation");
    
    const char* proton_paths[] = {
        getenv("HOME") ? strcat(strcpy((char[1024]){0}, getenv("HOME")), "/.steam/steam/steamapps/common") : NULL,
        getenv("HOME") ? strcat(strcpy((char[1024]){0}, getenv("HOME")), "/.local/share/Steam/steamapps/common") : NULL,
        "/usr/share/steam/steamapps/common",
        NULL
    };
    
    for (int i = 0; proton_paths[i]; i++) {
        if (!proton_paths[i]) continue;
        
        char search_path[1024];
        snprintf(search_path, sizeof(search_path), "%s/Proton*", proton_paths[i]);
        
        // Usar glob para encontrar diretórios Proton*
        glob_t glob_result;
        if (glob(search_path, GLOB_ONLYDIR, NULL, &glob_result) == 0) {
            for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                char proton_binary[1024];
                snprintf(proton_binary, sizeof(proton_binary), "%s/proton", glob_result.gl_pathv[j]);
                
                if (access(proton_binary, X_OK) == 0) {
                    strncpy(proton_info.proton_path, glob_result.gl_pathv[j], sizeof(proton_info.proton_path) - 1);
                    
                    // Extrair versão do nome do diretório
                    char* version = strrchr(glob_result.gl_pathv[j], '/');
                    if (version) {
                        strncpy(proton_info.proton_version, version + 1, sizeof(proton_info.proton_version) - 1);
                    }
                    
                    ULTRA_INFO("PROTON", "Found Proton installation: %s (%s)", 
                              proton_info.proton_path, proton_info.proton_version);
                    
                    globfree(&glob_result);
                    return 1;
                }
            }
            globfree(&glob_result);
        }
    }
    
    ULTRA_WARN("PROTON", "No Proton installation found");
    return 0;
}

// Inicializar suporte ao Proton
int proton_init(void) {
    if (proton_info.initialized) {
        return 1;
    }
    
    ULTRA_INFO("PROTON", "Initializing Proton support");
    
#if USE_PROTON
    // Usar configuração definida
    if (strlen(PROTON_PATH) > 0) {
        strncpy(proton_info.proton_path, PROTON_PATH, sizeof(proton_info.proton_path) - 1);
        strncpy(proton_info.proton_version, PROTON_VERSION, sizeof(proton_info.proton_version) - 1);
        ULTRA_INFO("PROTON", "Using configured Proton: %s", PROTON_PATH);
    } else {
        if (!detect_proton_installation()) {
            ULTRA_ERROR("PROTON", "Failed to detect Proton installation");
            return 0;
        }
    }
#else
    ULTRA_INFO("PROTON", "Proton support disabled in configuration");
    return 0;
#endif
    
    // Detectar Steam App ID
    const char* steam_app_id = getenv("SteamAppId");
    if (steam_app_id) {
        strncpy(proton_info.steam_app_id, steam_app_id, sizeof(proton_info.steam_app_id) - 1);
        ULTRA_INFO("PROTON", "Steam App ID: %s", steam_app_id);
    }
    
    // Detectar Steam User ID
    const char* steam_user_id = getenv("SteamUser");
    if (steam_user_id) {
        strncpy(proton_info.steam_user_id, steam_user_id, sizeof(proton_info.steam_user_id) - 1);
        ULTRA_INFO("PROTON", "Steam User: %s", steam_user_id);
    }
    
    // Configurar prefixo
    setup_proton_prefix();
    
    proton_info.initialized = 1;
    ULTRA_SUCCESS("PROTON", "Proton support initialized successfully");
    
    return 1;
}

// Configurar prefixo do Proton
int setup_proton_prefix(void) {
    ULTRA_TRACE("PROTON", "Setting up Proton prefix");
    
    // Determinar caminho do prefixo
    const char* steam_compat_data = getenv("STEAM_COMPAT_DATA_PATH");
    if (steam_compat_data) {
        snprintf(proton_info.prefix_path, sizeof(proton_info.prefix_path), 
                "%s/pfx", steam_compat_data);
    } else if (strlen(proton_info.steam_app_id) > 0) {
        const char* home = getenv("HOME");
        if (home) {
            snprintf(proton_info.prefix_path, sizeof(proton_info.prefix_path),
                    "%s/.steam/steam/steamapps/compatdata/%s/pfx", 
                    home, proton_info.steam_app_id);
        }
    } else {
        // Prefixo padrão
        const char* home = getenv("HOME");
        if (home) {
            snprintf(proton_info.prefix_path, sizeof(proton_info.prefix_path),
                    "%s/.steam/steam/steamapps/compatdata/barrierlayer/pfx", home);
        }
    }
    
    ULTRA_INFO("PROTON", "Proton prefix: %s", proton_info.prefix_path);
    
    // Criar diretório se não existir
    struct stat st;
    if (stat(proton_info.prefix_path, &st) != 0) {
        char mkdir_cmd[1024];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", proton_info.prefix_path);
        
        if (system(mkdir_cmd) == 0) {
            ULTRA_INFO("PROTON", "Created Proton prefix directory");
        } else {
            ULTRA_ERROR("PROTON", "Failed to create Proton prefix directory");
            return 0;
        }
    }
    
    return 1;
}

// Configurar variáveis de ambiente do Proton
void setup_proton_environment(void) {
    ULTRA_TRACE("PROTON", "Setting up Proton environment");
    
    if (!proton_info.initialized) {
        ULTRA_WARN("PROTON", "Proton not initialized, skipping environment setup");
        return;
    }
    
    // Configurar WINEPREFIX
    setenv("WINEPREFIX", proton_info.prefix_path, 1);
    ULTRA_DEBUG("PROTON", "Set WINEPREFIX=%s", proton_info.prefix_path);
    
    // Configurar WINEDLLPATH
    char winedllpath[1024];
    snprintf(winedllpath, sizeof(winedllpath), "%s/dist/lib64/wine:%s/dist/lib/wine", 
             proton_info.proton_path, proton_info.proton_path);
    setenv("WINEDLLPATH", winedllpath, 1);
    ULTRA_DEBUG("PROTON", "Set WINEDLLPATH=%s", winedllpath);
    
    // Configurar PATH
    char new_path[2048];
    const char* current_path = getenv("PATH");
    snprintf(new_path, sizeof(new_path), "%s/dist/bin:%s", 
             proton_info.proton_path, current_path ? current_path : "");
    setenv("PATH", new_path, 1);
    ULTRA_DEBUG("PROTON", "Updated PATH with Proton binaries");
    
#if USE_DXVK
    // Configurar DXVK
    setenv("DXVK_HUD", "0", 0);  // Não sobrescrever se já definido
    setenv("DXVK_LOG_LEVEL", "none", 0);
    ULTRA_DEBUG("PROTON", "Configured DXVK environment");
#endif

#if USE_VKD3D
    // Configurar VKD3D
    setenv("VKD3D_DEBUG", "none", 0);
    ULTRA_DEBUG("PROTON", "Configured VKD3D environment");
#endif

#if USE_ESYNC
    // Configurar Esync
    setenv("WINEESYNC", "1", 0);
    ULTRA_DEBUG("PROTON", "Enabled Esync");
#endif

#if USE_FSYNC
    // Configurar Fsync
    setenv("WINEFSYNC", "1", 0);
    ULTRA_DEBUG("PROTON", "Enabled Fsync");
#endif
    
    // Configurações anti-cheat específicas do Proton
    setup_proton_anticheat_environment();
    
    ULTRA_SUCCESS("PROTON", "Proton environment configured");
}

// Configurar ambiente anti-cheat específico do Proton
void setup_proton_anticheat_environment(void) {
    ULTRA_TRACE("PROTON", "Setting up Proton anti-cheat environment");
    
    // Mascarar BarrierLayer no ambiente Proton
    unsetenv("BARRIERLAYER_ACTIVE");
    unsetenv("BARRIERLAYER_VERSION");
    unsetenv("LD_PRELOAD");
    
    // Configurar variáveis que podem ser verificadas por anti-cheats
    setenv("STEAM_RUNTIME", "1", 1);
    setenv("STEAM_RUNTIME_LIBRARY_PATH", "/usr/lib/steam-runtime", 1);
    
    // Mascarar informações do sistema
    setenv("WINE_LARGE_ADDRESS_AWARE", "1", 1);
    setenv("WINEARCH", "win64", 1);
    setenv("WINEDEBUG", "-all", 1);
    
    ULTRA_DEBUG("PROTON", "Anti-cheat environment configured");
}

// Executar aplicação via Proton
int proton_run_application(const char* executable, char* const argv[]) {
    if (!proton_info.initialized) {
        ULTRA_ERROR("PROTON", "Proton not initialized");
        return -1;
    }
    
    ULTRA_INFO("PROTON", "Running application via Proton: %s", executable);
    
    // Configurar ambiente
    setup_proton_environment();
    
    // Construir comando do Proton
    char proton_cmd[1024];
    snprintf(proton_cmd, sizeof(proton_cmd), "%s/proton", proton_info.proton_path);
    
    // Preparar argumentos
    char** new_argv = malloc(sizeof(char*) * (64 + 1));  // Máximo 64 argumentos
    int argc = 0;
    
    new_argv[argc++] = proton_cmd;
    new_argv[argc++] = "run";
    new_argv[argc++] = (char*)executable;
    
    // Copiar argumentos originais
    if (argv) {
        for (int i = 1; argv[i] && argc < 63; i++) {
            new_argv[argc++] = argv[i];
        }
    }
    new_argv[argc] = NULL;
    
    ULTRA_DEBUG("PROTON", "Executing: %s run %s", proton_cmd, executable);
    
    // Executar
    pid_t pid = fork();
    if (pid == 0) {
        // Processo filho
        execv(proton_cmd, new_argv);
        ULTRA_ERROR("PROTON", "Failed to execute Proton: %s", strerror(errno));
        exit(1);
    } else if (pid > 0) {
        // Processo pai
        int status;
        waitpid(pid, &status, 0);
        
        free(new_argv);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            ULTRA_INFO("PROTON", "Application exited with code %d", exit_code);
            return exit_code;
        } else {
            ULTRA_ERROR("PROTON", "Application terminated abnormally");
            return -1;
        }
    } else {
        ULTRA_ERROR("PROTON", "Failed to fork process: %s", strerror(errno));
        free(new_argv);
        return -1;
    }
}

// Obter informações do Proton
const char* proton_get_version(void) {
    return proton_info.initialized ? proton_info.proton_version : NULL;
}

const char* proton_get_path(void) {
    return proton_info.initialized ? proton_info.proton_path : NULL;
}

const char* proton_get_prefix(void) {
    return proton_info.initialized ? proton_info.prefix_path : NULL;
}

// Verificar se o Proton está ativo
int proton_is_active(void) {
    return proton_info.initialized && (getenv("STEAM_COMPAT_DATA_PATH") != NULL || 
                                      getenv("SteamAppId") != NULL);
}

// Limpar recursos do Proton
void proton_cleanup(void) {
    if (proton_info.initialized) {
        ULTRA_INFO("PROTON", "Cleaning up Proton support");
        memset(&proton_info, 0, sizeof(proton_info));
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "include/ultra_logger.h"
#include "include/proton_support.h"
#include "include/config.h"

#define VERSION "2.0"

// Variáveis globais
static int verbose = 0;
static int daemon_mode = 0;
static char* target_executable = NULL;
static char* hook_library = NULL;

// Função para mostrar ajuda
void show_help(const char* program_name) {
    printf("BarrierLayer v%s - Advanced Anti-Cheat Bypass for Linux Gaming\n\n", VERSION);
    
    printf("Usage: %s [OPTIONS] [EXECUTABLE] [ARGS...]\n\n", program_name);
    
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -v, --verbose           Enable verbose output\n");
    printf("  -d, --daemon            Run in daemon mode\n");
    printf("  -l, --library PATH      Specify hook library path\n");
    printf("  -c, --config            Show configuration\n");
    printf("  -t, --test              Run self-test\n");
    printf("  -i, --info              Show system information\n");
    printf("  --version               Show version information\n");
    printf("\n");
    
    printf("Environment Variables:\n");
    printf("  BARRIERLAYER_LOG_LEVEL     Log level (0-4, default: 2)\n");
    printf("  BARRIERLAYER_DETAILED      Detailed logging (0/1, default: 1)\n");
    printf("  BARRIERLAYER_STACK_TRACE   Stack traces (0/1, default: 1)\n");
    printf("  BARRIERLAYER_STEALTH_MODE  Stealth mode (0/1, default: 1)\n");
    printf("\n");
    
    printf("Examples:\n");
    printf("  %s game.exe                    # Launch with auto-detection\n", program_name);
    printf("  %s -v game.exe                 # Launch with verbose output\n", program_name);
    printf("  %s -d                          # Run as daemon\n", program_name);
    printf("  %s -t                          # Run self-test\n", program_name);
    printf("\n");
    
    printf("For interactive launcher, use: ./barrierlayer-launcher.sh\n");
}

// Função para mostrar versão
void show_version(void) {
    printf("BarrierLayer v%s\n", VERSION);
    printf("Build: %s mode\n", BUILD_MODE);
    printf("Features:\n");
#if USE_WINE
    printf("  - Wine support: Enabled\n");
#else
    printf("  - Wine support: Disabled\n");
#endif
#if USE_PROTON
    printf("  - Proton support: Enabled\n");
#else
    printf("  - Proton support: Disabled\n");
#endif
#if ENABLE_ULTRA_LOGGING
    printf("  - Ultra logging: Enabled\n");
#else
    printf("  - Ultra logging: Disabled\n");
#endif
#if BUILD_KERNEL
    printf("  - Kernel module: Enabled\n");
#else
    printf("  - Kernel module: Disabled\n");
#endif
#if BUILD_GUI
    printf("  - GUI: Enabled\n");
#else
    printf("  - GUI: Disabled\n");
#endif
    printf("\n");
    printf("System: %s %s (%s)\n", BARRIERLAYER_DISTRO, BARRIERLAYER_DISTRO_VERSION, BARRIERLAYER_ARCH);
    printf("Kernel: %s\n", BARRIERLAYER_KERNEL_VERSION);
}

// Função para mostrar configuração
void show_config(void) {
    printf("BarrierLayer Configuration:\n\n");
    
    printf("System Information:\n");
    printf("  Distribution: %s %s\n", BARRIERLAYER_DISTRO, BARRIERLAYER_DISTRO_VERSION);
    printf("  Architecture: %s\n", BARRIERLAYER_ARCH);
    printf("  Kernel: %s\n", BARRIERLAYER_KERNEL_VERSION);
    printf("\n");
    
    printf("Build Configuration:\n");
    printf("  Build Mode: %s\n", BUILD_MODE);
    printf("  Kernel Module: %s\n", BUILD_KERNEL ? "Enabled" : "Disabled");
    printf("  GUI: %s\n", BUILD_GUI ? "Enabled" : "Disabled");
    printf("\n");
    
    printf("Wine/Proton Support:\n");
    printf("  Wine: %s\n", USE_WINE ? "Enabled" : "Disabled");
    printf("  Proton: %s\n", USE_PROTON ? "Enabled" : "Disabled");
    printf("\n");
    
    printf("Anti-Cheat Protection:\n");
    printf("  EAC: %s\n", ENABLE_EAC ? "Enabled" : "Disabled");
    printf("  BattlEye: %s\n", ENABLE_BATTLEYE ? "Enabled" : "Disabled");
    printf("  Vanguard: %s\n", ENABLE_VANGUARD ? "Enabled" : "Disabled");
    printf("\n");
    
    printf("Logging Configuration:\n");
    printf("  Log Level: %d\n", LOG_LEVEL);
    printf("  Ultra Logging: %s\n", ENABLE_ULTRA_LOGGING ? "Enabled" : "Disabled");
    printf("  Stack Traces: %s\n", ENABLE_STACK_TRACES ? "Enabled" : "Disabled");
    printf("  File Logging: %s\n", ENABLE_FILE_LOGGING ? "Enabled" : "Disabled");
    printf("  Log Directory: %s\n", LOG_DIRECTORY);
    printf("\n");
    
    printf("Stealth Configuration:\n");
    printf("  Remove LD_PRELOAD: %s\n", REMOVE_LD_PRELOAD ? "Enabled" : "Disabled");
    printf("  Library Unlinking: %s\n", ENABLE_UNLINKING ? "Enabled" : "Disabled");
    printf("  Block Sensitive Files: %s\n", BLOCK_SENSITIVE_FILES ? "Enabled" : "Disabled");
    printf("\n");
}

// Função para mostrar informações do sistema
void show_info(void) {
    printf("BarrierLayer System Information:\n\n");
    
    // Informações do sistema
    system("echo 'System:' && lsb_release -a 2>/dev/null || uname -a");
    printf("\n");
    
    // Informações de hardware
    printf("Hardware:\n");
    system("echo '  CPU:' && grep 'model name' /proc/cpuinfo | head -1 | cut -d':' -f2 | sed 's/^ *//'");
    system("echo '  Memory:' && free -h | grep '^Mem:' | awk '{print $2 \" total, \" $3 \" used, \" $7 \" available\"}'");
    printf("\n");
    
    // Verificar dependências
    printf("Dependencies:\n");
    system("echo -n '  gcc: ' && (gcc --version | head -1 || echo 'Not found')");
    system("echo -n '  nasm: ' && (nasm --version | head -1 || echo 'Not found')");
    system("echo -n '  Wine: ' && (wine --version 2>/dev/null || echo 'Not found')");
    system("echo -n '  Steam: ' && (steam --version 2>/dev/null | head -1 || echo 'Not found')");
    printf("\n");
    
    // Status dos arquivos
    printf("BarrierLayer Files:\n");
    printf("  Hook Library: %s\n", access("bin/barrierlayer_hook.so", F_OK) == 0 ? "✅ Present" : "❌ Missing");
    printf("  GUI: %s\n", access("bin/barrierlayer_gui", F_OK) == 0 ? "✅ Present" : "❌ Missing");
    printf("  Kernel Module: %s\n", access("kernel/barrierlayer_minimal.ko", F_OK) == 0 ? "✅ Present" : "❌ Missing");
    printf("  Configuration: %s\n", access("config.mk", F_OK) == 0 ? "✅ Present" : "❌ Missing");
    printf("\n");
}

// Função para executar self-test
int run_self_test(void) {
    printf("BarrierLayer Self-Test:\n\n");
    
    int tests_passed = 0;
    int total_tests = 0;
    
    // Teste 1: Verificar biblioteca de hooks
    total_tests++;
    printf("Test 1: Hook library... ");
    if (access("bin/barrierlayer_hook.so", F_OK) == 0) {
        printf("✅ PASS\n");
        tests_passed++;
    } else {
        printf("❌ FAIL (library not found)\n");
    }
    
    // Teste 2: Verificar ultra logger
    total_tests++;
    printf("Test 2: Ultra logger... ");
#if ENABLE_ULTRA_LOGGING
    if (ultra_logger_init() == 0) {
        printf("✅ PASS\n");
        tests_passed++;
        ultra_logger_cleanup();
    } else {
        printf("❌ FAIL (initialization failed)\n");
    }
#else
    printf("⚠️  SKIP (disabled)\n");
#endif
    
    // Teste 3: Verificar Proton support
    total_tests++;
    printf("Test 3: Proton support... ");
#if USE_PROTON
    if (proton_init()) {
        printf("✅ PASS\n");
        tests_passed++;
        proton_cleanup();
    } else {
        printf("⚠️  WARN (no Proton found)\n");
    }
#else
    printf("⚠️  SKIP (disabled)\n");
#endif
    
    // Teste 4: Verificar permissões
    total_tests++;
    printf("Test 4: Permissions... ");
    if (access("bin/barrierlayer_hook.so", R_OK) == 0) {
        printf("✅ PASS\n");
        tests_passed++;
    } else {
        printf("❌ FAIL (no read permission)\n");
    }
    
    // Teste 5: Verificar ambiente
    total_tests++;
    printf("Test 5: Environment... ");
    if (getenv("HOME") && getenv("PATH")) {
        printf("✅ PASS\n");
        tests_passed++;
    } else {
        printf("❌ FAIL (missing environment variables)\n");
    }
    
    printf("\nResults: %d/%d tests passed\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All tests passed! BarrierLayer is ready to use.\n");
        return 0;
    } else {
        printf("⚠️  Some tests failed. Please check the configuration.\n");
        return 1;
    }
}

// Função para executar aplicação com hooks
int launch_with_hooks(const char* executable, char* const argv[]) {
    // Determinar caminho da biblioteca
    if (!hook_library) {
        hook_library = "bin/barrierlayer_hook.so";
    }
    
    // Verificar se biblioteca existe
    if (access(hook_library, F_OK) != 0) {
        fprintf(stderr, "Error: Hook library not found: %s\n", hook_library);
        fprintf(stderr, "Please run 'make' to build BarrierLayer first.\n");
        return 1;
    }
    
    // Configurar LD_PRELOAD
    const char* current_preload = getenv("LD_PRELOAD");
    char* new_preload;
    
    if (current_preload) {
        asprintf(&new_preload, "%s:%s", hook_library, current_preload);
    } else {
        new_preload = strdup(hook_library);
    }
    
    setenv("LD_PRELOAD", new_preload, 1);
    
    // Configurar variáveis do BarrierLayer
    if (!getenv("BARRIERLAYER_LOG_LEVEL")) {
        setenv("BARRIERLAYER_LOG_LEVEL", "2", 1);
    }
    if (!getenv("BARRIERLAYER_DETAILED")) {
        setenv("BARRIERLAYER_DETAILED", "1", 1);
    }
    if (!getenv("BARRIERLAYER_STEALTH_MODE")) {
        setenv("BARRIERLAYER_STEALTH_MODE", "1", 1);
    }
    
    if (verbose) {
        printf("Launching: %s\n", executable);
        printf("Hook library: %s\n", hook_library);
        printf("LD_PRELOAD: %s\n", new_preload);
    }
    
    // Executar aplicação
    execv(executable, argv);
    
    // Se chegou aqui, execv falhou
    fprintf(stderr, "Error: Failed to execute %s: %s\n", executable, strerror(errno));
    free(new_preload);
    return 1;
}

// Função para modo daemon
int daemon_mode_main(void) {
    printf("BarrierLayer daemon mode - monitoring system...\n");
    
    // Inicializar ultra logger se disponível
#if ENABLE_ULTRA_LOGGING
    if (ultra_logger_init() != 0) {
        fprintf(stderr, "Warning: Failed to initialize ultra logger\n");
    }
#endif
    
    // Loop principal do daemon
    while (1) {
        // Monitorar sistema, logs, etc.
        sleep(1);
        
        // Verificar sinais de parada
        // (implementar handler de sinal se necessário)
    }
    
#if ENABLE_ULTRA_LOGGING
    ultra_logger_cleanup();
#endif
    
    return 0;
}

// Função principal
int main(int argc, char* argv[]) {
    int opt;
    int show_help_flag = 0;
    int show_version_flag = 0;
    int show_config_flag = 0;
    int show_info_flag = 0;
    int run_test_flag = 0;
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"verbose", no_argument, 0, 'v'},
        {"daemon", no_argument, 0, 'd'},
        {"library", required_argument, 0, 'l'},
        {"config", no_argument, 0, 'c'},
        {"test", no_argument, 0, 't'},
        {"info", no_argument, 0, 'i'},
        {"version", no_argument, 0, 0},
        {0, 0, 0, 0}
    };
    
    // Parse argumentos
    while ((opt = getopt_long(argc, argv, "hvdl:cti", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                show_help_flag = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'd':
                daemon_mode = 1;
                break;
            case 'l':
                hook_library = strdup(optarg);
                break;
            case 'c':
                show_config_flag = 1;
                break;
            case 't':
                run_test_flag = 1;
                break;
            case 'i':
                show_info_flag = 1;
                break;
            case 0:
                if (strcmp(long_options[optind-1].name, "version") == 0) {
                    show_version_flag = 1;
                }
                break;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    // Processar flags
    if (show_help_flag) {
        show_help(argv[0]);
        return 0;
    }
    
    if (show_version_flag) {
        show_version();
        return 0;
    }
    
    if (show_config_flag) {
        show_config();
        return 0;
    }
    
    if (show_info_flag) {
        show_info();
        return 0;
    }
    
    if (run_test_flag) {
        return run_self_test();
    }
    
    if (daemon_mode) {
        return daemon_mode_main();
    }
    
    // Se não há executável especificado, mostrar ajuda
    if (optind >= argc) {
        show_help(argv[0]);
        return 1;
    }
    
    // Executar aplicação com hooks
    target_executable = argv[optind];
    return launch_with_hooks(target_executable, &argv[optind]);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <signal.h>
#include <sched.h>

#include "../include/ultra_logger.h"

// Configurações do logger
#define MAX_LOG_BUFFER_SIZE 8192
#define MAX_STACK_FRAMES 32
#define MAX_LOG_FILES 10
#define LOG_FILE_MAX_SIZE (100 * 1024 * 1024) // 100MB
#define LOG_ROTATION_CHECK_INTERVAL 1000

// Estrutura para controle do logger
typedef struct {
    FILE *log_files[MAX_LOG_FILES];
    char log_paths[MAX_LOG_FILES][PATH_MAX];
    int current_file;
    size_t current_size;
    pthread_mutex_t log_mutex;
    pthread_t rotation_thread;
    int rotation_counter;
    bool initialized;
    bool detailed_mode;
    bool stack_trace_mode;
    bool performance_mode;
    int log_level;
} ultra_logger_t;

static ultra_logger_t g_logger = {0};

// Categorias de log com cores
typedef struct {
    const char *name;
    const char *color;
    const char *prefix;
} log_category_t;

static const log_category_t log_categories[] = {
    {"KERNEL", "\033[1;31m", "🔴 KERN"},     // Vermelho
    {"SYSCALL", "\033[1;33m", "🟡 SYSC"},   // Amarelo
    {"HOOK", "\033[1;32m", "🟢 HOOK"},      // Verde
    {"ANTICHEAT", "\033[1;35m", "🟣 AC"},   // Roxo
    {"PROCESS", "\033[1;36m", "🔵 PROC"},   // Ciano
    {"FILE", "\033[1;34m", "📁 FILE"},      // Azul
    {"NETWORK", "\033[1;37m", "🌐 NET"},    // Branco
    {"MEMORY", "\033[1;31m", "💾 MEM"},     // Vermelho
    {"REGISTRY", "\033[1;33m", "📋 REG"},   // Amarelo
    {"CRYPTO", "\033[1;32m", "🔐 CRYP"},    // Verde
    {"DEBUG", "\033[1;35m", "🐛 DBG"},      // Roxo
    {"ERROR", "\033[1;91m", "❌ ERR"},      // Vermelho brilhante
    {"WARNING", "\033[1;93m", "⚠️  WARN"},  // Amarelo brilhante
    {"INFO", "\033[1;92m", "ℹ️  INFO"},     // Verde brilhante
    {"TRACE", "\033[1;90m", "🔍 TRAC"},     // Cinza
    {NULL, NULL, NULL}
};

// Função para obter timestamp com precisão de microssegundos
static void get_precise_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    struct tm *tm_info;
    char time_str[64];
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buffer, size, "%s.%06ld", time_str, tv.tv_usec);
}

// Função para obter informações do processo atual
static void get_process_info(char *buffer, size_t size) {
    pid_t pid = getpid();
    pid_t tid = syscall(SYS_gettid);
    pid_t ppid = getppid();
    
    char comm[256] = {0};
    char cmdline[1024] = {0};
    char proc_path[64];
    FILE *f;
    
    // Obter nome do processo
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/comm", pid);
    f = fopen(proc_path, "r");
    if (f) {
        fgets(comm, sizeof(comm), f);
        fclose(f);
        // Remover newline
        char *nl = strchr(comm, '\n');
        if (nl) *nl = '\0';
    }
    
    // Obter linha de comando
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/cmdline", pid);
    f = fopen(proc_path, "r");
    if (f) {
        size_t len = fread(cmdline, 1, sizeof(cmdline) - 1, f);
        fclose(f);
        // Substituir nulls por espaços
        for (size_t i = 0; i < len; i++) {
            if (cmdline[i] == '\0') cmdline[i] = ' ';
        }
    }
    
    snprintf(buffer, size, "PID:%d TID:%d PPID:%d COMM:'%s' CMD:'%s'", 
             pid, tid, ppid, comm, cmdline);
}

// Função para obter stack trace
static void get_stack_trace(char *buffer, size_t size) {
    void *frames[MAX_STACK_FRAMES];
    char **symbols;
    int frame_count;
    size_t offset = 0;
    
    frame_count = backtrace(frames, MAX_STACK_FRAMES);
    symbols = backtrace_symbols(frames, frame_count);
    
    if (symbols) {
        offset += snprintf(buffer + offset, size - offset, "STACK_TRACE:\n");
        
        for (int i = 1; i < frame_count && offset < size - 100; i++) {
            // Tentar obter informações mais detalhadas com dladdr
            Dl_info info;
            if (dladdr(frames[i], &info)) {
                offset += snprintf(buffer + offset, size - offset,
                    "  [%02d] %p <%s+0x%lx> (%s)\n", i,
                    frames[i],
                    info.dli_sname ? info.dli_sname : "unknown",
                    (unsigned long)frames[i] - (unsigned long)info.dli_saddr,
                    info.dli_fname ? info.dli_fname : "unknown");
            } else {
                offset += snprintf(buffer + offset, size - offset,
                    "  [%02d] %s\n", i, symbols[i]);
            }
        }
        
        free(symbols);
    } else {
        snprintf(buffer, size, "STACK_TRACE: Unable to obtain stack trace");
    }
}

// Função para obter informações do sistema
static void get_system_info(char *buffer, size_t size) {
    struct utsname uts;
    struct rusage usage;
    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    long page_size = sysconf(_SC_PAGESIZE);
    
    uname(&uts);
    getrusage(RUSAGE_SELF, &usage);
    
    snprintf(buffer, size,
        "SYSTEM: %s %s %s | CPU_COUNT:%ld PAGE_SIZE:%ld | "
        "MEM_USAGE:%ldKB CPU_TIME:%ld.%06lds",
        uts.sysname, uts.release, uts.machine,
        cpu_count, page_size,
        usage.ru_maxrss,
        usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
}

// Função para rotacionar logs
static void rotate_logs(void) {
    pthread_mutex_lock(&g_logger.log_mutex);
    
    if (g_logger.log_files[g_logger.current_file]) {
        fclose(g_logger.log_files[g_logger.current_file]);
        g_logger.log_files[g_logger.current_file] = NULL;
    }
    
    g_logger.current_file = (g_logger.current_file + 1) % MAX_LOG_FILES;
    g_logger.current_size = 0;
    g_logger.rotation_counter++;
    
    // Criar novo arquivo de log
    snprintf(g_logger.log_paths[g_logger.current_file], PATH_MAX,
        "/tmp/barrierlayer_ultra_%d_%d.log", 
        getpid(), g_logger.rotation_counter);
    
    g_logger.log_files[g_logger.current_file] = 
        fopen(g_logger.log_paths[g_logger.current_file], "w");
    
    if (g_logger.log_files[g_logger.current_file]) {
        fprintf(g_logger.log_files[g_logger.current_file],
            "=== BarrierLayer Ultra Logger - New Session ===\n");
        fprintf(g_logger.log_files[g_logger.current_file],
            "Rotation: %d | PID: %d | Time: %ld\n\n",
            g_logger.rotation_counter, getpid(), time(NULL));
        fflush(g_logger.log_files[g_logger.current_file]);
    }
    
    pthread_mutex_unlock(&g_logger.log_mutex);
}

// Thread para rotação automática de logs
static void* rotation_thread_func(void *arg) {
    (void)arg;
    
    while (g_logger.initialized) {
        sleep(10); // Verificar a cada 10 segundos
        
        if (g_logger.current_size > LOG_FILE_MAX_SIZE) {
            rotate_logs();
        }
    }
    
    return NULL;
}

// Inicializar o ultra logger
int ultra_logger_init(void) {
    if (g_logger.initialized) {
        return 0;
    }
    
    memset(&g_logger, 0, sizeof(g_logger));
    
    if (pthread_mutex_init(&g_logger.log_mutex, NULL) != 0) {
        return -1;
    }
    
    // Configurações padrão
    g_logger.detailed_mode = true;
    g_logger.stack_trace_mode = true;
    g_logger.performance_mode = false;
    g_logger.log_level = ULTRA_LOG_TRACE;
    
    // Verificar variáveis de ambiente
    char *env_val;
    if ((env_val = getenv("BARRIERLAYER_LOG_LEVEL"))) {
        g_logger.log_level = atoi(env_val);
    }
    if ((env_val = getenv("BARRIERLAYER_DETAILED"))) {
        g_logger.detailed_mode = (strcmp(env_val, "1") == 0);
    }
    if ((env_val = getenv("BARRIERLAYER_STACK_TRACE"))) {
        g_logger.stack_trace_mode = (strcmp(env_val, "1") == 0);
    }
    if ((env_val = getenv("BARRIERLAYER_PERFORMANCE"))) {
        g_logger.performance_mode = (strcmp(env_val, "1") == 0);
    }
    
    // Criar primeiro arquivo de log
    rotate_logs();
    
    // Iniciar thread de rotação
    if (pthread_create(&g_logger.rotation_thread, NULL, rotation_thread_func, NULL) != 0) {
        pthread_mutex_destroy(&g_logger.log_mutex);
        return -1;
    }
    
    g_logger.initialized = true;
    
    // Log inicial
    ultra_log(ULTRA_LOG_INFO, "INIT", "Ultra Logger initialized successfully");
    
    return 0;
}

// Finalizar o ultra logger
void ultra_logger_cleanup(void) {
    if (!g_logger.initialized) {
        return;
    }
    
    ultra_log(ULTRA_LOG_INFO, "CLEANUP", "Ultra Logger shutting down");
    
    g_logger.initialized = false;
    
    // Aguardar thread de rotação
    pthread_join(g_logger.rotation_thread, NULL);
    
    // Fechar todos os arquivos
    pthread_mutex_lock(&g_logger.log_mutex);
    for (int i = 0; i < MAX_LOG_FILES; i++) {
        if (g_logger.log_files[i]) {
            fclose(g_logger.log_files[i]);
            g_logger.log_files[i] = NULL;
        }
    }
    pthread_mutex_unlock(&g_logger.log_mutex);
    
    pthread_mutex_destroy(&g_logger.log_mutex);
}

// Função principal de logging
void ultra_log(int level, const char *category, const char *format, ...) {
    if (!g_logger.initialized || level < g_logger.log_level) {
        return;
    }
    
    char timestamp[64];
    char process_info[512];
    char system_info[256];
    char stack_trace[4096];
    char message[1024];
    char final_buffer[MAX_LOG_BUFFER_SIZE];
    
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Obter informações detalhadas
    get_precise_timestamp(timestamp, sizeof(timestamp));
    
    if (g_logger.detailed_mode) {
        get_process_info(process_info, sizeof(process_info));
        get_system_info(system_info, sizeof(system_info));
    } else {
        snprintf(process_info, sizeof(process_info), "PID:%d", getpid());
        system_info[0] = '\0';
    }
    
    if (g_logger.stack_trace_mode && level <= ULTRA_LOG_ERROR) {
        get_stack_trace(stack_trace, sizeof(stack_trace));
    } else {
        stack_trace[0] = '\0';
    }
    
    // Encontrar categoria
    const log_category_t *cat = NULL;
    for (int i = 0; log_categories[i].name; i++) {
        if (strcmp(log_categories[i].name, category) == 0) {
            cat = &log_categories[i];
            break;
        }
    }
    
    if (!cat) {
        cat = &log_categories[13]; // INFO como padrão
    }
    
    // Montar mensagem final
    int written = snprintf(final_buffer, sizeof(final_buffer),
        "%s[%s] %s %s | %s\n%s%s%s%s\n",
        cat->color, timestamp, cat->prefix, process_info, message,
        g_logger.detailed_mode ? system_info : "",
        g_logger.detailed_mode ? "\n" : "",
        stack_trace[0] ? stack_trace : "",
        "\033[0m"); // Reset cor
    
    // Escrever no arquivo
    pthread_mutex_lock(&g_logger.log_mutex);
    
    if (g_logger.log_files[g_logger.current_file]) {
        fwrite(final_buffer, 1, written, g_logger.log_files[g_logger.current_file]);
        fflush(g_logger.log_files[g_logger.current_file]);
        g_logger.current_size += written;
        
        // Também escrever no stderr se for erro crítico
        if (level <= ULTRA_LOG_ERROR) {
            fwrite(final_buffer, 1, written, stderr);
        }
    }
    
    pthread_mutex_unlock(&g_logger.log_mutex);
}

// Função para logging de syscalls
void ultra_log_syscall(const char *syscall_name, long syscall_nr, 
                      unsigned long *args, long ret_value, const char *details) {
    if (!g_logger.initialized) {
        return;
    }
    
    char args_str[256] = {0};
    if (args) {
        snprintf(args_str, sizeof(args_str),
            "ARGS:[0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx]",
            args[0], args[1], args[2], args[3], args[4], args[5]);
    }
    
    ultra_log(ULTRA_LOG_TRACE, "SYSCALL",
        "%s(nr=%ld) %s RET=%ld %s",
        syscall_name, syscall_nr, args_str, ret_value,
        details ? details : "");
}

// Função para logging de hooks
void ultra_log_hook(const char *function_name, const char *library, 
                   void *original_addr, void *hook_addr, const char *details) {
    ultra_log(ULTRA_LOG_INFO, "HOOK",
        "HOOKED %s@%s: orig=%p hook=%p %s",
        function_name, library, original_addr, hook_addr,
        details ? details : "");
}

// Função para logging de anti-cheat
void ultra_log_anticheat(const char *anticheat_name, const char *action, 
                        const char *details) {
    ultra_log(ULTRA_LOG_WARNING, "ANTICHEAT",
        "%s: %s - %s",
        anticheat_name, action, details);
}

// Função para obter estatísticas do logger
void ultra_logger_stats(ultra_logger_stats_t *stats) {
    if (!stats || !g_logger.initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_logger.log_mutex);
    
    stats->total_files = MAX_LOG_FILES;
    stats->current_file = g_logger.current_file;
    stats->current_size = g_logger.current_size;
    stats->rotation_count = g_logger.rotation_counter;
    stats->detailed_mode = g_logger.detailed_mode;
    stats->stack_trace_mode = g_logger.stack_trace_mode;
    stats->performance_mode = g_logger.performance_mode;
    stats->log_level = g_logger.log_level;
    
    // Copiar caminhos dos arquivos
    for (int i = 0; i < MAX_LOG_FILES; i++) {
        strncpy(stats->log_paths[i], g_logger.log_paths[i], PATH_MAX - 1);
        stats->log_paths[i][PATH_MAX - 1] = '\0';
    }
    
    pthread_mutex_unlock(&g_logger.log_mutex);
}
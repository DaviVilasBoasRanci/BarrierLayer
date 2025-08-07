#ifndef ULTRA_LOGGER_H
#define ULTRA_LOGGER_H

#include <stdbool.h>
#include <limits.h>

// Níveis de log
#define ULTRA_LOG_TRACE   0
#define ULTRA_LOG_DEBUG   1
#define ULTRA_LOG_INFO    2
#define ULTRA_LOG_WARNING 3
#define ULTRA_LOG_ERROR   4
#define ULTRA_LOG_FATAL   5

// Estrutura para estatísticas do logger
typedef struct {
    int total_files;
    int current_file;
    size_t current_size;
    int rotation_count;
    bool detailed_mode;
    bool stack_trace_mode;
    bool performance_mode;
    int log_level;
    char log_paths[10][PATH_MAX];
} ultra_logger_stats_t;

// Funções principais
int ultra_logger_init(void);
void ultra_logger_cleanup(void);
void ultra_log(int level, const char *category, const char *format, ...);

// Funções especializadas
void ultra_log_syscall(const char *syscall_name, long syscall_nr, 
                      unsigned long *args, long ret_value, const char *details);
void ultra_log_hook(const char *function_name, const char *library, 
                   void *original_addr, void *hook_addr, const char *details);
void ultra_log_anticheat(const char *anticheat_name, const char *action, 
                        const char *details);

// Função para obter estatísticas
void ultra_logger_stats(ultra_logger_stats_t *stats);

// Macros para facilitar o uso
#define ULTRA_TRACE(category, ...) ultra_log(ULTRA_LOG_TRACE, category, __VA_ARGS__)
#define ULTRA_DEBUG(category, ...) ultra_log(ULTRA_LOG_DEBUG, category, __VA_ARGS__)
#define ULTRA_INFO(category, ...) ultra_log(ULTRA_LOG_INFO, category, __VA_ARGS__)
#define ULTRA_WARN(category, ...) ultra_log(ULTRA_LOG_WARNING, category, __VA_ARGS__)
#define ULTRA_ERROR(category, ...) ultra_log(ULTRA_LOG_ERROR, category, __VA_ARGS__)
#define ULTRA_FATAL(category, ...) ultra_log(ULTRA_LOG_FATAL, category, __VA_ARGS__)

// Macros específicas
#define LOG_SYSCALL(name, nr, args, ret, details) \
    ultra_log_syscall(name, nr, args, ret, details)

#define LOG_HOOK(func, lib, orig, hook, details) \
    ultra_log_hook(func, lib, orig, hook, details)

#define LOG_ANTICHEAT(ac, action, details) \
    ultra_log_anticheat(ac, action, details)

#endif // ULTRA_LOGGER_H
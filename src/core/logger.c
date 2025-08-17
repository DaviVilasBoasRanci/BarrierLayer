
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h> // NEW: For errno

static int logger_initialized = 0;
static FILE* log_file = NULL;

void logger_init(const char *log_path) {
    if (!logger_initialized) {
        log_file = fopen(log_path, "a");
        if (!log_file) { // NEW: Check if fopen failed
            fprintf(stderr, "LOGGER_ERROR: Failed to open log file %s: %s\n", log_path, strerror(errno));
        }
        if (log_file) {
            time_t now = time(NULL);
            char time_buf[20];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
            fprintf(log_file, "[%s] BARRIERLAYER INITIALIZED\n", time_buf);
            fflush(log_file);
        }
        logger_initialized = 1;
    }
}

void logger_log(const char *log_path, const char *msg) {
    logger_init(log_path);
    if (!log_file) { // NEW: Check if log_file is still NULL after init
        fprintf(stderr, "LOGGER_ERROR: Log file not open for %s: %s\n", log_path, strerror(errno));
        return; // Prevent further attempts to write to NULL file
    }
    if (log_file) {
        time_t now = time(NULL);
        char time_buf[20];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log_file, "[%s] %s\n", time_buf, msg);
        fflush(log_file);
    }
}

void logger_logf(const char *log_path, const char *format, ...) {
    logger_init(log_path);
    if (!log_file) { // NEW: Check if log_file is still NULL after init
        fprintf(stderr, "LOGGER_ERROR: Log file not open for %s: %s\n", log_path, strerror(errno));
        return; // Prevent further attempts to write to NULL file
    }
    if (log_file) {
        time_t now = time(NULL);
        char time_buf[20];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log_file, "[%s] ", time_buf);
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

void logger_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    logger_initialized = 0;
}


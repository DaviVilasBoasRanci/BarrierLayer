#include <stdio.h>
#include <stdlib.h>

// Logger functions have been intentionally stubbed out to prevent deadlocks
// caused by re-entrant calls from within hooked functions.

void logger_init(const char *log_path) {
    (void)log_path; // Suppress unused parameter warning
}

void logger_log(const char *log_path, const char *msg) {
    (void)log_path; // Suppress unused parameter warning
    (void)msg;      // Suppress unused parameter warning
}

void logger_logf(const char *log_path, const char *format, ...) {
    (void)log_path; // Suppress unused parameter warning
    (void)format;   // Suppress unused parameter warning
}

void logger_close() {
    // Do nothing
}
#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

// Exemplo de uso em um hook:
// logger_log(LOG_PATH, "Interceptando OpenProcess");

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include "logger.h"

// Ponteiro para a função original OpenProcess
static void* (*real_OpenProcess)(unsigned long, int, unsigned long) = NULL;

// Nosso hook para OpenProcess
void* OpenProcess(unsigned long dwDesiredAccess, int bInheritHandle, unsigned long dwProcessId) {
    if (!real_OpenProcess) {
        real_OpenProcess = dlsym(RTLD_NEXT, "OpenProcess");
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "HOOK: OpenProcess | PID: %lu", dwProcessId);
    logger_log(LOG_PATH, msg);

    if (real_OpenProcess) {
        return real_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    } else {
        return NULL;
    }
}

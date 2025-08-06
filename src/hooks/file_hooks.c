#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

// Exemplo de uso em um hook:
// logger_log(LOG_PATH, "Interceptando CreateFileW");

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <dlfcn.h>
#include "logger.h"

// Função construtora, chamada quando a biblioteca é carregada
__attribute__((constructor))
void init_logger() {
    logger_init(LOG_PATH);
    logger_log(LOG_PATH, "Biblioteca de hooks do BarrierLayer carregada.");
}

// Função destrutora, chamada quando a biblioteca é descarregada
__attribute__((destructor))
void close_logger() {
    logger_log(LOG_PATH, "Biblioteca de hooks do BarrierLayer descarregada.");
    // Não precisa chamar logger_close, pois logger_log já gerencia o arquivo
}

static void* (*real_CreateFileW)(const wchar_t*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*) = NULL;

void* CreateFileW(const wchar_t* lpFileName, unsigned long dwDesiredAccess, unsigned long dwShareMode, void* lpSecurityAttributes, unsigned long dwCreationDisposition, unsigned long dwFlagsAndAttributes, void* hTemplateFile) {
    if (!real_CreateFileW) {
        real_CreateFileW = dlsym(RTLD_NEXT, "CreateFileW");
    }

    char filename[260] = {0};
    wcstombs(filename, lpFileName, sizeof(filename) - 1);
    logger_log(LOG_PATH, filename);

    if (real_CreateFileW) {
        return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    } else {
        return (void*)-1;
    }
}

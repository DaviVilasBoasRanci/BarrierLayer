#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

// Exemplo de uso em um hook:
// logger_log(LOG_PATH, "Interceptando CreateFileW");

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <dlfcn.h>
#include <string.h>
#include "logger.h"

// Função construtora, chamada quando a biblioteca é carregada
__attribute__((constructor))
void h1() {
    unsetenv("LD_PRELOAD"); // Remove LD_PRELOAD para stealth
    // --- Unlinking do loader: remove a biblioteca da lista de módulos carregados ---
    void* handle = dlopen(NULL, RTLD_NOW);
    if (handle) {
        struct link_map {
            void* l_addr; char* l_name; void* l_ld; struct link_map* l_next; struct link_map* l_prev;
        } *map;
        dlinfo(handle, 2, &map); // 2 = RTLD_DI_LINKMAP
        while (map) {
            if (strstr(map->l_name, "barrierlayer_hook.so")) {
                if (map->l_prev) map->l_prev->l_next = map->l_next;
                if (map->l_next) map->l_next->l_prev = map->l_prev;
                break;
            }
            map = map->l_next;
        }
    }
    logger_init(LOG_PATH);
    char msg[] = {0x42,0x4c,0x20,0x69,0x6e,0x69,0x74,0x0}; // "BL init"
    logger_log(LOG_PATH, msg);
}

// Função destrutora, chamada quando a biblioteca é descarregada
__attribute__((destructor))
void h2() {
    char msg[] = {0x42,0x4c,0x20,0x66,0x69,0x6e,0x0}; // "BL fin"
    logger_log(LOG_PATH, msg);
    // Não precisa chamar logger_close, pois logger_log já gerencia o arquivo
}

static void* (*real_CreateFileW)(const wchar_t*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*) = NULL;

// Hook para CreateFileW
void* CreateFileW(const wchar_t* lpFileName, unsigned long dwDesiredAccess, unsigned long dwShareMode, void* lpSecurityAttributes, unsigned long dwCreationDisposition, unsigned long dwFlagsAndAttributes, void* hTemplateFile) {
    if (!real_CreateFileW) {
        real_CreateFileW = dlsym(RTLD_NEXT, "CreateFileW");
    }
    char filename[260] = {0};
    wcstombs(filename, lpFileName, sizeof(filename) - 1);
    char msg[512];
    snprintf(msg, sizeof(msg), "HOOK: CreateFileW | File: %s", filename);
    logger_log(LOG_PATH, msg);
    if (real_CreateFileW) {
        return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    } else {
        return (void*)-1;
    }
}

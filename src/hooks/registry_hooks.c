#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

// Exemplo de uso em um hook:
// logger_log(LOG_PATH, "Interceptando RegOpenKeyExW");

#define _GNU_SOURCE
#include <dlfcn.h>
#include <wchar.h>
#include <stdlib.h> // Para wcstombs
#include "logger.h"
#include <stdio.h>

// --- RegOpenKeyExW ---
typedef long (*RegOpenKeyExW_f)(void*, const wchar_t*, unsigned long, unsigned long, void**);
static RegOpenKeyExW_f real_RegOpenKeyExW = NULL;

// Hook para RegOpenKeyExW
long RegOpenKeyExW(void* hKey, const wchar_t* lpSubKey, unsigned long ulOptions, unsigned long samDesired, void** phkResult) {
    if (!real_RegOpenKeyExW) {
        real_RegOpenKeyExW = dlsym(RTLD_NEXT, "RegOpenKeyExW");
    }
    char subKey[512] = {0};
    wcstombs(subKey, lpSubKey, sizeof(subKey) - 1);
    char msg[256];
    snprintf(msg, sizeof(msg), "HOOK: RegOpenKeyExW | Chave: %s", subKey);
    logger_log(LOG_PATH, msg);
    if (real_RegOpenKeyExW) {
        return real_RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
    } else {
        return 2; // Retorna ERROR_FILE_NOT_FOUND
    }
}

// --- RegQueryValueExW ---
typedef long (*RegQueryValueExW_f)(void*, const wchar_t*, unsigned long*, unsigned long*, void*, unsigned long*);
static RegQueryValueExW_f real_RegQueryValueExW = NULL;

long RegQueryValueExW(void* hKey, const wchar_t* lpValueName, unsigned long* lpReserved, unsigned long* lpType, void* lpData, unsigned long* lpcbData) {
    if (!real_RegQueryValueExW) {
        real_RegQueryValueExW = dlsym(RTLD_NEXT, "RegQueryValueExW");
    }

    char valueName[260] = {0};
    wcstombs(valueName, lpValueName, sizeof(valueName) - 1);
    log_message("HOOK: RegQueryValueExW | Nome do Valor: %s", valueName);

    if (real_RegQueryValueExW) {
        return real_RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    } else {
        return 2; // Retorna ERROR_FILE_NOT_FOUND
    }
}

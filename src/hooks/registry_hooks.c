#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <wchar.h>
#include <stddef.h> // Incluído para definir wchar_t
#include "../include/logger.h"
#include "../include/path_utils.h"

// Tipos de dados da API do Windows
#ifndef _WINDEF_
#define _WINDEF_
    typedef void* LPVOID;
    typedef unsigned long DWORD;
    typedef unsigned char* LPBYTE;
#endif

#ifndef _WINNT_
#define _WINNT_
    typedef void* HKEY;
    typedef const wchar_t* LPCWSTR;
    typedef wchar_t* LPWSTR;
    typedef long LSTATUS;
#endif

// --- Hook para RegOpenKeyExW ---
typedef LSTATUS (*RegOpenKeyExW_t)(HKEY, LPCWSTR, DWORD, DWORD, HKEY*);
static RegOpenKeyExW_t original_RegOpenKeyExW = NULL;

LSTATUS RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY *phkResult) {
    if (original_RegOpenKeyExW == NULL) {
        original_RegOpenKeyExW = dlsym(RTLD_NEXT, "RegOpenKeyExW");
    }

    char subKey[260] = {0};
    wcstombs(subKey, lpSubKey, sizeof(subKey) - 1);
    char msg[512];
    snprintf(msg, sizeof(msg), "HOOK: RegOpenKeyExW | Nome da Chave: %s", subKey);
    logger_log(get_log_path(), msg);

    return original_RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

// --- Hook para RegQueryValueExW ---
typedef LSTATUS (*RegQueryValueExW_t)(HKEY, LPCWSTR, DWORD*, DWORD*, LPBYTE, DWORD*);
static RegQueryValueExW_t original_RegQueryValueExW = NULL;

LSTATUS RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, DWORD* lpReserved, DWORD* lpType, LPBYTE lpData, DWORD* lpcbData) {
    if (original_RegQueryValueExW == NULL) {
        original_RegQueryValueExW = dlsym(RTLD_NEXT, "RegQueryValueExW");
    }

    char valueName[260] = {0};
    wcstombs(valueName, lpValueName, sizeof(valueName) - 1);
    char msg[512];
    snprintf(msg, sizeof(msg), "HOOK: RegQueryValueExW | Nome do Valor: %s", valueName);
    logger_log(get_log_path(), msg);

    return original_RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}
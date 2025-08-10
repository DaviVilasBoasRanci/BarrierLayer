#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stddef.h> // Incluído para consistência

// Tipos de dados da API do Windows
#ifndef _WINDEF_
#define _WINDEF_
    typedef void* LPVOID;
    typedef const void* LPCVOID;
    typedef unsigned long DWORD;
    typedef int BOOL;
    typedef void* HANDLE;
#endif

#ifndef _WINNT_
#define _WINNT_
    #if defined(__x86_64__)
        typedef unsigned long long SIZE_T;
    #else
        typedef unsigned int SIZE_T;
    #endif
#endif

// --- Hook para ReadProcessMemory ---
typedef BOOL (*ReadProcessMemory_t)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T*);
static ReadProcessMemory_t original_ReadProcessMemory = NULL;

BOOL ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead) {
    if (original_ReadProcessMemory == NULL) {
        original_ReadProcessMemory = dlsym(RTLD_NEXT, "ReadProcessMemory");
    }

    // Corrigido o especificador de formato para SIZE_T em sistemas de 64 bits
    fprintf(stderr, "[BarrierLayer Memory Hook] Interceptado: ReadProcessMemory em handle %p, endereço %p, tamanho %llu\n", 
            hProcess, lpBaseAddress, (unsigned long long)nSize);
    fflush(stderr);

    return original_ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
}

// --- Hook para WriteProcessMemory ---
typedef BOOL (*WriteProcessMemory_t)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
static WriteProcessMemory_t original_WriteProcessMemory = NULL;

BOOL WriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten) {
    if (original_WriteProcessMemory == NULL) {
        original_WriteProcessMemory = dlsym(RTLD_NEXT, "WriteProcessMemory");
    }

    // Corrigido o especificador de formato para SIZE_T em sistemas de 64 bits
    fprintf(stderr, "[BarrierLayer Memory Hook] Interceptado: WriteProcessMemory em handle %p, endereço %p, tamanho %llu\n", 
            hProcess, lpBaseAddress, (unsigned long long)nSize);
    fflush(stderr);

    return original_WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}
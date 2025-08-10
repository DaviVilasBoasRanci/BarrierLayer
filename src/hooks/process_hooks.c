#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stddef.h> // Incluído para definir wchar_t

// Tipos de dados da API do Windows, para compatibilidade
#ifndef _WINDEF_
#define _WINDEF_
    typedef void* LPVOID;
    typedef const char* LPCSTR;
    typedef char* LPSTR;
    typedef unsigned long DWORD;
    typedef int BOOL;
    typedef void* HANDLE;
    #define TRUE 1
    #define FALSE 0
#endif

// Estruturas necessárias para CreateProcessW
typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _STARTUPINFOW {
    DWORD cb;
    LPVOID lpReserved;
    LPVOID lpDesktop;
    LPVOID lpTitle;
    // ... (campos omitidos para brevidade, não são usados diretamente no hook)
} STARTUPINFOW, *LPSTARTUPINFOW;

// Ponteiro para a função original
typedef BOOL (*CreateProcessW_t)(
    const wchar_t*,
    wchar_t*,
    LPVOID,
    LPVOID,
    BOOL,
    DWORD,
    LPVOID,
    const wchar_t*,
    LPVOID, // Simplificado de LPSTARTUPINFOW para evitar redefinição completa
    LPPROCESS_INFORMATION
);
static CreateProcessW_t original_CreateProcessW = NULL;

// Nosso hook para CreateProcessW
BOOL CreateProcessW(
    const wchar_t* lpApplicationName,
    wchar_t* lpCommandLine,
    LPVOID lpProcessAttributes,
    LPVOID lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    const wchar_t* lpCurrentDirectory,
    LPVOID lpStartupInfo, // Simplificado
    LPPROCESS_INFORMATION lpProcessInformation
) {
    if (original_CreateProcessW == NULL) {
        original_CreateProcessW = dlsym(RTLD_NEXT, "CreateProcessW");
    }

    fprintf(stderr, "[BarrierLayer Process Hook] Interceptado: CreateProcessW para -> %ls\n", lpApplicationName ? lpApplicationName : lpCommandLine);
    fflush(stderr);

    return original_CreateProcessW(
        lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
        bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation
    );
}

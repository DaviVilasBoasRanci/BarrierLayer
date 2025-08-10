#define _GNU_SOURCE
#include <stdio.h>
<<<<<<< HEAD
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h> // Para exit()

__attribute__((constructor))
void barrierlayer_constructor(void) {
    fprintf(stderr, "--- [BarrierLayer] Biblioteca barrierlayer_hook.so carregada com sucesso! ---\n");
    fflush(stderr);
}

// --- Hook para open ---
typedef int (*original_open_t)(const char*, int, ...);

int open(const char *path, int flags, ...) {
    static original_open_t original_open = NULL;

    // Apenas procuramos pelo símbolo uma vez.
    if (original_open == NULL) {
        // Limpa erros antigos do dl
        dlerror();
        original_open = dlsym(RTLD_NEXT, "open");
        
        // Adicionamos uma verificação de erro robusta
        const char *dlsym_error = dlerror();
        if (dlsym_error != NULL) {
            fprintf(stderr, "[BarrierLayer FATAL] Erro ao encontrar a função 'open' original via dlsym: %s\n", dlsym_error);
            fflush(stderr);
            exit(1); // Falha explicitamente
        }
    }

    // Lógica para extrair o argumento `mode` opcional
    if ((flags & O_CREAT)) {
        va_list args;
        va_start(args, flags);
        int mode = va_arg(args, int);
        va_end(args);

        fprintf(stderr, "[BarrierLayer Hook] Interceptado: open(\"%s\", %d, %o)\n", path, flags, mode);
        fflush(stderr);
        return original_open(path, flags, mode);
    } else {
        fprintf(stderr, "[BarrierLayer Hook] Interceptado: open(\"%s\", %d)\n", path, flags);
        fflush(stderr);
        return original_open(path, flags);
    }
=======
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include "../include/logger.h"
#include "../include/ultra_logger.h"

// Definições do Windows para compatibilidade
typedef void* HANDLE;
typedef const wchar_t* LPCWSTR;
typedef unsigned long DWORD;
typedef void* LPSECURITY_ATTRIBUTES;
#define WINAPI
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define CREATE_ALWAYS 2
#define CREATE_NEW 1
#define ERROR_FILE_NOT_FOUND 2

// Função mock para SetLastError
void SetLastError(DWORD error) { errno = error; }
DWORD GetLastError(void) { return errno; }

#define LOG_PATH "/tmp/barrierlayer_activity.log"

// Ponteiro para função original
static void* (*real_CreateFileW)(const wchar_t*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*) = NULL;

// Hook para CreateFileW com logging ultra-detalhado
void* CreateFileW(const wchar_t* lpFileName, unsigned long dwDesiredAccess, unsigned long dwShareMode, 
                  void* lpSecurityAttributes, unsigned long dwCreationDisposition, 
                  unsigned long dwFlagsAndAttributes, void* hTemplateFile) {
    
    // Inicializar função original se necessário
    if (!real_CreateFileW) {
        real_CreateFileW = dlsym(RTLD_NEXT, "CreateFileW");
    }
    
    // Log detalhado da chamada
    char filename_utf8[1024] = {0};
    if (lpFileName) {
        wcstombs(filename_utf8, lpFileName, sizeof(filename_utf8) - 1);
    }
    
    ULTRA_TRACE("FILE", "CreateFileW called: file='%s' access=0x%lx share=0x%lx disp=0x%lx flags=0x%lx",
                filename_utf8, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes);
    
    // Verificar se é tentativa de acesso a arquivos sensíveis
    if (lpFileName) {
        const wchar_t *sensitive_files[] = {
            L"barrierlayer", L"hook", L"inject", L"cheat", L"bypass", 
            L"wine", L"proton", L"ld_preload", NULL
        };
        
        for (int i = 0; sensitive_files[i]; i++) {
            if (wcsstr(lpFileName, sensitive_files[i])) {
                ULTRA_WARN("FILE", "Sensitive file access attempt blocked: %s", filename_utf8);
                SetLastError(ERROR_FILE_NOT_FOUND);
                return INVALID_HANDLE_VALUE;
            }
        }
    }
    
    // Chamar função original ou simular comportamento
    void* result = INVALID_HANDLE_VALUE;
    
    if (real_CreateFileW) {
        result = real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, 
                                 lpSecurityAttributes, dwCreationDisposition, 
                                 dwFlagsAndAttributes, hTemplateFile);
    } else {
        // Simular comportamento do Windows no Linux
        if (lpFileName) {
            // Converter caminho Windows para Linux
            char linux_path[1024];
            snprintf(linux_path, sizeof(linux_path), "/tmp/wine_files/%s", filename_utf8);
            
            // Criar diretório se não existir
            mkdir("/tmp/wine_files", 0755);
            
            // Mapear flags do Windows para Linux
            int linux_flags = O_RDONLY;
            if (dwDesiredAccess & GENERIC_WRITE) linux_flags = O_WRONLY;
            if (dwDesiredAccess & (GENERIC_READ | GENERIC_WRITE)) linux_flags = O_RDWR;
            if (dwCreationDisposition == CREATE_ALWAYS) linux_flags |= O_CREAT | O_TRUNC;
            if (dwCreationDisposition == CREATE_NEW) linux_flags |= O_CREAT | O_EXCL;
            
            int fd = open(linux_path, linux_flags, 0644);
            if (fd >= 0) {
                result = (void*)(intptr_t)fd;
            }
        }
    }
    
    ULTRA_TRACE("FILE", "CreateFileW result: handle=%p error=%lu", result, GetLastError());
    
    // Log syscall equivalente
    unsigned long args[6] = {
        (unsigned long)lpFileName, dwDesiredAccess, dwShareMode,
        (unsigned long)lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes
    };
    LOG_SYSCALL("CreateFileW", 0, args, (long)result, filename_utf8);
    
    return result;
}

// Função construtora, chamada quando a biblioteca é carregada
__attribute__((constructor))
void file_hooks_init() {
    // Inicializar ultra logger
    if (ultra_logger_init() == 0) {
        ULTRA_INFO("INIT", "File hooks initialized with ultra logging");
    }
    
    // Stealth: remover LD_PRELOAD
    unsetenv("LD_PRELOAD");
    
    // Unlinking do loader: técnica avançada de stealth
    void* handle = dlopen(NULL, RTLD_NOW);
    if (handle) {
        // Tentar mascarar a presença da biblioteca
        // Método mais seguro sem acessar estruturas internas
        dlclose(handle);
    }
    
    ULTRA_INFO("STEALTH", "File hooks stealth mode activated");
>>>>>>> a909be7df856e5d04815b7b49ee1cc853f80a638
}
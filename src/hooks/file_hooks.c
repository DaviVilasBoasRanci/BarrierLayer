#define _GNU_SOURCE
#include <stdio.h>
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
}
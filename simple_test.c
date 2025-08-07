#include <stdio.h>
#include <dlfcn.h>

int main() {
    printf("=== Teste Simples do BarrierLayer ===\n");
    
    // Tentar carregar a biblioteca diretamente
    void* handle = dlopen("./bin/barrierlayer_hook.so", RTLD_LAZY);
    if (handle) {
        printf("✅ Biblioteca barrierlayer_hook.so carregada com sucesso!\n");
        
        // Verificar se algumas funções estão presentes
        void* func1 = dlsym(handle, "CreateFileW");
        void* func2 = dlsym(handle, "OpenProcess");
        void* func3 = dlsym(handle, "RegOpenKeyExW");
        
        printf("📊 Verificação de hooks:\n");
        printf("   CreateFileW: %s\n", func1 ? "✅ Presente" : "❌ Ausente");
        printf("   OpenProcess: %s\n", func2 ? "✅ Presente" : "❌ Ausente");
        printf("   RegOpenKeyExW: %s\n", func3 ? "✅ Presente" : "❌ Ausente");
        
        dlclose(handle);
    } else {
        printf("❌ Erro ao carregar biblioteca: %s\n", dlerror());
    }
    
    printf("\n🎯 Teste concluído!\n");
    return 0;
}
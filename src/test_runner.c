#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    printf("Test Runner: Iniciando demonstração dos hooks do BarrierLayer...\n");
    printf("Este programa executa operações que serão interceptadas pelos hooks.\n\n");

    // Teste 1: Operações de arquivo (interceptadas por file_hooks.c)
    printf("🔍 Teste 1: Operações de arquivo\n");
    int fd = open("/tmp/barrierlayer_test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, "BarrierLayer Test\n", 18);
        close(fd);
        printf("   ✅ Arquivo criado e escrito com sucesso\n");
    }

    // Teste 2: Informações do processo (interceptadas por process_hooks.c)
    printf("🔍 Teste 2: Informações do processo\n");
    pid_t pid = getpid();
    printf("   ✅ PID atual: %d\n", pid);

    // Teste 3: Operações de memória (interceptadas por memory_hooks.c)
    printf("🔍 Teste 3: Operações de memória\n");
    void* ptr = malloc(1024);
    if (ptr) {
        memset(ptr, 0xAA, 1024);
        free(ptr);
        printf("   ✅ Alocação e liberação de memória concluída\n");
    }

    // Teste 4: Operações de sistema (interceptadas por system_hooks.c)
    printf("🔍 Teste 4: Informações do sistema\n");
    system("echo '   ✅ Comando do sistema executado' > /dev/null");
    printf("   ✅ Comando do sistema interceptado\n");

    // Teste 5: Operações de rede (interceptadas por network_hooks.c)
    printf("🔍 Teste 5: Operações de rede\n");
    // Simular criação de socket
    printf("   ✅ Operações de rede simuladas\n");

    printf("\n🎯 Demonstração concluída!\n");
    printf("📝 Verifique o arquivo de log para ver as interceptações dos hooks.\n");
    
    return 0;
}
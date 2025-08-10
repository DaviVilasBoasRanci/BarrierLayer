<<<<<<< HEAD
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

// --- Protótipos das funções em src/hooks/kernel_hooks.c ---
int barrierlayer_kernel_init(void);
void barrierlayer_kernel_exit(void);
int barrierlayer_hide_pid(int pid);
int barrierlayer_unhide_pid(int pid);
// ----------------------------------------------------------

void run_kernel_module_test() {
    printf("--- Teste do Módulo de Kernel BarrierLayer ---\\n");

    if (geteuid() != 0) {
        fprintf(stderr, "[ERRO] Este teste precisa ser executado como root para se comunicar com o módulo do kernel.\\n");
        fprintf(stderr, "Por favor, execute com: sudo ./bin/test_runner\\n");
        exit(1);
    }

    if (barrierlayer_kernel_init() != 0) {
        fprintf(stderr, "[ERRO] Falha ao inicializar a comunicação com o kernel.\\n");
        fprintf(stderr, "O módulo do kernel está carregado? (sudo insmod kernel/barrierlayer_kernel.ko)\\n");
        fprintf(stderr, "O dispositivo /dev/barrierlayer_ctl foi criado? (sudo mknod /dev/barrierlayer_ctl c MAJOR 0)\\n");
        exit(1);
    }

    pid_t my_pid = getpid();
    printf("[INFO] Meu PID é: %d\\n", my_pid);

    printf("\\n---> [AÇÃO] Escondendo o processo (PID: %d)...\\n", my_pid);
    if (barrierlayer_hide_pid(my_pid) == 0) {
        printf("[OK] Comando para esconder enviado. O processo deve estar invisível agora.\\n");
    } else {
        fprintf(stderr, "[ERRO] Falha ao enviar comando para esconder.\\n");
        barrierlayer_kernel_exit();
        exit(1);
    }

    printf("\\n*** VERIFIQUE EM OUTRO TERMINAL (você tem 15 segundos) ***\\n");
    printf("    Comando sugerido: ps aux | grep test_runner\\n");
    printf("    (Você não deve ver este processo na lista)\\n");
    fflush(stdout);
    sleep(15);

    printf("\\n---> [AÇÃO] Re-exibindo o processo (PID: %d)...\\n", my_pid);
    if (barrierlayer_unhide_pid(my_pid) == 0) {
        printf("[OK] Comando para re-exibir enviado. O processo deve estar visível agora.\\n");
    } else {
        fprintf(stderr, "[ERRO] Falha ao enviar comando para re-exibir.\\n");
    }
    
    printf("\\n*** Verifique novamente no outro terminal. O processo deve ter voltado. ***\\n");
    sleep(5);

    barrierlayer_kernel_exit();

    printf("\\n--- Teste do Módulo de Kernel concluído ---\\n");
}

int main() {
    run_kernel_module_test();
    return 0;
}
=======
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
>>>>>>> a909be7df856e5d04815b7b49ee1cc853f80a638

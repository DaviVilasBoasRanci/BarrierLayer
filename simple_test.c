#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// --- Protótipos das funções em src/hooks/kernel_hooks.c ---
// Em um projeto maior, estes estariam em um arquivo de cabeçalho (`.h`)
int barrierlayer_kernel_init(void);
void barrierlayer_kernel_exit(void);
int barrierlayer_hide_pid(int pid);
int barrierlayer_unhide_pid(int pid);
// ----------------------------------------------------------

int main() {
    printf("--- Teste do Módulo de Kernel BarrierLayer ---\
");

    // Verifica se o programa está sendo executado como root
    if (geteuid() != 0) {
        fprintf(stderr, "[ERRO] Este teste precisa ser executado como root para se comunicar com o módulo do kernel.\
");
        fprintf(stderr, "Por favor, execute com: sudo ./simple_test\
");
        return 1;
    }

    if (barrierlayer_kernel_init() != 0) {
        fprintf(stderr, "[ERRO] Falha ao inicializar a comunicação com o kernel.\
");
        fprintf(stderr, "O módulo do kernel está carregado? (sudo insmod kernel/barrierlayer_kernel.ko)\
");
        fprintf(stderr, "O dispositivo /dev/barrierlayer_ctl foi criado? (sudo mknod /dev/barrierlayer_ctl c MAJOR 0)\
");
        return 1;
    }

    pid_t my_pid = getpid();
    printf("[INFO] Meu PID é: %d\
", my_pid);

    printf("\n---> [AÇÃO] Escondendo o processo (PID: %d)...\
", my_pid);
    if (barrierlayer_hide_pid(my_pid) == 0) {
        printf("[OK] Comando para esconder enviado. O processo deve estar invisível agora.\
");
    } else {
        fprintf(stderr, "[ERRO] Falha ao enviar comando para esconder.\
");
        barrierlayer_kernel_exit();
        return 1;
    }

    printf("\n*** VERIFIQUE EM OUTRO TERMINAL (você tem 15 segundos) ***\
");
    printf("    Comando sugerido: ps aux | grep simple_test\
");
    printf("    (Você não deve ver este processo na lista)\
");
    fflush(stdout);
    sleep(15);

    printf("\n---> [AÇÃO] Re-exibindo o processo (PID: %d)...\
", my_pid);
    if (barrierlayer_unhide_pid(my_pid) == 0) {
        printf("[OK] Comando para re-exibir enviado. O processo deve estar visível agora.\
");
    } else {
        fprintf(stderr, "[ERRO] Falha ao enviar comando para re-exibir.\
");
    }
    
    printf("\n*** Verifique novamente no outro terminal. O processo deve ter voltado. ***\
");
    sleep(5);

    barrierlayer_kernel_exit();

    printf("\n--- Teste concluído ---\
");
    return 0;
}

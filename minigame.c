// minigame.c - Jogo simples para testar hooks do BarrierLayer
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
    FILE *f = fopen("score.txt", "a+");
    if (f) {
        fprintf(f, "Iniciando o jogo...\n");
        fclose(f);
    }

    printf("=== Adivinhe o Número (1-10) ===\n");
    int secret = (rand() % 10) + 1;
    int guess = 0, tentativas = 0;

    while (guess != secret) {
        printf("Digite seu palpite: ");
        scanf("%d", &guess);
        tentativas++;
        if (guess < secret) printf("Maior!\n");
        else if (guess > secret) printf("Menor!\n");
    }

    printf("Parabéns! Você acertou em %d tentativas.\n", tentativas);

    // Testa chamada de processo
    DWORD pid = GetCurrentProcessId();
    printf("PID do processo: %lu\n", (unsigned long)pid);

    // Testa chamada de registro (opcional, pode não funcionar em Wine puro)
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        printf("Registro aberto com sucesso!\n");
        RegCloseKey(hKey);
    } else {
        printf("Falha ao abrir registro.\n");
    }

    // Salva pontuação
    f = fopen("score.txt", "a+");
    if (f) {
        fprintf(f, "Tentativas: %d\n", tentativas);
        fclose(f);
    }

    printf("Fim do jogo!\n");
    return 0;
}
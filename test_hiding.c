#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main() {
    printf("--- Iniciando Teste de File Hiding ---\n");

    const char* hidden_path = "/usr/bin/wine"; // Este caminho está em files.conf
    const char* normal_path = "/etc/hosts";      // Este caminho não deve ser bloqueado

    int hidden_result;
    int normal_result;
    int final_result = 0;

    // --- Teste 1: Tentar acessar um caminho escondido ---
    printf("1. Tentando acessar o caminho escondido: %s\n", hidden_path);
    hidden_result = access(hidden_path, F_OK);

    if (hidden_result == -1 && errno == ENOENT) {
        printf("   [SUCESSO] Acesso negado como esperado (errno=ENOENT).\n");
    } else if (hidden_result == 0) {
        printf("   [FALHA] Hook não funcionou! O caminho foi encontrado.\n");
        final_result = 1;
    } else {
        printf("   [FALHA] Resultado inesperado. Retorno: %d, errno: %d\n", hidden_result, errno);
        final_result = 1;
    }

    // --- Teste 2: Garantir que caminhos normais não são bloqueados ---
    printf("\n2. Tentando acessar um caminho normal: %s\n", normal_path);
    normal_result = access(normal_path, F_OK);

    if (normal_result == 0) {
        printf("   [SUCESSO] Acesso permitido como esperado.\n");
    } else {
        printf("   [FALHA] Hook bloqueou um caminho que não deveria! Retorno: %d, errno: %d\n", normal_result, errno);
        final_result = 1;
    }

    // --- Resultado Final ---
    printf("\n--- Teste Concluído ---\n");
    if (final_result == 0) {
        printf("✅ RESULTADO: SUCESSO GERAL! Os hooks de file hiding estão funcionando corretamente.\n");
    } else {
        printf("❌ RESULTADO: FALHA GERAL! Verifique os logs e a implementação do hook.\n");
    }

    return final_result;
}

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

// Enumeração para os perfis de desempenho
typedef enum {
    PROFILE_SECURITY,  // Padrão: todas as verificações ativadas
    PROFILE_PERFORMANCE // Desempenho: desativa algumas verificações custosas
} performance_profile_t;

// Retorna o perfil de desempenho atualmente ativo.
// O perfil é determinado pela variável de ambiente BARRIERLAYER_PERF_PROFILE.
performance_profile_t get_performance_profile(void);

#endif // PERFORMANCE_H

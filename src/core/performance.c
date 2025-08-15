#include "../include/performance.h"
#include <stdlib.h>
#include <string.h>

static performance_profile_t current_profile = PROFILE_SECURITY; // Padrão
static int profile_initialized = 0;

performance_profile_t get_performance_profile(void) {
    if (!profile_initialized) {
        const char* profile_env = getenv("BARRIERLAYER_PERF_PROFILE");
        if (profile_env) {
            if (strcasecmp(profile_env, "PERFORMANCE") == 0) {
                current_profile = PROFILE_PERFORMANCE;
            }
            // Outros perfis podem ser adicionados aqui
        }
        profile_initialized = 1;
    }
    return current_profile;
}

#ifndef PROTON_SUPPORT_H
#define PROTON_SUPPORT_H

#include <glob.h>

#ifdef __cplusplus
extern "C" {
#endif

// Funções principais do suporte ao Proton
int detect_proton_installation(void);
int proton_init(void);
int setup_proton_prefix(void);
void setup_proton_environment(void);
void setup_proton_anticheat_environment(void);
int proton_run_application(const char* executable, char* const argv[]);

// Funções de informação
const char* proton_get_version(void);
const char* proton_get_path(void);
const char* proton_get_prefix(void);
int proton_is_active(void);

// Limpeza
void proton_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* PROTON_SUPPORT_H */
#ifndef BARRIERLAYER_H
#define BARRIERLAYER_H

#include <stdint.h>
#include <sys/types.h>

// Definições de status
#define BL_SUCCESS 0
#define BL_ERROR   -1

// Flags de configuração
#define BL_FLAG_EAC      0x0001
#define BL_FLAG_BATTLEYE 0x0002
#define BL_FLAG_SANDBOX  0x0004
#define BL_FLAG_DEBUG    0x0008

// Estrutura principal do BarrierLayer
typedef struct {
    uint32_t flags;
    void* eac_context;
    void* battleye_context;
    void* sandbox_context;
    char* game_path;
} barrierlayer_context_t;

// Funções principais
int bl_initialize(const char* game_path, uint32_t flags);
int bl_shutdown(void);

// Funções do EAC
int bl_init_eac(void);
int bl_handle_eac_check(void* data, size_t size);
int bl_cleanup_eac(void);

// Funções do BattlEye
int bl_init_battleye(void);
int bl_handle_battleye_check(void* data, size_t size);
int bl_cleanup_battleye(void);

// Funções do Sandbox
int bl_init_sandbox(void);
int bl_sandbox_check_violation(void);
int bl_cleanup_sandbox(void);

// Funções de logging
void bl_log_info(const char* msg);
void bl_log_error(const char* msg);
void bl_log_debug(const char* msg);

// Funções de virtualização
int bl_setup_virtualization(void);
int bl_handle_virtualization_check(void);
int bl_cleanup_virtualization(void);

// Funções de Wine/Proton
int bl_setup_wine_hooks(void);
int bl_handle_wine_call(uint32_t syscall_num, void* params);
int bl_cleanup_wine_hooks(void);

#endif // BARRIERLAYER_H

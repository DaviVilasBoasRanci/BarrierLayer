#ifndef SYSCALL_HOOKS_H
#define SYSCALL_HOOKS_H

// Funções exportadas do assembly
extern void hook_syscall(void);
extern void setup_syscall_hook(void);

// Funções auxiliares em C
int initialize_syscall_hooks(void);
void cleanup_syscall_hooks(void);

// Estruturas para comunicação com o sistema anti-cheat
typedef struct {
    unsigned long syscall_number;
    unsigned long parameters[6];
    unsigned long result;
} syscall_info_t;

// Callbacks para personalização do comportamento
typedef int (*syscall_handler_t)(syscall_info_t *info);

// Registro de handlers personalizados
int register_syscall_handler(unsigned long syscall_number, syscall_handler_t handler);

#endif // SYSCALL_HOOKS_H

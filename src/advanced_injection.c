#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <stdbool.h>

#define MAX_PAYLOAD_SIZE 4096
#define MAX_PROCESSES 32
#define INJECTION_MAGIC 0xDEADBEEF

// Estrutura para gerenciar processos injetados
struct injected_process {
    pid_t pid;
    void *injected_addr;
    size_t injected_size;
    bool is_active;
};

// Estrutura para payload de injeção
struct injection_payload {
    uint32_t magic;
    uint32_t size;
    void (*hook_function)(void);
    char code[MAX_PAYLOAD_SIZE - 16];
};

static struct injected_process injected_procs[MAX_PROCESSES];
static int injected_count = 0;

// Função para ler memória de processo remoto
static int read_process_memory(pid_t pid, void *addr, void *buffer, size_t size) {
    int fd;
    char mem_path[64];
    ssize_t bytes_read;
    
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    fd = open(mem_path, O_RDONLY);
    if (fd == -1) {
        return -1;
    }
    
    if (lseek(fd, (off_t)addr, SEEK_SET) == -1) {
        close(fd);
        return -1;
    }
    
    bytes_read = read(fd, buffer, size);
    close(fd);
    
    return bytes_read == (ssize_t)size ? 0 : -1;
}

// Função para escrever memória de processo remoto
static int write_process_memory(pid_t pid, void *addr, const void *buffer, size_t size) {
    int fd;
    char mem_path[64];
    ssize_t bytes_written;
    
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    fd = open(mem_path, O_WRONLY);
    if (fd == -1) {
        return -1;
    }
    
    if (lseek(fd, (off_t)addr, SEEK_SET) == -1) {
        close(fd);
        return -1;
    }
    
    bytes_written = write(fd, buffer, size);
    close(fd);
    
    return bytes_written == (ssize_t)size ? 0 : -1;
}

// Função para encontrar região de memória executável
static void* find_executable_region(pid_t pid, size_t size) {
    FILE *maps;
    char maps_path[64];
    char line[256];
    void *addr = NULL;
    
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
    maps = fopen(maps_path, "r");
    if (!maps) {
        return NULL;
    }
    
    while (fgets(line, sizeof(line), maps)) {
        unsigned long start, end;
        char perms[8];
        
        if (sscanf(line, "%lx-%lx %7s", &start, &end, perms) == 3) {
            // Procurar por região executável com espaço suficiente
            if (strstr(perms, "x") && (end - start) >= size) {
                addr = (void*)start;
                break;
            }
        }
    }
    
    fclose(maps);
    return addr;
}

// Shellcode para hook básico (x86_64)
static unsigned char hook_shellcode[] = {
    // Salvar registradores
    0x50,                           // push %rax
    0x51,                           // push %rcx
    0x52,                           // push %rdx
    0x53,                           // push %rbx
    0x54,                           // push %rsp
    0x55,                           // push %rbp
    0x56,                           // push %rsi
    0x57,                           // push %rdi
    0x41, 0x50,                     // push %r8
    0x41, 0x51,                     // push %r9
    0x41, 0x52,                     // push %r10
    0x41, 0x53,                     // push %r11
    0x41, 0x54,                     // push %r12
    0x41, 0x55,                     // push %r13
    0x41, 0x56,                     // push %r14
    0x41, 0x57,                     // push %r15
    
    // Chamar função de hook personalizada
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov $hook_func, %rax
    0xFF, 0xD0,                     // call *%rax
    
    // Restaurar registradores
    0x41, 0x5F,                     // pop %r15
    0x41, 0x5E,                     // pop %r14
    0x41, 0x5D,                     // pop %r13
    0x41, 0x5C,                     // pop %r12
    0x41, 0x5B,                     // pop %r11
    0x41, 0x5A,                     // pop %r10
    0x41, 0x59,                     // pop %r9
    0x41, 0x58,                     // pop %r8
    0x5F,                           // pop %rdi
    0x5E,                           // pop %rsi
    0x5D,                           // pop %rbp
    0x5C,                           // pop %rsp
    0x5B,                           // pop %rbx
    0x5A,                           // pop %rdx
    0x59,                           // pop %rcx
    0x58,                           // pop %rax
    
    // Jump para função original
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov $orig_func, %rax
    0xFF, 0xE0                      // jmp *%rax
};

// Função para criar payload de injeção
static struct injection_payload* create_injection_payload(void (*hook_func)(void), void *orig_func) {
    struct injection_payload *payload;
    
    payload = malloc(sizeof(struct injection_payload));
    if (!payload) {
        return NULL;
    }
    
    payload->magic = INJECTION_MAGIC;
    payload->size = sizeof(hook_shellcode);
    payload->hook_function = hook_func;
    
    // Copiar shellcode e patchear endereços
    memcpy(payload->code, hook_shellcode, sizeof(hook_shellcode));
    
    // Patchear endereço da função de hook (offset 18)
    *(void**)(payload->code + 18) = hook_func;
    
    // Patchear endereço da função original (offset 58)
    *(void**)(payload->code + 58) = orig_func;
    
    return payload;
}

// Função para injetar código em processo alvo
int inject_code_into_process(pid_t pid, void (*hook_func)(void), void *target_func) {
    struct injection_payload *payload;
    void *remote_addr;
    void *exec_region;
    long orig_instruction;
    unsigned char jump_code[14];
    int status;
    
    printf("Iniciando injeção de código no processo %d...\n", pid);
    
    // Anexar ao processo
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace ATTACH");
        return -1;
    }
    
    // Aguardar processo parar
    waitpid(pid, &status, 0);
    
    // Criar payload
    payload = create_injection_payload(hook_func, target_func);
    if (!payload) {
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }
    
    // Encontrar região executável
    exec_region = find_executable_region(pid, payload->size);
    if (!exec_region) {
        printf("Não foi possível encontrar região executável adequada\n");
        free(payload);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }
    
    // Alocar memória no processo remoto
    remote_addr = mmap(NULL, payload->size, PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (remote_addr == MAP_FAILED) {
        perror("mmap");
        free(payload);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }
    
    // Escrever payload na memória remota
    if (write_process_memory(pid, remote_addr, payload, payload->size) == -1) {
        printf("Falha ao escrever payload na memória remota\n");
        munmap(remote_addr, payload->size);
        free(payload);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }
    
    // Ler instrução original da função alvo
    if (read_process_memory(pid, target_func, &orig_instruction, sizeof(long)) == -1) {
        printf("Falha ao ler instrução original\n");
        munmap(remote_addr, payload->size);
        free(payload);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }
    
    // Criar jump para o código injetado
    jump_code[0] = 0x48; // REX.W prefix
    jump_code[1] = 0xB8; // MOV RAX, imm64
    *(void**)(jump_code + 2) = remote_addr;
    jump_code[10] = 0xFF; // JMP RAX
    jump_code[11] = 0xE0;
    jump_code[12] = 0x90; // NOP
    jump_code[13] = 0x90; // NOP
    
    // Escrever jump na função alvo
    if (write_process_memory(pid, target_func, jump_code, sizeof(jump_code)) == -1) {
        printf("Falha ao escrever jump na função alvo\n");
        munmap(remote_addr, payload->size);
        free(payload);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }
    
    // Registrar processo injetado
    if (injected_count < MAX_PROCESSES) {
        injected_procs[injected_count].pid = pid;
        injected_procs[injected_count].injected_addr = remote_addr;
        injected_procs[injected_count].injected_size = payload->size;
        injected_procs[injected_count].is_active = true;
        injected_count++;
    }
    
    // Desanexar do processo
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    
    free(payload);
    printf("Injeção de código concluída com sucesso!\n");
    return 0;
}

// Função para remover injeção de código
int remove_code_injection(pid_t pid) {
    int i;
    
    for (i = 0; i < injected_count; i++) {
        if (injected_procs[i].pid == pid && injected_procs[i].is_active) {
            // Anexar ao processo
            if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
                continue;
            }
            
            int status;
            waitpid(pid, &status, 0);
            
            // Limpar memória injetada
            munmap(injected_procs[i].injected_addr, injected_procs[i].injected_size);
            
            // Desanexar
            ptrace(PTRACE_DETACH, pid, NULL, NULL);
            
            injected_procs[i].is_active = false;
            printf("Injeção removida do processo %d\n", pid);
            return 0;
        }
    }
    
    return -1;
}

// Função para listar processos injetados
void list_injected_processes(void) {
    int i;
    
    printf("Processos com código injetado:\n");
    for (i = 0; i < injected_count; i++) {
        if (injected_procs[i].is_active) {
            printf("  PID: %d, Endereço: %p, Tamanho: %zu bytes\n",
                   injected_procs[i].pid,
                   injected_procs[i].injected_addr,
                   injected_procs[i].injected_size);
        }
    }
}

// Função de hook de exemplo para demonstração
void example_hook_function(void) {
    // Esta função seria chamada quando a função hookada for executada
    // Aqui você pode implementar a lógica de bypass de anti-cheat
    
    // Exemplo: log da chamada
    printf("Hook executado! PID: %d\n", getpid());
    
    // Aqui você pode implementar:
    // - Mascaramento de informações sensíveis
    // - Bypass de verificações de integridade
    // - Modificação de retornos de função
    // - Etc.
}

// Função para injetar em processo por nome
int inject_by_process_name(const char *process_name, void (*hook_func)(void), void *target_func) {
    FILE *proc;
    char line[256];
    pid_t pid;
    int injected = 0;
    
    proc = popen("ps -eo pid,comm --no-headers", "r");
    if (!proc) {
        return -1;
    }
    
    while (fgets(line, sizeof(line), proc)) {
        char comm[256];
        if (sscanf(line, "%d %255s", &pid, comm) == 2) {
            if (strstr(comm, process_name)) {
                printf("Encontrado processo %s com PID %d\n", process_name, pid);
                if (inject_code_into_process(pid, hook_func, target_func) == 0) {
                    injected++;
                }
            }
        }
    }
    
    pclose(proc);
    return injected;
}

// Função para cleanup de todas as injeções
void cleanup_all_injections(void) {
    int i;
    
    for (i = 0; i < injected_count; i++) {
        if (injected_procs[i].is_active) {
            remove_code_injection(injected_procs[i].pid);
        }
    }
    
    injected_count = 0;
    printf("Todas as injeções foram removidas\n");
}

// Função principal para demonstração
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <pid_alvo>\n", argv[0]);
        printf("Ou: %s --process-name <nome_processo>\n", argv[0]);
        printf("Ou: %s --list\n", argv[0]);
        printf("Ou: %s --cleanup\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "--list") == 0) {
        list_injected_processes();
        return 0;
    }
    
    if (strcmp(argv[1], "--cleanup") == 0) {
        cleanup_all_injections();
        return 0;
    }
    
    if (strcmp(argv[1], "--process-name") == 0) {
        if (argc < 3) {
            printf("Nome do processo não especificado\n");
            return 1;
        }
        
        // Para demonstração, vamos usar uma função fictícia como alvo
        void *target_func = (void*)0x400000; // Endereço fictício
        
        int result = inject_by_process_name(argv[2], example_hook_function, target_func);
        printf("Injetado em %d processos\n", result);
        return 0;
    }
    
    // Injeção por PID
    pid_t target_pid = atoi(argv[1]);
    if (target_pid <= 0) {
        printf("PID inválido: %s\n", argv[1]);
        return 1;
    }
    
    // Para demonstração, vamos usar uma função fictícia como alvo
    void *target_func = (void*)0x400000; // Endereço fictício
    
    if (inject_code_into_process(target_pid, example_hook_function, target_func) == 0) {
        printf("Injeção bem-sucedida no processo %d\n", target_pid);
        list_injected_processes();
    } else {
        printf("Falha na injeção no processo %d\n", target_pid);
    }
    
    return 0;
}

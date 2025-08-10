#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
<<<<<<< HEAD
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dirent.h>
#include <linux/version.h>
#include <linux/kprobes.h> // Usaremos kprobes para encontrar kallsyms_lookup_name

#include "../src/include/barrierlayer.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gemini");
MODULE_DESCRIPTION("BarrierLayer Kernel Module for Process Hiding");

#define MAX_HIDDEN_PIDS 32

// --- Typedefs para ponteiros de função ---
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
    typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
#else
    // Definição legada para kernels mais antigos
    typedef asmlinkage long (*t_syscall)(unsigned int, const char __user *, unsigned int, unsigned int);
#endif

// --- Variáveis Globais ---
static unsigned long **sys_call_table;
static t_syscall orig_getdents64;
static int hidden_pids[MAX_HIDDEN_PIDS];
static int hidden_pids_count = 0;
static int major_number;

// --- Funções para manipulação da memória ---
static void disable_write_protection(void) {
    write_cr0(read_cr0() & (~0x10000));
}
static void enable_write_protection(void) {
    write_cr0(read_cr0() | 0x10000);
}

// --- O Hook: nossa versão da getdents64 ---
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
asmlinkage long hooked_getdents64(const struct pt_regs *regs) {
    pr_info("BarrierLayer: [DEBUG] hooked_getdents64 CALLED!\n");

    long nread;
    struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)regs->si;
    nread = orig_getdents64(regs);
#else
asmlinkage long hooked_getdents64(unsigned int fd, struct linux_dirent64 __user *dirent, unsigned int count) {
    long nread;
    nread = orig_getdents64(fd, dirent, count);
#endif

    long bpos;
    struct linux_dirent64 *d;
    int i;

    if (nread <= 0) return nread;

    for (bpos = 0; bpos < nread;) {
        d = (struct linux_dirent64 *)((char *)dirent + bpos);
        for (i = 0; i < hidden_pids_count; i++) {
            char pid_str[10];
            snprintf(pid_str, sizeof(pid_str), "%d", hidden_pids[i]);
            if (strcmp(d->d_name, pid_str) == 0) {
                pr_info("BarrierLayer: [DEBUG] Match found! Hiding PID %s\n", d->d_name);
                int reclen = d->d_reclen;
                int err = nread - (bpos + reclen);
                if (err > 0) {
                    memmove(d, (char *)d + reclen, err);
                }
                nread -= reclen;
                goto next_record; // Evita o incremento de bpos
            }
        }
        bpos += d->d_reclen;
    next_record:
        ;
    }
    return nread;
}

// --- Implementação do Dispositivo de Caractere (IOCTL) ---
static long barrierlayer_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    barrierlayer_ioctl_data data;
    int pid_to_manage, i, found;

    if (copy_from_user(&data, (barrierlayer_ioctl_data __user *)arg, sizeof(data))) return -EFAULT;

    pid_to_manage = data.pid;

    switch (cmd) {
        case BL_IOCTL_HIDE_PID:
            if (hidden_pids_count < MAX_HIDDEN_PIDS) {
                pr_info("BarrierLayer: Hiding PID %d\n", pid_to_manage);
                hidden_pids[hidden_pids_count++] = pid_to_manage;
            } else {
                pr_warn("BarrierLayer: Max hidden PIDs reached\n");
                return -ENOMEM;
            }
            break;
        case BL_IOCTL_UNHIDE_PID:
            pr_info("BarrierLayer: Unhiding PID %d\n", pid_to_manage);
            found = 0;
            for (i = 0; i < hidden_pids_count; i++) {
                if (hidden_pids[i] == pid_to_manage) {
                    hidden_pids[i] = hidden_pids[hidden_pids_count - 1];
                    hidden_pids_count--;
                    found = 1;
                    break;
                }
            }
            if (!found) return -EINVAL;
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static struct file_operations fops = { .unlocked_ioctl = barrierlayer_ioctl };

// --- Inicialização e Saída do Módulo ---
static int __init barrierlayer_init(void) {
    kallsyms_lookup_name_t kallsyms_lookup_name;
    struct kprobe kp = { .symbol_name = "kallsyms_lookup_name" };

    pr_info("BarrierLayer: Initializing Kernel Module...\n");

    // Usa kprobe para obter o endereço de kallsyms_lookup_name
    if (register_kprobe(&kp) < 0) {
        pr_err("BarrierLayer: kprobe registration failed for kallsyms_lookup_name\n");
        return -EFAULT;
    }
    kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
    unregister_kprobe(&kp);

    sys_call_table = (unsigned long **)kallsyms_lookup_name("sys_call_table");
    if (!sys_call_table) {
        pr_err("BarrierLayer: sys_call_table not found!\n");
        return -ENOSYS;
    }
    pr_info("BarrierLayer: sys_call_table found at %p\n", sys_call_table);

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("BarrierLayer: Failed to register a major number\n");
        return major_number;
    }
    pr_info("BarrierLayer: Registered correctly with major number %d\n", major_number);

    orig_getdents64 = (t_syscall)sys_call_table[__NR_getdents64];
    disable_write_protection();
    sys_call_table[__NR_getdents64] = (unsigned long *)hooked_getdents64;
    enable_write_protection();

    pr_info("BarrierLayer: [DEBUG] Value after hook attempt: %p, expecting %p\n", sys_call_table[__NR_getdents64], hooked_getdents64);

    pr_info("BarrierLayer: Hooked getdents64 successfully.\n");
    pr_info("BarrierLayer: Module loaded.\n");
    return 0;
}

static void __exit barrierlayer_exit(void) {
    if (sys_call_table && orig_getdents64) {
        disable_write_protection();
        sys_call_table[__NR_getdents64] = (unsigned long *)orig_getdents64;
        enable_write_protection();
        pr_info("BarrierLayer: Unhooked getdents64.\n");
    }
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("BarrierLayer: Module unloaded.\n");
=======
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/security.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <asm/paravirt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BarrierLayer Team");
MODULE_DESCRIPTION("BarrierLayer Kernel Mode Anti-Cheat Bypass");
MODULE_VERSION("2.0");

// Configurações do módulo
#define BARRIERLAYER_PROC_NAME "barrierlayer"
#define MAX_LOG_ENTRIES 10000
#define LOG_BUFFER_SIZE 512

// Estrutura para logs detalhados
struct barrierlayer_log_entry {
    struct timespec64 timestamp;
    pid_t pid;
    pid_t tgid;
    char comm[TASK_COMM_LEN];
    char syscall_name[64];
    unsigned long syscall_nr;
    unsigned long args[6];
    long ret_value;
    char details[256];
    struct list_head list;
};

// Variáveis globais
static struct list_head log_list;
static spinlock_t log_lock;
static atomic_t log_count = ATOMIC_INIT(0);
static struct proc_dir_entry *proc_entry;

// Tabela de syscalls original
static unsigned long *sys_call_table;
static bool table_found = false;

// Ponteiros para syscalls originais
static asmlinkage long (*original_sys_open)(const char __user *filename, int flags, umode_t mode);
static asmlinkage long (*original_sys_openat)(int dfd, const char __user *filename, int flags, umode_t mode);
static asmlinkage long (*original_sys_read)(unsigned int fd, char __user *buf, size_t count);
static asmlinkage long (*original_sys_write)(unsigned int fd, const char __user *buf, size_t count);
static asmlinkage long (*original_sys_close)(unsigned int fd);
static asmlinkage long (*original_sys_execve)(const char __user *filename, const char __user *const __user *argv, const char __user *const __user *envp);
static asmlinkage long (*original_sys_clone)(unsigned long clone_flags, unsigned long newsp, int __user *parent_tidptr, int __user *child_tidptr, unsigned long tls);
static asmlinkage long (*original_sys_mmap)(unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long off);
static asmlinkage long (*original_sys_munmap)(unsigned long addr, size_t len);
static asmlinkage long (*original_sys_mprotect)(unsigned long start, size_t len, unsigned long prot);
static asmlinkage long (*original_sys_ptrace)(long request, long pid, unsigned long addr, unsigned long data);
static asmlinkage long (*original_sys_kill)(pid_t pid, int sig);

// Função para adicionar entrada de log
static void add_log_entry(const char *syscall_name, unsigned long syscall_nr, 
                         unsigned long *args, long ret_value, const char *details) {
    struct barrierlayer_log_entry *entry;
    unsigned long flags;
    
    // Limitar número de entradas para evitar overflow de memória
    if (atomic_read(&log_count) >= MAX_LOG_ENTRIES) {
        return;
    }
    
    entry = kmalloc(sizeof(struct barrierlayer_log_entry), GFP_ATOMIC);
    if (!entry) {
        return;
    }
    
    // Preencher dados da entrada
    ktime_get_real_ts64(&entry->timestamp);
    entry->pid = current->pid;
    entry->tgid = current->tgid;
    strncpy(entry->comm, current->comm, TASK_COMM_LEN);
    strncpy(entry->syscall_name, syscall_name, 63);
    entry->syscall_name[63] = '\0';
    entry->syscall_nr = syscall_nr;
    entry->ret_value = ret_value;
    
    // Copiar argumentos
    if (args) {
        memcpy(entry->args, args, sizeof(unsigned long) * 6);
    } else {
        memset(entry->args, 0, sizeof(unsigned long) * 6);
    }
    
    // Copiar detalhes
    if (details) {
        strncpy(entry->details, details, 255);
        entry->details[255] = '\0';
    } else {
        entry->details[0] = '\0';
    }
    
    // Adicionar à lista
    spin_lock_irqsave(&log_lock, flags);
    list_add_tail(&entry->list, &log_list);
    atomic_inc(&log_count);
    spin_unlock_irqrestore(&log_lock, flags);
}

// Função para detectar anti-cheats conhecidos
static bool is_anticheat_process(void) {
    const char *anticheat_names[] = {
        "EasyAntiCheat", "BEService", "BattlEye", "VAC", "ESEA", 
        "FaceIT", "Vanguard", "GameGuard", "nProtect", "Xigncode",
        "PunkBuster", "Ricochet", "TruePlay", NULL
    };
    int i;
    
    for (i = 0; anticheat_names[i]; i++) {
        if (strstr(current->comm, anticheat_names[i])) {
            return true;
        }
    }
    return false;
}

// Função para mascarar informações sensíveis
static void mask_sensitive_info(char *buffer, size_t size) {
    const char *sensitive_patterns[] = {
        "LD_PRELOAD", "barrierlayer", "hook", "inject", "cheat", 
        "bypass", "wine", "proton", NULL
    };
    int i;
    
    for (i = 0; sensitive_patterns[i]; i++) {
        char *pos = strstr(buffer, sensitive_patterns[i]);
        if (pos && (pos + strlen(sensitive_patterns[i]) <= buffer + size)) {
            memset(pos, 'X', strlen(sensitive_patterns[i]));
        }
    }
}

// Hook para sys_open
static asmlinkage long hooked_sys_open(const char __user *filename, int flags, umode_t mode) {
    long ret;
    char *kfilename;
    char details[256];
    unsigned long args[6] = {(unsigned long)filename, flags, mode, 0, 0, 0};
    
    ret = original_sys_open(filename, flags, mode);
    
    // Copiar filename do userspace
    kfilename = kmalloc(PATH_MAX, GFP_KERNEL);
    if (kfilename && strncpy_from_user(kfilename, filename, PATH_MAX) > 0) {
        // Mascarar informações sensíveis se for processo anti-cheat
        if (is_anticheat_process()) {
            mask_sensitive_info(kfilename, PATH_MAX);
        }
        
        snprintf(details, sizeof(details), "file='%s' flags=0x%x mode=0%o fd=%ld", 
                kfilename, flags, mode, ret);
        add_log_entry("sys_open", __NR_open, args, ret, details);
        kfree(kfilename);
    } else {
        snprintf(details, sizeof(details), "file=<invalid> flags=0x%x mode=0%o fd=%ld", 
                flags, mode, ret);
        add_log_entry("sys_open", __NR_open, args, ret, details);
    }
    
    return ret;
}

// Hook para sys_openat
static asmlinkage long hooked_sys_openat(int dfd, const char __user *filename, int flags, umode_t mode) {
    long ret;
    char *kfilename;
    char details[256];
    unsigned long args[6] = {dfd, (unsigned long)filename, flags, mode, 0, 0};
    
    ret = original_sys_openat(dfd, filename, flags, mode);
    
    kfilename = kmalloc(PATH_MAX, GFP_KERNEL);
    if (kfilename && strncpy_from_user(kfilename, filename, PATH_MAX) > 0) {
        if (is_anticheat_process()) {
            mask_sensitive_info(kfilename, PATH_MAX);
        }
        
        snprintf(details, sizeof(details), "dfd=%d file='%s' flags=0x%x mode=0%o fd=%ld", 
                dfd, kfilename, flags, mode, ret);
        add_log_entry("sys_openat", __NR_openat, args, ret, details);
        kfree(kfilename);
    } else {
        snprintf(details, sizeof(details), "dfd=%d file=<invalid> flags=0x%x mode=0%o fd=%ld", 
                dfd, flags, mode, ret);
        add_log_entry("sys_openat", __NR_openat, args, ret, details);
    }
    
    return ret;
}

// Hook para sys_read
static asmlinkage long hooked_sys_read(unsigned int fd, char __user *buf, size_t count) {
    long ret;
    char details[256];
    unsigned long args[6] = {fd, (unsigned long)buf, count, 0, 0, 0};
    
    ret = original_sys_read(fd, buf, count);
    
    snprintf(details, sizeof(details), "fd=%u buf=%p count=%zu bytes_read=%ld", 
            fd, buf, count, ret);
    add_log_entry("sys_read", __NR_read, args, ret, details);
    
    return ret;
}

// Hook para sys_write
static asmlinkage long hooked_sys_write(unsigned int fd, const char __user *buf, size_t count) {
    long ret;
    char details[256];
    unsigned long args[6] = {fd, (unsigned long)buf, count, 0, 0, 0};
    
    ret = original_sys_write(fd, buf, count);
    
    snprintf(details, sizeof(details), "fd=%u buf=%p count=%zu bytes_written=%ld", 
            fd, buf, count, ret);
    add_log_entry("sys_write", __NR_write, args, ret, details);
    
    return ret;
}

// Hook para sys_execve
static asmlinkage long hooked_sys_execve(const char __user *filename, 
                                       const char __user *const __user *argv, 
                                       const char __user *const __user *envp) {
    long ret;
    char *kfilename;
    char details[256];
    unsigned long args[6] = {(unsigned long)filename, (unsigned long)argv, (unsigned long)envp, 0, 0, 0};
    
    ret = original_sys_execve(filename, argv, envp);
    
    kfilename = kmalloc(PATH_MAX, GFP_KERNEL);
    if (kfilename && strncpy_from_user(kfilename, filename, PATH_MAX) > 0) {
        if (is_anticheat_process()) {
            mask_sensitive_info(kfilename, PATH_MAX);
        }
        
        snprintf(details, sizeof(details), "exec='%s' pid=%d tgid=%d ret=%ld", 
                kfilename, current->pid, current->tgid, ret);
        add_log_entry("sys_execve", __NR_execve, args, ret, details);
        kfree(kfilename);
    } else {
        snprintf(details, sizeof(details), "exec=<invalid> pid=%d tgid=%d ret=%ld", 
                current->pid, current->tgid, ret);
        add_log_entry("sys_execve", __NR_execve, args, ret, details);
    }
    
    return ret;
}

// Hook para sys_mmap
static asmlinkage long hooked_sys_mmap(unsigned long addr, unsigned long len, 
                                     unsigned long prot, unsigned long flags, 
                                     unsigned long fd, unsigned long off) {
    long ret;
    char details[256];
    unsigned long args[6] = {addr, len, prot, flags, fd, off};
    
    ret = original_sys_mmap(addr, len, prot, flags, fd, off);
    
    snprintf(details, sizeof(details), "addr=0x%lx len=%lu prot=0x%lx flags=0x%lx fd=%lu off=%lu mapped=0x%lx", 
            addr, len, prot, flags, fd, off, ret);
    add_log_entry("sys_mmap", __NR_mmap, args, ret, details);
    
    return ret;
}

// Hook para sys_ptrace
static asmlinkage long hooked_sys_ptrace(long request, long pid, unsigned long addr, unsigned long data) {
    long ret;
    char details[256];
    unsigned long args[6] = {request, pid, addr, data, 0, 0};
    
    // Se for um anti-cheat tentando usar ptrace, retornar erro
    if (is_anticheat_process() && (request == PTRACE_ATTACH || request == PTRACE_TRACEME)) {
        add_log_entry("sys_ptrace", __NR_ptrace, args, -EPERM, "BLOCKED: Anti-cheat ptrace attempt");
        return -EPERM;
    }
    
    ret = original_sys_ptrace(request, pid, addr, data);
    
    snprintf(details, sizeof(details), "req=%ld pid=%ld addr=0x%lx data=0x%lx ret=%ld", 
            request, pid, addr, data, ret);
    add_log_entry("sys_ptrace", __NR_ptrace, args, ret, details);
    
    return ret;
}

// Função para encontrar a tabela de syscalls
static unsigned long *find_sys_call_table(void) {
    unsigned long *sct;
    
    // Tentar encontrar via kallsyms
    sct = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    if (sct) {
        return sct;
    }
    
    // Método alternativo: buscar na memória
    // Este é um método mais avançado que pode ser necessário em alguns kernels
    return NULL;
}

// Função para desabilitar proteção de escrita
static inline void disable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    write_cr0(cr0);
}

// Função para habilitar proteção de escrita
static inline void enable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    write_cr0(cr0);
}

// Função para instalar hooks
static int install_hooks(void) {
    if (!sys_call_table) {
        printk(KERN_ERR "BarrierLayer: sys_call_table not found\n");
        return -1;
    }
    
    // Salvar syscalls originais
    original_sys_open = (void *)sys_call_table[__NR_open];
    original_sys_openat = (void *)sys_call_table[__NR_openat];
    original_sys_read = (void *)sys_call_table[__NR_read];
    original_sys_write = (void *)sys_call_table[__NR_write];
    original_sys_close = (void *)sys_call_table[__NR_close];
    original_sys_execve = (void *)sys_call_table[__NR_execve];
    original_sys_mmap = (void *)sys_call_table[__NR_mmap];
    original_sys_ptrace = (void *)sys_call_table[__NR_ptrace];
    
    // Desabilitar proteção de escrita
    disable_write_protection();
    
    // Instalar hooks
    sys_call_table[__NR_open] = (unsigned long)hooked_sys_open;
    sys_call_table[__NR_openat] = (unsigned long)hooked_sys_openat;
    sys_call_table[__NR_read] = (unsigned long)hooked_sys_read;
    sys_call_table[__NR_write] = (unsigned long)hooked_sys_write;
    sys_call_table[__NR_execve] = (unsigned long)hooked_sys_execve;
    sys_call_table[__NR_mmap] = (unsigned long)hooked_sys_mmap;
    sys_call_table[__NR_ptrace] = (unsigned long)hooked_sys_ptrace;
    
    // Reabilitar proteção de escrita
    enable_write_protection();
    
    printk(KERN_INFO "BarrierLayer: Kernel hooks installed successfully\n");
    return 0;
}

// Função para remover hooks
static void remove_hooks(void) {
    if (!sys_call_table) {
        return;
    }
    
    // Desabilitar proteção de escrita
    disable_write_protection();
    
    // Restaurar syscalls originais
    sys_call_table[__NR_open] = (unsigned long)original_sys_open;
    sys_call_table[__NR_openat] = (unsigned long)original_sys_openat;
    sys_call_table[__NR_read] = (unsigned long)original_sys_read;
    sys_call_table[__NR_write] = (unsigned long)original_sys_write;
    sys_call_table[__NR_execve] = (unsigned long)original_sys_execve;
    sys_call_table[__NR_mmap] = (unsigned long)original_sys_mmap;
    sys_call_table[__NR_ptrace] = (unsigned long)original_sys_ptrace;
    
    // Reabilitar proteção de escrita
    enable_write_protection();
    
    printk(KERN_INFO "BarrierLayer: Kernel hooks removed successfully\n");
}

// Função para mostrar logs via /proc
static int barrierlayer_proc_show(struct seq_file *m, void *v) {
    struct barrierlayer_log_entry *entry;
    unsigned long flags;
    int count = 0;
    
    seq_printf(m, "BarrierLayer Kernel Mode - Detailed Activity Log\n");
    seq_printf(m, "==============================================\n");
    seq_printf(m, "Total entries: %d\n\n", atomic_read(&log_count));
    
    spin_lock_irqsave(&log_lock, flags);
    list_for_each_entry(entry, &log_list, list) {
        seq_printf(m, "[%lu.%09lu] PID:%d TGID:%d COMM:%s\n",
                  entry->timestamp.tv_sec, entry->timestamp.tv_nsec,
                  entry->pid, entry->tgid, entry->comm);
        seq_printf(m, "  SYSCALL: %s (nr=%lu) RET=%ld\n",
                  entry->syscall_name, entry->syscall_nr, entry->ret_value);
        seq_printf(m, "  ARGS: [0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx]\n",
                  entry->args[0], entry->args[1], entry->args[2],
                  entry->args[3], entry->args[4], entry->args[5]);
        seq_printf(m, "  DETAILS: %s\n\n", entry->details);
        
        if (++count > 100) { // Limitar output para evitar overflow
            seq_printf(m, "... (showing first 100 entries)\n");
            break;
        }
    }
    spin_unlock_irqrestore(&log_lock, flags);
    
    return 0;
}

static int barrierlayer_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, barrierlayer_proc_show, NULL);
}

static const struct proc_ops barrierlayer_proc_ops = {
    .proc_open = barrierlayer_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// Função de inicialização do módulo
static int __init barrierlayer_init(void) {
    printk(KERN_INFO "BarrierLayer: Initializing kernel mode anti-cheat bypass\n");
    
    // Inicializar estruturas de dados
    INIT_LIST_HEAD(&log_list);
    spin_lock_init(&log_lock);
    
    // Encontrar tabela de syscalls
    sys_call_table = find_sys_call_table();
    if (!sys_call_table) {
        printk(KERN_ERR "BarrierLayer: Failed to find sys_call_table\n");
        return -ENOENT;
    }
    
    table_found = true;
    printk(KERN_INFO "BarrierLayer: sys_call_table found at 0x%p\n", sys_call_table);
    
    // Instalar hooks
    if (install_hooks() != 0) {
        printk(KERN_ERR "BarrierLayer: Failed to install hooks\n");
        return -EFAULT;
    }
    
    // Criar entrada no /proc
    proc_entry = proc_create(BARRIERLAYER_PROC_NAME, 0444, NULL, &barrierlayer_proc_ops);
    if (!proc_entry) {
        printk(KERN_ERR "BarrierLayer: Failed to create /proc entry\n");
        remove_hooks();
        return -ENOMEM;
    }
    
    printk(KERN_INFO "BarrierLayer: Kernel module loaded successfully\n");
    printk(KERN_INFO "BarrierLayer: Logs available at /proc/%s\n", BARRIERLAYER_PROC_NAME);
    
    return 0;
}

// Função de limpeza do módulo
static void __exit barrierlayer_exit(void) {
    struct barrierlayer_log_entry *entry, *tmp;
    unsigned long flags;
    
    printk(KERN_INFO "BarrierLayer: Unloading kernel module\n");
    
    // Remover entrada do /proc
    if (proc_entry) {
        proc_remove(proc_entry);
    }
    
    // Remover hooks
    remove_hooks();
    
    // Limpar lista de logs
    spin_lock_irqsave(&log_lock, flags);
    list_for_each_entry_safe(entry, tmp, &log_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }
    spin_unlock_irqrestore(&log_lock, flags);
    
    printk(KERN_INFO "BarrierLayer: Kernel module unloaded successfully\n");
>>>>>>> a909be7df856e5d04815b7b49ee1cc853f80a638
}

module_init(barrierlayer_init);
module_exit(barrierlayer_exit);
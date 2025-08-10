#ifndef BARRIERLAYER_H
#define BARRIERLAYER_H

// Inclui os cabeçalhos necessários para a definição de IOCTL
#include <linux/ioctl.h>

// --- IOCTL Definitions for BarrierLayer Kernel Module ---

// O 'magic number' é um número único usado para identificar os comandos IOCTL
// do nosso driver, evitando conflitos com outros drivers.
#define BARRIERLAYER_IOCTL_MAGIC 'k'

// Estrutura para passar dados para os comandos IOCTL.
// Por enquanto, apenas o PID é necessário.
// Usar uma struct torna a adição de novos parâmetros mais fácil no futuro.
typedef struct {
    int pid;
} barrierlayer_ioctl_data;

// Definição dos comandos IOCTL:
// _IOW = IOCTL com dados escritos (Write) do user-space para o kernel-space.

// Comando para instruir o kernel a esconder um processo com um PID específico.
#define BL_IOCTL_HIDE_PID _IOW(BARRIERLAYER_IOCTL_MAGIC, 1, barrierlayer_ioctl_data)

// Comando para instruir o kernel a parar de esconder um processo.
#define BL_IOCTL_UNHIDE_PID _IOW(BARRIERLAYER_IOCTL_MAGIC, 2, barrierlayer_ioctl_data)

// --- Device Configuration ---

// Nome do dispositivo que será criado em /dev/ para comunicação.
#define DEVICE_NAME "barrierlayer_ctl"

#endif // BARRIERLAYER_H
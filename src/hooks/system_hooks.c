#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

// Exemplo de uso em um hook:
// logger_log(LOG_PATH, "Interceptando NtQuerySystemInformation");

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h> // Para NULL
#include "logger.h"
#include <stdio.h>

// Definição (simplificada) das classes de informação do sistema
#define SystemProcessInformation 5
#define SystemModuleInformation 11

// --- NtQuerySystemInformation ---
typedef unsigned long (*NtQuerySystemInformation_f)(unsigned int, void*, unsigned long, unsigned long*);
static NtQuerySystemInformation_f real_NtQuerySystemInformation = NULL;

// Hook para NtQuerySystemInformation
unsigned long NtQuerySystemInformation(unsigned int SystemInformationClass, void* SystemInformation, unsigned long SystemInformationLength, unsigned long* ReturnLength) {
    if (!real_NtQuerySystemInformation) {
        real_NtQuerySystemInformation = dlsym(RTLD_NEXT, "NtQuerySystemInformation");
    }
    const char* class_name = "Classe Desconhecida";
    switch (SystemInformationClass) {
        case SystemProcessInformation: class_name = "SystemProcessInformation (5)"; break;
        case SystemModuleInformation:  class_name = "SystemModuleInformation (11)"; break;
        // Adicione outros cases conforme necessário
    }
    char msg[160];
    snprintf(msg, sizeof(msg), "HOOK: NtQuerySystemInformation | Classe: %s (%u)", class_name, SystemInformationClass);
    logger_log(LOG_PATH, msg);
    if (real_NtQuerySystemInformation) {
        return real_NtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
    } else {
        return 0xC0000002; // Retorna STATUS_NOT_IMPLEMENTED
    }
}

#define _GNU_SOURCE
#include <stdio.h>
#include <wchar.h>
#include <dlfcn.h>
#include <unistd.h> // Para getpid()

// --- Definições de Ponteiros de Função ---
typedef void* (*CreateFileW_f)(const wchar_t*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
typedef void* (*OpenProcess_f)(unsigned long, int, unsigned long);
typedef long (*RegOpenKeyExW_f)(void*, const wchar_t*, unsigned long, unsigned long, void**);
typedef long (*RegQueryValueExW_f)(void*, const wchar_t*, unsigned long*, unsigned long*, void*, unsigned long*);
typedef unsigned long (*NtQuerySystemInformation_f)(unsigned int, void*, unsigned long, unsigned long*);

// --- Função para carregar e testar um hook ---
void test_hook(void* handle, const char* func_name) {
    void* func = dlsym(handle, func_name);
    printf("\n--- Testando %s ---\n", func_name);
    if (func) {
        if (strcmp(func_name, "CreateFileW") == 0) {
            ((CreateFileW_f)func)(L"C:\\Windows\\System32\\drivers\\beacpi.sys", 0, 0, NULL, 0, 0, NULL);
        } else if (strcmp(func_name, "OpenProcess") == 0) {
            ((OpenProcess_f)func)(0x1F0FFF, 0, getpid());
        } else if (strcmp(func_name, "RegOpenKeyExW") == 0) {
            ((RegOpenKeyExW_f)func)(NULL, L"HARDWARE\\DESCRIPTION\\System", 0, 0, NULL);
        } else if (strcmp(func_name, "RegQueryValueExW") == 0) {
            ((RegQueryValueExW_f)func)(NULL, L"SystemBiosVersion", NULL, NULL, NULL, NULL);
        } else if (strcmp(func_name, "NtQuerySystemInformation") == 0) {
            ((NtQuerySystemInformation_f)func)(5, NULL, 0, NULL); // SystemProcessInformation
        }
        printf("Chamada para %s concluída.\n", func_name);
    } else {
        fprintf(stderr, "Aviso: Não foi possível encontrar %s via dlsym.\n", func_name);
    }
}

int main() {
    printf("Test Runner: Iniciando bateria de testes de hooks...\n");
    void* handle = RTLD_NEXT;

    test_hook(handle, "CreateFileW");
    test_hook(handle, "OpenProcess");
    test_hook(handle, "RegOpenKeyExW");
    test_hook(handle, "RegQueryValueExW");
    test_hook(handle, "NtQuerySystemInformation");

    printf("\nTest Runner: Todos os testes foram executados.\n");
    return 0;
}
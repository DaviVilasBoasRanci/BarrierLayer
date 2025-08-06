#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"
#include <stdint.h>
// Ofuscação de string para logs stealth
static void logx(const char* tag, void* a, void* b) {
    char buf[128];
    snprintf(buf, sizeof(buf), "X%p-%p", a, b);
    logger_log(LOG_PATH, buf);
}

// Exemplo de uso em um hook:
// logger_log(LOG_PATH, "Interceptando OpenProcess");

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <wchar.h>
#include "logger.h"

// Ponteiro para a função original OpenProcess
static void* (*real_OpenProcess)(unsigned long, int, unsigned long) = NULL;
// Ponteiro para a função original CreateProcessW
static int (*real_CreateProcessW)(const wchar_t*, const wchar_t*, void*, void*, int, unsigned long, void*, const wchar_t*, void*, void*) = NULL;

// Hook para OpenProcess
void* OpenProcess(unsigned long dwDesiredAccess, int bInheritHandle, unsigned long dwProcessId) {
    if (!real_OpenProcess) {
        real_OpenProcess = dlsym(RTLD_NEXT, "OpenProcess");
    }
    logx("A", (void*)dwProcessId, 0);
    if (real_OpenProcess) {
        return real_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    } else {
        return NULL;
    }
}

// Hook para CreateProcessW
int CreateProcessW(const wchar_t* lpApplicationName, const wchar_t* lpCommandLine, void* lpProcessAttributes, void* lpThreadAttributes, int bInheritHandles, unsigned long dwCreationFlags, void* lpEnvironment, const wchar_t* lpCurrentDirectory, void* lpStartupInfo, void* lpProcessInformation) {
    if (!real_CreateProcessW) {
        real_CreateProcessW = dlsym(RTLD_NEXT, "CreateProcessW");
    }
    char appName[260] = {0};
    if (lpApplicationName) wcstombs(appName, lpApplicationName, sizeof(appName) - 1);
    logx("B", (void*)lpApplicationName, (void*)lpCommandLine);
    if (real_CreateProcessW) {
        return real_CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    } else {
        return 0;
    }
}

// Ponteiro para a função original TerminateProcess
static int (*real_TerminateProcess)(void*, unsigned int) = NULL;
// Hook para TerminateProcess
int TerminateProcess(void* hProcess, unsigned int uExitCode) {
    if (!real_TerminateProcess) {
        real_TerminateProcess = dlsym(RTLD_NEXT, "TerminateProcess");
    }
    logx("C", hProcess, (void*)(uintptr_t)uExitCode);
    if (real_TerminateProcess) {
        return real_TerminateProcess(hProcess, uExitCode);
    } else {
        return 0;
    }
}

// Ponteiro para a função original VirtualAllocEx
static void* (*real_VirtualAllocEx)(void*, void*, size_t, unsigned long, unsigned long) = NULL;
// Hook para VirtualAllocEx
void* VirtualAllocEx(void* hProcess, void* lpAddress, size_t dwSize, unsigned long flAllocationType, unsigned long flProtect) {
    if (!real_VirtualAllocEx) {
        real_VirtualAllocEx = dlsym(RTLD_NEXT, "VirtualAllocEx");
    }
    logx("D", hProcess, (void*)dwSize);
    if (real_VirtualAllocEx) {
        return real_VirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
    } else {
        return NULL;
    }
}

// Ponteiro para a função original ReadProcessMemory
static int (*real_ReadProcessMemory)(void*, const void*, void*, size_t, size_t*) = NULL;
// Hook para ReadProcessMemory
int ReadProcessMemory(void* hProcess, const void* lpBaseAddress, void* lpBuffer, size_t nSize, size_t* lpNumberOfBytesRead) {
    if (!real_ReadProcessMemory) {
        real_ReadProcessMemory = dlsym(RTLD_NEXT, "ReadProcessMemory");
    }
    logx("E", hProcess, (void*)lpBaseAddress);
    if (real_ReadProcessMemory) {
        return real_ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
    } else {
        return 0;
    }
}

// Ponteiro para a função original CreateRemoteThread
static void* (*real_CreateRemoteThread)(void*, void*, size_t, void*, void*, unsigned long, unsigned long*) = NULL;
// Hook para CreateRemoteThread
void* CreateRemoteThread(void* hProcess, void* lpThreadAttributes, size_t dwStackSize, void* lpStartAddress, void* lpParameter, unsigned long dwCreationFlags, unsigned long* lpThreadId) {
    if (!real_CreateRemoteThread) {
        real_CreateRemoteThread = dlsym(RTLD_NEXT, "CreateRemoteThread");
    }
    logx("F", hProcess, lpStartAddress);
    if (real_CreateRemoteThread) {
        return real_CreateRemoteThread(hProcess, lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
    } else {
        return NULL;
    }
}

// Ponteiro para a função original GetCurrentProcessId
static unsigned long (*real_GetCurrentProcessId)(void) = NULL;
// Hook para GetCurrentProcessId
unsigned long GetCurrentProcessId(void) {
    if (!real_GetCurrentProcessId) {
        real_GetCurrentProcessId = dlsym(RTLD_NEXT, "GetCurrentProcessId");
    }
    logx("G", 0, 0);
    if (real_GetCurrentProcessId) {
        return real_GetCurrentProcessId();
    } else {
        return 0;
    }
}

// Ponteiro para a função original WriteProcessMemory
static int (*real_WriteProcessMemory)(void*, void*, const void*, size_t, size_t*) = NULL;
// Hook para WriteProcessMemory
int WriteProcessMemory(void* hProcess, void* lpBaseAddress, const void* lpBuffer, size_t nSize, size_t* lpNumberOfBytesWritten) {
    if (!real_WriteProcessMemory) {
        real_WriteProcessMemory = dlsym(RTLD_NEXT, "WriteProcessMemory");
    }
    logx("H", hProcess, lpBaseAddress);
    if (real_WriteProcessMemory) {
        return real_WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
    } else {
        return 0;
    }
}

// Ponteiro para a função original SuspendThread
static int (*real_SuspendThread)(void*) = NULL;
// Hook para SuspendThread
int SuspendThread(void* hThread) {
    if (!real_SuspendThread) {
        real_SuspendThread = dlsym(RTLD_NEXT, "SuspendThread");
    }
    logx("I", hThread, 0);
    if (real_SuspendThread) {
        return real_SuspendThread(hThread);
    } else {
        return -1;
    }
}

// Ponteiro para a função original ResumeThread
static int (*real_ResumeThread)(void*) = NULL;
// Hook para ResumeThread
int ResumeThread(void* hThread) {
    if (!real_ResumeThread) {
        real_ResumeThread = dlsym(RTLD_NEXT, "ResumeThread");
    }
    logx("J", hThread, 0);
    if (real_ResumeThread) {
        return real_ResumeThread(hThread);
    } else {
        return -1;
    }
}

// Ponteiro para a função original GetThreadContext
static int (*real_GetThreadContext)(void*, void*) = NULL;
// Hook para GetThreadContext
int GetThreadContext(void* hThread, void* lpContext) {
    if (!real_GetThreadContext) {
        real_GetThreadContext = dlsym(RTLD_NEXT, "GetThreadContext");
    }
    logx("K", hThread, 0);
    if (real_GetThreadContext) {
        return real_GetThreadContext(hThread, lpContext);
    } else {
        return 0;
    }
}

// Ponteiro para a função original SetThreadContext
static int (*real_SetThreadContext)(void*, const void*) = NULL;
// Hook para SetThreadContext
int SetThreadContext(void* hThread, const void* lpContext) {
    if (!real_SetThreadContext) {
        real_SetThreadContext = dlsym(RTLD_NEXT, "SetThreadContext");
    }
    logx("L", hThread, 0);
    if (real_SetThreadContext) {
        return real_SetThreadContext(hThread, lpContext);
    } else {
        return 0;
    }
}

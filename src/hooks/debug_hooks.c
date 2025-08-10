#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>
#include "logger.h"

// Ofuscação de logs para debugging
static void logdbg(const char* func, void* param1, uint32_t param2) {
    char buf[128];
    snprintf(buf, sizeof(buf), "DBG:%s|%p:%u", func, param1, param2);
    logger_log(LOG_PATH, buf);
}

// --- IsDebuggerPresent ---
static int (*real_IsDebuggerPresent)(void) = NULL;
int IsDebuggerPresent(void) {
    if (!real_IsDebuggerPresent) {
        real_IsDebuggerPresent = dlsym(RTLD_NEXT, "IsDebuggerPresent");
    }
    logdbg("IsDebuggerPresent", 0, 0);
    // Sempre retorna FALSE para evitar detecção de debugger
    return 0;
}

// --- CheckRemoteDebuggerPresent ---
static int (*real_CheckRemoteDebuggerPresent)(void*, int*) = NULL;
int CheckRemoteDebuggerPresent(void* hProcess, int* pbDebuggerPresent) {
    if (!real_CheckRemoteDebuggerPresent) {
        real_CheckRemoteDebuggerPresent = dlsym(RTLD_NEXT, "CheckRemoteDebuggerPresent");
    }
    logdbg("CheckRemoteDebuggerPresent", hProcess, 0);
    if (pbDebuggerPresent) *pbDebuggerPresent = 0; // FALSE
    return 1; // TRUE (success)
}

// --- OutputDebugStringA ---
static void (*real_OutputDebugStringA)(const char*) = NULL;
void OutputDebugStringA(const char* lpOutputString) {
    if (!real_OutputDebugStringA) {
        real_OutputDebugStringA = dlsym(RTLD_NEXT, "OutputDebugStringA");
    }
    logdbg("OutputDebugStringA", (void*)lpOutputString, 0);
    if (real_OutputDebugStringA) {
        real_OutputDebugStringA(lpOutputString);
    }
}

// --- OutputDebugStringW ---
static void (*real_OutputDebugStringW)(const wchar_t*) = NULL;
void OutputDebugStringW(const wchar_t* lpOutputString) {
    if (!real_OutputDebugStringW) {
        real_OutputDebugStringW = dlsym(RTLD_NEXT, "OutputDebugStringW");
    }
    logdbg("OutputDebugStringW", (void*)lpOutputString, 0);
    if (real_OutputDebugStringW) {
        real_OutputDebugStringW(lpOutputString);
    }
}

// --- DebugBreak ---
static void (*real_DebugBreak)(void) = NULL;
void DebugBreak(void) {
    if (!real_DebugBreak) {
        real_DebugBreak = dlsym(RTLD_NEXT, "DebugBreak");
    }
    logdbg("DebugBreak", 0, 0);
    // Não executa o breakpoint para evitar parada do programa
}

// --- NtQueryInformationProcess (para detecção de debugger) ---
static uint32_t (*real_NtQueryInformationProcess_dbg)(void*, uint32_t, void*, uint32_t, uint32_t*) = NULL;
uint32_t NtQueryInformationProcess_dbg(void* ProcessHandle, uint32_t ProcessInformationClass, void* ProcessInformation, uint32_t ProcessInformationLength, uint32_t* ReturnLength) {
    if (!real_NtQueryInformationProcess_dbg) {
        real_NtQueryInformationProcess_dbg = dlsym(RTLD_NEXT, "NtQueryInformationProcess");
    }
    logdbg("NtQueryInformationProcess_dbg", ProcessHandle, ProcessInformationClass);
    
    // Interceptar classes específicas usadas para detecção de debugger
    switch (ProcessInformationClass) {
        case 7: // ProcessDebugPort
            if (ProcessInformation && ProcessInformationLength >= sizeof(void*)) {
                *(void**)ProcessInformation = NULL; // Sem debugger
                if (ReturnLength) *ReturnLength = sizeof(void*);
                return 0; // STATUS_SUCCESS
            }
            break;
        case 30: // ProcessDebugFlags
            if (ProcessInformation && ProcessInformationLength >= sizeof(uint32_t)) {
                *(uint32_t*)ProcessInformation = 1; // PDEBUG_FLAGS_DONT_DEBUG
                if (ReturnLength) *ReturnLength = sizeof(uint32_t);
                return 0;
            }
            break;
        case 31: // ProcessDebugObjectHandle
            if (ProcessInformation && ProcessInformationLength >= sizeof(void*)) {
                *(void**)ProcessInformation = NULL; // Sem debug object
                if (ReturnLength) *ReturnLength = sizeof(void*);
                return 0;
            }
            break;
    }
    
    if (real_NtQueryInformationProcess_dbg) {
        return real_NtQueryInformationProcess_dbg(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
    }
    return 0xC0000001; // STATUS_UNSUCCESSFUL
}

// --- NtSetInformationThread (para anti-debugging) ---
static uint32_t (*real_NtSetInformationThread)(void*, uint32_t, void*, uint32_t) = NULL;
uint32_t NtSetInformationThread(void* ThreadHandle, uint32_t ThreadInformationClass, void* ThreadInformation, uint32_t ThreadInformationLength) {
    if (!real_NtSetInformationThread) {
        real_NtSetInformationThread = dlsym(RTLD_NEXT, "NtSetInformationThread");
    }
    logdbg("NtSetInformationThread", ThreadHandle, ThreadInformationClass);
    
    // Interceptar ThreadHideFromDebugger (17)
    if (ThreadInformationClass == 17) {
        // Simular sucesso mas não aplicar o hide
        return 0; // STATUS_SUCCESS
    }
    
    if (real_NtSetInformationThread) {
        return real_NtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
    }
    return 0;
}

// --- GetTickCount ---
static uint32_t (*real_GetTickCount)(void) = NULL;
uint32_t GetTickCount(void) {
    if (!real_GetTickCount) {
        real_GetTickCount = dlsym(RTLD_NEXT, "GetTickCount");
    }
    logdbg("GetTickCount", 0, 0);
    if (real_GetTickCount) {
        return real_GetTickCount();
    }
    return 0;
}

// --- GetTickCount64 ---
static uint64_t (*real_GetTickCount64)(void) = NULL;
uint64_t GetTickCount64(void) {
    if (!real_GetTickCount64) {
        real_GetTickCount64 = dlsym(RTLD_NEXT, "GetTickCount64");
    }
    logdbg("GetTickCount64", 0, 0);
    if (real_GetTickCount64) {
        return real_GetTickCount64();
    }
    return 0;
}

// --- QueryPerformanceCounter ---
static int (*real_QueryPerformanceCounter)(int64_t*) = NULL;
int QueryPerformanceCounter(int64_t* lpPerformanceCount) {
    if (!real_QueryPerformanceCounter) {
        real_QueryPerformanceCounter = dlsym(RTLD_NEXT, "QueryPerformanceCounter");
    }
    logdbg("QueryPerformanceCounter", lpPerformanceCount, 0);
    if (real_QueryPerformanceCounter) {
        return real_QueryPerformanceCounter(lpPerformanceCount);
    }
    return 0;
}

// --- QueryPerformanceFrequency ---
static int (*real_QueryPerformanceFrequency)(int64_t*) = NULL;
int QueryPerformanceFrequency(int64_t* lpFrequency) {
    if (!real_QueryPerformanceFrequency) {
        real_QueryPerformanceFrequency = dlsym(RTLD_NEXT, "QueryPerformanceFrequency");
    }
    logdbg("QueryPerformanceFrequency", lpFrequency, 0);
    if (real_QueryPerformanceFrequency) {
        return real_QueryPerformanceFrequency(lpFrequency);
    }
    return 0;
}

// --- timeGetTime ---
static uint32_t (*real_timeGetTime)(void) = NULL;
uint32_t timeGetTime(void) {
    if (!real_timeGetTime) {
        real_timeGetTime = dlsym(RTLD_NEXT, "timeGetTime");
    }
    logdbg("timeGetTime", 0, 0);
    if (real_timeGetTime) {
        return real_timeGetTime();
    }
    return 0;
}

// --- SetUnhandledExceptionFilter ---
static void* (*real_SetUnhandledExceptionFilter)(void*) = NULL;
void* SetUnhandledExceptionFilter(void* lpTopLevelExceptionFilter) {
    if (!real_SetUnhandledExceptionFilter) {
        real_SetUnhandledExceptionFilter = dlsym(RTLD_NEXT, "SetUnhandledExceptionFilter");
    }
    logdbg("SetUnhandledExceptionFilter", lpTopLevelExceptionFilter, 0);
    if (real_SetUnhandledExceptionFilter) {
        return real_SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
    }
    return NULL;
}

// --- UnhandledExceptionFilter ---
static int32_t (*real_UnhandledExceptionFilter)(void*) = NULL;
int32_t UnhandledExceptionFilter(void* ExceptionInfo) {
    if (!real_UnhandledExceptionFilter) {
        real_UnhandledExceptionFilter = dlsym(RTLD_NEXT, "UnhandledExceptionFilter");
    }
    logdbg("UnhandledExceptionFilter", ExceptionInfo, 0);
    if (real_UnhandledExceptionFilter) {
        return real_UnhandledExceptionFilter(ExceptionInfo);
    }
    return 0; // EXCEPTION_CONTINUE_SEARCH
}

// --- RaiseException ---
static void (*real_RaiseException)(uint32_t, uint32_t, uint32_t, const uintptr_t*) = NULL;
void RaiseException(uint32_t dwExceptionCode, uint32_t dwExceptionFlags, uint32_t nNumberOfArguments, const uintptr_t* lpArguments) {
    if (!real_RaiseException) {
        real_RaiseException = dlsym(RTLD_NEXT, "RaiseException");
    }
    logdbg("RaiseException", (void*)(uintptr_t)dwExceptionCode, dwExceptionFlags);
    if (real_RaiseException) {
        real_RaiseException(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments);
    }
}

// --- FindWindowA ---
static void* (*real_FindWindowA)(const char*, const char*) = NULL;
void* FindWindowA(const char* lpClassName, const char* lpWindowName) {
    if (!real_FindWindowA) {
        real_FindWindowA = dlsym(RTLD_NEXT, "FindWindowA");
    }
    logdbg("FindWindowA", (void*)lpClassName, 0);
    
    // Interceptar janelas de debuggers conhecidos
    if (lpClassName) {
        if (strstr(lpClassName, "OLLYDBG") || 
            strstr(lpClassName, "WinDbgFrameClass") ||
            strstr(lpClassName, "ID") ||
            strstr(lpClassName, "Zeta Debugger")) {
            return NULL; // Não encontrado
        }
    }
    
    if (real_FindWindowA) {
        return real_FindWindowA(lpClassName, lpWindowName);
    }
    return NULL;
}

// --- FindWindowW ---
static void* (*real_FindWindowW)(const wchar_t*, const wchar_t*) = NULL;
void* FindWindowW(const wchar_t* lpClassName, const wchar_t* lpWindowName) {
    if (!real_FindWindowW) {
        real_FindWindowW = dlsym(RTLD_NEXT, "FindWindowW");
    }
    logdbg("FindWindowW", (void*)lpClassName, 0);
    
    // Interceptar janelas de debuggers conhecidos
    if (lpClassName) {
        wchar_t debugger_classes[][32] = {
            L"OLLYDBG", L"WinDbgFrameClass", L"ID", L"Zeta Debugger"
        };
        for (int i = 0; i < 4; i++) {
            if (wcsstr(lpClassName, debugger_classes[i])) {
                return NULL; // Não encontrado
            }
        }
    }
    
    if (real_FindWindowW) {
        return real_FindWindowW(lpClassName, lpWindowName);
    }
    return NULL;
}

// --- GetWindowTextA ---
static int (*real_GetWindowTextA)(void*, char*, int) = NULL;
int GetWindowTextA(void* hWnd, char* lpString, int nMaxCount) {
    if (!real_GetWindowTextA) {
        real_GetWindowTextA = dlsym(RTLD_NEXT, "GetWindowTextA");
    }
    logdbg("GetWindowTextA", hWnd, nMaxCount);
    if (real_GetWindowTextA) {
        return real_GetWindowTextA(hWnd, lpString, nMaxCount);
    }
    return 0;
}

// --- EnumWindows ---
static int (*real_EnumWindows)(void*, intptr_t) = NULL;
int EnumWindows(void* lpEnumFunc, intptr_t lParam) {
    if (!real_EnumWindows) {
        real_EnumWindows = dlsym(RTLD_NEXT, "EnumWindows");
    }
    logdbg("EnumWindows", lpEnumFunc, 0);
    if (real_EnumWindows) {
        return real_EnumWindows(lpEnumFunc, lParam);
    }
    return 0;
}
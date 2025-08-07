#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include "logger.h"

// Ofuscação de logs para threading
static void logthread(const char* func, void* handle, uint32_t id) {
    char buf[128];
    snprintf(buf, sizeof(buf), "THR:%s|%p:%u", func, handle, id);
    logger_log(LOG_PATH, buf);
}

// --- CreateThread ---
static void* (*real_CreateThread)(void*, size_t, void*, void*, uint32_t, uint32_t*) = NULL;
void* CreateThread(void* lpThreadAttributes, size_t dwStackSize, void* lpStartAddress, void* lpParameter, uint32_t dwCreationFlags, uint32_t* lpThreadId) {
    if (!real_CreateThread) {
        real_CreateThread = dlsym(RTLD_NEXT, "CreateThread");
    }
    logthread("CreateThread", lpStartAddress, dwCreationFlags);
    if (real_CreateThread) {
        return real_CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
    }
    return NULL;
}

// --- ExitThread ---
static void (*real_ExitThread)(uint32_t) = NULL;
void ExitThread(uint32_t dwExitCode) {
    if (!real_ExitThread) {
        real_ExitThread = dlsym(RTLD_NEXT, "ExitThread");
    }
    logthread("ExitThread", 0, dwExitCode);
    if (real_ExitThread) {
        real_ExitThread(dwExitCode);
    }
}

// --- TerminateThread ---
static int (*real_TerminateThread)(void*, uint32_t) = NULL;
int TerminateThread(void* hThread, uint32_t dwExitCode) {
    if (!real_TerminateThread) {
        real_TerminateThread = dlsym(RTLD_NEXT, "TerminateThread");
    }
    logthread("TerminateThread", hThread, dwExitCode);
    if (real_TerminateThread) {
        return real_TerminateThread(hThread, dwExitCode);
    }
    return 0;
}

// --- GetCurrentThread ---
static void* (*real_GetCurrentThread)(void) = NULL;
void* GetCurrentThread(void) {
    if (!real_GetCurrentThread) {
        real_GetCurrentThread = dlsym(RTLD_NEXT, "GetCurrentThread");
    }
    logthread("GetCurrentThread", 0, 0);
    if (real_GetCurrentThread) {
        return real_GetCurrentThread();
    }
    return NULL;
}

// --- GetCurrentThreadId ---
static uint32_t (*real_GetCurrentThreadId)(void) = NULL;
uint32_t GetCurrentThreadId(void) {
    if (!real_GetCurrentThreadId) {
        real_GetCurrentThreadId = dlsym(RTLD_NEXT, "GetCurrentThreadId");
    }
    logthread("GetCurrentThreadId", 0, 0);
    if (real_GetCurrentThreadId) {
        return real_GetCurrentThreadId();
    }
    return 0;
}

// --- GetThreadId ---
static uint32_t (*real_GetThreadId)(void*) = NULL;
uint32_t GetThreadId(void* Thread) {
    if (!real_GetThreadId) {
        real_GetThreadId = dlsym(RTLD_NEXT, "GetThreadId");
    }
    logthread("GetThreadId", Thread, 0);
    if (real_GetThreadId) {
        return real_GetThreadId(Thread);
    }
    return 0;
}

// --- OpenThread ---
static void* (*real_OpenThread)(uint32_t, int, uint32_t) = NULL;
void* OpenThread(uint32_t dwDesiredAccess, int bInheritHandle, uint32_t dwThreadId) {
    if (!real_OpenThread) {
        real_OpenThread = dlsym(RTLD_NEXT, "OpenThread");
    }
    logthread("OpenThread", (void*)(uintptr_t)dwDesiredAccess, dwThreadId);
    if (real_OpenThread) {
        return real_OpenThread(dwDesiredAccess, bInheritHandle, dwThreadId);
    }
    return NULL;
}

// --- WaitForSingleObject ---
static uint32_t (*real_WaitForSingleObject)(void*, uint32_t) = NULL;
uint32_t WaitForSingleObject(void* hHandle, uint32_t dwMilliseconds) {
    if (!real_WaitForSingleObject) {
        real_WaitForSingleObject = dlsym(RTLD_NEXT, "WaitForSingleObject");
    }
    logthread("WaitForSingleObject", hHandle, dwMilliseconds);
    if (real_WaitForSingleObject) {
        return real_WaitForSingleObject(hHandle, dwMilliseconds);
    }
    return 0xFFFFFFFF; // WAIT_FAILED
}

// --- WaitForMultipleObjects ---
static uint32_t (*real_WaitForMultipleObjects)(uint32_t, const void**, int, uint32_t) = NULL;
uint32_t WaitForMultipleObjects(uint32_t nCount, const void** lpHandles, int bWaitAll, uint32_t dwMilliseconds) {
    if (!real_WaitForMultipleObjects) {
        real_WaitForMultipleObjects = dlsym(RTLD_NEXT, "WaitForMultipleObjects");
    }
    logthread("WaitForMultipleObjects", (void*)(uintptr_t)nCount, dwMilliseconds);
    if (real_WaitForMultipleObjects) {
        return real_WaitForMultipleObjects(nCount, lpHandles, bWaitAll, dwMilliseconds);
    }
    return 0xFFFFFFFF;
}

// --- CreateMutexW ---
static void* (*real_CreateMutexW)(void*, int, const wchar_t*) = NULL;
void* CreateMutexW(void* lpMutexAttributes, int bInitialOwner, const wchar_t* lpName) {
    if (!real_CreateMutexW) {
        real_CreateMutexW = dlsym(RTLD_NEXT, "CreateMutexW");
    }
    logthread("CreateMutexW", (void*)lpName, bInitialOwner);
    if (real_CreateMutexW) {
        return real_CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
    }
    return NULL;
}

// --- OpenMutexW ---
static void* (*real_OpenMutexW)(uint32_t, int, const wchar_t*) = NULL;
void* OpenMutexW(uint32_t dwDesiredAccess, int bInheritHandle, const wchar_t* lpName) {
    if (!real_OpenMutexW) {
        real_OpenMutexW = dlsym(RTLD_NEXT, "OpenMutexW");
    }
    logthread("OpenMutexW", (void*)lpName, dwDesiredAccess);
    if (real_OpenMutexW) {
        return real_OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
    }
    return NULL;
}

// --- ReleaseMutex ---
static int (*real_ReleaseMutex)(void*) = NULL;
int ReleaseMutex(void* hMutex) {
    if (!real_ReleaseMutex) {
        real_ReleaseMutex = dlsym(RTLD_NEXT, "ReleaseMutex");
    }
    logthread("ReleaseMutex", hMutex, 0);
    if (real_ReleaseMutex) {
        return real_ReleaseMutex(hMutex);
    }
    return 0;
}

// --- CreateSemaphoreW ---
static void* (*real_CreateSemaphoreW)(void*, int32_t, int32_t, const wchar_t*) = NULL;
void* CreateSemaphoreW(void* lpSemaphoreAttributes, int32_t lInitialCount, int32_t lMaximumCount, const wchar_t* lpName) {
    if (!real_CreateSemaphoreW) {
        real_CreateSemaphoreW = dlsym(RTLD_NEXT, "CreateSemaphoreW");
    }
    logthread("CreateSemaphoreW", (void*)lpName, lInitialCount);
    if (real_CreateSemaphoreW) {
        return real_CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);
    }
    return NULL;
}

// --- ReleaseSemaphore ---
static int (*real_ReleaseSemaphore)(void*, int32_t, int32_t*) = NULL;
int ReleaseSemaphore(void* hSemaphore, int32_t lReleaseCount, int32_t* lpPreviousCount) {
    if (!real_ReleaseSemaphore) {
        real_ReleaseSemaphore = dlsym(RTLD_NEXT, "ReleaseSemaphore");
    }
    logthread("ReleaseSemaphore", hSemaphore, lReleaseCount);
    if (real_ReleaseSemaphore) {
        return real_ReleaseSemaphore(hSemaphore, lReleaseCount, lpPreviousCount);
    }
    return 0;
}

// --- CreateEventW ---
static void* (*real_CreateEventW)(void*, int, int, const wchar_t*) = NULL;
void* CreateEventW(void* lpEventAttributes, int bManualReset, int bInitialState, const wchar_t* lpName) {
    if (!real_CreateEventW) {
        real_CreateEventW = dlsym(RTLD_NEXT, "CreateEventW");
    }
    logthread("CreateEventW", (void*)lpName, bManualReset);
    if (real_CreateEventW) {
        return real_CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);
    }
    return NULL;
}

// --- SetEvent ---
static int (*real_SetEvent)(void*) = NULL;
int SetEvent(void* hEvent) {
    if (!real_SetEvent) {
        real_SetEvent = dlsym(RTLD_NEXT, "SetEvent");
    }
    logthread("SetEvent", hEvent, 0);
    if (real_SetEvent) {
        return real_SetEvent(hEvent);
    }
    return 0;
}

// --- ResetEvent ---
static int (*real_ResetEvent)(void*) = NULL;
int ResetEvent(void* hEvent) {
    if (!real_ResetEvent) {
        real_ResetEvent = dlsym(RTLD_NEXT, "ResetEvent");
    }
    logthread("ResetEvent", hEvent, 0);
    if (real_ResetEvent) {
        return real_ResetEvent(hEvent);
    }
    return 0;
}

// --- PulseEvent ---
static int (*real_PulseEvent)(void*) = NULL;
int PulseEvent(void* hEvent) {
    if (!real_PulseEvent) {
        real_PulseEvent = dlsym(RTLD_NEXT, "PulseEvent");
    }
    logthread("PulseEvent", hEvent, 0);
    if (real_PulseEvent) {
        return real_PulseEvent(hEvent);
    }
    return 0;
}

// --- InitializeCriticalSection ---
static void (*real_InitializeCriticalSection)(void*) = NULL;
void InitializeCriticalSection(void* lpCriticalSection) {
    if (!real_InitializeCriticalSection) {
        real_InitializeCriticalSection = dlsym(RTLD_NEXT, "InitializeCriticalSection");
    }
    logthread("InitializeCriticalSection", lpCriticalSection, 0);
    if (real_InitializeCriticalSection) {
        real_InitializeCriticalSection(lpCriticalSection);
    }
}

// --- EnterCriticalSection ---
static void (*real_EnterCriticalSection)(void*) = NULL;
void EnterCriticalSection(void* lpCriticalSection) {
    if (!real_EnterCriticalSection) {
        real_EnterCriticalSection = dlsym(RTLD_NEXT, "EnterCriticalSection");
    }
    logthread("EnterCriticalSection", lpCriticalSection, 0);
    if (real_EnterCriticalSection) {
        real_EnterCriticalSection(lpCriticalSection);
    }
}

// --- LeaveCriticalSection ---
static void (*real_LeaveCriticalSection)(void*) = NULL;
void LeaveCriticalSection(void* lpCriticalSection) {
    if (!real_LeaveCriticalSection) {
        real_LeaveCriticalSection = dlsym(RTLD_NEXT, "LeaveCriticalSection");
    }
    logthread("LeaveCriticalSection", lpCriticalSection, 0);
    if (real_LeaveCriticalSection) {
        real_LeaveCriticalSection(lpCriticalSection);
    }
}

// --- DeleteCriticalSection ---
static void (*real_DeleteCriticalSection)(void*) = NULL;
void DeleteCriticalSection(void* lpCriticalSection) {
    if (!real_DeleteCriticalSection) {
        real_DeleteCriticalSection = dlsym(RTLD_NEXT, "DeleteCriticalSection");
    }
    logthread("DeleteCriticalSection", lpCriticalSection, 0);
    if (real_DeleteCriticalSection) {
        real_DeleteCriticalSection(lpCriticalSection);
    }
}

// --- Sleep ---
static void (*real_Sleep)(uint32_t) = NULL;
void Sleep(uint32_t dwMilliseconds) {
    if (!real_Sleep) {
        real_Sleep = dlsym(RTLD_NEXT, "Sleep");
    }
    logthread("Sleep", 0, dwMilliseconds);
    if (real_Sleep) {
        real_Sleep(dwMilliseconds);
    }
}

// --- SleepEx ---
static uint32_t (*real_SleepEx)(uint32_t, int) = NULL;
uint32_t SleepEx(uint32_t dwMilliseconds, int bAlertable) {
    if (!real_SleepEx) {
        real_SleepEx = dlsym(RTLD_NEXT, "SleepEx");
    }
    logthread("SleepEx", (void*)(uintptr_t)bAlertable, dwMilliseconds);
    if (real_SleepEx) {
        return real_SleepEx(dwMilliseconds, bAlertable);
    }
    return 0;
}
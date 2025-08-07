#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include "logger.h"

// Ofuscação de logs para memória
static void logmem(const char* func, void* addr, size_t size) {
    char buf[128];
    snprintf(buf, sizeof(buf), "MEM:%s|%p:%zu", func, addr, size);
    logger_log(LOG_PATH, buf);
}

// --- VirtualAlloc ---
static void* (*real_VirtualAlloc)(void*, size_t, uint32_t, uint32_t) = NULL;
void* VirtualAlloc(void* lpAddress, size_t dwSize, uint32_t flAllocationType, uint32_t flProtect) {
    if (!real_VirtualAlloc) {
        real_VirtualAlloc = dlsym(RTLD_NEXT, "VirtualAlloc");
    }
    logmem("VirtualAlloc", lpAddress, dwSize);
    if (real_VirtualAlloc) {
        return real_VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    }
    return NULL;
}

// --- VirtualFree ---
static int (*real_VirtualFree)(void*, size_t, uint32_t) = NULL;
int VirtualFree(void* lpAddress, size_t dwSize, uint32_t dwFreeType) {
    if (!real_VirtualFree) {
        real_VirtualFree = dlsym(RTLD_NEXT, "VirtualFree");
    }
    logmem("VirtualFree", lpAddress, dwSize);
    if (real_VirtualFree) {
        return real_VirtualFree(lpAddress, dwSize, dwFreeType);
    }
    return 0;
}

// --- VirtualProtect ---
static int (*real_VirtualProtect)(void*, size_t, uint32_t, uint32_t*) = NULL;
int VirtualProtect(void* lpAddress, size_t dwSize, uint32_t flNewProtect, uint32_t* lpflOldProtect) {
    if (!real_VirtualProtect) {
        real_VirtualProtect = dlsym(RTLD_NEXT, "VirtualProtect");
    }
    logmem("VirtualProtect", lpAddress, dwSize);
    if (real_VirtualProtect) {
        return real_VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect);
    }
    return 0;
}

// --- VirtualQuery ---
static size_t (*real_VirtualQuery)(const void*, void*, size_t) = NULL;
size_t VirtualQuery(const void* lpAddress, void* lpBuffer, size_t dwLength) {
    if (!real_VirtualQuery) {
        real_VirtualQuery = dlsym(RTLD_NEXT, "VirtualQuery");
    }
    logmem("VirtualQuery", (void*)lpAddress, dwLength);
    if (real_VirtualQuery) {
        return real_VirtualQuery(lpAddress, lpBuffer, dwLength);
    }
    return 0;
}

// --- VirtualLock ---
static int (*real_VirtualLock)(void*, size_t) = NULL;
int VirtualLock(void* lpAddress, size_t dwSize) {
    if (!real_VirtualLock) {
        real_VirtualLock = dlsym(RTLD_NEXT, "VirtualLock");
    }
    logmem("VirtualLock", lpAddress, dwSize);
    if (real_VirtualLock) {
        return real_VirtualLock(lpAddress, dwSize);
    }
    return 0;
}

// --- VirtualUnlock ---
static int (*real_VirtualUnlock)(void*, size_t) = NULL;
int VirtualUnlock(void* lpAddress, size_t dwSize) {
    if (!real_VirtualUnlock) {
        real_VirtualUnlock = dlsym(RTLD_NEXT, "VirtualUnlock");
    }
    logmem("VirtualUnlock", lpAddress, dwSize);
    if (real_VirtualUnlock) {
        return real_VirtualUnlock(lpAddress, dwSize);
    }
    return 0;
}

// --- HeapCreate ---
static void* (*real_HeapCreate)(uint32_t, size_t, size_t) = NULL;
void* HeapCreate(uint32_t flOptions, size_t dwInitialSize, size_t dwMaximumSize) {
    if (!real_HeapCreate) {
        real_HeapCreate = dlsym(RTLD_NEXT, "HeapCreate");
    }
    logmem("HeapCreate", (void*)(uintptr_t)flOptions, dwInitialSize);
    if (real_HeapCreate) {
        return real_HeapCreate(flOptions, dwInitialSize, dwMaximumSize);
    }
    return NULL;
}

// --- HeapDestroy ---
static int (*real_HeapDestroy)(void*) = NULL;
int HeapDestroy(void* hHeap) {
    if (!real_HeapDestroy) {
        real_HeapDestroy = dlsym(RTLD_NEXT, "HeapDestroy");
    }
    logmem("HeapDestroy", hHeap, 0);
    if (real_HeapDestroy) {
        return real_HeapDestroy(hHeap);
    }
    return 0;
}

// --- HeapAlloc ---
static void* (*real_HeapAlloc)(void*, uint32_t, size_t) = NULL;
void* HeapAlloc(void* hHeap, uint32_t dwFlags, size_t dwBytes) {
    if (!real_HeapAlloc) {
        real_HeapAlloc = dlsym(RTLD_NEXT, "HeapAlloc");
    }
    logmem("HeapAlloc", hHeap, dwBytes);
    if (real_HeapAlloc) {
        return real_HeapAlloc(hHeap, dwFlags, dwBytes);
    }
    return NULL;
}

// --- HeapFree ---
static int (*real_HeapFree)(void*, uint32_t, void*) = NULL;
int HeapFree(void* hHeap, uint32_t dwFlags, void* lpMem) {
    if (!real_HeapFree) {
        real_HeapFree = dlsym(RTLD_NEXT, "HeapFree");
    }
    logmem("HeapFree", hHeap, (size_t)lpMem);
    if (real_HeapFree) {
        return real_HeapFree(hHeap, dwFlags, lpMem);
    }
    return 0;
}

// --- HeapReAlloc ---
static void* (*real_HeapReAlloc)(void*, uint32_t, void*, size_t) = NULL;
void* HeapReAlloc(void* hHeap, uint32_t dwFlags, void* lpMem, size_t dwBytes) {
    if (!real_HeapReAlloc) {
        real_HeapReAlloc = dlsym(RTLD_NEXT, "HeapReAlloc");
    }
    logmem("HeapReAlloc", hHeap, dwBytes);
    if (real_HeapReAlloc) {
        return real_HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
    }
    return NULL;
}

// --- HeapSize ---
static size_t (*real_HeapSize)(void*, uint32_t, const void*) = NULL;
size_t HeapSize(void* hHeap, uint32_t dwFlags, const void* lpMem) {
    if (!real_HeapSize) {
        real_HeapSize = dlsym(RTLD_NEXT, "HeapSize");
    }
    logmem("HeapSize", hHeap, (size_t)lpMem);
    if (real_HeapSize) {
        return real_HeapSize(hHeap, dwFlags, lpMem);
    }
    return 0;
}

// --- GetProcessHeap ---
static void* (*real_GetProcessHeap)(void) = NULL;
void* GetProcessHeap(void) {
    if (!real_GetProcessHeap) {
        real_GetProcessHeap = dlsym(RTLD_NEXT, "GetProcessHeap");
    }
    logmem("GetProcessHeap", 0, 0);
    if (real_GetProcessHeap) {
        return real_GetProcessHeap();
    }
    return NULL;
}

// --- GetProcessHeaps ---
static uint32_t (*real_GetProcessHeaps)(uint32_t, void**) = NULL;
uint32_t GetProcessHeaps(uint32_t NumberOfHeaps, void** ProcessHeaps) {
    if (!real_GetProcessHeaps) {
        real_GetProcessHeaps = dlsym(RTLD_NEXT, "GetProcessHeaps");
    }
    logmem("GetProcessHeaps", (void*)(uintptr_t)NumberOfHeaps, 0);
    if (real_GetProcessHeaps) {
        return real_GetProcessHeaps(NumberOfHeaps, ProcessHeaps);
    }
    return 0;
}

// --- GlobalAlloc ---
static void* (*real_GlobalAlloc)(uint32_t, size_t) = NULL;
void* GlobalAlloc(uint32_t uFlags, size_t dwBytes) {
    if (!real_GlobalAlloc) {
        real_GlobalAlloc = dlsym(RTLD_NEXT, "GlobalAlloc");
    }
    logmem("GlobalAlloc", (void*)(uintptr_t)uFlags, dwBytes);
    if (real_GlobalAlloc) {
        return real_GlobalAlloc(uFlags, dwBytes);
    }
    return NULL;
}

// --- GlobalFree ---
static void* (*real_GlobalFree)(void*) = NULL;
void* GlobalFree(void* hMem) {
    if (!real_GlobalFree) {
        real_GlobalFree = dlsym(RTLD_NEXT, "GlobalFree");
    }
    logmem("GlobalFree", hMem, 0);
    if (real_GlobalFree) {
        return real_GlobalFree(hMem);
    }
    return hMem;
}

// --- GlobalLock ---
static void* (*real_GlobalLock)(void*) = NULL;
void* GlobalLock(void* hMem) {
    if (!real_GlobalLock) {
        real_GlobalLock = dlsym(RTLD_NEXT, "GlobalLock");
    }
    logmem("GlobalLock", hMem, 0);
    if (real_GlobalLock) {
        return real_GlobalLock(hMem);
    }
    return NULL;
}

// --- GlobalUnlock ---
static int (*real_GlobalUnlock)(void*) = NULL;
int GlobalUnlock(void* hMem) {
    if (!real_GlobalUnlock) {
        real_GlobalUnlock = dlsym(RTLD_NEXT, "GlobalUnlock");
    }
    logmem("GlobalUnlock", hMem, 0);
    if (real_GlobalUnlock) {
        return real_GlobalUnlock(hMem);
    }
    return 0;
}

// --- LocalAlloc ---
static void* (*real_LocalAlloc)(uint32_t, size_t) = NULL;
void* LocalAlloc(uint32_t uFlags, size_t uBytes) {
    if (!real_LocalAlloc) {
        real_LocalAlloc = dlsym(RTLD_NEXT, "LocalAlloc");
    }
    logmem("LocalAlloc", (void*)(uintptr_t)uFlags, uBytes);
    if (real_LocalAlloc) {
        return real_LocalAlloc(uFlags, uBytes);
    }
    return NULL;
}

// --- LocalFree ---
static void* (*real_LocalFree)(void*) = NULL;
void* LocalFree(void* hMem) {
    if (!real_LocalFree) {
        real_LocalFree = dlsym(RTLD_NEXT, "LocalFree");
    }
    logmem("LocalFree", hMem, 0);
    if (real_LocalFree) {
        return real_LocalFree(hMem);
    }
    return hMem;
}

// --- CreateFileMappingW ---
static void* (*real_CreateFileMappingW)(void*, void*, uint32_t, uint32_t, uint32_t, const wchar_t*) = NULL;
void* CreateFileMappingW(void* hFile, void* lpFileMappingAttributes, uint32_t flProtect, uint32_t dwMaximumSizeHigh, uint32_t dwMaximumSizeLow, const wchar_t* lpName) {
    if (!real_CreateFileMappingW) {
        real_CreateFileMappingW = dlsym(RTLD_NEXT, "CreateFileMappingW");
    }
    logmem("CreateFileMappingW", hFile, dwMaximumSizeLow);
    if (real_CreateFileMappingW) {
        return real_CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }
    return NULL;
}

// --- MapViewOfFile ---
static void* (*real_MapViewOfFile)(void*, uint32_t, uint32_t, uint32_t, size_t) = NULL;
void* MapViewOfFile(void* hFileMappingObject, uint32_t dwDesiredAccess, uint32_t dwFileOffsetHigh, uint32_t dwFileOffsetLow, size_t dwNumberOfBytesToMap) {
    if (!real_MapViewOfFile) {
        real_MapViewOfFile = dlsym(RTLD_NEXT, "MapViewOfFile");
    }
    logmem("MapViewOfFile", hFileMappingObject, dwNumberOfBytesToMap);
    if (real_MapViewOfFile) {
        return real_MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
    }
    return NULL;
}

// --- UnmapViewOfFile ---
static int (*real_UnmapViewOfFile)(const void*) = NULL;
int UnmapViewOfFile(const void* lpBaseAddress) {
    if (!real_UnmapViewOfFile) {
        real_UnmapViewOfFile = dlsym(RTLD_NEXT, "UnmapViewOfFile");
    }
    logmem("UnmapViewOfFile", (void*)lpBaseAddress, 0);
    if (real_UnmapViewOfFile) {
        return real_UnmapViewOfFile(lpBaseAddress);
    }
    return 0;
}
#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include "logger.h"

// Ofuscação de logs para hardware
static void loghw(const char* func, void* param1, void* param2) {
    char buf[128];
    snprintf(buf, sizeof(buf), "HW:%s|%p-%p", func, param1, param2);
    logger_log(LOG_PATH, buf);
}

// --- GetSystemInfo ---
static void (*real_GetSystemInfo)(void*) = NULL;
void GetSystemInfo(void* lpSystemInfo) {
    if (!real_GetSystemInfo) {
        real_GetSystemInfo = dlsym(RTLD_NEXT, "GetSystemInfo");
    }
    loghw("GetSystemInfo", lpSystemInfo, 0);
    if (real_GetSystemInfo) {
        real_GetSystemInfo(lpSystemInfo);
    }
}

// --- GetNativeSystemInfo ---
static void (*real_GetNativeSystemInfo)(void*) = NULL;
void GetNativeSystemInfo(void* lpSystemInfo) {
    if (!real_GetNativeSystemInfo) {
        real_GetNativeSystemInfo = dlsym(RTLD_NEXT, "GetNativeSystemInfo");
    }
    loghw("GetNativeSystemInfo", lpSystemInfo, 0);
    if (real_GetNativeSystemInfo) {
        real_GetNativeSystemInfo(lpSystemInfo);
    }
}

// --- GetComputerNameW ---
static int (*real_GetComputerNameW)(wchar_t*, uint32_t*) = NULL;
int GetComputerNameW(wchar_t* lpBuffer, uint32_t* nSize) {
    if (!real_GetComputerNameW) {
        real_GetComputerNameW = dlsym(RTLD_NEXT, "GetComputerNameW");
    }
    loghw("GetComputerNameW", lpBuffer, (void*)(uintptr_t)*nSize);
    if (real_GetComputerNameW) {
        return real_GetComputerNameW(lpBuffer, nSize);
    }
    return 0;
}

// --- GetUserNameW ---
static int (*real_GetUserNameW)(wchar_t*, uint32_t*) = NULL;
int GetUserNameW(wchar_t* lpBuffer, uint32_t* pcbBuffer) {
    if (!real_GetUserNameW) {
        real_GetUserNameW = dlsym(RTLD_NEXT, "GetUserNameW");
    }
    loghw("GetUserNameW", lpBuffer, (void*)(uintptr_t)*pcbBuffer);
    if (real_GetUserNameW) {
        return real_GetUserNameW(lpBuffer, pcbBuffer);
    }
    return 0;
}

// --- GetVolumeInformationW ---
static int (*real_GetVolumeInformationW)(const wchar_t*, wchar_t*, uint32_t, uint32_t*, uint32_t*, uint32_t*, wchar_t*, uint32_t) = NULL;
int GetVolumeInformationW(const wchar_t* lpRootPathName, wchar_t* lpVolumeNameBuffer, uint32_t nVolumeNameSize, uint32_t* lpVolumeSerialNumber, uint32_t* lpMaximumComponentLength, uint32_t* lpFileSystemFlags, wchar_t* lpFileSystemNameBuffer, uint32_t nFileSystemNameSize) {
    if (!real_GetVolumeInformationW) {
        real_GetVolumeInformationW = dlsym(RTLD_NEXT, "GetVolumeInformationW");
    }
    loghw("GetVolumeInformationW", (void*)lpRootPathName, lpVolumeNameBuffer);
    if (real_GetVolumeInformationW) {
        return real_GetVolumeInformationW(lpRootPathName, lpVolumeNameBuffer, nVolumeNameSize, lpVolumeSerialNumber, lpMaximumComponentLength, lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);
    }
    return 0;
}

// --- GetDiskFreeSpaceW ---
static int (*real_GetDiskFreeSpaceW)(const wchar_t*, uint32_t*, uint32_t*, uint32_t*, uint32_t*) = NULL;
int GetDiskFreeSpaceW(const wchar_t* lpRootPathName, uint32_t* lpSectorsPerCluster, uint32_t* lpBytesPerSector, uint32_t* lpNumberOfFreeClusters, uint32_t* lpTotalNumberOfClusters) {
    if (!real_GetDiskFreeSpaceW) {
        real_GetDiskFreeSpaceW = dlsym(RTLD_NEXT, "GetDiskFreeSpaceW");
    }
    loghw("GetDiskFreeSpaceW", (void*)lpRootPathName, 0);
    if (real_GetDiskFreeSpaceW) {
        return real_GetDiskFreeSpaceW(lpRootPathName, lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters);
    }
    return 0;
}

// --- GetLogicalDrives ---
static uint32_t (*real_GetLogicalDrives)(void) = NULL;
uint32_t GetLogicalDrives(void) {
    if (!real_GetLogicalDrives) {
        real_GetLogicalDrives = dlsym(RTLD_NEXT, "GetLogicalDrives");
    }
    loghw("GetLogicalDrives", 0, 0);
    if (real_GetLogicalDrives) {
        return real_GetLogicalDrives();
    }
    return 0;
}

// --- GetLogicalDriveStringsW ---
static uint32_t (*real_GetLogicalDriveStringsW)(uint32_t, wchar_t*) = NULL;
uint32_t GetLogicalDriveStringsW(uint32_t nBufferLength, wchar_t* lpBuffer) {
    if (!real_GetLogicalDriveStringsW) {
        real_GetLogicalDriveStringsW = dlsym(RTLD_NEXT, "GetLogicalDriveStringsW");
    }
    loghw("GetLogicalDriveStringsW", (void*)(uintptr_t)nBufferLength, lpBuffer);
    if (real_GetLogicalDriveStringsW) {
        return real_GetLogicalDriveStringsW(nBufferLength, lpBuffer);
    }
    return 0;
}

// --- GetDriveTypeW ---
static uint32_t (*real_GetDriveTypeW)(const wchar_t*) = NULL;
uint32_t GetDriveTypeW(const wchar_t* lpRootPathName) {
    if (!real_GetDriveTypeW) {
        real_GetDriveTypeW = dlsym(RTLD_NEXT, "GetDriveTypeW");
    }
    loghw("GetDriveTypeW", (void*)lpRootPathName, 0);
    if (real_GetDriveTypeW) {
        return real_GetDriveTypeW(lpRootPathName);
    }
    return 0;
}

// --- GetSystemMetrics ---
static int (*real_GetSystemMetrics)(int) = NULL;
int GetSystemMetrics(int nIndex) {
    if (!real_GetSystemMetrics) {
        real_GetSystemMetrics = dlsym(RTLD_NEXT, "GetSystemMetrics");
    }
    loghw("GetSystemMetrics", (void*)(uintptr_t)nIndex, 0);
    if (real_GetSystemMetrics) {
        return real_GetSystemMetrics(nIndex);
    }
    return 0;
}

// --- GetSystemDirectoryW ---
static uint32_t (*real_GetSystemDirectoryW)(wchar_t*, uint32_t) = NULL;
uint32_t GetSystemDirectoryW(wchar_t* lpBuffer, uint32_t uSize) {
    if (!real_GetSystemDirectoryW) {
        real_GetSystemDirectoryW = dlsym(RTLD_NEXT, "GetSystemDirectoryW");
    }
    loghw("GetSystemDirectoryW", lpBuffer, (void*)(uintptr_t)uSize);
    if (real_GetSystemDirectoryW) {
        return real_GetSystemDirectoryW(lpBuffer, uSize);
    }
    return 0;
}

// --- GetWindowsDirectoryW ---
static uint32_t (*real_GetWindowsDirectoryW)(wchar_t*, uint32_t) = NULL;
uint32_t GetWindowsDirectoryW(wchar_t* lpBuffer, uint32_t uSize) {
    if (!real_GetWindowsDirectoryW) {
        real_GetWindowsDirectoryW = dlsym(RTLD_NEXT, "GetWindowsDirectoryW");
    }
    loghw("GetWindowsDirectoryW", lpBuffer, (void*)(uintptr_t)uSize);
    if (real_GetWindowsDirectoryW) {
        return real_GetWindowsDirectoryW(lpBuffer, uSize);
    }
    return 0;
}

// --- GetTempPathW ---
static uint32_t (*real_GetTempPathW)(uint32_t, wchar_t*) = NULL;
uint32_t GetTempPathW(uint32_t nBufferLength, wchar_t* lpBuffer) {
    if (!real_GetTempPathW) {
        real_GetTempPathW = dlsym(RTLD_NEXT, "GetTempPathW");
    }
    loghw("GetTempPathW", (void*)(uintptr_t)nBufferLength, lpBuffer);
    if (real_GetTempPathW) {
        return real_GetTempPathW(nBufferLength, lpBuffer);
    }
    return 0;
}

// --- GetCurrentDirectoryW ---
static uint32_t (*real_GetCurrentDirectoryW)(uint32_t, wchar_t*) = NULL;
uint32_t GetCurrentDirectoryW(uint32_t nBufferLength, wchar_t* lpBuffer) {
    if (!real_GetCurrentDirectoryW) {
        real_GetCurrentDirectoryW = dlsym(RTLD_NEXT, "GetCurrentDirectoryW");
    }
    loghw("GetCurrentDirectoryW", (void*)(uintptr_t)nBufferLength, lpBuffer);
    if (real_GetCurrentDirectoryW) {
        return real_GetCurrentDirectoryW(nBufferLength, lpBuffer);
    }
    return 0;
}

// --- SetCurrentDirectoryW ---
static int (*real_SetCurrentDirectoryW)(const wchar_t*) = NULL;
int SetCurrentDirectoryW(const wchar_t* lpPathName) {
    if (!real_SetCurrentDirectoryW) {
        real_SetCurrentDirectoryW = dlsym(RTLD_NEXT, "SetCurrentDirectoryW");
    }
    loghw("SetCurrentDirectoryW", (void*)lpPathName, 0);
    if (real_SetCurrentDirectoryW) {
        return real_SetCurrentDirectoryW(lpPathName);
    }
    return 0;
}

// --- GetModuleHandleW ---
static void* (*real_GetModuleHandleW)(const wchar_t*) = NULL;
void* GetModuleHandleW(const wchar_t* lpModuleName) {
    if (!real_GetModuleHandleW) {
        real_GetModuleHandleW = dlsym(RTLD_NEXT, "GetModuleHandleW");
    }
    loghw("GetModuleHandleW", (void*)lpModuleName, 0);
    if (real_GetModuleHandleW) {
        return real_GetModuleHandleW(lpModuleName);
    }
    return NULL;
}

// --- GetModuleFileNameW ---
static uint32_t (*real_GetModuleFileNameW)(void*, wchar_t*, uint32_t) = NULL;
uint32_t GetModuleFileNameW(void* hModule, wchar_t* lpFilename, uint32_t nSize) {
    if (!real_GetModuleFileNameW) {
        real_GetModuleFileNameW = dlsym(RTLD_NEXT, "GetModuleFileNameW");
    }
    loghw("GetModuleFileNameW", hModule, (void*)(uintptr_t)nSize);
    if (real_GetModuleFileNameW) {
        return real_GetModuleFileNameW(hModule, lpFilename, nSize);
    }
    return 0;
}

// --- LoadLibraryW ---
static void* (*real_LoadLibraryW)(const wchar_t*) = NULL;
void* LoadLibraryW(const wchar_t* lpLibFileName) {
    if (!real_LoadLibraryW) {
        real_LoadLibraryW = dlsym(RTLD_NEXT, "LoadLibraryW");
    }
    loghw("LoadLibraryW", (void*)lpLibFileName, 0);
    if (real_LoadLibraryW) {
        return real_LoadLibraryW(lpLibFileName);
    }
    return NULL;
}

// --- LoadLibraryExW ---
static void* (*real_LoadLibraryExW)(const wchar_t*, void*, uint32_t) = NULL;
void* LoadLibraryExW(const wchar_t* lpLibFileName, void* hFile, uint32_t dwFlags) {
    if (!real_LoadLibraryExW) {
        real_LoadLibraryExW = dlsym(RTLD_NEXT, "LoadLibraryExW");
    }
    loghw("LoadLibraryExW", (void*)lpLibFileName, (void*)(uintptr_t)dwFlags);
    if (real_LoadLibraryExW) {
        return real_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    }
    return NULL;
}

// --- FreeLibrary ---
static int (*real_FreeLibrary)(void*) = NULL;
int FreeLibrary(void* hLibModule) {
    if (!real_FreeLibrary) {
        real_FreeLibrary = dlsym(RTLD_NEXT, "FreeLibrary");
    }
    loghw("FreeLibrary", hLibModule, 0);
    if (real_FreeLibrary) {
        return real_FreeLibrary(hLibModule);
    }
    return 0;
}

// --- GetProcAddress ---
static void* (*real_GetProcAddress)(void*, const char*) = NULL;
void* GetProcAddress(void* hModule, const char* lpProcName) {
    if (!real_GetProcAddress) {
        real_GetProcAddress = dlsym(RTLD_NEXT, "GetProcAddress");
    }
    loghw("GetProcAddress", hModule, (void*)lpProcName);
    if (real_GetProcAddress) {
        return real_GetProcAddress(hModule, lpProcName);
    }
    return NULL;
}
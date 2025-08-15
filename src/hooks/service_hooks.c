#include "../include/logger.h"
#include "../include/path_utils.h"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>

// Ofuscação de logs para serviços
static void logsvc(const char* func, void* param1, uint32_t param2) {
    char buf[128];
    snprintf(buf, sizeof(buf), "SVC:%s|%p:%u", func, param1, param2);
    logger_log(get_log_path(), buf);
}

// --- OpenSCManagerW ---
static void* (*real_OpenSCManagerW)(const wchar_t*, const wchar_t*, uint32_t) = NULL;
void* OpenSCManagerW(const wchar_t* lpMachineName, const wchar_t* lpDatabaseName, uint32_t dwDesiredAccess) {
    if (!real_OpenSCManagerW) {
        real_OpenSCManagerW = dlsym(RTLD_NEXT, "OpenSCManagerW");
    }
    logsvc("OpenSCManagerW", (void*)lpMachineName, dwDesiredAccess);
    if (real_OpenSCManagerW) {
        return real_OpenSCManagerW(lpMachineName, lpDatabaseName, dwDesiredAccess);
    }
    return NULL;
}

// --- CloseServiceHandle ---
static int (*real_CloseServiceHandle)(void*) = NULL;
int CloseServiceHandle(void* hSCObject) {
    if (!real_CloseServiceHandle) {
        real_CloseServiceHandle = dlsym(RTLD_NEXT, "CloseServiceHandle");
    }
    logsvc("CloseServiceHandle", hSCObject, 0);
    if (real_CloseServiceHandle) {
        return real_CloseServiceHandle(hSCObject);
    }
    return 0;
}

// --- CreateServiceW ---
static void* (*real_CreateServiceW)(void*, const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t, const wchar_t*, const wchar_t*, uint32_t*, const wchar_t*, const wchar_t*, const wchar_t*) = NULL;
void* CreateServiceW(void* hSCManager, const wchar_t* lpServiceName, const wchar_t* lpDisplayName, uint32_t dwDesiredAccess, uint32_t dwServiceType, uint32_t dwStartType, uint32_t dwErrorControl, const wchar_t* lpBinaryPathName, const wchar_t* lpLoadOrderGroup, uint32_t* lpdwTagId, const wchar_t* lpDependencies, const wchar_t* lpServiceStartName, const wchar_t* lpPassword) {
    if (!real_CreateServiceW) {
        real_CreateServiceW = dlsym(RTLD_NEXT, "CreateServiceW");
    }
    logsvc("CreateServiceW", hSCManager, dwServiceType);
    if (real_CreateServiceW) {
        return real_CreateServiceW(hSCManager, lpServiceName, lpDisplayName, dwDesiredAccess, dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName, lpLoadOrderGroup, lpdwTagId, lpDependencies, lpServiceStartName, lpPassword);
    }
    return NULL;
}

// --- OpenServiceW ---
static void* (*real_OpenServiceW)(void*, const wchar_t*, uint32_t) = NULL;
void* OpenServiceW(void* hSCManager, const wchar_t* lpServiceName, uint32_t dwDesiredAccess) {
    if (!real_OpenServiceW) {
        real_OpenServiceW = dlsym(RTLD_NEXT, "OpenServiceW");
    }
    logsvc("OpenServiceW", hSCManager, dwDesiredAccess);
    if (real_OpenServiceW) {
        return real_OpenServiceW(hSCManager, lpServiceName, dwDesiredAccess);
    }
    return NULL;
}

// --- DeleteService ---
static int (*real_DeleteService)(void*) = NULL;
int DeleteService(void* hService) {
    if (!real_DeleteService) {
        real_DeleteService = dlsym(RTLD_NEXT, "DeleteService");
    }
    logsvc("DeleteService", hService, 0);
    if (real_DeleteService) {
        return real_DeleteService(hService);
    }
    return 0;
}

// --- StartServiceW ---
static int (*real_StartServiceW)(void*, uint32_t, const wchar_t**) = NULL;
int StartServiceW(void* hService, uint32_t dwNumServiceArgs, const wchar_t** lpServiceArgVectors) {
    if (!real_StartServiceW) {
        real_StartServiceW = dlsym(RTLD_NEXT, "StartServiceW");
    }
    logsvc("StartServiceW", hService, dwNumServiceArgs);
    if (real_StartServiceW) {
        return real_StartServiceW(hService, dwNumServiceArgs, lpServiceArgVectors);
    }
    return 0;
}

// --- ControlService ---
static int (*real_ControlService)(void*, uint32_t, void*) = NULL;
int ControlService(void* hService, uint32_t dwControl, void* lpServiceStatus) {
    if (!real_ControlService) {
        real_ControlService = dlsym(RTLD_NEXT, "ControlService");
    }
    logsvc("ControlService", hService, dwControl);
    if (real_ControlService) {
        return real_ControlService(hService, dwControl, lpServiceStatus);
    }
    return 0;
}

// --- QueryServiceStatusEx ---
static int (*real_QueryServiceStatusEx)(void*, uint32_t, uint8_t*, uint32_t, uint32_t*) = NULL;
int QueryServiceStatusEx(void* hService, uint32_t InfoLevel, uint8_t* lpBuffer, uint32_t cbBufSize, uint32_t* pcbBytesNeeded) {
    if (!real_QueryServiceStatusEx) {
        real_QueryServiceStatusEx = dlsym(RTLD_NEXT, "QueryServiceStatusEx");
    }
    logsvc("QueryServiceStatusEx", hService, InfoLevel);
    if (real_QueryServiceStatusEx) {
        return real_QueryServiceStatusEx(hService, InfoLevel, lpBuffer, cbBufSize, pcbBytesNeeded);
    }
    return 0;
}

// --- EnumServicesStatusExW ---
static int (*real_EnumServicesStatusExW)(void*, uint32_t, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t*, uint32_t*, uint32_t*, const wchar_t*) = NULL;
int EnumServicesStatusExW(void* hSCManager, uint32_t InfoLevel, uint32_t dwServiceType, uint32_t dwServiceState, uint8_t* lpServices, uint32_t cbBufSize, uint32_t* pcbBytesNeeded, uint32_t* lpServicesReturned, uint32_t* lpResumeHandle, const wchar_t* pszGroupName) {
    if (!real_EnumServicesStatusExW) {
        real_EnumServicesStatusExW = dlsym(RTLD_NEXT, "EnumServicesStatusExW");
    }
    logsvc("EnumServicesStatusExW", hSCManager, dwServiceType);
    if (real_EnumServicesStatusExW) {
        return real_EnumServicesStatusExW(hSCManager, InfoLevel, dwServiceType, dwServiceState, lpServices, cbBufSize, pcbBytesNeeded, lpServicesReturned, lpResumeHandle, pszGroupName);
    }
    return 0;
}

// --- QueryServiceConfigW ---
static int (*real_QueryServiceConfigW)(void*, void*, uint32_t, uint32_t*) = NULL;
int QueryServiceConfigW(void* hService, void* lpServiceConfig, uint32_t cbBufSize, uint32_t* pcbBytesNeeded) {
    if (!real_QueryServiceConfigW) {
        real_QueryServiceConfigW = dlsym(RTLD_NEXT, "QueryServiceConfigW");
    }
    logsvc("QueryServiceConfigW", hService, cbBufSize);
    if (real_QueryServiceConfigW) {
        return real_QueryServiceConfigW(hService, lpServiceConfig, cbBufSize, pcbBytesNeeded);
    }
    return 0;
}

// --- ChangeServiceConfigW ---
static int (*real_ChangeServiceConfigW)(void*, uint32_t, uint32_t, uint32_t, const wchar_t*, const wchar_t*, uint32_t*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*) = NULL;
int ChangeServiceConfigW(void* hService, uint32_t dwServiceType, uint32_t dwStartType, uint32_t dwErrorControl, const wchar_t* lpBinaryPathName, const wchar_t* lpLoadOrderGroup, uint32_t* lpdwTagId, const wchar_t* lpDependencies, const wchar_t* lpServiceStartName, const wchar_t* lpPassword, const wchar_t* lpDisplayName) {
    if (!real_ChangeServiceConfigW) {
        real_ChangeServiceConfigW = dlsym(RTLD_NEXT, "ChangeServiceConfigW");
    }
    logsvc("ChangeServiceConfigW", hService, dwServiceType);
    if (real_ChangeServiceConfigW) {
        return real_ChangeServiceConfigW(hService, dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName, lpLoadOrderGroup, lpdwTagId, lpDependencies, lpServiceStartName, lpPassword, lpDisplayName);
    }
    return 0;
}

// --- RegisterServiceCtrlHandlerW ---
static void* (*real_RegisterServiceCtrlHandlerW)(const wchar_t*, void*) = NULL;
void* RegisterServiceCtrlHandlerW(const wchar_t* lpServiceName, void* lpHandlerProc) {
    if (!real_RegisterServiceCtrlHandlerW) {
        real_RegisterServiceCtrlHandlerW = dlsym(RTLD_NEXT, "RegisterServiceCtrlHandlerW");
    }
    logsvc("RegisterServiceCtrlHandlerW", (void*)lpServiceName, 0);
    if (real_RegisterServiceCtrlHandlerW) {
        return real_RegisterServiceCtrlHandlerW(lpServiceName, lpHandlerProc);
    }
    return NULL;
}

// --- SetServiceStatus ---
static int (*real_SetServiceStatus)(void*, void*) = NULL;
int SetServiceStatus(void* hServiceStatus, void* lpServiceStatus) {
    if (!real_SetServiceStatus) {
        real_SetServiceStatus = dlsym(RTLD_NEXT, "SetServiceStatus");
    }
    logsvc("SetServiceStatus", hServiceStatus, 0);
    if (real_SetServiceStatus) {
        return real_SetServiceStatus(hServiceStatus, lpServiceStatus);
    }
    return 0;
}

// --- StartServiceCtrlDispatcherW ---
static int (*real_StartServiceCtrlDispatcherW)(const void*) = NULL;
int StartServiceCtrlDispatcherW(const void* lpServiceStartTable) {
    if (!real_StartServiceCtrlDispatcherW) {
        real_StartServiceCtrlDispatcherW = dlsym(RTLD_NEXT, "StartServiceCtrlDispatcherW");
    }
    logsvc("StartServiceCtrlDispatcherW", (void*)lpServiceStartTable, 0);
    if (real_StartServiceCtrlDispatcherW) {
        return real_StartServiceCtrlDispatcherW(lpServiceStartTable);
    }
    return 0;
}

// --- GetServiceKeyNameW ---
static int (*real_GetServiceKeyNameW)(void*, const wchar_t*, wchar_t*, uint32_t*) = NULL;
int GetServiceKeyNameW(void* hSCManager, const wchar_t* lpDisplayName, wchar_t* lpServiceName, uint32_t* lpcchBuffer) {
    if (!real_GetServiceKeyNameW) {
        real_GetServiceKeyNameW = dlsym(RTLD_NEXT, "GetServiceKeyNameW");
    }
    logsvc("GetServiceKeyNameW", hSCManager, *lpcchBuffer);
    if (real_GetServiceKeyNameW) {
        return real_GetServiceKeyNameW(hSCManager, lpDisplayName, lpServiceName, lpcchBuffer);
    }
    return 0;
}

// --- GetServiceDisplayNameW ---
static int (*real_GetServiceDisplayNameW)(void*, const wchar_t*, wchar_t*, uint32_t*) = NULL;
int GetServiceDisplayNameW(void* hSCManager, const wchar_t* lpServiceName, wchar_t* lpDisplayName, uint32_t* lpcchBuffer) {
    if (!real_GetServiceDisplayNameW) {
        real_GetServiceDisplayNameW = dlsym(RTLD_NEXT, "GetServiceDisplayNameW");
    }
    logsvc("GetServiceDisplayNameW", hSCManager, *lpcchBuffer);
    if (real_GetServiceDisplayNameW) {
        return real_GetServiceDisplayNameW(hSCManager, lpServiceName, lpDisplayName, lpcchBuffer);
    }
    return 0;
}

// --- LockServiceDatabase ---
static void* (*real_LockServiceDatabase)(void*) = NULL;
void* LockServiceDatabase(void* hSCManager) {
    if (!real_LockServiceDatabase) {
        real_LockServiceDatabase = dlsym(RTLD_NEXT, "LockServiceDatabase");
    }
    logsvc("LockServiceDatabase", hSCManager, 0);
    if (real_LockServiceDatabase) {
        return real_LockServiceDatabase(hSCManager);
    }
    return NULL;
}

// --- UnlockServiceDatabase ---
static int (*real_UnlockServiceDatabase)(void*) = NULL;
int UnlockServiceDatabase(void* ScLock) {
    if (!real_UnlockServiceDatabase) {
        real_UnlockServiceDatabase = dlsym(RTLD_NEXT, "UnlockServiceDatabase");
    }
    logsvc("UnlockServiceDatabase", ScLock, 0);
    if (real_UnlockServiceDatabase) {
        return real_UnlockServiceDatabase(ScLock);
    }
    return 0;
}

// --- NotifyServiceStatusChangeW ---
static uint32_t (*real_NotifyServiceStatusChangeW)(void*, uint32_t, void*) = NULL;
uint32_t NotifyServiceStatusChangeW(void* hService, uint32_t dwNotifyMask, void* pNotifyBuffer) {
    if (!real_NotifyServiceStatusChangeW) {
        real_NotifyServiceStatusChangeW = dlsym(RTLD_NEXT, "NotifyServiceStatusChangeW");
    }
    logsvc("NotifyServiceStatusChangeW", hService, dwNotifyMask);
    if (real_NotifyServiceStatusChangeW) {
        return real_NotifyServiceStatusChangeW(hService, dwNotifyMask, pNotifyBuffer);
    }
    return 0x00000057; // ERROR_INVALID_PARAMETER
}
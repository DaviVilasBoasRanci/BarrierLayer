#include "../include/logger.h"
#include "../include/path_utils.h"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include <stddef.h>

// Ofuscação de logs para kernel
static void logkernel(const char* func, void* param1, uint32_t param2) {
    char buf[128];
    snprintf(buf, sizeof(buf), "KERN:%s|%p:%u", func, param1, param2);
    logger_log(get_log_path(), buf);
}

// --- NtCreateFile ---
static uint32_t (*real_NtCreateFile)(void**, uint32_t, void*, void*, void**, uint32_t, uint32_t, uint32_t, uint32_t, void*, uint32_t) = NULL;
uint32_t NtCreateFile(void** FileHandle, uint32_t DesiredAccess, void* ObjectAttributes, void* IoStatusBlock, void** AllocationSize, uint32_t FileAttributes, uint32_t ShareAccess, uint32_t CreateDisposition, uint32_t CreateOptions, void* EaBuffer, uint32_t EaLength) {
    if (!real_NtCreateFile) {
        real_NtCreateFile = dlsym(RTLD_NEXT, "NtCreateFile");
    }
    logkernel("NtCreateFile", ObjectAttributes, DesiredAccess);
    if (real_NtCreateFile) {
        return real_NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
    }
    return 0xC0000001; // STATUS_UNSUCCESSFUL
}

// --- NtOpenFile ---
static uint32_t (*real_NtOpenFile)(void**, uint32_t, void*, void*, uint32_t, uint32_t) = NULL;
uint32_t NtOpenFile(void** FileHandle, uint32_t DesiredAccess, void* ObjectAttributes, void* IoStatusBlock, uint32_t ShareAccess, uint32_t OpenOptions) {
    if (!real_NtOpenFile) {
        real_NtOpenFile = dlsym(RTLD_NEXT, "NtOpenFile");
    }
    logkernel("NtOpenFile", ObjectAttributes, DesiredAccess);
    if (real_NtOpenFile) {
        return real_NtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    }
    return 0xC0000001;
}

// --- NtReadFile ---
static uint32_t (*real_NtReadFile)(void*, void*, void*, void*, void*, void*, uint32_t, void*, void*) = NULL;
uint32_t NtReadFile(void* FileHandle, void* Event, void* ApcRoutine, void* ApcContext, void* IoStatusBlock, void* Buffer, uint32_t Length, void* ByteOffset, void* Key) {
    if (!real_NtReadFile) {
        real_NtReadFile = dlsym(RTLD_NEXT, "NtReadFile");
    }
    logkernel("NtReadFile", FileHandle, Length);
    if (real_NtReadFile) {
        return real_NtReadFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key);
    }
    return 0xC0000001;
}

// --- NtWriteFile ---
static uint32_t (*real_NtWriteFile)(void*, void*, void*, void*, void*, void*, uint32_t, void*, void*) = NULL;
uint32_t NtWriteFile(void* FileHandle, void* Event, void* ApcRoutine, void* ApcContext, void* IoStatusBlock, void* Buffer, uint32_t Length, void* ByteOffset, void* Key) {
    if (!real_NtWriteFile) {
        real_NtWriteFile = dlsym(RTLD_NEXT, "NtWriteFile");
    }
    logkernel("NtWriteFile", FileHandle, Length);
    if (real_NtWriteFile) {
        return real_NtWriteFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key);
    }
    return 0xC0000001;
}

// --- NtClose ---
static uint32_t (*real_NtClose)(void*) = NULL;
uint32_t NtClose(void* Handle) {
    if (!real_NtClose) {
        real_NtClose = dlsym(RTLD_NEXT, "NtClose");
    }
    logkernel("NtClose", Handle, 0);
    if (real_NtClose) {
        return real_NtClose(Handle);
    }
    return 0;
}

// --- NtQueryInformationFile ---
static uint32_t (*real_NtQueryInformationFile)(void*, void*, void*, uint32_t, uint32_t) = NULL;
uint32_t NtQueryInformationFile(void* FileHandle, void* IoStatusBlock, void* FileInformation, uint32_t Length, uint32_t FileInformationClass) {
    if (!real_NtQueryInformationFile) {
        real_NtQueryInformationFile = dlsym(RTLD_NEXT, "NtQueryInformationFile");
    }
    logkernel("NtQueryInformationFile", FileHandle, FileInformationClass);
    if (real_NtQueryInformationFile) {
        return real_NtQueryInformationFile(FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);
    }
    return 0xC0000001;
}

// --- NtSetInformationFile ---
static uint32_t (*real_NtSetInformationFile)(void*, void*, void*, uint32_t, uint32_t) = NULL;
uint32_t NtSetInformationFile(void* FileHandle, void* IoStatusBlock, void* FileInformation, uint32_t Length, uint32_t FileInformationClass) {
    if (!real_NtSetInformationFile) {
        real_NtSetInformationFile = dlsym(RTLD_NEXT, "NtSetInformationFile");
    }
    logkernel("NtSetInformationFile", FileHandle, FileInformationClass);
    if (real_NtSetInformationFile) {
        return real_NtSetInformationFile(FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);
    }
    return 0xC0000001;
}

// --- NtCreateProcess ---
static uint32_t (*real_NtCreateProcess)(void**, uint32_t, void*, void*, int, void*, void*, void*, void*) = NULL;
uint32_t NtCreateProcess(void** ProcessHandle, uint32_t DesiredAccess, void* ObjectAttributes, void* ParentProcess, int InheritObjectTable, void* SectionHandle, void* DebugPort, void* ExceptionPort, void* ProcessParameters) {
    if (!real_NtCreateProcess) {
        real_NtCreateProcess = dlsym(RTLD_NEXT, "NtCreateProcess");
    }
    logkernel("NtCreateProcess", ObjectAttributes, DesiredAccess);
    if (real_NtCreateProcess) {
        return real_NtCreateProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ParentProcess, InheritObjectTable, SectionHandle, DebugPort, ExceptionPort, ProcessParameters);
    }
    return 0xC0000001;
}

// --- NtOpenProcess ---
static uint32_t (*real_NtOpenProcess)(void**, uint32_t, void*, void*) = NULL;
uint32_t NtOpenProcess(void** ProcessHandle, uint32_t DesiredAccess, void* ObjectAttributes, void* ClientId) {
    if (!real_NtOpenProcess) {
        real_NtOpenProcess = dlsym(RTLD_NEXT, "NtOpenProcess");
    }
    logkernel("NtOpenProcess", ObjectAttributes, DesiredAccess);
    if (real_NtOpenProcess) {
        return real_NtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
    }
    return 0xC0000001;
}

// --- NtTerminateProcess ---
static uint32_t (*real_NtTerminateProcess)(void*, uint32_t) = NULL;
uint32_t NtTerminateProcess(void* ProcessHandle, uint32_t ExitStatus) {
    if (!real_NtTerminateProcess) {
        real_NtTerminateProcess = dlsym(RTLD_NEXT, "NtTerminateProcess");
    }
    logkernel("NtTerminateProcess", ProcessHandle, ExitStatus);
    if (real_NtTerminateProcess) {
        return real_NtTerminateProcess(ProcessHandle, ExitStatus);
    }
    return 0xC0000001;
}

// --- NtQueryInformationProcess ---
static uint32_t (*real_NtQueryInformationProcess)(void*, uint32_t, void*, uint32_t, uint32_t*) = NULL;
uint32_t NtQueryInformationProcess(void* ProcessHandle, uint32_t ProcessInformationClass, void* ProcessInformation, uint32_t ProcessInformationLength, uint32_t* ReturnLength) {
    if (!real_NtQueryInformationProcess) {
        real_NtQueryInformationProcess = dlsym(RTLD_NEXT, "NtQueryInformationProcess");
    }
    logkernel("NtQueryInformationProcess", ProcessHandle, ProcessInformationClass);
    if (real_NtQueryInformationProcess) {
        return real_NtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
    }
    return 0xC0000001;
}

// --- NtSetInformationProcess ---
static uint32_t (*real_NtSetInformationProcess)(void*, uint32_t, void*, uint32_t) = NULL;
uint32_t NtSetInformationProcess(void* ProcessHandle, uint32_t ProcessInformationClass, void* ProcessInformation, uint32_t ProcessInformationLength) {
    if (!real_NtSetInformationProcess) {
        real_NtSetInformationProcess = dlsym(RTLD_NEXT, "NtSetInformationProcess");
    }
    logkernel("NtSetInformationProcess", ProcessHandle, ProcessInformationClass);
    if (real_NtSetInformationProcess) {
        return real_NtSetInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength);
    }
    return 0xC0000001;
}

// --- NtCreateThread ---
static uint32_t (*real_NtCreateThread)(void**, uint32_t, void*, void*, void*, void*, int, size_t, size_t, size_t, void*) = NULL;
uint32_t NtCreateThread(void** ThreadHandle, uint32_t DesiredAccess, void* ObjectAttributes, void* ProcessHandle, void* ClientId, void* ThreadContext, int CreateSuspended, size_t ZeroBits, size_t StackCommit, size_t StackReserve, void* StartParameter) {
    if (!real_NtCreateThread) {
        real_NtCreateThread = dlsym(RTLD_NEXT, "NtCreateThread");
    }
    logkernel("NtCreateThread", ProcessHandle, DesiredAccess);
    if (real_NtCreateThread) {
        return real_NtCreateThread(ThreadHandle, DesiredAccess, ObjectAttributes, ProcessHandle, ClientId, ThreadContext, CreateSuspended, ZeroBits, StackCommit, StackReserve, StartParameter);
    }
    return 0xC0000001;
}

// --- NtOpenThread ---
static uint32_t (*real_NtOpenThread)(void**, uint32_t, void*, void*) = NULL;
uint32_t NtOpenThread(void** ThreadHandle, uint32_t DesiredAccess, void* ObjectAttributes, void* ClientId) {
    if (!real_NtOpenThread) {
        real_NtOpenThread = dlsym(RTLD_NEXT, "NtOpenThread");
    }
    logkernel("NtOpenThread", ObjectAttributes, DesiredAccess);
    if (real_NtOpenThread) {
        return real_NtOpenThread(ThreadHandle, DesiredAccess, ObjectAttributes, ClientId);
    }
    return 0xC0000001;
}

// --- NtSuspendThread ---
static uint32_t (*real_NtSuspendThread)(void*, uint32_t*) = NULL;
uint32_t NtSuspendThread(void* ThreadHandle, uint32_t* PreviousSuspendCount) {
    if (!real_NtSuspendThread) {
        real_NtSuspendThread = dlsym(RTLD_NEXT, "NtSuspendThread");
    }
    logkernel("NtSuspendThread", ThreadHandle, 0);
    if (real_NtSuspendThread) {
        return real_NtSuspendThread(ThreadHandle, PreviousSuspendCount);
    }
    return 0xC0000001;
}

// --- NtResumeThread ---
static uint32_t (*real_NtResumeThread)(void*, uint32_t*) = NULL;
uint32_t NtResumeThread(void* ThreadHandle, uint32_t* PreviousSuspendCount) {
    if (!real_NtResumeThread) {
        real_NtResumeThread = dlsym(RTLD_NEXT, "NtResumeThread");
    }
    logkernel("NtResumeThread", ThreadHandle, 0);
    if (real_NtResumeThread) {
        return real_NtResumeThread(ThreadHandle, PreviousSuspendCount);
    }
    return 0xC0000001;
}

// --- NtGetContextThread ---
static uint32_t (*real_NtGetContextThread)(void*, void*) = NULL;
uint32_t NtGetContextThread(void* ThreadHandle, void* ThreadContext) {
    if (!real_NtGetContextThread) {
        real_NtGetContextThread = dlsym(RTLD_NEXT, "NtGetContextThread");
    }
    logkernel("NtGetContextThread", ThreadHandle, 0);
    if (real_NtGetContextThread) {
        return real_NtGetContextThread(ThreadHandle, ThreadContext);
    }
    return 0xC0000001;
}

// --- NtSetContextThread ---
static uint32_t (*real_NtSetContextThread)(void*, void*) = NULL;
uint32_t NtSetContextThread(void* ThreadHandle, void* ThreadContext) {
    if (!real_NtSetContextThread) {
        real_NtSetContextThread = dlsym(RTLD_NEXT, "NtSetContextThread");
    }
    logkernel("NtSetContextThread", ThreadHandle, 0);
    if (real_NtSetContextThread) {
        return real_NtSetContextThread(ThreadHandle, ThreadContext);
    }
    return 0xC0000001;
}

// --- NtAllocateVirtualMemory ---
static uint32_t (*real_NtAllocateVirtualMemory)(void*, void**, uintptr_t, size_t*, uint32_t, uint32_t) = NULL;
uint32_t NtAllocateVirtualMemory(void* ProcessHandle, void** BaseAddress, uintptr_t ZeroBits, size_t* RegionSize, uint32_t AllocationType, uint32_t Protect) {
    if (!real_NtAllocateVirtualMemory) {
        real_NtAllocateVirtualMemory = dlsym(RTLD_NEXT, "NtAllocateVirtualMemory");
    }
    logkernel("NtAllocateVirtualMemory", ProcessHandle, *RegionSize);
    if (real_NtAllocateVirtualMemory) {
        return real_NtAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
    }
    return 0xC0000001;
}

// --- NtFreeVirtualMemory ---
static uint32_t (*real_NtFreeVirtualMemory)(void*, void**, size_t*, uint32_t) = NULL;
uint32_t NtFreeVirtualMemory(void* ProcessHandle, void** BaseAddress, size_t* RegionSize, uint32_t FreeType) {
    if (!real_NtFreeVirtualMemory) {
        real_NtFreeVirtualMemory = dlsym(RTLD_NEXT, "NtFreeVirtualMemory");
    }
    logkernel("NtFreeVirtualMemory", ProcessHandle, *RegionSize);
    if (real_NtFreeVirtualMemory) {
        return real_NtFreeVirtualMemory(ProcessHandle, BaseAddress, RegionSize, FreeType);
    }
    return 0xC0000001;
}

// --- NtProtectVirtualMemory ---
static uint32_t (*real_NtProtectVirtualMemory)(void*, void**, size_t*, uint32_t, uint32_t*) = NULL;
uint32_t NtProtectVirtualMemory(void* ProcessHandle, void** BaseAddress, size_t* RegionSize, uint32_t NewProtect, uint32_t* OldProtect) {
    if (!real_NtProtectVirtualMemory) {
        real_NtProtectVirtualMemory = dlsym(RTLD_NEXT, "NtProtectVirtualMemory");
    }
    logkernel("NtProtectVirtualMemory", ProcessHandle, *RegionSize);
    if (real_NtProtectVirtualMemory) {
        return real_NtProtectVirtualMemory(ProcessHandle, BaseAddress, RegionSize, NewProtect, OldProtect);
    }
    return 0xC0000001;
}

// --- NtQueryVirtualMemory ---
static uint32_t (*real_NtQueryVirtualMemory)(void*, void*, uint32_t, void*, size_t, size_t*) = NULL;
uint32_t NtQueryVirtualMemory(void* ProcessHandle, void* BaseAddress, uint32_t MemoryInformationClass, void* MemoryInformation, size_t MemoryInformationLength, size_t* ReturnLength) {
    if (!real_NtQueryVirtualMemory) {
        real_NtQueryVirtualMemory = dlsym(RTLD_NEXT, "NtQueryVirtualMemory");
    }
    logkernel("NtQueryVirtualMemory", ProcessHandle, MemoryInformationClass);
    if (real_NtQueryVirtualMemory) {
        return real_NtQueryVirtualMemory(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
    }
    return 0xC0000001;
}
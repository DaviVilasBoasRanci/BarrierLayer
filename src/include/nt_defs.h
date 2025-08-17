// src/include/nt_defs.h

#ifndef NT_DEFS_H
#define NT_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <wchar.h> // For UNICODE_STRING

// Basic NTSTATUS codes (simplified for common success/failure)
#define STATUS_SUCCESS ((uint32_t)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH ((uint32_t)0xC0000004L)
#define STATUS_BUFFER_TOO_SMALL ((uint32_t)0xC0000023L)
#define STATUS_UNSUCCESSFUL ((uint32_t)0xC0000001L)

// OBJECT_ATTRIBUTES (simplified)
typedef struct _OBJECT_ATTRIBUTES {
    uint32_t Length;
    void* RootDirectory;
    void* ObjectName; // Usually a UNICODE_STRING*
    uint32_t Attributes;
    void* SecurityDescriptor;
    void* SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

// UNICODE_STRING (simplified)
typedef struct _UNICODE_STRING {
    uint16_t Length;
    uint16_t MaximumLength;
    wchar_t* Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// CLIENT_ID (simplified)
typedef struct _CLIENT_ID {
    void* UniqueProcess; // PID
    void* UniqueThread;  // TID
} CLIENT_ID, *PCLIENT_ID;

// SYSTEM_PROCESS_INFORMATION (simplified, focusing on relevant fields)
// This structure is variable-length, followed by SYSTEM_THREAD_INFORMATIONs
typedef struct _SYSTEM_PROCESS_INFORMATION {
    uint32_t NextEntryOffset;
    uint32_t NumberOfThreads;
    uint64_t SpareLi1[3]; // Placeholder for large integer fields
    uint64_t CreateTime;
    uint64_t UserTime;
    uint64_t KernelTime;
    UNICODE_STRING ImageName;
    uint32_t BasePriority;
    void* UniqueProcessId; // PID
    void* InheritedFromUniqueProcessId; // Parent PID
    uint32_t HandleCount;
    uint32_t SessionId;
    uintptr_t PageDirectoryBase; // Placeholder
    size_t QuotaPeakNonPagedPoolUsage;
    size_t QuotaNonPagedPoolUsage;
    size_t QuotaPeakPagedPoolUsage;
    size_t QuotaPagedPoolUsage;
    size_t PeakPagefileUsage;
    size_t PagefileUsage;
    size_t PrivatePageCount;
    uint64_t ReadOperationCount;
    uint64_t WriteOperationCount;
    uint64_t OtherOperationCount;
    uint64_t ReadTransferCount;
    uint64_t WriteTransferCount;
    uint64_t OtherTransferCount;
    // SYSTEM_THREAD_INFORMATION follows here (not explicitly defined in this struct)
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

// SYSTEM_INFORMATION_CLASS enum (partial, for NtQuerySystemInformation)
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemProcessorInformation = 1,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5, // This is the one we care about for process enumeration
    SystemProcessorPerformanceInformation = 8,
    SystemModuleInformation = 11,
    SystemLockInformation = 12,
    SystemStackTraceInformation = 13,
    SystemPagedPoolInformation = 14,
    SystemNonPagedPoolInformation = 15,
    SystemHandleInformation = 16,
    SystemObjectInformation = 17,
    SystemPagefileInformation = 18,
    SystemVdmInstemulInformation = 19,
    SystemVdmBopInformation = 20,
    SystemFileCacheInformation = 21,
    SystemFlagsInformation = 22,
    SystemCallTimeInformation = 23,
    SystemSpeculationControlInformation = 159, // Example of a newer class
    // ... many more
} SYSTEM_INFORMATION_CLASS;

#endif // NT_DEFS_H

# System Call (Syscall) and API Analysis

This document details the Windows APIs that need to be investigated and potentially intercepted by BarrierLayer to ensure compatibility with anti-cheat systems.

## Function Categories
## Essential function lists to add to the hook

### 1. Process Manipulation

- **Description:** Functions used to create, manipulate, and terminate processes.
- **APIs of Interest:**
  - `CreateProcessA / CreateProcessW`
  - `OpenProcess`
  - `TerminateProcess`
  - `VirtualAllocEx / VirtualFreeEx`
  - `ReadProcessMemory / WriteProcessMemory`
  - `CreateRemoteThread`

### 2. File Access

- **Description:** Functions for interacting with the file system.
- **APIs of Interest:**
  - `CreateFileA / CreateFileW`
  - `ReadFile / ReadFileEx`
  - `WriteFile / WriteFileEx`
  - `GetFileAttributesA / GetFileAttributesW`

### 3. Windows Registry Operations

- **Description:** Functions for accessing and modifying the Windows Registry.
- **APIs of Interest:**
  - `RegOpenKeyExA / RegOpenKeyExW`
  - `RegQueryValueExA / RegQueryValueExW`
  - `RegSetValueExA / RegSetValueExW`
  - `RegDeleteKeyA / RegDeleteKeyW`

### 4. Network Communication

- **Description:** Socket and network functions for external communication.
- **APIs of Interest:**
  - `socket`
  - `connect`
  - `send / sendto`
  - `recv / recvfrom`
  - `WSAStartup`

### 5. Kernel and Driver Functions (Low Level)

- **Description:** Direct interactions with the Windows kernel, often through `ntdll.dll`.
- **APIs of Interest:**
  - `NtCreateFile`
  - `NtOpenProcess`
  - `NtReadVirtualMemory / NtWriteVirtualMemory`
  - `DeviceIoControl` (for communication with drivers)

### 6. System and Hardware Enumeration

- **Description:** Functions used to obtain information about the system, hardware, and loaded drivers.
- **APIs of Interest:**
  - `EnumProcesses`
  - `EnumProcessModules`
  - `GetSystemInfo`
  - `SetupDiGetClassDevsA / SetupDiGetClassDevsW`

## Comprehensive List of APIs for Hooking

**Process Manipulation**
`CreateProcessA / CreateProcessW`
`OpenProcess`
`TerminateProcess`
`VirtualAlloc / VirtualAllocEx / VirtualFree / VirtualFreeEx`
`ReadProcessMemory / WriteProcessMemory`
`CreateRemoteThread`
`GetCurrentProcess / GetCurrentProcessId`
`GetProcessId`
`GetProcessTimes`
`GetProcessHandleCount`
`GetPriorityClass / SetPriorityClass`
`SuspendThread / ResumeThread`
`OpenThread`
`GetThreadContext / SetThreadContext`
`CreateThread`
`ExitProcess / ExitThread`
`GetExitCodeProcess / GetExitCodeThread`
`WaitForSingleObject / WaitForMultipleObjects`
`DuplicateHandle`
`CloseHandle`
`EnumProcesses`
`EnumProcessModules / EnumProcessModulesEx`
`GetModuleFileNameExA / GetModuleFileNameExW`
`GetModuleBaseNameA / GetModuleBaseNameW`
`GetProcessAffinityMask / SetProcessAffinityMask`
`GetProcessIoCounters`
`GetProcessMemoryInfo`
`FlushInstructionCache`
`IsWow64Process`
`QueryFullProcessImageNameA / QueryFullProcessImageNameW`

**File Manipulation**
`CreateFileA / CreateFileW`
`ReadFile / ReadFileEx`
`WriteFile / WriteFileEx`
`CloseHandle`
`DeleteFileA / DeleteFileW`
`GetFileAttributesA / GetFileAttributesW`
`SetFileAttributesA / SetFileAttributesW`
`MoveFileA / MoveFileW`
`CopyFileA / CopyFileW`
`FindFirstFileA / FindFirstFileW`
`FindNextFileA / FindNextFileW`
`FindClose`
`SetEndOfFile`
`SetFilePointer / SetFilePointerEx`
`GetFileSize / GetFileSizeEx`
`GetFileInformationByHandle`
`GetFileType`
`LockFile / LockFileEx`
`UnlockFile / UnlockFileEx`
`FlushFileBuffers`
`GetFileTime / SetFileTime`
`GetFileSecurityA / GetFileSecurityW`
`SetFileSecurityA / SetFileSecurityW`
`GetVolumeInformationA / GetVolumeInformationW`
`GetDiskFreeSpaceA / GetDiskFreeSpaceW`
`GetDiskFreeSpaceExA / GetDiskFreeSpaceExW`
`CreateDirectoryA / CreateDirectoryW`
`RemoveDirectoryA / RemoveDirectoryW`
`GetTempFileNameA / GetTempFileNameW`
`GetTempPathA / GetTempPathW`

**Windows Registry**
`RegOpenKeyExA / RegOpenKeyExW`
`RegQueryValueExA / RegQueryValueExW`
`RegSetValueExA / RegSetValueExW`
`RegDeleteKeyA / RegDeleteKeyW`
`RegCloseKey`
`RegCreateKeyA / RegCreateKeyW`
`RegCreateKeyExA / RegCreateKeyExW`
`RegDeleteValueA / RegDeleteValueW`
`RegEnumKeyA / RegEnumKeyW`
`RegEnumKeyExA / RegEnumKeyExW`
`RegEnumValueA / RegEnumValueW`
`RegFlushKey`
`RegLoadKeyA / RegLoadKeyW`
`RegNotifyChangeKeyValue`
`RegReplaceKeyA / RegReplaceKeyW`
`RegRestoreKeyA / RegRestoreKeyW`
`RegSaveKeyA / RegSaveKeyW`
`RegUnLoadKeyA / RegUnLoadKeyW`

**Network Communication**
`socket`
`connect`
`send / sendto`
`recv / recvfrom`
`bind`
`listen`
`accept`
`closesocket`
`select`
`ioctlsocket`
`getsockopt / setsockopt`
`getpeername`
`getsockname`
`shutdown`
`WSAStartup / WSACleanup`
`WSAGetLastError`
`WSAIoctl`
`gethostbyname / gethostbyaddr`
`getaddrinfo / freeaddrinfo`
`inet_addr / inet_ntoa`
`gethostname`
`getnameinfo`

**Kernel/NTDLL Functions**
`NtQuerySystemInformation`
`NtOpenProcess`
`NtReadVirtualMemory / NtWriteVirtualMemory`
`NtCreateFile`
`NtDeviceIoControlFile`
`NtQueryInformationProcess`
`NtQueryObject`
`NtQueryDirectoryFile`
`NtOpenThread`
`NtSuspendThread / NtResumeThread`
`NtTerminateProcess`
`NtAllocateVirtualMemory / NtFreeVirtualMemory`
`NtProtectVirtualMemory`
`NtMapViewOfSection / NtUnmapViewOfSection`
`NtCreateSection / NtOpenSection`
`NtSetInformationProcess / NtSetInformationThread`
`NtQueryInformationThread`
`NtQueryVirtualMemory`
`NtFlushInstructionCache`
`NtQueryPerformanceCounter`

**System/Hardware Enumeration**
`GetSystemInfo / GetNativeSystemInfo`
`GetComputerNameA / GetComputerNameW`
`GetVolumeInformationA / GetVolumeInformationW`
`SetupDiGetClassDevsA / SetupDiGetClassDevsW`
`EnumProcesses / EnumProcessModules / EnumProcessModulesEx`
`GetLogicalDrives / GetLogicalDriveStringsA / GetLogicalDriveStringsW`
`GetDriveTypeA / GetDriveTypeW`
`GetDiskFreeSpaceA / GetDiskFreeSpaceW`
`GetDiskFreeSpaceExA / GetDiskFreeSpaceExW`
`GetAdaptersInfo / GetIfTable / GetIpAddrTable / GetTcpTable / GetUdpTable / GetNetworkParams`

**Other Relevant APIs**
`LoadLibraryA / LoadLibraryW`
`FreeLibrary`
`GetProcAddress`
`GetModuleHandleA / GetModuleHandleW`
`GetTickCount / GetTickCount64`
`QueryPerformanceCounter`
`Sleep / SleepEx`
`SetLastError / GetLastError`
`RtlMoveMemory / RtlCopyMemory / RtlZeroMemory / RtlFillMemory / RtlCompareMemory`
`RtlAllocateHeap / RtlFreeHeap / RtlReAllocateHeap`
`RtlGetVersion`
`RtlInitUnicodeString / RtlInitAnsiString`
`RtlUnicodeStringToAnsiString / RtlAnsiStringToUnicodeString`
`RtlEqualUnicodeString / RtlEqualString`
`RtlCharToInteger / RtlIntegerToChar`
`RtlTimeToSecondsSince1970 / RtlSecondsSince1970ToTime`
`RtlTimeToTimeParts / RtlTimePartsToTime`
`RtlTimeToTimeOfDay / RtlTimeOfDayToTime`
`RtlTimeToSystemTime / RtlSystemTimeToTime`
`RtlTimeToFileTime / RtlFileTimeToTime`
`RtlTimeToDosDateTime / RtlDosDateTimeToTime`
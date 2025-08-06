# Análise de Chamadas de Sistema (Syscalls) e APIs

Este documento detalha as APIs do Windows que precisam ser investigadas e potencialmente interceptadas pelo BarrierLayer para garantir a compatibilidade com os sistemas anti-cheat.

## Categorias de Funções
## Listas essenciais de funções para adicionar no hook
Manipulação de Processos
CreateProcessA / CreateProcessW
OpenProcess
TerminateProcess
VirtualAlloc / VirtualAllocEx / VirtualFree / VirtualFreeEx
ReadProcessMemory / WriteProcessMemory
CreateRemoteThread
GetCurrentProcess / GetCurrentProcessId
GetProcessId
GetProcessTimes
GetProcessHandleCount
GetPriorityClass / SetPriorityClass
SuspendThread / ResumeThread
OpenThread
GetThreadContext / SetThreadContext
CreateThread
ExitProcess / ExitThread
GetExitCodeProcess / GetExitCodeThread
WaitForSingleObject / WaitForMultipleObjects
DuplicateHandle
CloseHandle
EnumProcesses
EnumProcessModules / EnumProcessModulesEx
GetModuleFileNameExA / GetModuleFileNameExW
GetModuleBaseNameA / GetModuleBaseNameW
GetProcessAffinityMask / SetProcessAffinityMask
GetProcessIoCounters
GetProcessMemoryInfo
FlushInstructionCache
IsWow64Process
QueryFullProcessImageNameA / QueryFullProcessImageNameW
Manipulação de Arquivos
CreateFileA / CreateFileW
ReadFile / ReadFileEx
WriteFile / WriteFileEx
CloseHandle
DeleteFileA / DeleteFileW
GetFileAttributesA / GetFileAttributesW
SetFileAttributesA / SetFileAttributesW
MoveFileA / MoveFileW
CopyFileA / CopyFileW
FindFirstFileA / FindFirstFileW
FindNextFileA / FindNextFileW
FindClose
SetEndOfFile
SetFilePointer / SetFilePointerEx
GetFileSize / GetFileSizeEx
GetFileInformationByHandle
GetFileType
LockFile / LockFileEx
UnlockFile / UnlockFileEx
FlushFileBuffers
GetFileTime / SetFileTime
GetFileSecurityA / GetFileSecurityW
SetFileSecurityA / SetFileSecurityW
GetVolumeInformationA / GetVolumeInformationW
GetDiskFreeSpaceA / GetDiskFreeSpaceW
GetDiskFreeSpaceExA / GetDiskFreeSpaceExW
CreateDirectoryA / CreateDirectoryW
RemoveDirectoryA / RemoveDirectoryW
GetTempFileNameA / GetTempFileNameW
GetTempPathA / GetTempPathW
Registro do Windows
RegOpenKeyExA / RegOpenKeyExW
RegQueryValueExA / RegQueryValueExW
RegSetValueExA / RegSetValueExW
RegDeleteKeyA / RegDeleteKeyW
RegCloseKey
RegCreateKeyA / RegCreateKeyW
RegCreateKeyExA / RegCreateKeyExW
RegDeleteValueA / RegDeleteValueW
RegEnumKeyA / RegEnumKeyW
RegEnumKeyExA / RegEnumKeyExW
RegEnumValueA / RegEnumValueW
RegFlushKey
RegLoadKeyA / RegLoadKeyW
RegNotifyChangeKeyValue
RegReplaceKeyA / RegReplaceKeyW
RegRestoreKeyA / RegRestoreKeyW
RegSaveKeyA / RegSaveKeyW
RegUnLoadKeyA / RegUnLoadKeyW
Comunicação de Rede
socket
connect
send / sendto
recv / recvfrom
bind
listen
accept
closesocket
select
ioctlsocket
getsockopt / setsockopt
getpeername
getsockname
shutdown
WSAStartup / WSACleanup
WSAGetLastError
WSAIoctl
gethostbyname / gethostbyaddr
getaddrinfo / freeaddrinfo
inet_addr / inet_ntoa
gethostname
getnameinfo
Funções de Kernel/NTDLL
NtQuerySystemInformation
NtOpenProcess
NtReadVirtualMemory / NtWriteVirtualMemory
NtCreateFile
NtDeviceIoControlFile
NtQueryInformationProcess
NtQueryObject
NtQueryDirectoryFile
NtOpenThread
NtSuspendThread / NtResumeThread
NtTerminateProcess
NtAllocateVirtualMemory / NtFreeVirtualMemory
NtProtectVirtualMemory
NtMapViewOfSection / NtUnmapViewOfSection
NtCreateSection / NtOpenSection
NtSetInformationProcess / NtSetInformationThread
NtQueryInformationThread
NtQueryVirtualMemory
NtFlushInstructionCache
NtQueryPerformanceCounter
Enumeração de Sistema/Hardware
GetSystemInfo / GetNativeSystemInfo
GetComputerNameA / GetComputerNameW
GetVolumeInformationA / GetVolumeInformationW
SetupDiGetClassDevsA / SetupDiGetClassDevsW
EnumProcesses / EnumProcessModules / EnumProcessModulesEx
GetLogicalDrives / GetLogicalDriveStringsA / GetLogicalDriveStringsW
GetDriveTypeA / GetDriveTypeW
GetDiskFreeSpaceA / GetDiskFreeSpaceW
GetDiskFreeSpaceExA / GetDiskFreeSpaceExW
GetAdaptersInfo / GetIfTable / GetIpAddrTable / GetTcpTable / GetUdpTable / GetNetworkParams
Outras APIs Relevantes
LoadLibraryA / LoadLibraryW
FreeLibrary
GetProcAddress
GetModuleHandleA / GetModuleHandleW
GetTickCount / GetTickCount64
QueryPerformanceCounter
Sleep / SleepEx
SetLastError / GetLastError
RtlMoveMemory / RtlCopyMemory / RtlZeroMemory / RtlFillMemory / RtlCompareMemory
RtlAllocateHeap / RtlFreeHeap / RtlReAllocateHeap
RtlGetVersion
RtlInitUnicodeString / RtlInitAnsiString
RtlUnicodeStringToAnsiString / RtlAnsiStringToUnicodeString
RtlEqualUnicodeString / RtlEqualString
RtlCharToInteger / RtlIntegerToChar
RtlTimeToSecondsSince1970 / RtlSecondsSince1970ToTime
RtlTimeToTimeParts / RtlTimePartsToTime
RtlTimeToTimeOfDay / RtlTimeOfDayToTime
RtlTimeToSystemTime / RtlSystemTimeToTime
RtlTimeToFileTime / RtlFileTimeToTime
RtlTimeToDosDateTime / RtlDosDateTimeToTime
### 1. Manipulação de Processos

- **Descrição:** Funções usadas para criar, manipular e finalizar processos.
- **APIs de Interesse:**
  - `CreateProcessA / CreateProcessW`
  - `OpenProcess`
  - `TerminateProcess`
  - `VirtualAllocEx / VirtualFreeEx`
  - `ReadProcessMemory / WriteProcessMemory`
  - `CreateRemoteThread`

### 2. Acesso a Arquivos

- **Descrição:** Funções para interagir com o sistema de arquivos.
- **APIs de Interesse:**
  - `CreateFileA / CreateFileW`
  - `ReadFile / ReadFileEx`
  - `WriteFile / WriteFileEx`
  - `GetFileAttributesA / GetFileAttributesW`

### 3. Operações de Registro do Windows

- **Descrição:** Funções para acessar e modificar o Registro do Windows.
- **APIs de Interesse:**
  - `RegOpenKeyExA / RegOpenKeyExW`
  - `RegQueryValueExA / RegQueryValueExW`
  - `RegSetValueExA / RegSetValueExW`
  - `RegDeleteKeyA / RegDeleteKeyW`

### 4. Comunicação de Rede

- **Descrição:** Funções de socket e rede para comunicação externa.
- **APIs de Interesse:**
  - `socket`
  - `connect`
  - `send / sendto`
  - `recv / recvfrom`
  - `WSAStartup`

### 5. Funções de Kernel e Drivers (Nível Baixo)

- **Descrição:** Interações diretas com o kernel do Windows, frequentemente através da `ntdll.dll`.
- **APIs de Interesse:**
  - `NtCreateFile`
  - `NtOpenProcess`
  - `NtReadVirtualMemory / NtWriteVirtualMemory`
  - `DeviceIoControl` (para comunicação com drivers)

### 6. Enumeração de Sistema e Hardware

- **Descrição:** Funções usadas para obter informações sobre o sistema, hardware e drivers carregados.
- **APIs de Interesse:**
  - `EnumProcesses`
  - `EnumProcessModules`
  - `GetSystemInfo`
  - `SetupDiGetClassDevsA / SetupDiGetClassDevsW`

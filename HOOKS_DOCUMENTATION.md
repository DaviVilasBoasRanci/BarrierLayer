# BarrierLayer Hooks Documentation

This document provides a technical overview of the hook mechanisms implemented in BarrierLayer.

## 1. User-Mode Hooks (`barrierlayer_hook.so`)

These hooks are implemented in a shared library (`.so`) that is loaded into a target process using the `LD_PRELOAD` environment variable. The primary goal is to intercept Win32 API calls provided by the Wine/Proton environment.

### `src/hooks/file_hooks.c`

Monitors file access, one of the most common anti-cheat operations to verify game data integrity.

-   **`fopen()`**: Intercepts high-level calls to open files.
-   **`open()`**: Intercepts low-level (POSIX) calls to open files. It is more comprehensive than `fopen`.

### `src/hooks/process_hooks.c`

Monitors the creation and manipulation of processes. Essential for understanding how anti-cheat launches and monitors its own processes or game processes.

-   **`CreateProcessW()`**: Intercepts the creation of new processes in the Windows environment.

### `src/hooks/memory_hooks.c`

Monitors read and write operations in memory, which are central to both cheat operation and detection by anti-cheats.

-   **`ReadProcessMemory()`**: Intercepts attempts to read a process's memory. Used by anti-cheats to scan for cheat signatures.
-   **`WriteProcessMemory()`**: Intercepts attempts to write to a process's memory.

### `src/hooks/registry_hooks.c`

Monitors access to the Windows registry (simulated by Wine). Anti-cheat may use the registry to detect the presence of prohibited software or to store state information.

-   **`RegOpenKeyExW()`**: Intercepts the opening of registry keys.
-   **`RegQueryValueExW()`**: Intercepts the reading of values from a registry key.

### `src/hooks/crypto_hooks.c`

Monitors the use of cryptographic functions. Allows observing when anti-cheat is verifying file signatures or protecting network communication.

-   **`CryptCreateHash()`**: Intercepts the initialization of a hash operation (e.g., SHA-256, MD5).
-   **`CryptHashData()`**: Intercepts when data is added to a hash to be calculated.

## 2. Kernel-Mode Hooks (`barrierlayer_kernel.ko`)

This component is a Linux Kernel Module (LKM) designed to intercept system calls (syscalls) directly. This is the most powerful and stealthy form of hooking.

### `kernel/barrierlayer_kernel.c`

-   **`getdents64` Hook (Process Hiding)**:
    -   **Objective**: To hide the existence of a process (such as the game itself or BarrierLayer) from other applications by intercepting the syscall used to list directory contents and filtering the result to remove entries from `/proc`.
    -   **Current Status**: **EXPERIMENTAL / NON-FUNCTIONAL**. The current implementation cannot modify the syscall table due to security protections in modern kernels. Debugging has been paused to focus on user-mode hooks, which are easier to implement and test.
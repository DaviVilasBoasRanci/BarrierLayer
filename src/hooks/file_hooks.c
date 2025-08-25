#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <limits.h>
#include "../include/logger.h"
#include "../include/ultra_logger.h"
#include "../include/performance.h"
#include "../include/file_hooks.h"
#include "gdi_hooks.h" // New include
#include "user32_hooks.h" // New include

#define MAX_RULES 100
#define MAX_LINE_LEN 256

// Windows type definitions for compatibility
typedef void* HANDLE;
typedef const wchar_t* LPCWSTR;
typedef unsigned long DWORD;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define ERROR_FILE_NOT_FOUND 2
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

// --- Configuration ---
static char config_path[PATH_MAX];
static char* hidden_paths[MAX_RULES];
static int num_hidden_paths = 0;

// --- Helper Functions ---

// Mock for SetLastError to map to errno
void SetLastError(DWORD error) { errno = error; }

// Checks if a given path should be hidden based on config
static int is_path_hidden(const char* path) {
    if (!path) return 0;
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] is_path_hidden: Checking path: %s\n", path); fflush(stderr);
#endif
    for (int i = 0; i < num_hidden_paths; i++) {
        if (strcmp(path, hidden_paths[i]) == 0) {
#ifdef DEBUG
            fprintf(stderr, "[DEBUG] is_path_hidden: Path '%s' matched rule '%s'. HIDING.\n", path, hidden_paths[i]); fflush(stderr);
#endif
            return 1;
        }
    }
    return 0;
}

// Load hidden paths from config file
static void load_file_hook_config(const char* path) {
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] load_file_hook_config: Attempting to open config: %s\n", path); fflush(stderr);
#endif
    FILE* file = fopen(path, "r");
    if (!file) {
#ifdef DEBUG
        fprintf(stderr, "[DEBUG] load_file_hook_config: FAILED to open config file.\n"); fflush(stderr);
#endif
        ULTRA_WARN("FILE_HOOKS", "File hook config not found at %s, skipping.", path);
        return;
    }
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] load_file_hook_config: Config file opened successfully.\n"); fflush(stderr);
#endif

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        line[strcspn(line, "\n")] = 0;

        char* command = strtok(line, " \t");
        if (!command) continue;

        if (strcmp(command, "hide_path") == 0) {
            char* path_to_hide = strtok(NULL, "");
            if (path_to_hide && num_hidden_paths < MAX_RULES) {
                // Skip leading whitespace
                while (*path_to_hide == ' ' || *path_to_hide == '\t') path_to_hide++;
                hidden_paths[num_hidden_paths++] = strdup(path_to_hide);
#ifdef DEBUG
                fprintf(stderr, "[DEBUG] load_file_hook_config: Loaded rule: hide_path %s\n", path_to_hide); fflush(stderr);
#endif
            }
        }
    }
    fclose(file);
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] load_file_hook_config: Total rules loaded: %d\n", num_hidden_paths); fflush(stderr);
#endif
    ULTRA_INFO("FILE_HOOKS", "Loaded %d file hiding rules.", num_hidden_paths);
}

// --- Hooked Functions ---

// CreateFileW (Windows API)
static HANDLE (*real_CreateFileW)(LPCWSTR, DWORD, DWORD, void*, DWORD, DWORD, HANDLE) = NULL;
HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    if (!real_CreateFileW) {
        real_CreateFileW = dlsym(RTLD_NEXT, "CreateFileW");
    }

    char filename_utf8[1024] = {0};
    if (lpFileName) {
        wcstombs(filename_utf8, lpFileName, sizeof(filename_utf8) - 1);
    }

    if (get_performance_profile() == PROFILE_SECURITY && is_path_hidden(filename_utf8)) {
        ULTRA_WARN("FILE", "Blocked access to hidden path (CreateFileW): %s", filename_utf8);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    ULTRA_TRACE("FILE", "CreateFileW called: file='%s'", filename_utf8);
    return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// GetFileAttributesW (Windows API)
static DWORD (*real_GetFileAttributesW)(LPCWSTR) = NULL;
DWORD GetFileAttributesW(LPCWSTR lpFileName) {
    if (!real_GetFileAttributesW) {
        real_GetFileAttributesW = dlsym(RTLD_NEXT, "GetFileAttributesW");
    }

    char filename_utf8[1024] = {0};
    if (lpFileName) {
        wcstombs(filename_utf8, lpFileName, sizeof(filename_utf8) - 1);
    }

    if (get_performance_profile() == PROFILE_SECURITY && is_path_hidden(filename_utf8)) {
        ULTRA_WARN("FILE", "Blocked access to hidden path (GetFileAttributesW): %s", filename_utf8);
        return INVALID_FILE_ATTRIBUTES;
    }

    return real_GetFileAttributesW(lpFileName);
}

// fopen (Standard C Library)
static FILE* (*real_fopen)(const char*, const char*) = NULL;
FILE* fopen(const char* pathname, const char* mode) {
    if (!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }

    if (get_performance_profile() == PROFILE_SECURITY && is_path_hidden(pathname)) {
        ULTRA_WARN("FILE", "Blocked access to hidden path (fopen): %s", pathname);
        errno = ENOENT;
        return NULL;
    }

    return real_fopen(pathname, mode);
}

// access (POSIX)
static int (*real_access)(const char*, int) = NULL;
int access(const char* pathname, int mode) {
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] access() hook called for path: %s\n", pathname); fflush(stderr);
#endif
    if (!real_access) {
        real_access = dlsym(RTLD_NEXT, "access");
    }

    if (get_performance_profile() == PROFILE_SECURITY && is_path_hidden(pathname)) {
        ULTRA_WARN("FILE", "Blocked access to hidden path (access): %s", pathname);
        errno = ENOENT;
        return -1;
    }

    return real_access(pathname, mode);
}


// --- Initialization ---

__attribute__((constructor))
void file_hooks_init() {
    if (ultra_logger_init() == 0) {
        ULTRA_INFO("INIT", "File hooks initialized with ultra logging");
    }

    const char* home_dir = getenv("HOME");
    if (home_dir) {
        snprintf(config_path, sizeof(config_path), "%s/BarrierLayer/files.conf", home_dir);
    } else {
        snprintf(config_path, sizeof(config_path), "./files.conf");
    }
    load_file_hook_config(config_path);

    // Initialize GDI and User32 hooks
    init_gdi_hooks();
    init_user32_hooks();

    // Stealth techniques
    // unsetenv("LD_PRELOAD"); // Comentado para depuração
    ULTRA_INFO("STEALTH", "File hooks stealth mode DEACTIVATED for debugging");
}

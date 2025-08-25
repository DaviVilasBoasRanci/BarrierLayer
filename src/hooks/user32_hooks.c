#include <windows.h> // Added here as the first include
#include "user32_hooks.h"
#include "logger.h"
#include "ultra_logger.h"
#include <stdio.h>   // For snprintf

// Original function pointers
HŴND (WINAPI *Original_CreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
HŴND (WINAPI *Original_CreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
ATOM (WINAPI *Original_RegisterClassExA)(const WNDCLASSEXA *lpwcx);
ATOM (WINAPI *Original_RegisterClassExW)(const WNDCLASSEXW *lpwcx);
BOOL (WINAPI *Original_GetMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL (WINAPI *Original_GetMessageW)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
LRESULT (WINAPI *Original_DispatchMessageA)(const MSG *lpMsg);
LRESULT (WINAPI *Original_DispatchMessageW)(const MSG *lpMsg);
BOOL (WINAPI *Original_PeekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL (WINAPI *Original_PeekMessageW)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

// Helper to convert wide char to char for logging
static void WideCharToChar(const WCHAR* wstr, char* str, size_t len) {
    if (!wstr) { str[0] = '\0'; return; }
    snprintf(str, len, "%ls", wstr);
    str[len - 1] = '\0'; // Ensure null termination
}

// Hook functions
HŴND WINAPI Hook_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    HWND result = Original_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    ULTRA_LOG(INFO, "User32 Hook: CreateWindowExA(class=\"%%s\", name=\"%%s\") -> %%p", lpClassName ? lpClassName : "NULL", lpWindowName ? lpWindowName : "NULL", result);
    return result;
}

HŴND WINAPI Hook_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    char className[256], windowName[256];
    WideCharToChar(lpClassName, className, sizeof(className));
    WideCharToChar(lpWindowName, windowName, sizeof(windowName));
    HWND result = Original_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    ULTRA_LOG(INFO, "User32 Hook: CreateWindowExW(class=\"%%s\", name=\"%%s\") -> %%p", className, windowName, result);
    return result;
}

ATOM WINAPI Hook_RegisterClassExA(const WNDCLASSEXA *lpwcx) {
    ATOM result = Original_RegisterClassExA(lpwcx);
    ULTRA_LOG(INFO, "User32 Hook: RegisterClassExA(class=\"%%s\") -> %%hu", lpwcx ? lpwcx->lpszClassName : "NULL", result);
    return result;
}

ATOM WINAPI Hook_RegisterClassExW(const WNDCLASSEXW *lpwcx) {
    char className[256];
    WideCharToChar(lpwcx ? lpwcx->lpszClassName : NULL, className, sizeof(className));
    ATOM result = Original_RegisterClassExW(lpwcx);
    ULTRA_LOG(INFO, "User32 Hook: RegisterClassExW(class=\"%%s\") -> %%hu", className, result);
    return result;
}

BOOL WINAPI Hook_GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    BOOL result = Original_GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    ULTRA_LOG(INFO, "User32 Hook: GetMessageA(hWnd=%%p, msg=%%u) -> %%d", hWnd, lpMsg ? lpMsg->message : 0, result);
    return result;
}

BOOL WINAPI Hook_GetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    BOOL result = Original_GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    ULTRA_LOG(INFO, "User32 Hook: GetMessageW(hWnd=%%p, msg=%%u) -> %%d", hWnd, lpMsg ? lpMsg->message : 0, result);
    return result;
}

LRESULT WINAPI Hook_DispatchMessageA(const MSG *lpMsg) {
    LRESULT result = Original_DispatchMessageA(lpMsg);
    ULTRA_LOG(INFO, "User32 Hook: DispatchMessageA(msg=%%u) -> %%ld", lpMsg ? lpMsg->message : 0, result);
    return result;
}

LRESULT WINAPI Hook_DispatchMessageW(const MSG *lpMsg) {
    LRESULT result = Original_DispatchMessageW(lpMsg);
    ULTRA_LOG(INFO, "User32 Hook: DispatchMessageW(msg=%%u) -> %%ld", lpMsg ? lpMsg->message : 0, result);
    return result;
}

BOOL WINAPI Hook_PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    BOOL result = Original_PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    ULTRA_LOG(INFO, "User32 Hook: PeekMessageA(hWnd=%%p, msg=%%u) -> %%d", hWnd, lpMsg ? lpMsg->message : 0, result);
    return result;
}

BOOL WINAPI Hook_PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    BOOL result = Original_PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    ULTRA_LOG(INFO, "User32 Hook: PeekMessageW(hWnd=%%p, msg=%%u) -> %%d", hWnd, lpMsg ? lpMsg->message : 0, result);
    return result;
}

void init_user32_hooks(void) {
    // Initialize hooks for User32 functions
    // Use the HOOK_FUNCTION macro from barrierlayer.h or similar mechanism
    // For now, just log initialization
    ULTRA_LOG(INFO, "User32 hooks initialized.");
}
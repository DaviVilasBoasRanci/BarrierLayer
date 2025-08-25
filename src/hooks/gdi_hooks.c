#include <windows.h> // Added here as the first include
#include "gdi_hooks.h"
#include "logger.h"
#include "ultra_logger.h"
#include <stdio.h>   // For snprintf

// Original function pointers
HDC (WINAPI *Original_CreateCompatibleDC)(HDC hdc);
HBITMAP (WINAPI *Original_CreateBitmap)(int nWidth, int nHeight, UINT cPlanes, UINT cBitsPerPixel, const VOID *lpvBits);
HGDIOBJ (WINAPI *Original_SelectObject)(HDC hdc, HGDIOBJ hgdiobj);
BOOL (WINAPI *Original_BitBlt)(HDC hdcDest, int xDest, int yDest, int cx, int cy, HDC hdcSrc, int xSrc, int ySrc, DWORD rop);
BOOL (WINAPI *Original_StretchBlt)(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, DWORD rop);

// Hook functions
HDC WINAPI Hook_CreateCompatibleDC(HDC hdc) {
    HDC result = Original_CreateCompatibleDC(hdc);
    ULTRA_LOG(INFO, "GDI Hook: CreateCompatibleDC(hdc=%p) -> %p", hdc, result);
    return result;
}

HBITMAP WINAPI Hook_CreateBitmap(int nWidth, int nHeight, UINT cPlanes, UINT cBitsPerPixel, const VOID *lpvBits) {
    HBITMAP result = Original_CreateBitmap(nWidth, nHeight, cPlanes, cBitsPerPixel, lpvBits);
    ULTRA_LOG(INFO, "GDI Hook: CreateBitmap(width=%d, height=%d, planes=%u, bpp=%u) -> %p", nWidth, nHeight, cPlanes, cBitsPerPixel, result);
    return result;
}

HGDIOBJ WINAPI Hook_SelectObject(HDC hdc, HGDIOBJ hgdiobj) {
    HGDIOBJ result = Original_SelectObject(hdc, hgdiobj);
    ULTRA_LOG(INFO, "GDI Hook: SelectObject(hdc=%p, hgdiobj=%p) -> %p", hdc, hgdiobj, result);
    return result;
}

BOOL WINAPI Hook_BitBlt(HDC hdcDest, int xDest, int yDest, int cx, int cy, HDC hdcSrc, int xSrc, int ySrc, DWORD rop) {
    BOOL result = Original_BitBlt(hdcDest, xDest, yDest, cx, cy, hdcSrc, xSrc, ySrc, rop);
    ULTRA_LOG(INFO, "GDI Hook: BitBlt(hdcDest=%p, xDest=%d, yDest=%d, cx=%d, cy=%d, hdcSrc=%p, xSrc=%d, ySrc=%d, rop=0x%lX) -> %d", hdcDest, xDest, yDest, cx, cy, hdcSrc, xSrc, ySrc, rop, result);
    return result;
}

BOOL WINAPI Hook_StretchBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, DWORD rop) {
    BOOL result = Original_StretchBlt(hdcDest, xDest, yDest, cxDest, cyDest, hdcSrc, xSrc, ySrc, cxSrc, cySrc, rop);
    ULTRA_LOG(INFO, "GDI Hook: StretchBlt(hdcDest=%p, xDest=%d, yDest=%d, cxDest=%d, cyDest=%d, hdcSrc=%p, xSrc=%d, ySrc=%d, cxSrc=%d, cySrc=%d, rop=0x%lX) -> %d", hdcDest, xDest, yDest, cxDest, cyDest, hdcSrc, xSrc, ySrc, cxSrc, cySrc, rop, result);
    return result;
}

void init_gdi_hooks(void) {
    // Initialize hooks for GDI functions
    // Use the HOOK_FUNCTION macro from barrierlayer.h or similar mechanism
    // For now, just log initialization
    ULTRA_LOG(INFO, "GDI hooks initialized.");
}
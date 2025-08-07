#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include "logger.h"

// Ofuscação de logs para criptografia
static void logcrypto(const char* func, void* param1, int size) {
    char buf[128];
    snprintf(buf, sizeof(buf), "CRYPTO:%s|%p:%d", func, param1, size);
    logger_log(LOG_PATH, buf);
}

// --- CryptAcquireContextW ---
static int (*real_CryptAcquireContextW)(void**, const wchar_t*, const wchar_t*, uint32_t, uint32_t) = NULL;
int CryptAcquireContextW(void** phProv, const wchar_t* szContainer, const wchar_t* szProvider, uint32_t dwProvType, uint32_t dwFlags) {
    if (!real_CryptAcquireContextW) {
        real_CryptAcquireContextW = dlsym(RTLD_NEXT, "CryptAcquireContextW");
    }
    logcrypto("CryptAcquireContextW", (void*)szContainer, dwProvType);
    if (real_CryptAcquireContextW) {
        return real_CryptAcquireContextW(phProv, szContainer, szProvider, dwProvType, dwFlags);
    }
    return 0;
}

// --- CryptReleaseContext ---
static int (*real_CryptReleaseContext)(void*, uint32_t) = NULL;
int CryptReleaseContext(void* hProv, uint32_t dwFlags) {
    if (!real_CryptReleaseContext) {
        real_CryptReleaseContext = dlsym(RTLD_NEXT, "CryptReleaseContext");
    }
    logcrypto("CryptReleaseContext", hProv, dwFlags);
    if (real_CryptReleaseContext) {
        return real_CryptReleaseContext(hProv, dwFlags);
    }
    return 0;
}

// --- CryptCreateHash ---
static int (*real_CryptCreateHash)(void*, uint32_t, void*, uint32_t, void**) = NULL;
int CryptCreateHash(void* hProv, uint32_t Algid, void* hKey, uint32_t dwFlags, void** phHash) {
    if (!real_CryptCreateHash) {
        real_CryptCreateHash = dlsym(RTLD_NEXT, "CryptCreateHash");
    }
    logcrypto("CryptCreateHash", hProv, Algid);
    if (real_CryptCreateHash) {
        return real_CryptCreateHash(hProv, Algid, hKey, dwFlags, phHash);
    }
    return 0;
}

// --- CryptHashData ---
static int (*real_CryptHashData)(void*, const uint8_t*, uint32_t, uint32_t) = NULL;
int CryptHashData(void* hHash, const uint8_t* pbData, uint32_t dwDataLen, uint32_t dwFlags) {
    if (!real_CryptHashData) {
        real_CryptHashData = dlsym(RTLD_NEXT, "CryptHashData");
    }
    logcrypto("CryptHashData", hHash, dwDataLen);
    if (real_CryptHashData) {
        return real_CryptHashData(hHash, pbData, dwDataLen, dwFlags);
    }
    return 0;
}

// --- CryptGetHashParam ---
static int (*real_CryptGetHashParam)(void*, uint32_t, uint8_t*, uint32_t*, uint32_t) = NULL;
int CryptGetHashParam(void* hHash, uint32_t dwParam, uint8_t* pbData, uint32_t* pdwDataLen, uint32_t dwFlags) {
    if (!real_CryptGetHashParam) {
        real_CryptGetHashParam = dlsym(RTLD_NEXT, "CryptGetHashParam");
    }
    logcrypto("CryptGetHashParam", hHash, dwParam);
    if (real_CryptGetHashParam) {
        return real_CryptGetHashParam(hHash, dwParam, pbData, pdwDataLen, dwFlags);
    }
    return 0;
}

// --- CryptDestroyHash ---
static int (*real_CryptDestroyHash)(void*) = NULL;
int CryptDestroyHash(void* hHash) {
    if (!real_CryptDestroyHash) {
        real_CryptDestroyHash = dlsym(RTLD_NEXT, "CryptDestroyHash");
    }
    logcrypto("CryptDestroyHash", hHash, 0);
    if (real_CryptDestroyHash) {
        return real_CryptDestroyHash(hHash);
    }
    return 0;
}

// --- CryptGenRandom ---
static int (*real_CryptGenRandom)(void*, uint32_t, uint8_t*) = NULL;
int CryptGenRandom(void* hProv, uint32_t dwLen, uint8_t* pbBuffer) {
    if (!real_CryptGenRandom) {
        real_CryptGenRandom = dlsym(RTLD_NEXT, "CryptGenRandom");
    }
    logcrypto("CryptGenRandom", hProv, dwLen);
    if (real_CryptGenRandom) {
        return real_CryptGenRandom(hProv, dwLen, pbBuffer);
    }
    return 0;
}

// --- CryptGenKey ---
static int (*real_CryptGenKey)(void*, uint32_t, uint32_t, void**) = NULL;
int CryptGenKey(void* hProv, uint32_t Algid, uint32_t dwFlags, void** phKey) {
    if (!real_CryptGenKey) {
        real_CryptGenKey = dlsym(RTLD_NEXT, "CryptGenKey");
    }
    logcrypto("CryptGenKey", hProv, Algid);
    if (real_CryptGenKey) {
        return real_CryptGenKey(hProv, Algid, dwFlags, phKey);
    }
    return 0;
}

// --- CryptEncrypt ---
static int (*real_CryptEncrypt)(void*, void*, int, uint32_t, uint8_t*, uint32_t*, uint32_t) = NULL;
int CryptEncrypt(void* hKey, void* hHash, int Final, uint32_t dwFlags, uint8_t* pbData, uint32_t* pdwDataLen, uint32_t dwBufLen) {
    if (!real_CryptEncrypt) {
        real_CryptEncrypt = dlsym(RTLD_NEXT, "CryptEncrypt");
    }
    logcrypto("CryptEncrypt", hKey, *pdwDataLen);
    if (real_CryptEncrypt) {
        return real_CryptEncrypt(hKey, hHash, Final, dwFlags, pbData, pdwDataLen, dwBufLen);
    }
    return 0;
}

// --- CryptDecrypt ---
static int (*real_CryptDecrypt)(void*, void*, int, uint32_t, uint8_t*, uint32_t*) = NULL;
int CryptDecrypt(void* hKey, void* hHash, int Final, uint32_t dwFlags, uint8_t* pbData, uint32_t* pdwDataLen) {
    if (!real_CryptDecrypt) {
        real_CryptDecrypt = dlsym(RTLD_NEXT, "CryptDecrypt");
    }
    logcrypto("CryptDecrypt", hKey, *pdwDataLen);
    if (real_CryptDecrypt) {
        return real_CryptDecrypt(hKey, hHash, Final, dwFlags, pbData, pdwDataLen);
    }
    return 0;
}

// --- CryptDestroyKey ---
static int (*real_CryptDestroyKey)(void*) = NULL;
int CryptDestroyKey(void* hKey) {
    if (!real_CryptDestroyKey) {
        real_CryptDestroyKey = dlsym(RTLD_NEXT, "CryptDestroyKey");
    }
    logcrypto("CryptDestroyKey", hKey, 0);
    if (real_CryptDestroyKey) {
        return real_CryptDestroyKey(hKey);
    }
    return 0;
}

// --- BCryptOpenAlgorithmProvider ---
static uint32_t (*real_BCryptOpenAlgorithmProvider)(void**, const wchar_t*, const wchar_t*, uint32_t) = NULL;
uint32_t BCryptOpenAlgorithmProvider(void** phAlgorithm, const wchar_t* pszAlgId, const wchar_t* pszImplementation, uint32_t dwFlags) {
    if (!real_BCryptOpenAlgorithmProvider) {
        real_BCryptOpenAlgorithmProvider = dlsym(RTLD_NEXT, "BCryptOpenAlgorithmProvider");
    }
    logcrypto("BCryptOpenAlgorithmProvider", (void*)pszAlgId, dwFlags);
    if (real_BCryptOpenAlgorithmProvider) {
        return real_BCryptOpenAlgorithmProvider(phAlgorithm, pszAlgId, pszImplementation, dwFlags);
    }
    return 0xC0000001; // STATUS_UNSUCCESSFUL
}

// --- BCryptCloseAlgorithmProvider ---
static uint32_t (*real_BCryptCloseAlgorithmProvider)(void*, uint32_t) = NULL;
uint32_t BCryptCloseAlgorithmProvider(void* hAlgorithm, uint32_t dwFlags) {
    if (!real_BCryptCloseAlgorithmProvider) {
        real_BCryptCloseAlgorithmProvider = dlsym(RTLD_NEXT, "BCryptCloseAlgorithmProvider");
    }
    logcrypto("BCryptCloseAlgorithmProvider", hAlgorithm, dwFlags);
    if (real_BCryptCloseAlgorithmProvider) {
        return real_BCryptCloseAlgorithmProvider(hAlgorithm, dwFlags);
    }
    return 0;
}

// --- BCryptCreateHash ---
static uint32_t (*real_BCryptCreateHash)(void*, void**, uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t) = NULL;
uint32_t BCryptCreateHash(void* hAlgorithm, void** phHash, uint8_t* pbHashObject, uint32_t cbHashObject, uint8_t* pbSecret, uint32_t cbSecret, uint32_t dwFlags) {
    if (!real_BCryptCreateHash) {
        real_BCryptCreateHash = dlsym(RTLD_NEXT, "BCryptCreateHash");
    }
    logcrypto("BCryptCreateHash", hAlgorithm, cbHashObject);
    if (real_BCryptCreateHash) {
        return real_BCryptCreateHash(hAlgorithm, phHash, pbHashObject, cbHashObject, pbSecret, cbSecret, dwFlags);
    }
    return 0xC0000001;
}

// --- BCryptHashData ---
static uint32_t (*real_BCryptHashData)(void*, uint8_t*, uint32_t, uint32_t) = NULL;
uint32_t BCryptHashData(void* hHash, uint8_t* pbInput, uint32_t cbInput, uint32_t dwFlags) {
    if (!real_BCryptHashData) {
        real_BCryptHashData = dlsym(RTLD_NEXT, "BCryptHashData");
    }
    logcrypto("BCryptHashData", hHash, cbInput);
    if (real_BCryptHashData) {
        return real_BCryptHashData(hHash, pbInput, cbInput, dwFlags);
    }
    return 0xC0000001;
}

// --- BCryptFinishHash ---
static uint32_t (*real_BCryptFinishHash)(void*, uint8_t*, uint32_t, uint32_t) = NULL;
uint32_t BCryptFinishHash(void* hHash, uint8_t* pbOutput, uint32_t cbOutput, uint32_t dwFlags) {
    if (!real_BCryptFinishHash) {
        real_BCryptFinishHash = dlsym(RTLD_NEXT, "BCryptFinishHash");
    }
    logcrypto("BCryptFinishHash", hHash, cbOutput);
    if (real_BCryptFinishHash) {
        return real_BCryptFinishHash(hHash, pbOutput, cbOutput, dwFlags);
    }
    return 0xC0000001;
}

// --- BCryptDestroyHash ---
static uint32_t (*real_BCryptDestroyHash)(void*) = NULL;
uint32_t BCryptDestroyHash(void* hHash) {
    if (!real_BCryptDestroyHash) {
        real_BCryptDestroyHash = dlsym(RTLD_NEXT, "BCryptDestroyHash");
    }
    logcrypto("BCryptDestroyHash", hHash, 0);
    if (real_BCryptDestroyHash) {
        return real_BCryptDestroyHash(hHash);
    }
    return 0;
}

// --- BCryptGenRandom ---
static uint32_t (*real_BCryptGenRandom)(void*, uint8_t*, uint32_t, uint32_t) = NULL;
uint32_t BCryptGenRandom(void* hAlgorithm, uint8_t* pbBuffer, uint32_t cbBuffer, uint32_t dwFlags) {
    if (!real_BCryptGenRandom) {
        real_BCryptGenRandom = dlsym(RTLD_NEXT, "BCryptGenRandom");
    }
    logcrypto("BCryptGenRandom", hAlgorithm, cbBuffer);
    if (real_BCryptGenRandom) {
        return real_BCryptGenRandom(hAlgorithm, pbBuffer, cbBuffer, dwFlags);
    }
    return 0xC0000001;
}

// --- RtlGenRandom ---
static int (*real_RtlGenRandom)(void*, uint32_t) = NULL;
int RtlGenRandom(void* RandomBuffer, uint32_t RandomBufferLength) {
    if (!real_RtlGenRandom) {
        real_RtlGenRandom = dlsym(RTLD_NEXT, "RtlGenRandom");
    }
    logcrypto("RtlGenRandom", RandomBuffer, RandomBufferLength);
    if (real_RtlGenRandom) {
        return real_RtlGenRandom(RandomBuffer, RandomBufferLength);
    }
    return 0;
}
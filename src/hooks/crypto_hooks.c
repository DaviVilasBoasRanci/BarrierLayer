#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stddef.h> // Incluído para consistência

// Tipos de dados da API do Windows para Criptografia
#ifndef _WINDEF_
#define _WINDEF_
    typedef unsigned long DWORD;
    typedef int BOOL;
    typedef const unsigned char* BYTE_PTR;
#endif

#ifndef _WINNT_
#define _WINNT_
    #if defined(__x86_64__)
        typedef unsigned long long ULONG_PTR;
    #else
        typedef unsigned int ULONG_PTR;
    #endif
    typedef ULONG_PTR HCRYPTPROV;
    typedef ULONG_PTR HCRYPTHASH;
    typedef unsigned int ALG_ID;
#endif

// --- Hook para CryptCreateHash ---
typedef BOOL (*CryptCreateHash_t)(HCRYPTPROV, ALG_ID, HCRYPTHASH, DWORD, HCRYPTHASH*);
static CryptCreateHash_t original_CryptCreateHash = NULL;

BOOL CryptCreateHash(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTHASH hKey, DWORD dwFlags, HCRYPTHASH *phHash) {
    if (original_CryptCreateHash == NULL) {
        original_CryptCreateHash = dlsym(RTLD_NEXT, "CryptCreateHash");
    }

    fprintf(stderr, "[BarrierLayer Crypto Hook] Interceptado: CryptCreateHash com Algoritmo ID: %u\n", Algid);
    fflush(stderr);

    return original_CryptCreateHash(hProv, Algid, hKey, dwFlags, phHash);
}

// --- Hook para CryptHashData ---
typedef BOOL (*CryptHashData_t)(HCRYPTHASH, const BYTE_PTR, DWORD, DWORD);
static CryptHashData_t original_CryptHashData = NULL;

BOOL CryptHashData(HCRYPTHASH hHash, const BYTE_PTR pbData, DWORD dwDataLen, DWORD dwFlags) {
    if (original_CryptHashData == NULL) {
        original_CryptHashData = dlsym(RTLD_NEXT, "CryptHashData");
    }

    // Corrigido o especificador de formato para DWORD
    fprintf(stderr, "[BarrierLayer Crypto Hook] Interceptado: CryptHashData com %lu bytes de dados\n", (unsigned long)dwDataLen);
    fflush(stderr);

    return original_CryptHashData(hHash, pbData, dwDataLen, dwFlags);
}
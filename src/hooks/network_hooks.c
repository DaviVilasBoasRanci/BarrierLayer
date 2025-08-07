#include "../include/logger.h"
#define LOG_PATH "/home/davivbrdev/BarrierLayer/barrierlayer_activity.log"

#include <stdio.h>
#include <dlfcn.h>
#include <wchar.h>
#include <stdint.h>
#include "logger.h"

// Ofuscação de logs para stealth
static void lognet(const char* func, void* addr, int port) {
    char buf[128];
    snprintf(buf, sizeof(buf), "NET:%s|%p:%d", func, addr, port);
    logger_log(LOG_PATH, buf);
}

// --- WSAStartup ---
static int (*real_WSAStartup)(uint16_t, void*) = NULL;
int WSAStartup(uint16_t wVersionRequested, void* lpWSAData) {
    if (!real_WSAStartup) {
        real_WSAStartup = dlsym(RTLD_NEXT, "WSAStartup");
    }
    lognet("WSAStartup", (void*)(uintptr_t)wVersionRequested, 0);
    if (real_WSAStartup) {
        return real_WSAStartup(wVersionRequested, lpWSAData);
    }
    return 0;
}

// --- WSACleanup ---
static int (*real_WSACleanup)(void) = NULL;
int WSACleanup(void) {
    if (!real_WSACleanup) {
        real_WSACleanup = dlsym(RTLD_NEXT, "WSACleanup");
    }
    lognet("WSACleanup", 0, 0);
    if (real_WSACleanup) {
        return real_WSACleanup();
    }
    return 0;
}

// --- socket ---
static int (*real_socket)(int, int, int) = NULL;
int socket(int domain, int type, int protocol) {
    if (!real_socket) {
        real_socket = dlsym(RTLD_NEXT, "socket");
    }
    lognet("socket", (void*)(uintptr_t)domain, type);
    if (real_socket) {
        return real_socket(domain, type, protocol);
    }
    return -1;
}

// --- connect ---
static int (*real_connect)(int, const void*, int) = NULL;
int connect(int sockfd, const void* addr, int addrlen) {
    if (!real_connect) {
        real_connect = dlsym(RTLD_NEXT, "connect");
    }
    lognet("connect", (void*)(uintptr_t)sockfd, addrlen);
    if (real_connect) {
        return real_connect(sockfd, addr, addrlen);
    }
    return -1;
}

// --- bind ---
static int (*real_bind)(int, const void*, int) = NULL;
int bind(int sockfd, const void* addr, int addrlen) {
    if (!real_bind) {
        real_bind = dlsym(RTLD_NEXT, "bind");
    }
    lognet("bind", (void*)(uintptr_t)sockfd, addrlen);
    if (real_bind) {
        return real_bind(sockfd, addr, addrlen);
    }
    return -1;
}

// --- listen ---
static int (*real_listen)(int, int) = NULL;
int listen(int sockfd, int backlog) {
    if (!real_listen) {
        real_listen = dlsym(RTLD_NEXT, "listen");
    }
    lognet("listen", (void*)(uintptr_t)sockfd, backlog);
    if (real_listen) {
        return real_listen(sockfd, backlog);
    }
    return -1;
}

// --- accept ---
static int (*real_accept)(int, void*, int*) = NULL;
int accept(int sockfd, void* addr, int* addrlen) {
    if (!real_accept) {
        real_accept = dlsym(RTLD_NEXT, "accept");
    }
    lognet("accept", (void*)(uintptr_t)sockfd, 0);
    if (real_accept) {
        return real_accept(sockfd, addr, addrlen);
    }
    return -1;
}

// --- send ---
static int (*real_send)(int, const void*, int, int) = NULL;
int send(int sockfd, const void* buf, int len, int flags) {
    if (!real_send) {
        real_send = dlsym(RTLD_NEXT, "send");
    }
    lognet("send", (void*)(uintptr_t)sockfd, len);
    if (real_send) {
        return real_send(sockfd, buf, len, flags);
    }
    return -1;
}

// --- recv ---
static int (*real_recv)(int, void*, int, int) = NULL;
int recv(int sockfd, void* buf, int len, int flags) {
    if (!real_recv) {
        real_recv = dlsym(RTLD_NEXT, "recv");
    }
    lognet("recv", (void*)(uintptr_t)sockfd, len);
    if (real_recv) {
        return real_recv(sockfd, buf, len, flags);
    }
    return -1;
}

// --- sendto ---
static int (*real_sendto)(int, const void*, int, int, const void*, int) = NULL;
int sendto(int sockfd, const void* buf, int len, int flags, const void* dest_addr, int addrlen) {
    if (!real_sendto) {
        real_sendto = dlsym(RTLD_NEXT, "sendto");
    }
    lognet("sendto", (void*)(uintptr_t)sockfd, len);
    if (real_sendto) {
        return real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }
    return -1;
}

// --- recvfrom ---
static int (*real_recvfrom)(int, void*, int, int, void*, int*) = NULL;
int recvfrom(int sockfd, void* buf, int len, int flags, void* src_addr, int* addrlen) {
    if (!real_recvfrom) {
        real_recvfrom = dlsym(RTLD_NEXT, "recvfrom");
    }
    lognet("recvfrom", (void*)(uintptr_t)sockfd, len);
    if (real_recvfrom) {
        return real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }
    return -1;
}

// --- closesocket ---
static int (*real_closesocket)(int) = NULL;
int closesocket(int sockfd) {
    if (!real_closesocket) {
        real_closesocket = dlsym(RTLD_NEXT, "closesocket");
    }
    lognet("closesocket", (void*)(uintptr_t)sockfd, 0);
    if (real_closesocket) {
        return real_closesocket(sockfd);
    }
    return -1;
}

// --- gethostbyname ---
static void* (*real_gethostbyname)(const char*) = NULL;
void* gethostbyname(const char* name) {
    if (!real_gethostbyname) {
        real_gethostbyname = dlsym(RTLD_NEXT, "gethostbyname");
    }
    lognet("gethostbyname", (void*)name, 0);
    if (real_gethostbyname) {
        return real_gethostbyname(name);
    }
    return NULL;
}

// --- gethostbyaddr ---
static void* (*real_gethostbyaddr)(const void*, int, int) = NULL;
void* gethostbyaddr(const void* addr, int len, int type) {
    if (!real_gethostbyaddr) {
        real_gethostbyaddr = dlsym(RTLD_NEXT, "gethostbyaddr");
    }
    lognet("gethostbyaddr", (void*)addr, len);
    if (real_gethostbyaddr) {
        return real_gethostbyaddr(addr, len, type);
    }
    return NULL;
}

// --- getaddrinfo ---
static int (*real_getaddrinfo)(const char*, const char*, const void*, void**) = NULL;
int getaddrinfo(const char* node, const char* service, const void* hints, void** res) {
    if (!real_getaddrinfo) {
        real_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
    }
    lognet("getaddrinfo", (void*)node, 0);
    if (real_getaddrinfo) {
        return real_getaddrinfo(node, service, hints, res);
    }
    return -1;
}

// --- InternetOpenW ---
static void* (*real_InternetOpenW)(const wchar_t*, uint32_t, const wchar_t*, const wchar_t*, uint32_t) = NULL;
void* InternetOpenW(const wchar_t* lpszAgent, uint32_t dwAccessType, const wchar_t* lpszProxy, const wchar_t* lpszProxyBypass, uint32_t dwFlags) {
    if (!real_InternetOpenW) {
        real_InternetOpenW = dlsym(RTLD_NEXT, "InternetOpenW");
    }
    lognet("InternetOpenW", (void*)lpszAgent, dwAccessType);
    if (real_InternetOpenW) {
        return real_InternetOpenW(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags);
    }
    return NULL;
}

// --- InternetConnectW ---
static void* (*real_InternetConnectW)(void*, const wchar_t*, uint16_t, const wchar_t*, const wchar_t*, uint32_t, uint32_t, uintptr_t) = NULL;
void* InternetConnectW(void* hInternet, const wchar_t* lpszServerName, uint16_t nServerPort, const wchar_t* lpszUserName, const wchar_t* lpszPassword, uint32_t dwService, uint32_t dwFlags, uintptr_t dwContext) {
    if (!real_InternetConnectW) {
        real_InternetConnectW = dlsym(RTLD_NEXT, "InternetConnectW");
    }
    lognet("InternetConnectW", hInternet, nServerPort);
    if (real_InternetConnectW) {
        return real_InternetConnectW(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
    }
    return NULL;
}

// --- HttpOpenRequestW ---
static void* (*real_HttpOpenRequestW)(void*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t**, uint32_t, uintptr_t) = NULL;
void* HttpOpenRequestW(void* hConnect, const wchar_t* lpszVerb, const wchar_t* lpszObjectName, const wchar_t* lpszVersion, const wchar_t* lpszReferrer, const wchar_t** lplpszAcceptTypes, uint32_t dwFlags, uintptr_t dwContext) {
    if (!real_HttpOpenRequestW) {
        real_HttpOpenRequestW = dlsym(RTLD_NEXT, "HttpOpenRequestW");
    }
    lognet("HttpOpenRequestW", hConnect, 0);
    if (real_HttpOpenRequestW) {
        return real_HttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
    }
    return NULL;
}

// --- HttpSendRequestW ---
static int (*real_HttpSendRequestW)(void*, const wchar_t*, uint32_t, void*, uint32_t) = NULL;
int HttpSendRequestW(void* hRequest, const wchar_t* lpszHeaders, uint32_t dwHeadersLength, void* lpOptional, uint32_t dwOptionalLength) {
    if (!real_HttpSendRequestW) {
        real_HttpSendRequestW = dlsym(RTLD_NEXT, "HttpSendRequestW");
    }
    lognet("HttpSendRequestW", hRequest, dwOptionalLength);
    if (real_HttpSendRequestW) {
        return real_HttpSendRequestW(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
    }
    return 0;
}

// --- InternetReadFile ---
static int (*real_InternetReadFile)(void*, void*, uint32_t, uint32_t*) = NULL;
int InternetReadFile(void* hFile, void* lpBuffer, uint32_t dwNumberOfBytesToRead, uint32_t* lpdwNumberOfBytesRead) {
    if (!real_InternetReadFile) {
        real_InternetReadFile = dlsym(RTLD_NEXT, "InternetReadFile");
    }
    lognet("InternetReadFile", hFile, dwNumberOfBytesToRead);
    if (real_InternetReadFile) {
        return real_InternetReadFile(hFile, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
    }
    return 0;
}
#include "../include/logger.h"
#include "../include/path_utils.h"
#include "../include/performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <wchar.h>
#include <stdint.h>
#include <netdb.h>
#include <string.h>
#include <limits.h> 
#include <sys/socket.h>

#define MAX_DOMAINS 100
#define MAX_DOMAIN_LEN 256

static char log_path[PATH_MAX];
static char config_path[PATH_MAX];
static char* blocked_domains[MAX_DOMAINS];
static int num_blocked_domains = 0;

static void load_blocked_domains(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        logger_log(log_path, "Network config file not found, skipping.");
        return;
    }
    char line[MAX_DOMAIN_LEN];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        line[strcspn(line, "\n")] = 0;
        if (num_blocked_domains < MAX_DOMAINS) {
            blocked_domains[num_blocked_domains] = strdup(line);
            num_blocked_domains++;
        }
    }
    fclose(file);
    char log_buf[128];
    snprintf(log_buf, sizeof(log_buf), "Loaded %d domains from %s", num_blocked_domains, path);
    logger_log(log_path, log_buf);
}

__attribute__((constructor))
static void initialize_network_hooks() {
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        snprintf(log_path, sizeof(log_path), "%s/BarrierLayer/barrierlayer_activity.log", home_dir);
        snprintf(config_path, sizeof(config_path), "%s/BarrierLayer/network.conf", home_dir);
    } else {
        snprintf(log_path, sizeof(log_path), "./barrierlayer_activity.log");
        snprintf(config_path, sizeof(config_path), "./network.conf");
    }
    logger_log(log_path, "BarrierLayer network hooks initialized.");
    load_blocked_domains(config_path);
}

static void lognet(const char* func, void* addr, int port) {
    char buf[128];
    snprintf(buf, sizeof(buf), "NET:%s|%p:%d", func, addr, port);
    logger_log(log_path, buf);
}

// --- Corrected Signatures ---

static struct hostent* (*real_gethostbyname)(const char*) = NULL;
struct hostent* gethostbyname(const char* name) {
    if (get_performance_profile() == PROFILE_SECURITY) {
        for (int i = 0; i < num_blocked_domains; i++) {
            if (name && strcmp(name, blocked_domains[i]) == 0) {
                char log_buf[256];
                snprintf(log_buf, sizeof(log_buf), "BLOCKED: gethostbyname call for forbidden host: %s", name);
                logger_log(log_path, log_buf);
                h_errno = HOST_NOT_FOUND;
                return NULL;
            }
        }
    }
    if (!real_gethostbyname) {
        real_gethostbyname = dlsym(RTLD_NEXT, "gethostbyname");
    }
    lognet("gethostbyname", (void*)name, 0);
    if (real_gethostbyname) {
        return real_gethostbyname(name);
    }
    h_errno = HOST_NOT_FOUND;
    return NULL;
}

static int (*real_connect)(int, const struct sockaddr*, socklen_t) = NULL;
int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    if (!real_connect) {
        real_connect = dlsym(RTLD_NEXT, "connect");
    }
    lognet("connect", (void*)addr, addrlen);
    if (real_connect) {
        return real_connect(sockfd, addr, addrlen);
    }
    return -1;
}

static int (*real_bind)(int, const struct sockaddr*, socklen_t) = NULL;
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    if (!real_bind) {
        real_bind = dlsym(RTLD_NEXT, "bind");
    }
    lognet("bind", (void*)addr, addrlen);
    if (real_bind) {
        return real_bind(sockfd, addr, addrlen);
    }
    return -1;
}

static int (*real_accept)(int, struct sockaddr*, socklen_t*) = NULL;
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    if (!real_accept) {
        real_accept = dlsym(RTLD_NEXT, "accept");
    }
    lognet("accept", (void*)addr, 0);
    if (real_accept) {
        return real_accept(sockfd, addr, addrlen);
    }
    return -1;
}

static ssize_t (*real_send)(int, const void*, size_t, int) = NULL;
ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
    if (!real_send) {
        real_send = dlsym(RTLD_NEXT, "send");
    }
    lognet("send", (void*)buf, len);
    if (real_send) {
        return real_send(sockfd, buf, len, flags);
    }
    return -1;
}

static ssize_t (*real_recv)(int, void*, size_t, int) = NULL;
ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
    if (!real_recv) {
        real_recv = dlsym(RTLD_NEXT, "recv");
    }
    lognet("recv", buf, len);
    if (real_recv) {
        return real_recv(sockfd, buf, len, flags);
    }
    return -1;
}

static ssize_t (*real_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = NULL;
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
    if (!real_sendto) {
        real_sendto = dlsym(RTLD_NEXT, "sendto");
    }
    lognet("sendto", (void*)dest_addr, len);
    if (real_sendto) {
        return real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }
    return -1;
}

static ssize_t (*real_recvfrom)(int, void* restrict, size_t, int, struct sockaddr* restrict, socklen_t* restrict) = NULL;
ssize_t recvfrom(int sockfd, void* restrict buf, size_t len, int flags, struct sockaddr* restrict src_addr, socklen_t* restrict addrlen) {
    if (!real_recvfrom) {
        real_recvfrom = dlsym(RTLD_NEXT, "recvfrom");
    }
    lognet("recvfrom", src_addr, len);
    if (real_recvfrom) {
        return real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }
    return -1;
}

static struct hostent* (*real_gethostbyaddr)(const void*, socklen_t, int) = NULL;
struct hostent* gethostbyaddr(const void* addr, socklen_t len, int type) {
    if (!real_gethostbyaddr) {
        real_gethostbyaddr = dlsym(RTLD_NEXT, "gethostbyaddr");
    }
    lognet("gethostbyaddr", (void*)addr, len);
    if (real_gethostbyaddr) {
        return real_gethostbyaddr(addr, len, type);
    }
    return NULL;
}

// --- The rest of the hooks remain the same ---

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

static int (*real_getaddrinfo)(const char* restrict, const char* restrict, const struct addrinfo* restrict, struct addrinfo** restrict) = NULL;
int getaddrinfo(const char* restrict node, const char* restrict service, const struct addrinfo* restrict hints, struct addrinfo** restrict res) {
    if (!real_getaddrinfo) {
        real_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
    }
    lognet("getaddrinfo", (void*)node, 0);
    if (real_getaddrinfo) {
        return real_getaddrinfo(node, service, hints, res);
    }
    return -1;
}

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


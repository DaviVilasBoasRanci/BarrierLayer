#include "../include/logger.h"
#include "../include/path_utils.h"
#include "../include/performance.h"
#include "../include/network_hooks.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <wchar.h>
#include <stdint.h>
#include <netdb.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_RULES 100
#define MAX_LINE_LEN 512 // Increased for user agent

// Struct for redirection rules
typedef struct {
    char* domain;
    char* redirect_ip;
} RedirectRule;

static char log_path[PATH_MAX];
static char config_path[PATH_MAX];

// Rule storage
static char* blocked_domains[MAX_RULES];
static int num_blocked_domains = 0;
static RedirectRule* redirect_rules[MAX_RULES];
static int num_redirect_rules = 0;
static wchar_t* custom_user_agent = NULL;

// Static variables to hold the data for the fake hostent structure
static struct hostent fake_host;
static struct in_addr fake_addr;
static char* fake_addr_list[2];
static char* fake_aliases[1];

static void load_network_config(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        logger_log(log_path, "Network config file not found, skipping.");
        return;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        line[strcspn(line, "\n")] = 0; // Remove newline

        char* command = strtok(line, " \t");
        if (!command) continue;

        if (strcmp(command, "block") == 0) {
            char* domain = strtok(NULL, " \t");
            if (domain && num_blocked_domains < MAX_RULES) {
                blocked_domains[num_blocked_domains++] = strdup(domain);
            }
        } else if (strcmp(command, "redirect") == 0) {
            char* domain = strtok(NULL, " \t");
            char* ip = strtok(NULL, " \t");
            if (domain && ip && num_redirect_rules < MAX_RULES) {
                RedirectRule* rule = malloc(sizeof(RedirectRule));
                rule->domain = strdup(domain);
                rule->redirect_ip = strdup(ip);
                redirect_rules[num_redirect_rules++] = rule;
            }
        } else if (strcmp(command, "user_agent") == 0) {
            char* ua_string = strtok(NULL, ""); // Get the rest of the line
            if (ua_string) {
                // Skip leading whitespace
                while (*ua_string == ' ' || *ua_string == '\t') {
                    ua_string++;
                }
                
                if (custom_user_agent) free(custom_user_agent);
                
                // Convert char* to wchar_t*
                size_t needed = mbstowcs(NULL, ua_string, 0) + 1;
                custom_user_agent = malloc(needed * sizeof(wchar_t));
                if (custom_user_agent) {
                    mbstowcs(custom_user_agent, ua_string, needed);
                }
            }
        }
    }
    fclose(file);

    char log_buf[256];
    snprintf(log_buf, sizeof(log_buf), "Loaded %d block, %d redirect rules. User agent set: %s", 
        num_blocked_domains, num_redirect_rules, custom_user_agent ? "Yes" : "No");
    logger_log(log_path, log_buf);
}

__attribute__((constructor))
void initialize_network_hooks() {
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        snprintf(log_path, sizeof(log_path), "%s/BarrierLayer/barrierlayer_activity.log", home_dir);
        snprintf(config_path, sizeof(config_path), "%s/BarrierLayer/network.conf", home_dir);
    } else {
        snprintf(log_path, sizeof(log_path), "./barrierlayer_activity.log");
        snprintf(config_path, sizeof(config_path), "./network.conf");
    }
    logger_log(log_path, "BarrierLayer network hooks initialized.");
    load_network_config(config_path);
}

static void lognet(const char* func, void* addr, int port) {
    char buf[128];
    snprintf(buf, sizeof(buf), "NET:%s|%p:%d", func, addr, port);
    logger_log(log_path, buf);
}

// --- Hooked Functions ---

static struct hostent* (*real_gethostbyname)(const char*) = NULL;
struct hostent* gethostbyname(const char* name) {
    if (!name) {
        if (!real_gethostbyname) real_gethostbyname = dlsym(RTLD_NEXT, "gethostbyname");
        return real_gethostbyname(name);
    }

    if (get_performance_profile() == PROFILE_SECURITY) {
        for (int i = 0; i < num_redirect_rules; i++) {
            if (strcmp(name, redirect_rules[i]->domain) == 0) {
                char log_buf[256];
                snprintf(log_buf, sizeof(log_buf), "REDIRECT: gethostbyname call for '%s' to '%s'", name, redirect_rules[i]->redirect_ip);
                logger_log(log_path, log_buf);
                if (inet_pton(AF_INET, redirect_rules[i]->redirect_ip, &fake_addr) != 1) break;
                fake_host.h_name = (char*)name;
                fake_host.h_aliases = fake_aliases; fake_host.h_aliases[0] = NULL;
                fake_host.h_addrtype = AF_INET;
                fake_host.h_length = sizeof(struct in_addr);
                fake_addr_list[0] = (char*)&fake_addr; fake_addr_list[1] = NULL;
                fake_host.h_addr_list = fake_addr_list;
                return &fake_host;
            }
        }
        for (int i = 0; i < num_blocked_domains; i++) {
            if (strcmp(name, blocked_domains[i]) == 0) {
                char log_buf[256];
                snprintf(log_buf, sizeof(log_buf), "BLOCKED: gethostbyname call for forbidden host: %s", name);
                logger_log(log_path, log_buf);
                h_errno = HOST_NOT_FOUND;
                return NULL;
            }
        }
    }

    if (!real_gethostbyname) real_gethostbyname = dlsym(RTLD_NEXT, "gethostbyname");
    lognet("gethostbyname", (void*)name, 0);
    return real_gethostbyname(name);
}

static int (*real_HttpSendRequestW)(void*, const wchar_t*, uint32_t, void*, uint32_t) = NULL;
int HttpSendRequestW(void* hRequest, const wchar_t* lpszHeaders, uint32_t dwHeadersLength, void* lpOptional, uint32_t dwOptionalLength) {
    if (!real_HttpSendRequestW) {
        real_HttpSendRequestW = dlsym(RTLD_NEXT, "HttpSendRequestW");
    }

    const wchar_t* final_headers = lpszHeaders;
    uint32_t final_headers_len = dwHeadersLength;
    wchar_t* new_headers = NULL;

    if (custom_user_agent && get_performance_profile() == PROFILE_SECURITY) {
        const wchar_t* ua_header = L"User-Agent:";
        wchar_t* ua_start = lpszHeaders ? wcsstr(lpszHeaders, ua_header) : NULL;

        if (ua_start) {
            // User-Agent header exists, replace it
            wchar_t* ua_end = wcsstr(ua_start, L"\r\n");
            if (ua_end) {
                size_t before_len = ua_start - lpszHeaders;
                size_t after_offset = (ua_end - lpszHeaders) + 2; // +2 for \r\n
                size_t after_len = wcslen(&lpszHeaders[after_offset]);
                size_t new_ua_len = wcslen(custom_user_agent);
                
                // "User-Agent: " + new_ua + "\r\n"
                const wchar_t* ua_format = L"User-Agent: %s\r\n";
                size_t ua_part_len = wcslen(ua_format) - 2 + new_ua_len;
                wchar_t ua_part[ua_part_len + 1];
                swprintf(ua_part, ua_part_len + 1, ua_format, custom_user_agent);

                new_headers = malloc((before_len + ua_part_len + after_len + 1) * sizeof(wchar_t));
                
                wcsncpy(new_headers, lpszHeaders, before_len);
                new_headers[before_len] = L'\0';
                wcscat(new_headers, ua_part);
                wcscat(new_headers, &lpszHeaders[after_offset]);
                
                final_headers = new_headers;
                final_headers_len = wcslen(final_headers);
                logger_log(log_path, "SPOOFED: Replaced User-Agent header.");
            }
        } else {
            // User-Agent header does not exist, add it
            const wchar_t* ua_format = L"User-Agent: %s\r\n";
            size_t ua_len = wcslen(custom_user_agent);
            size_t ua_part_len = wcslen(ua_format) - 2 + ua_len;
            wchar_t ua_part[ua_part_len + 1];
            swprintf(ua_part, ua_part_len + 1, ua_format, custom_user_agent);

            size_t original_len = lpszHeaders ? wcslen(lpszHeaders) : 0;
            new_headers = malloc((original_len + ua_part_len + 1) * sizeof(wchar_t));

            if (lpszHeaders) wcscpy(new_headers, lpszHeaders);
            else new_headers[0] = L'\0';
            wcscat(new_headers, ua_part);

            final_headers = new_headers;
            final_headers_len = wcslen(final_headers);
            logger_log(log_path, "SPOOFED: Added custom User-Agent header.");
        }
    }

    int result = real_HttpSendRequestW(hRequest, final_headers, final_headers_len, lpOptional, dwOptionalLength);

    if (new_headers) {
        free(new_headers);
    }

    return result;
}

// --- Other hooks (no changes) ---

static int (*real_connect)(int, const struct sockaddr*, socklen_t) = NULL;
int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    if (!real_connect) real_connect = dlsym(RTLD_NEXT, "connect");
    lognet("connect", (void*)addr, addrlen);
    return real_connect ? real_connect(sockfd, addr, addrlen) : -1;
}

static int (*real_bind)(int, const struct sockaddr*, socklen_t) = NULL;
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    if (!real_bind) real_bind = dlsym(RTLD_NEXT, "bind");
    lognet("bind", (void*)addr, addrlen);
    return real_bind ? real_bind(sockfd, addr, addrlen) : -1;
}

static int (*real_accept)(int, struct sockaddr*, socklen_t*) = NULL;
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    if (!real_accept) real_accept = dlsym(RTLD_NEXT, "accept");
    lognet("accept", (void*)addr, 0);
    return real_accept ? real_accept(sockfd, addr, addrlen) : -1;
}

static ssize_t (*real_send)(int, const void*, size_t, int) = NULL;
ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
    if (!real_send) real_send = dlsym(RTLD_NEXT, "send");
    lognet("send", (void*)buf, len);
    return real_send ? real_send(sockfd, buf, len, flags) : -1;
}

static ssize_t (*real_recv)(int, void*, size_t, int) = NULL;
ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
    if (!real_recv) real_recv = dlsym(RTLD_NEXT, "recv");
    lognet("recv", buf, len);
    return real_recv ? real_recv(sockfd, buf, len, flags) : -1;
}

static ssize_t (*real_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = NULL;
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
    if (!real_sendto) real_sendto = dlsym(RTLD_NEXT, "sendto");
    lognet("sendto", (void*)dest_addr, len);
    return real_sendto ? real_sendto(sockfd, buf, len, flags, dest_addr, addrlen) : -1;
}

static ssize_t (*real_recvfrom)(int, void* restrict, size_t, int, struct sockaddr* restrict, socklen_t* restrict) = NULL;
ssize_t recvfrom(int sockfd, void* restrict buf, size_t len, int flags, struct sockaddr* restrict src_addr, socklen_t* restrict addrlen) {
    if (!real_recvfrom) real_recvfrom = dlsym(RTLD_NEXT, "recvfrom");
    lognet("recvfrom", src_addr, len);
    return real_recvfrom ? real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen) : -1;
}

static struct hostent* (*real_gethostbyaddr)(const void*, socklen_t, int) = NULL;
struct hostent* gethostbyaddr(const void* addr, socklen_t len, int type) {
    if (!real_gethostbyaddr) real_gethostbyaddr = dlsym(RTLD_NEXT, "gethostbyaddr");
    lognet("gethostbyaddr", (void*)addr, len);
    return real_gethostbyaddr ? real_gethostbyaddr(addr, len, type) : NULL;
}

static int (*real_WSAStartup)(uint16_t, void*) = NULL;
int WSAStartup(uint16_t wVersionRequested, void* lpWSAData) {
    if (!real_WSAStartup) real_WSAStartup = dlsym(RTLD_NEXT, "WSAStartup");
    lognet("WSAStartup", (void*)(uintptr_t)wVersionRequested, 0);
    return real_WSAStartup ? real_WSAStartup(wVersionRequested, lpWSAData) : 0;
}

static int (*real_WSACleanup)(void) = NULL;
int WSACleanup(void) {
    if (!real_WSACleanup) real_WSACleanup = dlsym(RTLD_NEXT, "WSACleanup");
    lognet("WSACleanup", 0, 0);
    return real_WSACleanup ? real_WSACleanup() : 0;
}

static int (*real_socket)(int, int, int) = NULL;
int socket(int domain, int type, int protocol) {
    if (!real_socket) real_socket = dlsym(RTLD_NEXT, "socket");
    lognet("socket", (void*)(uintptr_t)domain, type);
    return real_socket ? real_socket(domain, type, protocol) : -1;
}

static int (*real_closesocket)(int) = NULL;
int closesocket(int sockfd) {
    if (!real_closesocket) real_closesocket = dlsym(RTLD_NEXT, "closesocket");
    lognet("closesocket", (void*)(uintptr_t)sockfd, 0);
    return real_closesocket ? real_closesocket(sockfd) : -1;
}

static int (*real_getaddrinfo)(const char* restrict, const char* restrict, const struct addrinfo* restrict, struct addrinfo** restrict) = NULL;
int getaddrinfo(const char* restrict node, const char* restrict service, const struct addrinfo* restrict hints, struct addrinfo** restrict res) {
    if (!real_getaddrinfo) real_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
    lognet("getaddrinfo", (void*)node, 0);
    return real_getaddrinfo ? real_getaddrinfo(node, service, hints, res) : -1;
}

static void* (*real_InternetOpenW)(const wchar_t*, uint32_t, const wchar_t*, const wchar_t*, uint32_t) = NULL;
void* InternetOpenW(const wchar_t* lpszAgent, uint32_t dwAccessType, const wchar_t* lpszProxy, const wchar_t* lpszProxyBypass, uint32_t dwFlags) {
    if (!real_InternetOpenW) real_InternetOpenW = dlsym(RTLD_NEXT, "InternetOpenW");
    lognet("InternetOpenW", (void*)lpszAgent, dwAccessType);
    return real_InternetOpenW ? real_InternetOpenW(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags) : NULL;
}

static void* (*real_InternetConnectW)(void*, const wchar_t*, uint16_t, const wchar_t*, const wchar_t*, uint32_t, uint32_t, uintptr_t) = NULL;
void* InternetConnectW(void* hInternet, const wchar_t* lpszServerName, uint16_t nServerPort, const wchar_t* lpszUserName, const wchar_t* lpszPassword, uint32_t dwService, uint32_t dwFlags, uintptr_t dwContext) {
    if (!real_InternetConnectW) real_InternetConnectW = dlsym(RTLD_NEXT, "InternetConnectW");
    lognet("InternetConnectW", hInternet, nServerPort);
    return real_InternetConnectW ? real_InternetConnectW(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext) : NULL;
}

static void* (*real_HttpOpenRequestW)(void*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t**, uint32_t, uintptr_t) = NULL;
void* HttpOpenRequestW(void* hConnect, const wchar_t* lpszVerb, const wchar_t* lpszObjectName, const wchar_t* lpszVersion, const wchar_t* lpszReferrer, const wchar_t** lplpszAcceptTypes, uint32_t dwFlags, uintptr_t dwContext) {
    if (!real_HttpOpenRequestW) real_HttpOpenRequestW = dlsym(RTLD_NEXT, "HttpOpenRequestW");
    lognet("HttpOpenRequestW", hConnect, 0);
    return real_HttpOpenRequestW ? real_HttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext) : NULL;
}

static int (*real_InternetReadFile)(void*, void*, uint32_t, uint32_t*) = NULL;
int InternetReadFile(void* hFile, void* lpBuffer, uint32_t dwNumberOfBytesToRead, uint32_t* lpdwNumberOfBytesRead) {
    if (!real_InternetReadFile) real_InternetReadFile = dlsym(RTLD_NEXT, "InternetReadFile");
    lognet("InternetReadFile", hFile, dwNumberOfBytesToRead);
    return real_InternetReadFile ? real_InternetReadFile(hFile, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead) : 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <errno.h>
#include <seccomp.h>
#include "sandbox_core.h"

static void setup_seccomp_filters(void) {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL) {
        perror("seccomp_init failed");
        exit(1);
    }

    printf("SANDBOX_CORE: Adding seccomp rules...\n");
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(mount), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(reboot), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(kexec_load), 0);

    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load failed");
        seccomp_release(ctx);
        exit(1);
    }
    seccomp_release(ctx);
}

static void setup_network_isolation(void) {
    printf("SANDBOX_CORE: Setting up network isolation...\n");
    if (system("ip link set lo up") != 0) {
        fprintf(stderr, "SANDBOX_CORE: WARNING: Could not bring up loopback interface.\n");
    }
}

// Filesystem setup is now handled by the parent launcher before clone().
// This function now only applies settings that must be done from within the child process.
int initialize_sandbox(void) {
    printf("SANDBOX_CORE: Initializing final settings in child.\n");
    
    setup_seccomp_filters();
    setup_network_isolation();

    return 0;
}
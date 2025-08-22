#include <stdio.h>
#include <sys/mount.h>

int main() {
    printf("TEST_SECCOMP: Attempting to call mount()...\n");

    // We expect this call to be blocked by the seccomp filter.
    // The arguments don't need to be valid, as the syscall itself should be blocked.
    int result = mount("none", "/tmp", "tmpfs", 0, "");

    if (result == 0) {
        printf("TEST_SECCOMP: SUCCESS! Mount call was not blocked! (This is an error in the test)\n");
    } else {
        // We expect to get here, but the process should have been killed by the kernel
        // or the syscall should have returned an error.
        perror("TEST_SECCOMP: Mount call failed as expected");
    }

    printf("TEST_SECCOMP: This line should not be reached if the filter works with SCMP_ACT_KILL.\n");
    return 0;
}

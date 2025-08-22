#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main() {
    printf("UID: %d\n", getuid());
    printf("EUID: %d\n", geteuid());
    printf("GID: %d\n", getgid());
    printf("EGID: %d\n", getegid());

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Hostname: %s\n", hostname);
    } else {
        perror("gethostname");
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current Working Directory: %s\n", cwd);
    } else {
        perror("getcwd");
    }

    // Attempt to create a file outside the sandbox (e.g., in /tmp)
    const char *test_file_path = "/tmp/sandbox_test_file.txt";
    int fd = open(test_file_path, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (fd == -1) {
        printf("Attempt to create file %s: FAILED (%s)\n", test_file_path, strerror(errno));
    } else {
        printf("Attempt to create file %s: SUCCESS\n", test_file_path);
        close(fd);
        unlink(test_file_path); // Clean up
    }

    return 0;
}

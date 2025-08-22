#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("TEST_OVERLAY: Attempting to create a file at /overlay_test.txt...\n");

    FILE *f = fopen("/overlay_test.txt", "w");
    if (f == NULL) {
        perror("TEST_OVERLAY: Failed to create file");
        return 1;
    }

    fprintf(f, "This is a test file from inside the overlay sandbox.\n");
    fclose(f);

    printf("TEST_OVERLAY: Successfully created and wrote to /overlay_test.txt.\n");
    return 0;
}

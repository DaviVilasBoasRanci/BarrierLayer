#include <stdio.h>
#include <stdlib.h>

int main() {
    int result;

    printf("TEST_NETWORK: Pinging localhost (127.0.0.1)...");
    // This should succeed if the loopback interface is up.
    result = system("ping -c 1 127.0.0.1");
    if (result == 0) {
        printf("TEST_NETWORK: SUCCESS - Ping to localhost was successful.\n");
    } else {
        printf("TEST_NETWORK: FAILURE - Ping to localhost failed.\n");
    }

    printf("\nTEST_NETWORK: Pinging external address (8.8.8.8)...");
    // This should fail as there should be no external network connectivity.
    result = system("ping -c 1 8.8.8.8");
    if (result == 0) {
        printf("TEST_NETWORK: FAILURE - Ping to external address succeeded!\n");
    } else {
        printf("TEST_NETWORK: SUCCESS - Ping to external address failed as expected.\n");
    }

    return 0;
}
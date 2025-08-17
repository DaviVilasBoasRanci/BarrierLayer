#include <stdio.h>
#include <unistd.h> // For execl
#include <stdlib.h> // For exit

int main() {
    printf("=== Teste Simples do BarrierLayer (Executando ls) ===\n");
    
    // Try to execute ls from within the chroot's /bin
    execl("/bin/ls", "ls", NULL);
    
    // If execl returns, it means it failed
    perror("execl failed");
    return 1;
}

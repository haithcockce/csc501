#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void main() {
    int flags = O_WRONLY;
    printf("O_WRONLY: %d, O_CREAT: %d, O_TRUNC: %d\n", O_WRONLY, O_CREAT, O_TRUNC);
    flags |= O_CREAT | O_TRUNC;
    printf("%d\n", flags);
}

    

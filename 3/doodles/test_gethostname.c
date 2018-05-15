#include "socklib.h"

void main() {
    char host_test1[64];
    char* host_test2 = (char*) malloc(64);

    gethostname(host_test1, sizeof(host_test1));
    printf("Array: %s\n", host_test1);

    gethostname(host_test2, 64);
    printf("Malloc: %s\n", host_test2);
}

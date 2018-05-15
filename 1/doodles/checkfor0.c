#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int* a = (int*) malloc(100000 * sizeof(int));
    int i;
    for(i = 0; i < 100000; i++) {
        if(a[i] != 0) {
            printf("not 0 at %d\n: %x", i, a[i]);
            break;
        }
    }
    a = memset(a, 0, 100000 * sizeof(int));
    for(i = 0; i < 100000; i++) {
        if(a[i] != 0) {
            printf("not 0 at %d\n: %x", i, a[i]);
            break;
        }
    }
    free(a);
    return 0;
}

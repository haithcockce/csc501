#include <stdio.h>
#include <stdlib.h>

void main() {
   
    int i;
    for(i = 0; i < 10; i++) {
        printf("mod 10, %d, mod 10000, %d\n", random() % 10, random() % 10000);
    }
}


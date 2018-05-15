#include <stdio.h>

#define TRUE 1

void main() {
    printf("Reg: %d, Neg: %d\n", TRUE, !TRUE);
    if(TRUE) {
        printf("TRUE\n");
    }
    if(!TRUE) {
        printf("!TRUE\n");
    }
}

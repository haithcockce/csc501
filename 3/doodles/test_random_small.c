#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define RANDOM_NUMBER(limit) (srand(clock()), (rand() % limit))

void main() {
    printf("Testing left/right: %d\n", RANDOM_NUMBER(2));
    printf("Testing left/right: %d\n", RANDOM_NUMBER(2));
    printf("Testing left/right: %d\n", RANDOM_NUMBER(2));
    printf("Testing left/right: %d\n", RANDOM_NUMBER(2));
    printf("Testing left/right: %d\n", RANDOM_NUMBER(2));
}

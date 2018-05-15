#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define random_number(limit) (srand(clock()), (rand() % limit))

void main() {
    int i; 
    for(i = 0; i < 10; i++) {
        printf("Limit: %d, Random number: %d\n", 10, random_number(10));
        printf("Limit: %d, Random number: %d\n", 10000, random_number(10000));
    }
}

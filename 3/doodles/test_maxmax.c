#include <stdio.h>


#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MAXMAX(A, B, C, D) (MAX(MAX(A, B), MAX(C, D)))   

void main() {
    printf("MAXMAX(1, 3, 5, 7): %u\n", MAXMAX(1, 3, 5, 7));
    printf("MAXMAX(7, 3, 5, 1): %u\n", MAXMAX(7, 3, 5, 1));
    printf("MAXMAX(1, 3, 5, 2): %u\n", MAXMAX(1, 3, 5, 2));
    printf("MAXMAX(1, 9, 5, 7): %u\n", MAXMAX(1, 9, 5, 7));
}

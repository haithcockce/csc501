#include <stdio.h>

void main() {
    char a = 'b';

    switch(a) {
        case 'b':
            printf("Hit case 'b'\n");
        case 'a':
            printf("Hit case 'a'\n");
            break;
        default:
            printf("No case hit\n");
            break;
    }
}


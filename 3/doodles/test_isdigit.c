#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void main() {
    const char* s = "9201";
    if(isdigit(*s)) {
        printf("TRUE\n");
    }
    else {
        printf("FALSE\n");
    }


    s = "nope";
    if(isdigit(*s)) {
        printf("TRUE\n");
    }
    else {
        printf("FALSE\n");
    }

    s = "999nope";
    if(isdigit(*s)) {
        printf("TRUE\n");
    }
    else {
        printf("FALSE\n");
    }

    s = "nope999";
    if(isdigit(*s)) {
        printf("TRUE\n");
    }
    else {
        printf("FALSE\n");
    }

}


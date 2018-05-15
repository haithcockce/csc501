#include <stdio.h>

void main() {
    char* strings[] = {"Dog", "Cat", "Bear", NULL};
    char** str_ptr = strings;

    while(*str_ptr) {
        printf("str: %s\n", *str_ptr);
        str_ptr++;
    }
    str_ptr = (strings + 1);
    printf("str: %s\n", *str_ptr);
}

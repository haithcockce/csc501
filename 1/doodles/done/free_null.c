#include <stdio.h>
#include <stdlib.h>

typedef struct my_struct {
    char * member;
} my_struct;

int main() {

    my_struct a;
    a.member = (char*) malloc(64);
    a.member[0] = 1;
    free(a.member);
    a.member = NULL;
    if(a.member == NULL) {
        printf("Success\n");
    }
    return 0;
}

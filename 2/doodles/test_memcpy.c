#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct my_s {
    char* s;
    int i;
} my_s;

my_s* struct1;
my_s* struct2;
my_s* struct3;

void main() {
    struct1 = (my_s*) malloc(sizeof(my_s));
    struct1->s = "TEST STRING";
    struct1->i = 10;

    struct2 = (my_s*) malloc(sizeof(my_s));
    memcpy(struct2, struct1, sizeof(my_s));
    printf("struct2 str: %s\n", struct2->s);

    memcpy(struct3, struct1, sizeof(my_s));
    printf("BEFORE PRINTF\n");
    printf("struct3 str: %s\n", struct3->s);
}

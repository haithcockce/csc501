#include <stdio.h>
#include <stdlib.h>

typedef struct myint {
    int a;
} myint;

int main() {
    myint* a = (myint*) malloc(sizeof(myint));
    a->a = 10;
    printf("%d\n", a->a);
    a->a--;
    printf("%d\n", a->a);
    return 0;
}    

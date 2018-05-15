#include <stdio.h>
#include "bool_test.h"

#ifndef TRUE
#define TRUE 10
#endif 

#ifndef FALSE
#define FALSE -10
#endif

int main() {
    printf("%d\n", TRUE);
    printf("%d\n", FALSE);
    return 0;
}

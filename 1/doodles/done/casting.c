#include <stdio.h>
#include <stdlib.h>
#include "casting.h"


typedef struct __my_type {
    int id;
} __my_type;


int main() { 
    my_type x;

    x = init();
    calculate_stuff(x);
    return 0;
}

my_type init() {
    __my_type * __thing = (__my_type *) malloc(sizeof(__my_type));
    __thing->id = 100;
    return (my_type) __thing;
}

void calculate_stuff(my_type thing) {
    __my_type * __thing = (__my_type *) thing;
    printf("%d \n", __thing->id);
    __thing->id = 200;
    printf("%d \n", __thing->id);
}

#include <stdio.h>
#include <math.h>

void main() {
    printf("%f\n", sizeof(unsigned int));
    printf("%f\n", pow(2.0, ((double) sizeof(unsigned int) * 8)));
}

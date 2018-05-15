#include <stdio.h>

void main() {
    char buff[10];
    int i; 
    for(i = 0; i < 10; (buff[i] = i, i++));
    for(i = 0; i < 10; i++)
        printf("buff[%d] == %d\n", i, buff[i]);
}

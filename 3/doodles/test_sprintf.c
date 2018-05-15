#include <stdio.h>
#include <string.h>

#define TEST_STR "test"

void main() {
    char str[4];
    sprintf(str, "%d", -10);
    printf("%s\n", str);

    char* s = TEST_STR;
    printf("%s, %d\n", TEST_STR, strlen(TEST_STR));
}

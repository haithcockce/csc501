#include "socklib.h"

#define IS_NULL(x) (x == NULL ? true : false)

void main() {
    char* should_be_null;
    char shouldnt_be_null[4] = "asd";
    if(IS_NULL(should_be_null)) {
        printf("SUCCESS1\n");
    }
    if(!IS_NULL(shouldnt_be_null)) {
        printf("SUCCESS2\n");
    }
}

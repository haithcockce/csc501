#include <stdio.h>
#include <ucontext.h>


 typedef struct __my_t {
     unsigned int tid;                 
     struct __my_t* parent;           
     int status;                     
     struct __my_t* child_list[8192];           
     unsigned int ct_cnt;           
     ucontext_t context;           
     unsigned int wc_tid;       
 } __my_t;

int main() {
    printf("%d\n", sizeof(__my_t));
return 0;
}

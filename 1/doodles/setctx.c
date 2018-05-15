#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

void f2();
void f1();
void setup_uctx();

typedef struct Wrapper {
    ucontext_t context;
} Wrapper;

#define STACK_SIZE (8 * 1024)
ucontext_t uctx_main;
ucontext_t uctx_func1;
ucontext_t uctx_func2;
Wrapper* mainf;
Wrapper* func1;
Wrapper* func2;


void f1() {
    printf("Function 1 entered\n");
    printf("Function 1 switching to Main\n");

    //swapcontext(&uctx_func1, &uctx_func2);
    free(func1);
    setcontext(&(mainf->context));
}

void f2() {
    printf("Function 2 entered\n");
    printf("Function 2 switching to Function 1\n");

    //swapcontext(&uctx_func2, &uctx_func1);
    free(func2);
    setcontext(&(func1->context));
    printf("Function 2 returning\n");
}


int main(int argc, char* argv[]) {
    //char* func1_stack = (char*) malloc(STACK_SIZE);
    //char* func2_stack = (char*) malloc(STACK_SIZE);
    char func1_stack[STACK_SIZE];
    char func2_stack[STACK_SIZE];

    mainf = (Wrapper*) malloc(sizeof(Wrapper));
    func1 = (Wrapper*) malloc(sizeof(Wrapper));
    func2 = (Wrapper*) malloc(sizeof(Wrapper));
    
    getcontext(&uctx_func1);
    //uctx_func1.uc_stack.ss_sp = func1_stack;
    //uctx_func1.uc_stack.ss_size = sizeof(func1_stack);
    uctx_func1.uc_stack.ss_sp = malloc(STACK_SIZE);
    uctx_func1.uc_stack.ss_size = STACK_SIZE;
    uctx_func1.uc_link = &uctx_main;
    makecontext(&uctx_func1, f1, 0);

    getcontext(&uctx_func2);
    //uctx_func2.uc_stack.ss_sp = func2_stack;
    //uctx_func2.uc_stack.ss_size = sizeof(func2_stack);
    uctx_func2.uc_stack.ss_sp = malloc(STACK_SIZE);
    uctx_func2.uc_stack.ss_size = STACK_SIZE;
    uctx_func2.uc_link = &uctx_func1;
    makecontext(&uctx_func2, f2, 0);

    printf("Main swapping to Function 2\n");
    mainf->context = uctx_main;
    func1->context = uctx_func1;
    func2->context = uctx_func2;
    swapcontext(&(mainf->context), &(func2->context));
    
    printf("Main is exiting\n");

}

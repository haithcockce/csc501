#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void list_with_ints();

int main(int argc, char *argv[])
{
    printf("Loading int demo...\n");
    list_with_ints();
}

void list_with_ints()
{
    int i;
    GenericLinkedList* list = new_linked_list(sizeof(int), NULL);

    for(i = 0; i < 5; i++) {
        int* n = (int*) malloc(sizeof(int));
        *n = i;
        ll_append(list, n);
    }

    ll_node* ptr = list->head;


    printf("DEBUG: Printing all ints.\n");
    i = 0;
    while(ptr != NULL) {
        printf("Should be %d, found %d\n", i, *((int*)ptr->data));
        ptr = ptr->next;
        i++;
    }
    printf("\n\n\n");


    printf("DEBUG: Testing pop 5 times\n");
    for(i = 0; i < 5; i++) {
        int* n = (int*) malloc(sizeof(int));
        ll_pop(list, n);
        printf("Should be %d, found %d\n", i, *n);
        free(n);
    }
    printf("DEBUG: Testing pop when nothing to pop\n");
    printf("Should be 'List is empty', found ");
    int* n = (int*) malloc(sizeof(int));
    ll_pop(list, n);
    free(n);
    printf("\n\n\n");


    printf("DEBUG: Testing push 5 times\n");
    for(i = 0; i < 5; i++) {
        int* n = (int*) malloc(sizeof(int));
        *n = i;
        ll_push(list, n);
    }
    i = 4;
    ptr = list->head;
    while(ptr != NULL) {
        printf("Should be %d, found %d\n", i, *((int*)ptr->data));
        ptr = ptr->next;
        i--;
    }
    printf("\n\n\n");


    printf("DEBUG: Testing remove_from_end 5 times\n");
    n = (int*) malloc(sizeof(int));
    for(i = 0; i < 5; i++) {
        ll_remove_from_end(list, n);
        printf("Should be %d, found %d\n", i, *n);
    }
    printf("DEBUG: Testing remove when nothing to remove.\n");
    printf("Should be 'List is empty' found ");
    ll_remove_from_end(list, n);
    free(n);
    printf("\n\n\n");


    printf("DEBUG: Testing destroy\n");
    for(i = 0; i < 5; i++) {
        int* n = (int*) malloc(sizeof(int));
        *n = i;
        ll_append(list, n);
    }
    ll_destroy(list);
}


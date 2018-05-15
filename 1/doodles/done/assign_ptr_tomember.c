#include <stdio.h>                                                                                               
#include <stdlib.h>

typedef struct Node {
    char * list;
    struct Node* next;
} Node;

int main() {

    Node* mynode = (Node*) malloc(sizeof(Node));
    char* l = (char*) malloc(1000);
    mynode->list = l;
    mynode->list[0] = 'a';
    printf("%c\n", mynode->list[0]);
    free(mynode);
}

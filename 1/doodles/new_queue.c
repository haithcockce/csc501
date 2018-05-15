#include <stdio.h>
#include <stdlib.h>
#include <mythread.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


typedef struct Node {
    MyThread* t;
    struct Node* next;
} Node;

typedef struct {
    int length; 
    Node* head;
    Node* tail;
} Queue;

Queue* setup_queue();
void enqueue(Queue* q, MyThread* t);
int queue_is_empty(Queue* q);
int queue_length(Queue* q); 
void print_queue(Queue* q);
int* dequeue(Queue* q);

int main() {

    Queue* queue = setup_queue();
    printf("%d\n", queue->length);
    enqueue(queue, 1);
    printf("%d\n", queue->length);
    enqueue(queue, 2);
    printf("%d\n", queue->length);
    int* a = dequeue(queue);
    int* b = dequeue(queue);
    printf("%d\n", *a);
    printf("%d\n", *b);
    
    
    print_queue(queue);
    free(queue);

    return 0;
}

Queue* setup_queue() {
    Queue* to_ret = (Queue*) malloc(sizeof(Queue));
    to_ret->head = NULL;
    to_ret->tail = NULL;
    to_ret->length = 0;
    return to_ret;
}

void enqueue(Queue* q, MyThread* t) {
    Node* temp = (Node*) malloc(sizeof(Node));
    temp->data = t;
    temp->next = NULL;

    if(queue_is_empty(q)) {
        q->head = temp;
        q->tail = temp;
    }
    else {
        q->tail->next = temp;
        q->tail = temp;
    }
    q->length++;
        
}

int* dequeue(Queue* q) {
    if (queue_is_empty(q)) {
        return NULL;
    }
    /*  */
    int* data = q->head.data;
    q->head = q->head->next;
    /* You do not need to handle 
     * freeing of the things here
     * because the thread engine's
     * respondibility will be to
     * remove the thread info 
     * when the thread is done. */
    return data;
}

int queue_length(Queue* q) { 
    return q->length;
}

int queue_is_empty(Queue* q) {
    if(queue_length(q) == 0) {
        return TRUE;
    }
    return FALSE;
}

void print_queue(Queue* q) {
    Node* temp = q->head;
    while(temp != NULL) {
        printf("%d\n", temp->data);
        temp = temp->next;
    }
}

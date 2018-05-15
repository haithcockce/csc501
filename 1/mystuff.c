#include <stdio.h>
#include <stdlib.h>
#include "mystuff.h"

/*
 * A library for my library ¯\_(ツ)_/¯
 * Largely used for housing List operations
 */
int len(List* l) {
    return l->length;
}

List* setup_list() {
    List* to_ret = (List*) malloc(sizeof(List));
    to_ret->head = NULL;
    to_ret->tail = NULL;
    to_ret->length = 0;
    return to_ret;
}

List* setup_queue() {
    return setup_list();
}

void enqueue(List* l, __my_t* t) {
    Node* n = (Node*) malloc(sizeof(Node));
    n->t = t;
    n->next = NULL;

    if(is_empty(l)) {
        n->prev = NULL;
        l->head = n;
        l->tail = n;
    }
    else {
        n->prev = l->tail;
        l->tail->next = n;
        l->tail = n;
    }
    l->length++;
}

__my_t* dequeue(List* l) {
    if(is_empty(l)) {
        return NULL;
    }
    Node* to_free = l->head;
    __my_t* t = to_free->t;
    if(l->length == 1) {
        l->head = NULL;
        l->tail = NULL;
    }
    else {
        l->head = l->head->next;
    }
    free(to_free);
    l->length--;
    return t;
}

void remove_from_list(List* l, __my_t* t) {
    unsigned int tid = t->tid;
    //If the queue is empty, return
    if(is_empty(l)) {
        return;
    }
    Node* iterator = l->head;
    Node* to_free;
    //If the list has one item and we found the item to remove:
    if(iterator->t->tid == tid) {
        dequeue(l);
        return;
    }
    //if the list has 2 or more items:
    while(iterator != NULL && iterator->next != NULL) {
        if(iterator->next->t->tid == tid) {
            to_free = iterator->next;
            iterator->next = to_free->next;
            //If we are somewhere in the middle of the list
            if(to_free->next != NULL) {
                to_free->next->prev = iterator;
            }
            to_free->next = NULL;
            to_free->prev = NULL;
            to_free->t = NULL;
            free(to_free);
            l->length--;
            return;
        }
        //continue searching for the next one
        iterator = iterator->next;
    }
        
}

int is_empty(List* l) {
    if(l->length == 0) {
        return TRUE;
    }
    return FALSE;
}

__my_t* peek(List* l) {
    if(is_empty(l)) {
        return NULL;
    }
    return l->head->t;
}



/*
 * Semaphore list operations
 */
SList* setup_slist() {
    SList* to_ret = (SList*) malloc(sizeof(SList));
    to_ret->head = NULL;
    to_ret->tail = NULL;
    to_ret->length = 0;
    return to_ret;
}

int s_is_empty(SList* l) {
    if(l->length == 0) {
        return TRUE;
    }
    return FALSE;
}

SNode* speek(SList* l) {
    if(s_is_empty(l)) {
        return NULL;
    }
    return l->head;
}

void senqueue(SList* l, __sem* s) {
    SNode* n = (SNode*) malloc(sizeof(SNode));
    n->s = s;
    n->next = NULL;

    if(s_is_empty(l)) {
        n->prev = NULL;
        l->head = n;
        l->tail = n;
    }
    else {
        n->prev = l->tail;
        l->tail->next = n;
        l->tail = n;
    }
    l->length++;
}

__sem* sdequeue(SList* l) {
    if(s_is_empty(l)) {
        return NULL;
    }
    SNode* to_free = l->head;
    __sem* s = to_free->s;
    if(l->length == 1) {
        l->head = NULL;
        l->tail = NULL;
    }
    else {
        l->head = l->head->next;
    }
    free(to_free);
    l->length--;
    return s;
}

void remove_from_slist(SList* l, __sem* s) {
    unsigned int sid = s->sid;
    //If the queue is empty, return
    if(s_is_empty(l)) {
        return;
    }
    SNode* iterator = l->head;
    SNode* to_free;
    //If the list has one item and we found the item to remove:
    if(iterator->s->sid == sid) {
        sdequeue(l);
        return;
    }
    //if the list has 2 or more items:
    while(iterator != NULL && iterator->next != NULL) {
        if(iterator->next->s->sid == sid) {
            to_free = iterator->next;
            iterator->next = to_free->next;
            //If we are somewhere in the middle of the list
            if(to_free->next != NULL) {
                to_free->next->prev = iterator;
            }
            to_free->next = NULL;
            to_free->prev = NULL;
            to_free->s = NULL;
            free(to_free);
            l->length--;
            return;
        }
        //continue searching for the next one
        iterator = iterator->next;
    }       
}



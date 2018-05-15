#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GenericLinkedList* new_linked_list(unsigned int type_size, free_function func) {
	if(type_size < 1) {
		printf("Size needs to be 1 Byte or larger\n");
		return NULL;
	}

	GenericLinkedList* to_return = (GenericLinkedList*) malloc(sizeof(GenericLinkedList));
	to_return->length = 0;
	to_return->type_size = type_size;
	to_return->head = NULL;
	to_return->tail = NULL;
	to_return->curr = NULL;
	to_return->free_func = func;
	return to_return;
}

void ll_push(GenericLinkedList* list, void* element) {

	// Create the new node and copy the data into the node
	ll_node* to_add = (ll_node*) malloc(sizeof(ll_node));
	to_add->data = malloc(list->type_size);
	memcpy(to_add->data, element, list->type_size);

	// Set the tail to point to the same thing if it's the only node
	if(list->length == 0) {
        list->head = to_add;
		list->tail = list->head;
	}
    else {
        // Set the head to the new node
        to_add->next = list->head;
        to_add->prev = NULL;
        list->head->prev = to_add;
        list->head = to_add;
    }

	list->length++;
}

void ll_pop(GenericLinkedList* list, void* element) {

	// Sanity check the list first
	if(list->length == 0 || list->head == NULL) {
		printf("List is empty\n");
		return;
	}

	// Copy the data to a placeholder we will end up returning later
    if(element != NULL) {
    	memcpy(element, list->head->data, list->type_size);
    }

	// Adjust pointers and count appropriately
    if(list->curr == list->head) {
        list->curr = NULL;
    } 

    ll_node* to_remove = list->head;
	list->head = list->head->next;
	list->length--;

	// Free yo shit
	if(list->free_func) {
		list->free_func(to_remove->data);
	}
	free(to_remove->data);
	free(to_remove);
    if(list->head != NULL) {
	    list->head->prev = NULL;
    }
}

void ll_append(GenericLinkedList* list, void* element) {

    // Create the new node and copy the data into the node
    ll_node* to_add = (ll_node*) malloc(sizeof(ll_node));
    to_add->data = malloc(list->type_size);
    memcpy(to_add->data, element, list->type_size);
    
    // If we are the only thing in this list
    if(list->length == 0) {
        list->head = to_add;
        list->tail = list->head;
    }

    // Adjust pointers for the list and node
    else {
        to_add->prev = list->tail;
        to_add->next = NULL;
        list->tail->next = to_add;
        list->tail = to_add;
    }

    list->length++;
}

void ll_remove_from_end(GenericLinkedList* list, void* element) {

    // Sanity check the list first
    if(list->length == 0 || list->tail == NULL) {
        printf("List is empty\n");
        return;
    }

    // Copy the data to a placeholder we will end up returning later
    if(element != NULL) {
        memcpy(element, list->tail->data, list->type_size);
    }

    // Adjust pointers and count appropriately
    if(list->curr == list->tail) {
        list->curr = NULL;
    }
    ll_node* to_remove = list->tail;
    list->tail = list->tail->prev;
    list->length--;

    // Free yo shit
    if(list->free_func) {
        list->free_func(to_remove->data);
    }
    free(to_remove->data);
    free(to_remove);
    if(list->tail != NULL) {
        list->tail->next = NULL;
    }
}

void ll_destroy(GenericLinkedList* list) {

    while(list->length > 0) {
        ll_pop(list, NULL);
    }
}

void ll_head(GenericLinkedList* list, void* element) {

    // Sanity check the list first
    if(list->length == 0 || list->head == NULL) {
        printf("List is empty\n");
        return;
    }

    memcpy(element, list->head->data, list->type_size);
}

void ll_tail(GenericLinkedList* list, void* element) {

    // Sanity check the list first
    if(list->length == 0 || list->tail == NULL) {
        printf("List is empty\n");
        return;
    }

    memcpy(element, list->tail->data, list->type_size);
}

unsigned int ll_length(GenericLinkedList* list) {
    return list->length;
}

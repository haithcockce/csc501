#ifndef __GEN_LL_H
#define __GEN_LL_H


typedef int BOOL;

#ifndef TRUE
    #define TRUE 1
#endif 

#ifndef FALSE
    #define FALSE 0
#endif

typedef void (*free_function)(void*);
typedef BOOL (*list_iteractor)(void*);

typedef struct ll_node {
	void *data;
	struct ll_node* next;
	struct ll_node* prev;
} ll_node;

typedef struct GenericLinkedList{
	unsigned int length;
	unsigned int type_size;
	ll_node* head;
	ll_node* tail;
	ll_node* curr;
	free_function free_func;
} GenericLinkedList;

GenericLinkedList* new_linked_list(unsigned int type_size, free_function func);
void ll_destroy(GenericLinkedList* list);
void ll_push(GenericLinkedList* list, void* element);
void ll_pop(GenericLinkedList* list, void* element);
void ll_append(GenericLinkedList* list, void* element);
void ll_remove_from_end(GenericLinkedList* list, void* element);
unsigned int ll_length(GenericLinkedList* list);
 
// void ll_for_each(GenericLinkedList* list, list_iterator* iterator);
void ll_head(GenericLinkedList* list, void* element);
void ll_tail(GenericLinkedList* list, void* element);


#endif

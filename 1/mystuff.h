#include <ucontext.h>

/**************************************************************************************
 *************************************** MACROS ***************************************
 **************************************************************************************/

#ifndef TRUE
#define TRUE 1              // Boolean true
#endif
#ifndef FALSE
#define FALSE 0             // Boolean false
#endif

#define MAX_THREADS 8192    // Maximum amount of threads. The MAX_INT of 32-bit systems just in case
#define MAX_TID 8193        // max int for 32 bit systems 
#define STACK_SIZE 8192     // 8 KiB, used for the length of arrays
#define EXIT 0              // When a process has exited 
#define RUN 1               // When currently running 
#define READY 2             // When waiting in the runq
#define WCHLD 3             // When waiting on a specific child
#define WALL 4              // When waiting for all children
#define MAIN_TID 0          // Thread ID of the main thread
#define MAX_SEMS 8192       // Max amount of semaphores


/**************************************************************************************
 ******************************** TYPEDEFS AND STRUCTS ********************************
 **************************************************************************************/

// Thread container
typedef struct __my_t {
    unsigned int tid;       // This thread's id
    struct __my_t* parent;  // Parent thread 
    int status;             // Thread status. Refer above
    struct List* child_list;// List of children
    unsigned int ct_cnt;    // Child thread count
    ucontext_t context;     // The context of this thread
    unsigned int wc_tid;    // TID of child thread we wait on if WCHLD
    char sem_wait;          // TRUE if waiting on a semaphore, FALSE otherwise
} __my_t;

// Linked list node 
typedef struct Node {
    __my_t* t;              // Node's payload (the thread)                   
    struct Node* next;      // Next node
    struct Node* prev;      // Previous node
} Node;

// Linked List
typedef struct List {
    Node* head;             // Beginning of the list
    Node* tail;             // End of the list
    int length;             // Duh
} List;

// Semaphore container
typedef struct __sem {
    int val;                // Semaphore value
    struct List* waitq;    // Wait queue for the semaphore
    unsigned int sid;       // Semaphore id
} __sem;

// Semaphore node (Because I don't quite understand how void* works yet)
typedef struct SNode {
    __sem* s;
    struct SNode* next;
    struct SNode* prev;
} SNode;

// Linked List for the semaphores
typedef struct SList {
    SNode* head;
    SNode* tail;
    int length;
} SList;


/**************************************************************************************
 ************************************* PROTOTYPES *************************************
 **************************************************************************************/


// List prototypes
List* setup_list();
List* setup_queue();
void enqueue(List* l, __my_t* t);
__my_t* dequeue(List* l);
int is_empty(List* l);
int len(List* l);
__my_t* peek(List* l);
void remove_from_list(List* l, __my_t* t); // remove was already reserved from glibc

SList* setup_slist();
void senqueue(SList* l, __sem* s);
int s_is_empty(SList* l);
SNode* s_peek(SList* l);
void remove_from_slist(SList* l, __sem* s);
__sem* sdequeue(SList* l);


// Internal helper functions
unsigned int __get_next_available_tid();
unsigned int __get_next_available_sid();
void __remove_refs(__my_t* t);
int __is_child(__my_t* parent, __my_t* child);

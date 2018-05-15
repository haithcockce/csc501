#include <stdio.h>
//#include <stdbool.h>
//#include <ucontext.h>
#include <stdlib.h>
#include "mythread.h"
#include "mystuff.h" /* THIS MUST BE INCLUDED AFTER mythread.h AND NOT BEFORE */


/* TODO
 * Need to figure out remove references without dereferencing garbage
 * IE remove references of a child from parent and grandchild when parent
 * and gc are alive, when both are dead, and when either one but not the other is alive
 *
 */

/**************************************************************************************
 ********************************** GLOBAL VARIABLES **********************************
 **************************************************************************************/
/* current_t
 * After MyThreadInit, we will need to keep track of which threads put us back in this library. To
 * do so, we will simply assign it the "current" tid whenever we change. May need to modify this as
 * we go, but the idea is a way for us to keep track of parent ids when jumping vertically in the
 * hierarchy. -1 means the main thread.
 */
__my_t* current_t;

/* runq
 * This queue will house threads that are simply waiting to run. When a thread has yielded, it gets
 * queued here. If a thread terminates, then nothing happens with the terminating thread. After
 * either, the head of this queue will be removed and ran. When I am empty, then the program has
 * finished via MyThreadExit(). The main_t (see below) should never be in here.
 */
List* runq;

/* waitq
 * This queue will house any threads that are blocking on MyThreadJoin or MyThreadJoinAll
 * When either are called, the thread waiting_thread will be queued. When a thread exists, we will
 * need to check for its parent in here. If it exists in here, then we will need to remove the tid
 * of the child from ct_list and decrement ct_cnt of the thread. If and only if ct_cnt is 0, then
 * the thread can be removed from this queue and queued into the ready queue
 */
List* waitq;

/* main_t 
 * Points to the "main" thread of the library. When Exit is called, we will switch this this thread
 * to clean up the exiting and continue execution by dequeuing from the runq and run that thread.
 */
__my_t* main_t;

/* invoking_t
 * More of a placeholder than anything but used typically when transitioning between threads to 
 * indicate which thread is Yielding/waiting
 */
__my_t* invoking_t;

/* avail_tids
 * An array to track available thread ids (TIDS). A TID is available when TRUE == avail_tids[TID] 
 * and not available when FALSE == avail_tids[TID].
 */
unsigned int avail_tids[MAX_TID];

/* last_tid
 * A place holder for the last tid issued. Useful when keeping track of available tids. 
 */
unsigned long last_tid = 0;

/* in_main
 * A flag to help keep track if we are currently in the main thread or not. 
 */
char in_main;

/* semaphores
 * The list of semaphores. This is not guarenteed to be in a particular order. Creating new 
 * semaphores will append a semaphore and removal should do an inplace removal.
 */
SList* semaphores;

unsigned int avail_sids[MAX_SEMS];
unsigned int last_sid = 0;
/**************************************************************************************
 ************************************* FUNCTIONS **************************************
 **************************************************************************************/

/* MyThreadCreate
 *
 * Desc: 
 *   Creates a thread and enqueues the thread to the readyq. 
 *   Though users should ball out control on threads, a limit of MAX_THREADS (default of 8192) is
 *   placed. The only thing that will happen is a NULL is passed around. Users shouldn't need to 
 *   dereference something in the MyThread anyway as that breaks API. Regardless, the context is
 *   first created by getcontext, makecontext, and allocating the stack. Afterwards, the thread
 *   container is created, the thread context is saved into the thread, and the thread is then 
 *   enqueued into the runq. The thread is then returned (as a pointer of type MyThread).
 * Parm:
 *   void(*start_funct)(void *)
 *     Pointer to the function that this thread will work 
 *   void *args
 *     Pointer to the list of arguments to the function this thread will work
 * Retn:
 *   MyThread* 
 *     Pointer to the thread created. The internal representation of threads are via __my_t* but
 *     the API defined requires a MyThread* so the thread is cast __my_t* -> MyThread* then 
 *     returned
 * Mods:
 *   runq
 *     The new thread is enqueued to the runq
 */
MyThread MyThreadCreate (void(*start_funct)(void *), void *args) {

    // Don't make another thread if we have too many! 
    if(runq->length == MAX_THREADS) {
        return NULL;
    }
    int tid = __get_next_available_tid();
    if(tid == -1) {
        printf("No more available TID");
        return NULL;
    }

    // Setting up and creating the context 
    ucontext_t uctx;
    getcontext(&uctx);
    uctx.uc_stack.ss_sp = malloc(STACK_SIZE);
    uctx.uc_stack.ss_size = STACK_SIZE;
    uctx.uc_link = &(main_t->context);
    makecontext(&uctx, start_funct, 1, args);

    // Setting up and creating the thread 
    __my_t* t = (__my_t*) malloc(sizeof(__my_t));
    t->tid = tid;
    t->parent = current_t;
    t->child_list = setup_list();
    enqueue(t->parent->child_list, t);
    t->parent->ct_cnt = t->parent->ct_cnt + 1;
    t->status = READY;
    t->ct_cnt = 0;
    t->context = uctx;

    enqueue(runq, t);
    return (MyThread) t;
}

/* MyThreadYield
 *
 * Desc:
 *   Stops current execution and switches to the next available runnable thread. 
 *   Upon calling Yield, the current thread's status is changed and is enqueued into the runq. The
 *   next thread in the runq is removed and the context of the threads are switched. Returning from
 *   this function only means that the invoking thread was dequeued from the runq. Execution should
 *   continue after the yield was called. 
 * Parm:
 *   -
 * Retn:
 *   -
 * Mods:
 *   runq
 *     . The current thread is enqueued to the runq
 *     . The head of the runq is removed 
 *   current_t
 *     . The next runnable thread or main supersedes the current thread 
 *     . The current thread is also marked as READY
 *   invoking_t
 *     . The current thread becomes the invoking thread. 
 */
void MyThreadYield(void) {

    // Change the status and enqueue to runq
    current_t->status = READY;
    enqueue(runq, current_t);
    invoking_t = current_t;
   
    // Grab the next thread to run and mark it as such
    current_t = dequeue(runq); 
    if(current_t == NULL) {
        current_t = main_t;
        exit(1); // SANITY CHECK ONLY we should never hit this
    }
    current_t->status = RUN;
    swapcontext(&(invoking_t->context), &(current_t->context));
}




/* MyThreadJoin
 *
 * Desc:
 *   The current thread stops execution to wait on child thread thread to terminate if the thread
 *   is a child of the current thread.
 *   First we check if the child exists, exiting if not. Then we next check if the child is an 
 *   immediate child of the current thread, exiting if not. If so, then we set indicate this thread
 *   as waiting on the child in question, enqueue this thread into the waitq, grab the next 
 *   runnable thread and switch to that thread. 
 * Parm:
 *   MyThread thread
 *     A pointer to the child thread to wait on. This must be cast to __my_t* before handling it.
 * Retn:
 *   0
 *     If the child thread was an immediate child of the current thread and the child exited. 
 *   -1
 *     If the child does not exist (NULL) or is not an immediate child of the current thread.
 * Mods:
 *   current_t
 *     The current thread's status is set to WCHLD and the tid of the child to wait on is saved
 *     The current thread is also superseded by the next runnable thread
 *   waitq
 *     The current_t is appended to the waitq
 *   runq
 *     The next thread is removed from the runq
 */
int MyThreadJoin(MyThread thread){

    // Need to first determine if the current thread even has thread as a child or if the 
    // child thread even exists 
    if(thread == NULL) {
        return -1;
    }
    __my_t* child_t = (__my_t*) thread;
    int found = __is_child(current_t, child_t);
    if(!found) {
        return -1;
    }

    // Indicate which child we are now waiting on 
    current_t->status = WCHLD;
    current_t->wc_tid = child_t->tid;
    enqueue(waitq, current_t);
    invoking_t = current_t;

    
    // Switch contexts with the next thread in the queue
    current_t = dequeue(runq); 
    if(current_t == NULL) {
        exit(1);
        // SANITY CHECK ONLY we should never hit this
    }
    swapcontext(&(invoking_t->context), &(current_t->context));
    return 0;
}

/* MyThreadJoinAll
 *
 * Desc:
 *   Blocks execution of the current thread until all of its children exit.
 *   We first check if we even have children. If we do, we indicate that this thread is waiting on
 *   all its children and enqueue it to the waitq. From here, if the current thread is not the main
 *   thread, then simply wait until the last child signals this thread that it exited. Exit() 
 *   handles requeuing parent threads. Otherwise, if we are the main thread, simply keep looping 
 *   until the runq is empty. While we should handle the waitq as well, currently the requirements
 *   specify only to check if the runq is empty. When we return to this within main, we then need 
 *   to check that the previous current thread exited, cleaning it up if so. We will always return
 *   to the main thread in here since it should be the library's responsibility to clean exited 
 *   threads. 
 * Parm:
 *   -
 * Retn:
 *   - 
 * Mods:
 *   current_t
 *     . Changes the status to WALL to indicate waiting for all children to finish
 *     . The next runnable thread supersedes the current thread
 *   invoking_t
 *     . The current thread becomes the invoking thread
 *   in_main
 *     . Switches back and forth from TRUE/FALSE to indicate which thread we may be in. 
 *   runq
 *     . We dequeue from runq to grab the next runnable thread.
 *   waitq
 *     . We enqueue to the waitq to wait. 
 */
void MyThreadJoinAll(void){

    // If we do not have children, then do not wait
    if(current_t->ct_cnt == 0) {
        return;
    }

    
    // Modify the current thread to indicate waiting for all children to complete
    current_t->status = WALL;
    enqueue(waitq, current_t);
    invoking_t = current_t;

    // If not in main, simply grab the next runnable thread swap
    if(!in_main) {
        current_t = dequeue(runq);
        swapcontext(&(invoking_t->context), &(current_t->context));
    }

    // Otherwise, we will need to spin in a loop waiting for all children to finish
    // cleaning them up when they do finish
    else {
        while(!is_empty(runq)) {

            // Grab the next runnable thread 
            current_t = dequeue(runq);
            if(current_t == NULL) {
                current_t = main_t;
                break; // SANITY CHECK ONLY we should never hit this
            }
            else if(current_t->tid == MAIN_TID && is_empty(runq)) {
                return;
            }

            // Indicate we are about to be in a userspace thread and swap
            in_main = FALSE;
            swapcontext(&(main_t->context), &(current_t->context));
            in_main = TRUE;
            // If the thread exited, then clean it up
            if(current_t->status == EXIT) {
                free(current_t->child_list);
                free(current_t);
                current_t = main_t;
            }
        }
    }
}

/* MyThreadExit
 *
 * Desc:
 *   Exits the currently running thread
 *   Frees the current thread's TID, removes all references to itself from its parent and children, 
 *   and swaps context with the main thread so the main thread can clean this thread up. 
 *   __remove_refs handles requeueing of parents where applicable. 
 * Parm:
 *   -
 * Retn:
 *   - This function should actually never return. 
 * Mods:
 *   current_t->parent
 *     . removes the reference to the current thread from the parent
 *   current_t->child_list
 *     . removes all itself as the parent thread from all of its children
 *   current_t
 *     . sets itself as EXIT status
 */
void MyThreadExit(void){

    // Nothing to do if we are main
    if(current_t->tid == MAIN_TID) {
        return;
    }

    // remove all references to us
    __remove_refs(current_t);

    // Free the tid for use elsewhere 
    avail_tids[current_t->tid] = TRUE;

    // if the parent no longer has children, and if it is WCHLD, then queue parent in ready
    current_t->status = EXIT;
    swapcontext(&(current_t->context), &(main_t->context));
}

/*
 * abstraction for synchronization. Creates a sem
 */
MySemaphore MySemaphoreInit(int initialValue){
    if(!(semaphores->length < MAX_SEMS)) {
        return NULL;
    }
    int sid = __get_next_available_sid();
    if(sid == -1) {
        return NULL;
    }

    __sem* s = (__sem*) malloc(sizeof(__sem));
    s->waitq = (List*) malloc(sizeof(List));
    s->val = initialValue;
    s->sid = sid;
    senqueue(semaphores, s);
    return (MySemaphore) s;
}


void MySemaphoreSignal(MySemaphore sem){
    if(sem == NULL) {
        return;
    }
    __sem* s = (__sem*) sem;
    s->val++;
    if(s->val == 0 && !(is_empty(s->waitq))) {
        __my_t* t = dequeue(s->waitq);
        t->sem_wait = FALSE;
        enqueue(runq, t);
    }
}


void MySemaphoreWait(MySemaphore sem){
    if(sem == NULL) {
        return;
    }

    __sem* s = (__sem*) sem;
    s->val--;
    current_t->sem_wait = TRUE;
    enqueue(s->waitq, current_t);
    swapcontext(&(current_t->context), &(main_t->context));
}
int MySemaphoreDestroy(MySemaphore sem){
    if(sem == NULL) {
        return -1;
    }
    __sem* s = (__sem*) sem;
    if(s->waitq->length != 0) {
        return -1;
    }
    avail_sids[s->sid] = TRUE;
    remove_from_slist(semaphores, s);
    free(s->waitq);
    free(s);
    return 0;
}

// ****** CALLS ONLY FOR UNIX PROCESS ******
// Create and run the "main" thread

/* MyThreadInit
 *
 * Desc:
 *   Initializes the threading engine and infrastructure
 *   We first populate the list of available threads, then create the main thread container, 
 *   setup the run and waitqs, then create the first thread. From there, we spin waiting on the 
 *   runq to empty, cleaning up after ourselves upon exiting. 
 * Parm:
 *   void(*start_funct)(void *)
 *     . pointer to the first function to run after setting up the thread engine
 *   void *args 
 *     . pointer to the list of args passed to the first function  
 * Retn:
 *   -
 * Mods:
 *   - all the things
 */
void MyThreadInit (void(*start_funct)(void *), void *args){

    // Setup the available tid/sid lists
    int i;
    for(i = 0; i < MAX_TID; i++) {
        avail_tids[i] = TRUE;
    }
    avail_tids[0] = FALSE;

    for(i = 0; i < MAX_SEMS; i++) {
        avail_sids[i] = TRUE;
    }

    // Setting up the main thread. It is unecessary to call {get,make}context() for
    // the main thread as it is already in a function and has an execution context.
    main_t = (__my_t*) malloc(sizeof(__my_t));
    main_t->tid = 0;
    main_t->parent = NULL;
    main_t->status = READY;
    main_t->child_list = (List*) malloc(sizeof(List));
    main_t->ct_cnt = 0;
    main_t->wc_tid = -1;
    current_t = main_t;

    // Setting up the queues and semaphores 
    runq = setup_queue();
    waitq = setup_queue();
    semaphores = setup_slist();
    in_main = TRUE;

    // Let'er rip! Begin running the first thread and spin waiting on the runq to drain
    MyThreadCreate(start_funct, args);
    MyThreadJoinAll();

    //Clean up
    free(runq);
    free(waitq);
    free(main_t);
}




/**************************************************************************************
 ***************************** INTERNAL HELPER FUNCTIONS ******************************
 **************************************************************************************/

/* __get_next_available_tid 
 *
 * Desc:
 *   Grabs the next available tid. 
 *   Starting from the last_tid issues, we will loop until we find an available tid to hand out
 *   wrapping back around to 1 (0 is reserved for main_t) if we overflow. 
 * Parm:
 *   -
 * Retn:
 *   int
 *     . the next available tid
 *   -1 
 *     . when no tids are available 
 * Mods:
 *   avail_tids
 *     . The value of the next available tid is FALSE to indicate it is now in use
 *   last_tid
 *     . Updated to the last issued
 */
unsigned int __get_next_available_tid() {

    int i;
    for(i = last_tid + 1; i != last_tid; i++) {

        // handle wrap around
        if(i == MAX_TID) {
            i = 1;
        }

        // if we find an available tid, mark it used and return it
        if(avail_tids[i] == TRUE) {
            avail_tids[i] = FALSE;
            last_tid = i;
            return i;
        }
    }

    // none available 
    return -1;
}



/* __get_next_available_sid 
 *
 * Desc:
 *   Grabs the next available sid. 
 *   Starting from the last_sid issues, we will loop until we find an available tid to hand out
 *   wrapping back around to 0 if we overflow. 
 * Parm:
 *   -
 * Retn:
 *   int
 *     . the next available tid
 *   -1 
 *     . when no tids are available 
 * Mods:
 *   avail_sids
 *     . The value of the next available sid is FALSE to indicate it is now in use
 *   last_sid
 *     . Updated to the last issued
 */
unsigned int __get_next_available_sid() {

    int i;
    for(i = last_sid + 1; i != last_sid; i++) {

        // handle wrap around
        if(i == MAX_SEMS) {
            i = 0;
        }

        // if we find an available tid, mark it used and return it
        if(avail_sids[i] == TRUE) {
            avail_sids[i] = FALSE;
            last_sid = i;
            return i;
        }
    }

    // none available 
    return -1;
}


/* __remove_refs
 *
 * Desc:
 *   Removes all references to t from parent and children
 *   First grabs the parent thread and removes the current thread from its child_list. Then we walk
 *   through the list of children and remove their parent reference. For simplicity, there is not 
 *   need to rehome the children. If the parent of this thread was waiting on us, then requeue it
 *   into the runq. 
 * Parm:
 *   __my_t* t
 *     . the thread where references to this are removed from this thread's parent and children
 * Retn:
 *   -
 * Mods:
 *   t->parent->child_list
 *     . removes t from the parent's list of children
 *   t->child_list->*->parent
 *     . removes t from all of the children's parent references
 */
void __remove_refs(__my_t* t) {

    // grab the parent of t
    __my_t* parent = t->parent;

    // if the parent is already gone, skip to the child work
    if(parent != NULL) {

        // remove t from the parent
        remove_from_list(parent->child_list, t);
        parent->ct_cnt--;

        //if the parent was waiting on us, requeue to runq 
        if((parent->status == WALL && parent->ct_cnt == 0) ||
           (parent->status == WCHLD && parent->wc_tid == t->tid)) {
            remove_from_list(waitq, parent);
            parent->status = READY;
            parent->wc_tid = -1;
            enqueue(runq, parent);
        }
    }

    //Orphan the children
    __my_t* iterator;
    while(!is_empty(t->child_list)) {
        iterator = dequeue(t->child_list);
        iterator->parent = NULL;
    }
}

/* __is_child
 *
 * Desc:
 *   Determines if the child is an immediate child of parent
 *   We walk through the parent's child_list to see if the tid of each child matches the requested
 *   child's tid returning what we find. 
 * Parm:
 *   __my_t* parent
 *     . the parent to search through
 *   __my_t* child
 *     . the requsted child to search for
 * Retn:
 *   TRUE
 *     . if child is an immediate child of parent
 *   FALSE
 *     . otherwise
 * Mods:
 *   -
 */
int __is_child(__my_t* parent, __my_t* child) {
    Node* iterator = parent->child_list->head;
    while(iterator != NULL) {
        if(iterator->t->tid == child->tid) {
            return TRUE;
        }
        iterator = iterator->next;
    }
    return FALSE;
}


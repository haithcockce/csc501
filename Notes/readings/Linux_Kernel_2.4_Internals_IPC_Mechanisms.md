# Semaphores

### Overview
Semaphores (sem) are typically a locking mechanism that is stateful, IE you can programmatically react to the state of the semaphore, compared to a spinlock which is binary (either you are locked or not). You allocate semaphores in a set and access a semaphore via the ID. The semaphore is used to locking purposes by checking the state of the semaphore (its _value_). When using a semaphore, you can either read the value or update its value. When reading, you do not have to block, but when updating you have to block. If you block, you have to create a list of the operations that would modify the value of the semaphore. The list will have at its head the semops that would not modify the values (reads) while the tail contains all the semops that modify a sem value. You can fail on a semop if a semop can not wait and the operation is unable to complete or the semop would cause an invalid semvalue. Locking occurs with a global spinlock that governs access to the global set of semaphores when performing many operations (not creation) to keep operations atomic. 

### Terms
* __Semaphore Set__ The semaphore array created when you create a semaphore. Again, you allocate semaphores in a set/list, not one by one. This is possibly because you would want to allocate them on program startup as allocation within execution can cause race conditions since sem allocation is not an atomic operation. 
* __Semaphore Operation__ AKA _semop_, an action to perform on a semaphore's value. This is limited to the following: 
    * _Positive_ You will increment the value. This typically means that you are signalling the semaphore and releasing resources
    * _Negative_ You will decrement the value. This typically means that you are going to allocate resources and attempt to do something later, such as wait on something to occur wherever this is used in your code. 
    * _Zero_ You will simply wait until the semaphore value is reset to 0. This means waiting on semaphore signals
* __Semaphore ID__ AKA _semid_, the semaphore identifier
    
### Sem SysCalls 
* `sys_semget()` You either create a new semaphore set and the return value is the `semid` of the new set or you simply fetch the semaphore by finding the semaphore associated with `semid`. 
* `sys_semctl()` This is the function call to interact witht he semaphore's metadata, such as setting and getting sem statistics and timestamps, removing the semaphore, get the associated value or owning PID, etc.
* `sys_symop()` This actually performs the semop. The requested semop is copied into a buffer, access and values are validated and locks are grabbed. From there, we check if any operations have SEM_UNDO set or alter the semaphore values counting those operations if they do. When SEM_UNDO is asserted, we look for an undo structure for this set freeing any that are set to be free and allocating a new one if one does not exist, then we try the semops.
    * If we didn't block (IE no modifications to the sem values), then cool. 
    * If we failed (because a semop can't wait (IPC_NOWAIT) and it could not complete or because the operation would put the semaphore to an invalid state), then return error
    * If we block (IE we would have modified the semvals), then we setup a semaphore queue where all modifying semops are queued to the back while nonmodifying semops are pushed onto the front. We then put the process to sleep and queue the process to this sem. Upon waking up, the process:
        * Errors out if the semaphore has suddenly been removed
        * retries the semops
        * realizes that we actually were woken up by an interrupt and errors out
        * or the semops were finished and we return success or an error code

### Sem Structs
* __sem_array__ The semaphore set you allocate, operate on, and remove and contains timestamps, undo structs, etc
* __sem__ the sem itself 
* __seminfo__ not sure
* __semid64_ds__ buffer for various permissions and timestamps of a sem
* __sem_queue__ The queue of semops a process sleeps on until it finishes. Ties a task to a set of semops
* __sembuf__ buffer for moving semops from userspace to kernel space (more specifically from a space users can touch to one protected by the kernel's semaphore API). 
* __sem_undo__ not really sure actually. I guess the set of semops that would be taken to rever a sem back to its original state before semops are carried out? 

### Sem Helper Functions
* `newary()` Allocates memory for a set descriptor and each semaphore in set, updates the `used_sems` count
* `freeary()` removes semid, invalides undo struct list, all sleeping processes for this sem are woken, updates `used_sems` count, and free the memory that the semaphore had. Essentially wake up sleeping processes and free the semaphore and its memory
* `semctl_down()` Depending on the passed in option, either calls `freeary()` or updates the sem metadata
* `semctl_nolock()` Depending on passed in options, we either provide a copy of the sem metadata or timestamps 
* `semctl_main()` Depending on passed in options, either gets all the current values of a semset, sets all the values of a semset, grabs a copy of hte timestamps, gets a specific semval, gets the PID of the last process that performed a semop on the semset, gets the number of waiting processes (see `count_semncnt()`), gets the number of waiting processes waiting on a 0 semval, or set a specific semval
* `count_semncnt()` Gets a count of processes waiting on a semval to be less than 0
* `count_semzcnt()` Gets a count of processes waiting on a semval to be 0
* `update_queue()` sanity checks semvals for a set of semops, unblocks processes if necessary
* `try_atomic_semop()` Called by `sys_semop()` and `update_queue()`. If `update_queue()` determines that the semops are fine, then `try_atomic_semop()` will actually attempt to perform the semops, erroring out and undoing the semops if any of them will cause blocking or if a semop will adjust a value beyond system limits. If we did not indicate to undo the operations (`do_undo`) and we didn't error out, then the changes remain. If we indicated to undo the operations, then we undo all the operations after attempting them. 
* `free_undos(), alloc_undo()` Frees/allocates undo structs
* `sem_exit()` execute all the undo operations. 

# Message Queues

### Overview
A message queue consists of a queue for storing messages between a process/multiple processes, a sender wait queue to have processes wait to send off a message when the message queue is full, and a receive wait queue for receiving processes to wait when the queue is empty but the process is expecting a message, along with various metadata. You either place a message or retrive one. 

### Message Queue Syscalls
* `sys_msgget()` similar to `sys_semget()` either get a msgq or create a new one
* `sys_msgctl()` depending on the option passed in, either grab various msgq stats, set msgq info, or breakdown the msgq and remove it
* `sys_msgsnd()` sends a message in the queue. If someone is waiting on the message, then deliver it directly to the waiting process, otherwise simplue enqueue the message. On enqueueing, first we validate stuff and get locks. If the message is too big for the space remaining in the queue, then we:
    * Fail and return with EAGAIN if we can not wait (IPC_NOWAIT)
    * makes the sender block and queues the process to the sender wait queue. On wake, we
        * check if the msgq is still there erroring out if not
        * determine if we woke because of an interrupt and returns if so
        * or check if there is enough space yet and try again
* `sys_msgrcv()` Receives a message in the queue. First searches for a message in the queue. If one exists that matches the quest type, it is copied into the userspace buffer. If no message is found matching the request, the requesting task/proc is queued in the waiting receivers queue for a message to show up. 
    * If the message in question is larger than expected and no errors are allowed, we error out with E2BIG
    * If we do not find a message and we can not wait (IPC_NOWAIT), we error out (ENOMSG). Otherwise, block. 
    * On wake, we check if we directly received the message. If so, we free the message in the queue and return. Otherwise,
        * get the global msgq spinlock and recheck if we receied the message while getting the spinlock 
        * If we don't have the message then we retry unless we have a signal pending, then we were interrupted and we error out on this (EINTR) or we just has an error otherwise and error out. 
    * On success, we update stats and times and wake up all the waiting senders. 

### Msgq Structs
* __`msg_queue`__ The system msgq containing the waiting senders and receivers queues, the message queue itself, permissions set, timestamps, etc. 
* __`msg_msg`__ the description of the message such as size, type, the list of (something?), and the start of a Linked List of message segments if the message is too large to fit in the current block. The notes say that the message itself follows immediately after all the struct members, so I assume the msg_msg struct starts with header info then contains the message. 
* __`msg_msgseg`__ Linked List member when the message can not fit into the current block. If so, spill over goes to another block and the connection. Like msg_msg, I assume the message literally sits in this segment and the header of the segment simply points to the next segment. 
* __`msg_sender`__ This represents the process/task that is blocking and is queued to the waiting sender queue
* __`msg_receiver`__ similar to msg_sender except contains a bit more information to indicate what exactly the receiver is waiting on
* __`msgqid64_ds`, `msgqid_ds`__ msgq stats
 
### Msgq Helper Functions
* __`newque()`__ Similar to `newary()`, creates a new msgq (note a full msgq and not just the queue the IPC messages are stored in)
* __`freeque()`__ Similar to `freeary()`, deconstructs and frees a msgq
* __`ss_wakeup()`__ wakes up all waiting senders. Dequeues the senders as well if called from `freeque()`
* __`ss_add()`, `ss_del()`__ enqueues/dequeues a waiting sender proc/task
* __`expunge_all()`__ wakes up all waiting receivers and tells them the reason they woke up. 
* __`load_msg()`__ Puts the message from userspace into thea msgq. If the message can fit into a `msg_msg` then do so, but if not, fill the `msg_msg` then create any additional `msg_msgseg` to hold any spill over. 
* __`store_msg()`__ reassembles a message into the caller-provided userspace buffer. 
* __`free_msg()`__ Frees the memory associated with a message
* __`testmsg()`__ checks if the current message is the message we are looking for
* __`pipelined_send()`__ searches for and sends the message directly to the receiver, waking the receiver up. Potential receivers with a requested size smaller than the message size are woken up and error out with E2BIG. 
* __`copy_msgqid_to_user()`, `copy_msgqid_from_user()`__ Copies msgq information from/to a msgq to/from a userspace buffer all respectively

# Shared Memory 

### Overview
Allows areas of shared memory segments to be created and processes can be attached to them and removed from them in order to provide shared memory access and capabilities. 

### Shm Syscalls
* __`sys_shmget()`__ similar to the above. Get the shm segment via ID or create a new one
* __`sys_shmctl()`__ similar to the above. Get/set shm segment stats, timestamps, destroy shmseg, except you can call a function based on locking/nonlocking
* __`sys_shmat()`__ validates permissions and performs address arithmetic. Then run `do_mmap()` to generate a vm address to the shmseg. This is a file operation and as such the shm has a list of PIDs attached and a ref count.
* __`sys_shmdt()`__ search for the `vm_area_struct` associated with the requested within the `mm_struct` of the current process and unmap the mapping. If the shm has no additional attached processes, then the shmseg is destroyed. 

### SHM Structs
* __`shminfo64`__ shared memory info (shmmax and what not. however red hat docs indicates this is for sems and not shmsegs)
* __`shm_info`__ swap and allocated shm info 
* __`shmid_kernel`__ the shm metadata regarding permisions, number of attachmentsetc
* __`shmid64_ds`__ more info stuff
* __`shmem_inode_info`__ shm inode information including the spinlock for it, swap locations, lock flags, etc

### SHM Helper Functions
* __`newseg()`__ After validating permissions and making sure the limits of shm* are met, creates a new shmseg descriptor. Since shm resides in a tmpfs, the creation of the actual segment is delegated to `shmem_file_setup()`
* __`shm_get_stat()`__ calculates the sum total of shm in memory and swap
* __`shmem_lock()`__ determines if the shmseg is locked or unlocked. 
* __`shm_destroy()`__ desconstructs shmseg and frees it. This entails decrementing the ref count and what not
* __`shm_inc()`, `shm_close()`__ adds the PID to the shmseg and increments ref count, or removes the PID from the shmseg and decrements ref count. If ref count falls to 0, then we destroy the shmseg. 
* __`shm_file_setup()`__ if we are within kernel fs limits, then we create a new dentry in a tmpfs and allocate the other structs along with that. This returns the new file descriptor.

# IPC Primitives

### Overview
The sem, shm, and msgq stuff are all built on top of various IPC structs and functions that provide generic IPC operations. This also includes maintaining a list of generic IPC descriptors. 

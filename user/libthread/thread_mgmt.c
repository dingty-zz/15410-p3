/**
* @file thread_mgmt.c
*
* @brief Thread manage API
*
*         1. Maintaining threads: The thread library keeps tracks of all the 
*            created threads including the parent thread (task thread) itself
*            in a global doubly linked list. We store the parent thread because 
*            it's possible that child threads may join the parent so we have to
*            store the parent thread's tid on to the tcb.
*         2. Thread creation: first it finds an available place to create
*            the child stack by a linear search. Then call the thread_fork to 
*            run the child thread. It will notify the run_child function that
*            the creation is complete. Additional exception handler will be 
*            registered.
*         3. Thread join: When a thread wants to join another thread, it has to
*            wait until that thread exits (assuming the target thread exits). 
*            Then the caller thread will reap the exited thread by calling 
*            thread_reap function and remove its data structure out of the tcb. 
*            Because we treat task thread the same as child threads, it's 
*            possible that any threads in this program will join any other thread.
*         4. Thread exit: When a thread wants to exit, we first check if it's in 
*            single-threaded program. If so then the thread will just vanish(). 
*            Otherwise the thread will set it's state to EXIT and enter the zombie
*            state. It will finally be reaped by either the thread wants to join
*            it or the last thread of the program.
*        
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
* @bugs No known bugs
*
*/

#include <syscall.h>
#include "thr_internals.h"
#include "malloc.h"
#include "mutex_type.h"
#include "cond_type.h"
#include "linked_list.h"
#include "autostack.h"
#include "getesp.h"
#include "t_fork.h"

#define BYTE 1

/* @brief Note that we use lock in a fine-grained manner, because using a global
 *        mutex to lock everything will largely reduce effciency and speed */

// global mutex for avoiding race conditions
mutex_t m;
// global mutex to protect the global tcb
mutex_t list_mutex;
// global cond_var to signal when the child is created
cond_t list_cond;
// global tcb to maintain all the threads
list thread_list;

// Aligned page size
static unsigned int adjusted_size = 0;

// Stored program lower stack address, used for optimization
static void *program_stack_low = NULL;

extern int malloc_init();
extern void handler(void *arg, ureg_t *ureg);  // Thread handler
void run_child(void *(*func)(void *), void *arg);
int thr_init(unsigned int size);
int thr_create(void *(*func)(void *), void *arg);
int thr_join(int tid, void **statusp);
void thr_exit(void *status);
int thr_getid();
int thr_yield(int tid);
void thr_reap(thread_t *thread);
thread_t *search_thread_by_id(list *l, int tid);


/** @brief The function to initialize the thread library
 *         We first initialize all the global variables
 *  @param size Size of each thread stack
 *  @return 0 on success, -1 on error (unlikely)
 */
int thr_init(unsigned int size)
{
    // initialize global tcb and locks
    int ret = 0;
    ret = mutex_init(&m);
    ret |= mutex_init(&list_mutex);
    ret |= cond_init(&list_cond);
    ret |= malloc_init();
    list_init(&thread_list);

    // Store parent's thread (task thread) onto global tcb
    thread_t *parent = (thread_t *)malloc(sizeof(thread_t));
    parent -> status = THREAD_RUNNING;
    list_insert_last(&thread_list, make_node((void *)parent, gettid()));

    // Adjust the size so that it is page aligned
    int i = 1;
    while (1) {
        if (size <= PAGE_SIZE * i) {
            adjusted_size = i * PAGE_SIZE;  
            break;
        }
        else i++;
    }

    // Allocate optional stack space for interrupt handler of each thread
    adjusted_size += PAGE_SIZE;
    return ret;
}

/** @brief The function to create a child thread
 *
 *         We start from the program's lower stack and continuously decrease
 *         the pointer until we've found a spot to alloc a new page with the
 *         adjusted size. Once found, we create the child thread
 *         running on that stack by calling thread_fork function.
 *  @param func child's func
 *  @param arg child's arg for func
 *  @return child's tid or negative value when error occurs
 */
int thr_create(void *(*func)(void *), void *arg)
{
    // Indicate multi-threading program because once the app calls thr_create
    // it won't be single threaded program anymore
    global_stackinfo.is_single_threaded = 0;
    mutex_lock(&m);
    void *base_probing_pointer = NULL;
    if (program_stack_low != NULL)
        base_probing_pointer = program_stack_low;
    else
        base_probing_pointer = getesp();

    // Continuously try to allocate a new stack
    while (new_pages(base_probing_pointer, adjusted_size) != 0) {
        if (program_stack_low == NULL) base_probing_pointer--;
        else base_probing_pointer -= PAGE_SIZE;
    }

    /* Finally grab a new page */
    // We save the low address so we don't have to search 
    // within the program stack everytime
    if (program_stack_low == NULL) {
        program_stack_low = base_probing_pointer + adjusted_size;
    }

    /* Begin executing on this address */
    mutex_unlock(&m);
    int t_id = 
    thread_fork(base_probing_pointer + adjusted_size - PAGE_SIZE - BYTE, \
        func, arg);

    // Install thread handler for each child on the stack above
    swexn((void*)base_probing_pointer + adjusted_size + 4, handler, NULL, NULL);
    /* Create the data structure for this thread */
    int ret;
    thread_t *new_thread = (thread_t *)malloc(sizeof(thread_t));
    if (new_thread == NULL) {
        ret = -1;
        panic("Can't malloc enough memory on heap.");
    }
    new_thread -> tid = t_id;
    new_thread -> base = base_probing_pointer;
    new_thread -> status = THREAD_RUNNING;
    new_thread -> statusp = NULL;
    new_thread -> joining_tid = 0;
    ret |= mutex_init(&new_thread -> t_mutex);
    ret |= cond_init(&new_thread -> t_cond);

    // Insert it into tcb
    mutex_lock(&list_mutex);
    list_insert_last(&thread_list, make_node((void *)new_thread, t_id));
    // Signal the child that inserting is successful to let it run
    cond_signal(&list_cond);  
    mutex_unlock(&list_mutex);
    if (t_id < 0) {
        return -1;
    }

    return t_id;
}

/** @brief After creating a child thread, wait until the child thread's data 
 *         is inserted into the tcb and run the child by func(arg)
 *
 *  @param func child's func
 *  @param arg child's arg for func
 *  @return nothing
 */
void run_child(void *(*func)(void *), void *arg)
{
    void *status = NULL;
    int tid = gettid();
    mutex_lock(&list_mutex);
    // Wait until the child's thread data is added into tcb
    while (search_thread_by_id(&thread_list, tid) == NULL) {
        cond_wait(&list_cond, &list_mutex);
    }
    mutex_unlock(&list_mutex);

    // run the child
    status = (*func)(arg);
    thr_exit(status);  // normal thread exits
}

/** @brief Join the target thread
 *
 *         Wait until the target thread is exited and reap that zombie thread 
 *         by calling thread_reap
 *  @param tid the target thread id
 *  @param statusp The pointer to the status in thr_exit
 *  @return 0 on success, -1 on error
 */
int thr_join(int tid, void **statusp)
{
    int this_id = gettid();
    if (this_id == tid) {
        return -1; // can't join itself
    }

    mutex_lock(&list_mutex);
    thread_t *thread = search_thread_by_id(&thread_list, tid);
    mutex_unlock(&list_mutex);

    if (thread == NULL) {
        return 0; // target thread is not in the tcb or the 
                  // thread is reaped by someone else
    }

    mutex_lock(&thread -> t_mutex);
    if (thread -> joining_tid > 0) {
        mutex_unlock(&thread -> t_mutex);
        return -1;  // target thread is already joined by someone
    }
    else
        thread -> joining_tid = this_id;

    // Wait until the target thread exits
    while (thread -> status != THREAD_EXIT) 
        cond_wait(&thread -> t_cond, &thread -> t_mutex);

    if (statusp != NULL)
        *statusp = thread-> statusp;
    mutex_unlock(&thread -> t_mutex);
    thr_reap(thread); // reap this thread to recycle its resource

    mutex_lock(&list_mutex);
    list_delete_id(&thread_list, tid);
    mutex_unlock(&list_mutex);

    return 0;
}

/** @brief Exit a thread
 *         If the program is single threaded, we just vanish. Otherwise, we 
 *         just set the status to be THREAD_EXIT and wait to be reaped until
 *         either 1. another thread wants to reap this thread or 2. 
 *         The last thread will reap everything
 *
 *  @param status a status pointer
 *  @return nothing
 */
void thr_exit(void *status)
{
    // If running on a single threaded program, just vanish
    if (global_stackinfo.is_single_threaded) vanish(); 

    int tid = gettid();
    mutex_lock(&list_mutex);
    thread_t *thread = search_thread_by_id(&thread_list, tid);

    int live_count = 0;
    node *temp = thread_list. head;
    while (temp)
    {
        //lprintf("traversing %d", temp -> tid);
        if (((thread_t *)temp -> data) -> status == THREAD_RUNNING)
        {
            //  lprintf("i am searching %d", tid);
            live_count++;
        }
        temp = temp -> next;
    }

    /* @brief This means that this thread is the only thread that is running, it
     *        has to reap all the zombie threads (including itself) and exits
     */
    if (live_count == 1)  {
        /* only thread to exit */
        while (thread_list.length != 0) {
            node *temp = list_delete_first(&thread_list);
            thread_t *zombie = (thread_t *)(temp -> data);
            if (zombie -> tid == tid) {
                mutex_destroy(&zombie -> t_mutex);
                cond_destroy(&zombie -> t_cond);
            }
            else thr_reap(zombie);
        }
        mutex_unlock(&list_mutex);
        mutex_destroy(&list_mutex);
        cond_destroy(&list_cond);
        task_vanish(0);  // safely vanish
    }

    mutex_unlock(&list_mutex);

    // Sets the status to be exit and wait for being reaped
    mutex_lock(&thread -> t_mutex);
    thread->status = THREAD_EXIT;
    thread->statusp = status;
    cond_signal(&thread -> t_cond);
    mutex_unlock(&thread-> t_mutex);
    vanish(); // safely vanish
}


/** @brief The function to get tid of the thread
 *
 *  @return the tid
 */
int thr_getid()
{
    return gettid();
}


/** @brief Defers execution of the invoking thread to a later time in favor of
 *  the thread with ID tid.
 *
 *  If tid is -1, yield to some unspecified thread. 
 *  If the thread with ID tid is not runnable, or doesn't exist, return -1. 
 *  If success, return 0.
 *
 *  @param tid the thread to yield to.
 *  @return returns zero on success, and a negative number on error.
 */
int thr_yield(int tid)
{
    mutex_lock(&list_mutex);
    thread_t *target = search_thread_by_id(&thread_list, tid);
    mutex_unlock(&list_mutex);

    if(target == NULL || target -> status == THREAD_EXIT) return -1;
    return yield(tid);
}

/** @brief Search the thread data from the a list (specifically used 
 *         for tcb search) by tid
 *
 *  @param l a pointer to the list
 *  @param tid the target tid
 *  @return the data for the target thread if it's in the list, NULL otherwise
 */
thread_t *search_thread_by_id(list *l, int tid)
{
    node *result = list_search(l, tid);
    if (result == NULL) return NULL;
    else return (thread_t *)(result -> data);

}
/** @brief Reap the target thread to recycle it's resource. Called only in 
 *         join and exit.
 *
 *         Requires that the thread is not in the list and 
 *         it's not reaping itself i.e. calling this function in 
 *         this thread's context
 *  @param thread the pointer to the target thread
 *  @return void
 */
void thr_reap(thread_t *thread)
{
    mutex_destroy(&thread -> t_mutex);
    cond_destroy(&thread -> t_cond);
    remove_pages(thread -> base);  // Remove the stack+exception handler for
                                   // this thread. It guarantees to succeed
                                   // because thread -> base is the base address
                                   // of the page created by thr_create before
        free(thread);
}

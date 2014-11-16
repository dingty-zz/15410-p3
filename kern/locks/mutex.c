/**
* @file mutex.c
*
* @brief  The mutex is implemented by a spinlock. Note that we didn't ensure 
*         bounded waiting because otherwise we have to add additional queue 
*         and make use of more syscall like deschedule and make_runnable just 
*         like what cond_var does, which lead to more memory cost and system 
*         speed overhead. A spinlock is simple enough to suffice.
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
* @bugs No known bugs
*/
#include <syscall.h>
#include "mutex_type.h"
#include "control_block.h"
#include "atomic_xchange.h"
#include "process/scheduler.h"
#include "simics.h"
#include <stddef.h>

/** @brief The function to initialize a mutex, which is unlocked initially
 *
 *  @param mp A pointer to the mutex
 *  @return 0 on success and -1 an error (unlikely)
 */
int mutex_init(mutex_t *mp)
{
    mp -> status = MUTEX_UNLOCKED;
    mp -> tid = -1;
    list_init(&mp -> waiting_queue);
    return 0;
}

/** @brief The function to destory a mutex, vanish if doing some illegal behavior
 *
 *  @param mp A pointer to the mutex
 *  @return void
 */
void mutex_destroy(mutex_t *mp)
{
    // Vanish when trying to destroy a locking mutex
    if (mp -> status == MUTEX_LOCKED || mp -> waiting_queue.length) 
        return;    
    mp -> status = MUTEX_UNAVAILABLE;
}

/** @brief The function to lock a mutex
 *
 *  @param a pointer to the list to be initialized
 *  @return nothing
 */
void mutex_lock(mutex_t *mp)
{
    int is_locked = 0;
    while ((is_locked = atomic_xchange(&(mp->status))))  {
        lprintf("The current thread that holds the lock is %d", mp -> tid);
        list_insert_last(&mp -> waiting_queue, &current_thread -> mutex_waiting_queue_node);
        schedule(mp -> tid);     // yield to the thread who holds the lock
        list_delete(&mp -> waiting_queue, &current_thread -> mutex_waiting_queue_node);
    }
    mp -> tid = current_thread -> tid;
    // lprintf("Lock the mutex %p",mp);

}

/** @brief The function to initialize the doubly linked list
 *
 *  @param a pointer to the list to be initialized
 *  @return nothing
 */
void mutex_unlock(mutex_t *mp)
{
    // lprintf("unlock the mutex %p",mp);
    if (mp -> waiting_queue.length != 0)
    {
        TCB *target = NULL;

        // Peek the first waiting thread and yield to it
        node *n = list_begin(&mp -> waiting_queue);
        n = n -> next;
        target = list_entry(n, TCB, mutex_waiting_queue_node);
        mp -> tid = -1;
        mp -> status = MUTEX_UNLOCKED;
        yield(target -> tid);
    } else {
        mp -> tid = -1;
        mp -> status = MUTEX_UNLOCKED;
    }

}


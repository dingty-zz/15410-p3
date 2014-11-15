/**
* @file cond_var.c
*
* @brief The cond_var is implemented with a waitlist and a 
*        spinlock to protect the list
*        If the thread wants to wait, it will put itself into the waitlist
*        If the thread wants to wake up the thread, it will dequeue a thread
*        from the list and continuing wakening that thread until success
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
* @bugs No known bugs
*/

#include <syscall.h>
#include "cond_type.h"
#include "spinlock_type.h"
#include "linked_list.h"
#include "mutex.h"
#include "assert.h"
#include "malloc.h"

/** @brief The function to initialize the cond_var
 *
 *  @param cv a pointer to the cond_var
 *  @return 0 on success and -1 an error (unlikely)
 */
int cond_init(cond_t *cv)
{
    assert(cv != NULL);
    cv -> status = CVAR_AVAILABLE;
    spinlock_init(&(cv -> lock));
    list_init(&(cv -> wait_list));
    return 0;
}

/** @brief The function to destory a mutex, vanish if doing 
 *         some illegal behavior
 *
 *  @param cv a pointer to the cond_var
 *  @return nothing
 */
void cond_destroy(cond_t *cv)
{
    // Vanish when trying to destroy cond_var while threads are
    // blocked waiting on it.
    if (cv -> wait_list.length != 0) vanish();
    spinlock_destroy(&cv -> lock); // Destroy a spinlock
    cv -> status = CVAR_UNAVAILABLE;
}

/** @brief Wait a condition and release the associated mutex that 
 *         it needs to hold to check that condition
 *
 *  @param cv The cond_var
 *  @param mp The mutex
 *  @return void
 */
void cond_wait(cond_t *cv, mutex_t *mp)
{
    if (cv == NULL || cv -> status == CVAR_UNAVAILABLE ||
        mp -> status == MUTEX_UNAVAILABLE) return;
    int tid = gettid();

    spinlock_lock(&cv -> lock);
    mutex_unlock(mp);

    // Insert that thread into waitlist
    list_insert_last(&cv -> wait_list, make_node(NULL, tid));

    int reject = 0;
    spinlock_unlock(&cv -> lock);
    deschedule(&reject);  // Deschedule and unlock
    mutex_lock(mp);       // Grab the lock again
}

/** @brief The function to wake up a thread waiting on cv
 *
 *  @param cv The cond_var
 *  @return void
 */
void cond_signal(cond_t *cv)
{

    if (cv == NULL || cv -> status == CVAR_UNAVAILABLE) return;
    node *curr_node;
    spinlock_lock(&cv -> lock);

    curr_node = list_delete_first(&cv -> wait_list);
    if (curr_node != NULL) {
        while (1) {
            if (make_runnable(curr_node -> tid) == 0) break;
            else yield(-1); // If make runnable fails, we let other threads run
                            // until we successfully run the target thread
        }
    }

    spinlock_unlock(&cv -> lock);
}

/** @brief Wakes up all the sleeping threads in the list
 *
 *  @param cv The cond_var
 *  @return void
 */
void cond_broadcast(cond_t *cv)
{
    if (cv == NULL || cv -> status == CVAR_UNAVAILABLE) return;
    node *curr_node;
    spinlock_lock(&cv -> lock);
    while (cv -> wait_list.length != 0) {
        curr_node = list_delete_first(&cv -> wait_list);
          if (curr_node != NULL) {
              while (1) {
              if (make_runnable(curr_node -> tid) == 0) break;
              else yield(-1); // If make runnable fails, we let other threads run
                              // until we successfully run the target thread
          }
        }
    }
    spinlock_unlock(&cv -> lock);
}

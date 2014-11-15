/**
* @file mutex.c
*
* @brief  The mutex is implemented by a spinlock. Note that we didn't ensure 
*         bounded waiting because otherwise we have to add more complexity
*         to the code. We expect that our mutex should be as simple as 
*         possible. Cond_var, semaphore and rwlocks are all depend on mutex.
*         Therefore a spinlock is simple enough to suffice.
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
* @bugs No known bugs
*/
#include <syscall.h>
#include "mutex_type.h"
#include "malloc.h"
#include "spinlock_type.h"

/** @brief The function to initialize a mutex, which is unlocked initially
 *
 *  @param mp A pointer to the mutex
 *  @return 0 on success and -1 an error (unlikely)
 */
int mutex_init(mutex_t *mp)
{
    mp -> status = MUTEX_UNLOCKED;
    spinlock_init(&(mp -> lock));
    return 0;
}

/** @brief The function to destory a mutex, vanish if doing some illegal behavior
 *
 *  @param mp A pointer to the mutex
 *  @return void
 */
void mutex_destroy(mutex_t *mp)
{
    spinlock_destroy(&mp -> lock); // Destroy a spinlock

    // Vanish when trying to destroy a locking mutex
    if (mp -> status == MUTEX_LOCKED) vanish();  
    mp -> status = MUTEX_UNAVAILABLE;
}

/** @brief The function to lock a mutex
 *
 *  @param a pointer to the list to be initialized
 *  @return nothing
 */
void mutex_lock(mutex_t *mp)
{
    if (mp -> status == MUTEX_UNAVAILABLE) return;
    spinlock_lock(&mp -> lock);
    mp -> status = MUTEX_LOCKED;
}

/** @brief The function to initialize the doubly linked list
 *
 *  @param a pointer to the list to be initialized
 *  @return nothing
 */
void mutex_unlock(mutex_t *mp)
{
    if (mp -> status == MUTEX_UNAVAILABLE) return;
    mp -> status = MUTEX_UNLOCKED;
    spinlock_unlock(&mp -> lock);
}


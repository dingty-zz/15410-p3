/**
* @file spinlock.c
*
* @brief This file provides several functions to manipulate spinlock defined in spinlock.h
*        A spinlock just spins until it grabs a lock.
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
*
*/
#include <syscall.h>
#include "spinlock_type.h"
#include "assert.h"
#include "atomic_xchange.h"

/** @brief Initialize a spinlock
 *
 *  @param sl A pointer to the spinlock
 *  @return void
 */
void spinlock_init(spinlock_t *sl)
{
    sl -> state = SPINLOCK_UNLOCKED;
}

/** @brief Attempts to grab a lock
 *
 *         Using atomic_xchange until it grabs a lock
 *
 *  @param sl A pointer to the spinlock
 *  @return void
 */
void spinlock_lock(spinlock_t *sl)
{
    int is_locked;
    while ((is_locked = atomic_xchange(&(sl->state)))) continue;
}

/** @brief Unlock a spinlock
 *
 *  @param sl A pointer to the spinlock
 *  @return void
 */
void spinlock_unlock(spinlock_t *sl)
{
    sl -> state = SPINLOCK_UNLOCKED;
}

/** @brief Destory a spinlock, just sets it's status to unlock
 *
 *  @param sl A pointer to the spinlock
 *  @return void
 */
void spinlock_destroy(spinlock_t *sl)
{
	// Vanish when trying to destroy a locking spinlock
	if (sl -> state == SPINLOCK_LOCKED)  vanish();
    sl = SPINLOCK_UNLOCKED;
}
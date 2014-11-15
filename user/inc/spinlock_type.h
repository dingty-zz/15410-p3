 /**
 * @file spinlock_type.h
 *
 * @brief This file defines two status of the spinlock and 
 *		  several lock manipulation functions.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _SPINLOCK_TYPE_H
#define _SPINLOCK_TYPE_H

#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0


 /**
 * @brief The state can be either SPINLOCK_LOCKED or SPINLOCK_UNLOCKED.
 *		  We exchange this state with the lock (1) using atomic_xchange to 
 *		  achieve mutual exclusion.
 */
typedef struct spinlock {
    int state;
} spinlock_t;

void spinlock_init(spinlock_t *sl);
void spinlock_lock(spinlock_t *sl);
void spinlock_unlock(spinlock_t *sl);
void spinlock_destroy(spinlock_t *sl);

#endif /* _SPINLOCK_TYPE_H */

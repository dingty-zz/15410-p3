 /**
 * @file mutex_type.h
 *
 * @brief Defines a mutex and provides several mutex library functions
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */
#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H
 
#define MUTEX_LOCKED 1
#define MUTEX_UNLOCKED 0
#define MUTEX_UNAVAILABLE -1

typedef struct mutex {
    int status;			// The status for the mutex
    int tid;			// The thread who holds it
    int count;			// Number of threads who holds the lock
} mutex_t;

int mutex_init(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);
void mutex_destroy(mutex_t *mp);

#endif /* _MUTEX_TYPE_H */

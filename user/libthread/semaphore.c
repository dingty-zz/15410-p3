/**
* @file semaphore.c
*
* @brief The cond_var is implemented with a mutex and a cond_var. It's implemented on
*        top of mutexes and condition variables.
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
*
*/
#include <syscall.h>
#include "sem_type.h"
#include "spinlock_type.h"
#include "linked_list.h"
#include "mutex.h"
#include "cond_type.h"
#include "assert.h"

/** @brief The function to initialize the semaphore
 *
 *  @param sem The semaphore
 *  @param count The available source (semaphore value)
 *  @return 0 on success and -1 an error (unlikely)
 */
int sem_init(sem_t *sem, int count)
{
    mutex_init(&(sem -> mutex));
    cond_init(&(sem -> cvar));
    sem -> count = count;
    return 0;
}

/** @brief The function to destory a semaphore
 *
 *  @param sem The semaphore
 *  @return void
 */
void sem_destroy(sem_t *sem)
{
    mutex_destroy(&sem -> mutex);
    cond_destroy(&sem -> cvar);
    sem = 0;
}

/** @brief Allow to reduce the sema value or block on waiting
 *
 *  @param sem The semaphore
 *  @return void
 */
void sem_wait(sem_t *sem)
{
    mutex_lock(&sem -> mutex);
    while (sem -> count <= 0)
        cond_wait(&sem -> cvar, &sem -> mutex);     // Busy waiting until the condition is true
    sem -> count--;  // update sem value
    mutex_unlock(&sem -> mutex);

}

/** @brief Wake up a thread waiting on this semaphore
 *
 *  @param sem The semaphore
 *  @return void
 */
void sem_signal(sem_t *sem)
{
    mutex_lock(&sem -> mutex);
    sem -> count++; // update sem value
    cond_signal(&sem -> cvar);
    mutex_unlock(&sem -> mutex);
}
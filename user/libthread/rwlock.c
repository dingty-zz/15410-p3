/**
* @file rwlock.c
*
* @brief The implementation for rwlocks, more high level
*        descriptions are in the header file
*
* @author Jonathan Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
*
*/
#include <syscall.h>
#include <thread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <rwlock.h>

/** @brief The initialization function of reader writer lock
 *
 *  Initilize the reader writer lock. The reader writer lock structure
 *  has five semaphores in it. It works as the documentation in the
 *  README file states;
 *
 *  @param pointer to the lock structure
 *  @return 0 on success;
 */
int rwlock_init(rwlock_t *rwlock)
{
    assert(rwlock != NULL);
    rwlock->readcnt = 0;
    rwlock->writecnt = 0;
    sem_init(&rwlock->rcnt_mutex, 1);
    sem_init(&rwlock->wcnt_mutex, 1);
    sem_init(&rwlock->gen_mutex, 1);
    sem_init(&rwlock->w, 1);
    sem_init(&rwlock->r, 1);
    return 0;
}

/** @brief The function to destroy the lock
 *
 *  destroy every element in the structure, and set the counters;
 *
 *  @param pointer to the lock structure
 *  @return nothing
 */
void rwlock_destroy(rwlock_t *rwlock )
{
    rwlock->readcnt = NULL_CNT;
    rwlock->writecnt = NULL_CNT;
    sem_destroy(&rwlock->rcnt_mutex);
    sem_destroy(&rwlock->wcnt_mutex);
    sem_destroy(&rwlock->gen_mutex);
    sem_destroy(&rwlock->w);
    sem_destroy(&rwlock->r);
}

/** @brief The function to lock the rwlock
 *
 *  Lock the reader writer lock; two cases:
 *  1. if it's a reader section, then increase the reader count,
 *     and if this is the first reader that comes into the buffer,
 *     we lock the writer lock until the the last reader comes out
 *     in the unlock function;
 *  2. if it's the writer section, we increase the writer count.
 *     Since we only allow one writer, we don't check count.
 *     And then we lock both reader and writer.
 *
 *  @param pointer to the lock structure,
 *  @return 0 on success;
 */
void rwlock_lock(rwlock_t *rwlock, int type )
{
    //reader section
    if (type == RWLOCK_READ)
    {
        sem_wait(&rwlock->gen_mutex);
        sem_wait(&rwlock->r);
        sem_wait(&rwlock->rcnt_mutex);
        rwlock->readcnt++;
        if (rwlock->readcnt == 1)
            sem_wait(&rwlock->w);
        sem_signal(&rwlock->rcnt_mutex);
        sem_signal(&rwlock->r);
        sem_signal(&rwlock->gen_mutex);
    }
    //writer section
    else
    {
        sem_wait(&rwlock->wcnt_mutex);
        rwlock->writecnt++;
        sem_wait(&rwlock->r);
        sem_wait(&rwlock->w);
        sem_signal(&rwlock->wcnt_mutex);
    }
}

/** @brief The function to unlock the rwlock
 *
 *  Unlock the reader writer lock; two cases:
 *  1. If it's a reader section, then decrease the reader count,
 *     and if it's already the last reader in the buffer, we tell
 *     writer that it can proceed
 *  2. if it's the writer section, we increase the writer count.
 *     Since we only allow one writer, we don't check count.
 *     And then we lock both reader and writer.
 *
 *  @param pointer to the lock structure,
 *  @return 0 on success;
 */
void rwlock_unlock( rwlock_t *rwlock )
{
    //unlock a reader
    if (rwlock->readcnt > 0)
    {
        sem_wait(&rwlock->rcnt_mutex);
        rwlock->readcnt--;
        //tell writer you can proceed
        if (rwlock->readcnt == 0)
            sem_signal(&rwlock->w);
        sem_signal(&rwlock->rcnt_mutex);
    }
    //unlock a writer
    else
    {
        sem_wait(&rwlock->wcnt_mutex);
        rwlock->writecnt--;
        //tell reader you can proceed
        if (rwlock->writecnt == 0)
            sem_signal(&rwlock->r);
        sem_signal(&rwlock->wcnt_mutex);
    }

}

/** @brief Downgrade the lock in write mode when it's no longer
 *         requires exclusive access to the protected resource.
 *
 *  @param pointer to the lock structure
 *  @return nothing
 */
void rwlock_downgrade( rwlock_t *rwlock )
{
    sem_wait(&rwlock->wcnt_mutex);
    sem_wait(&rwlock->rcnt_mutex);
    rwlock->writecnt--;
    rwlock->readcnt++;
    sem_signal(&rwlock->rcnt_mutex);
    sem_signal(&rwlock->wcnt_mutex);
    sem_signal(&rwlock->r);
}
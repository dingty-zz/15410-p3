 /**
 * @file rwlock_type.h
 *
 * @brief We implement rw lock based on the binary semaphores. 
 *        Each semaphore has its own purpose. We used five semaphores in total
 *        for different types of purposes. The benefits for using binary
 *        semaphore are:
 *        1. It's clear to assign different responsibilities for different
 *           semaphores. Each of them will have its own purpose.
 *        2. The implementation will only depend on semaphores, instead of 
 *           a bunch of different syncronization services. This means that
 *           if any other syncronization services fail, as long as semaphore
 *           works, the reader writer lock will still work fine.
 *
 *        On top of the semaphores, we used two integers for recording total
 *        number of readers and writers. The interaction between semaphores and 
 *        the two counts are as following:
 *        A. When initilized, count will be set to 0 and all semaphores will be
 *           to 1 as unlocked state.
 *        B. If lock function is called, we need the check whether it's by 
 *           reader or by writer, and then we will use the general semaphore for 
 *           the lock action. We will use the reader semaphore to lock that only
 *           one thread could have the authorization to act on reader side. We 
 *           will finally use the reader count semaphore to lock the actions for
 *           the read count changes. When the first reader, we will lock writers
 *           from accessing.
 *           It it's writer's action, then we don't have the two first locks,
 *           since we wnat to give wrtier higher priority in the action queue.
 *           AKA, we are implementing the writer prefer method.
 *        C. If unlock function is called, we just need to lock count 
 *           semaphores, and change the corresponding count number, then unlock
 *           writer or reader depending on whether the actor is the last writer 
 *           or the last reader already
 *        D. For downgrade, we just decrease writer count and increase reader
 *           count, and finally unlock readers since the writer that was just
 *           now writing must be the only writer and all other readers, after
 *           the downgrade happens, will be granted the authorization to read
 *           unless a writer is queuing, which will have higher priority by 
 *           locking reader lock in line 93 in the source file of the 
 *           implementation.
 *         
 *
 *
 *        
 *
 * @author Jonathan Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 * @bugs   no known bugs
 */
 
#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <sem_type.h>

#define NULL_CNT -1

struct rwlock {
    int         readcnt;  //init 0
    int         writecnt; //init 0
    //protects readcnt
    sem_t       rcnt_mutex; //init 1
    //protects writecnt
    sem_t       wcnt_mutex; //init 1
    sem_t       gen_mutex; //init 1
    //protects write
    sem_t       w;       //init 1
    //protects read
    sem_t       r;       //init 1
};

typedef struct rwlock rwlock_t;

#endif /* _RWLOCK_TYPE_H */
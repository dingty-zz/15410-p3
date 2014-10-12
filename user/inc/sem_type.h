 /**
 * @file sem_type.h
 *
 * @brief Defines a semaphore and provides several semaphore library functions
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */
#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include "mutex_type.h"
#include "cond_type.h"


typedef struct sem {
	int 	count;		// Count of the usable source 
  	mutex_t mutex;		// Associated mutex
  	cond_t 	cvar;		// Associated cond_var
} sem_t;

int sem_init(sem_t *sem, int count);
void sem_destroy(sem_t *sem);
void sem_wait(sem_t *sem);
void sem_signal(sem_t *sem);

#endif /* _SEM_TYPE_H */

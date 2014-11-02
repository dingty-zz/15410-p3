 /**
 * @file cond_type.h
 *
 * @brief Defines a cond_var and provides several cond_var library functions
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 */
#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include "linked_list.h"
#include "spinlock_type.h"
#include "mutex_type.h"

#define CVAR_UNAVAILABLE -1
#define CVAR_AVAILABLE 0

typedef struct cond {
	int status; 			// The status of this condition variable
	spinlock_t lock;	// A spinlock to avoid race conditions when add/remove the waitlist
	list wait_list;		// The list that store waiting threads
} cond_t;

int cond_init(cond_t *cv);
void cond_destroy(cond_t *cv);
void cond_wait(cond_t *cv, mutex_t *mp);
void cond_signal(cond_t *cv);
void cond_broadcast(cond_t *cv);

#endif /* _COND_TYPE_H */

 /**
 * @file thr_internals.h
 *
 * @brief Defines a thread data structure to store
 *		  useful thread data for maintaining current threads.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _THR_INTERNALS_H
#define _THR_INTERNALS_H
#include "mutex_type.h"
#include "cond_type.h"
#include "linked_list.h"
 
#define THREAD_EXIT -1
#define THREAD_RUNNING 1

typedef struct thread_type {
    int 	tid;		// Thread id for this thread
	void 	*base;		// The page base for this thread's stack,
						// store it for future remove page use
	int 	status;		// Current status, exit or running
	void 	*statusp;	// The status pointer, used in join and exit
	int 	joining_tid;  // The tid of the thread who wants to join it
 
 	// The private mutex and cond_var to avoid race condition when try to 
 	// modify the thread data above and also used for thread join&exit   
    mutex_t t_mutex;	
    cond_t 	t_cond;
} thread_t;

// Search the thread data structure from the tcb by tid
thread_t *search_thread_by_id(list *l, int tid);

#endif /* _THR_INTERNALS_H */

 /**
 * @file do_switch.h
 *
 * @brief This file defines two status of the spinlock and 
 *		  several lock manipulation functions.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

 
void schedule(int tid);

TCB *context_switch(TCB *current, TCB *next);

void prepare_init_thread(TCB *next);

TCB *list_search_tid(list *l, int tid);

#endif /* _SCHEDULER_H */

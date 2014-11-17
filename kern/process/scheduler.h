 /**
 * @file scheduler.h
 *
 * @brief This file defines several functions that are used by the 
 *		  scheduling system
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
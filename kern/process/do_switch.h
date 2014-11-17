/**
 * @file do_switch.h
 *
 * @brief This file defines the doswitch function that context switches
 *		  from the current thread to the next thread
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _DO_SWITCH_H
#define _DO_SWITCH_H

#include "control_block.h"
/** @brief context switching from the current thread to the next 
 *	  	   thread by swapping the kernel stack pointer %esp
 *	@params current tcb pointer of the current thread
 *	@params next tcb pointer of the next thread
 *	@params state the state for the next thread
 */
void do_switch(TCB *current, TCB *next, int state);


#endif /* _DO_SWITCH_H */
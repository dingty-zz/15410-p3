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

#ifndef _DO_SWITCH_H
#define _DO_SWITCH_H

#include "control_block.h"
 
void do_switch(TCB *current, TCB *next, int state);


#endif /* _DO_SWITCH_H */

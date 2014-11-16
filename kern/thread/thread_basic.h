 /**
 * @file thread.h
 *
 * @brief This file defines two status of the spinlock and 
 *		  several lock manipulation functions.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _THREAD_H
#define _THREAD_H

#include "control_block.h"
 
void thr_init();

TCB *thr_create(unsigned int eip, int run);

#endif /* _THREAD_H */

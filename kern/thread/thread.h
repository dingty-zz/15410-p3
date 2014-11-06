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
 
int thr_init();

TCB *thr_create(simple_elf_t *se_hdr, int run);

int thr_exit();


#endif /* _THREAD_H */

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

#ifndef _PROCESS_H
#define _PROCESS_H

 
int process_init();


int process_create(const char *filename, int run);


int process_exit();


#endif /* _PROCESS_H */

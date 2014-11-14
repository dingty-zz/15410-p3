/** @file control_block.h
 *
 *  @brief sys land sys call handler, 
 * installed with interrupt number
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#ifndef _SYSCALL_HANDLER_H
#define _SYSCALL_HANDLER_H


void sys_swexn_wrapper();
void thread_fork_wrapper();

#endif
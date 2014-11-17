/** @file syscall_handler.h
 *
 *  @brief sys land sys call handler, installed in handler_install
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#ifndef _SYSCALL_HANDLER_H
#define _SYSCALL_HANDLER_H


/** @brief The wrapper for sys land swexn handler
 *
 *	This function will first push all the arguments, and call swexn_wrapper
 *	After that, it will check if the ureg is non null. If so, the wrapper will
 *  adopt the register values specified and then return to user mode
 *
 *  @no nothing
 *  @return nothing
 */
void sys_swexn_wrapper();

/** @brief The wrapper for sys land thread_fork
 *
 *  @param nothing
 *  @return nothing
 */
void thread_fork_wrapper();


void sys_vanish();
void sys_set_status();

#endif /* _SYSCALL_HANDLER_H */
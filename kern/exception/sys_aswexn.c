/** @file sys_aswexn.c
 *
 *  @brief Defines Asynchronous Software Exceptions system calls
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
#include <ureg.h>
#include "control_block.h"
#include "simics.h"

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_asignal(int tid, int signum){
	if (tid == -1)
	{
		/* code */
	}
	return 0;
}

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_await(sigmask_t mask){
	return 0;
}

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_amask(sigaction_t action, sigmask_t mask, sigmask_t *oldmaskp){
	return 0;
}

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_atimer(int mode, int period){
	return 0;
}

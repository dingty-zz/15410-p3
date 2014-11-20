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
int sys_asignal(int tid, int signum){
	return 0;
}

int sys_await(sigmask_t mask){
	return 0;
}

int sys_amask(sigaction_t action, sigmask_t mask, sigmask_t *oldmaskp){
	return 0;
}

int sys_atimer(int mode, int period){
	return 0;
}

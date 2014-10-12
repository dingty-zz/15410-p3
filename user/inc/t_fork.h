 /**
 * @file t_fork.h
 *
 * @brief This file contains thread_fork helper function to 
 *		  run child thread on that stack.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

/** @brief After creating a stack for a thread, we try to run child thread
 *		   on that stack
 *
 *		   Note that this is not a simple wrapper for thread_fork because
 *		   we have to move to the new stack area and run the child on that
 *		   stack. 
 *		   To do this, we have to push the arguments (func, arg) on to the 
 *		   newly created stack and call the run_child wrapper, which wait until
 *		   child's thread_t data structure is created and inserted into global tcb,
 *		   and finally run func(arg). The definition of run_child is in the
 *	 	   thread_mgmt.c file.
 * 
 *  @param thread_esp The stack pointer for the child's new stack
 *	@param func Child's function
 *	@param arg  Argument for func
 *  @return int The tid for child's thread
 */
int thread_fork(void *thread_esp, void *(*func)(void *), void *arg);
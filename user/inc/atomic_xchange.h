 /**
 * @file atomic_xchange.c
 *
 * @brief A atomic exchange function to achieve mutual exclusion.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */


/** @brief Using XCHG instruction to atomically exchanging registers
 *   	   to ensure mutual exclusion
 *  	
 *		   When XCHG instruction is performed, the bus will be locked,
 *		   we use this feature to achieve mutual exclusion. We call this
 *		   function in our spinlock implementation.
 *
 *  @param state_ptr Current status of the lock, use it to exchange the lock (1)
 *					 in side the assembly.
 *  @return int The exchanged result.
 */
int atomic_xchange(int *state_ptr);
#include <syscall.h>
#include "simics.h"
#include "loader.h"
#include "control_block.h"
#include "console.h"

extern TCB *current_thread;

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
void sys_halt(void)
{
	clear_console();        
    lprintf("Shutting down...");
    sim_halt();

    // TODO power off
    while(1);
}

int sys_readfile(char *filename, char *buf, int count, int offset)
{
	// verify filename ,buf
	if (count < 0 || offset < 0)
	{
	return -1;
	}
	int size = 0;
	size = getbytes(filename, offset, count, buf);

	return size;

}

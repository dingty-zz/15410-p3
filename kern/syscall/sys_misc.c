#include <syscall.h>
#include "simics.h"
#include "loader.h"

extern TCB *current_thread;

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
	current_thread -> state = THREAD_NONSCHEDULABLE;
	size = getbytes(filename, offset, count, buf);
	current_thread -> state = THREAD_RUNNING;
	return size;

}

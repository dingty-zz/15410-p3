#include <syscall.h>


void _halt(void)
{
	clear_console();        
    printf("Shutting down...");
    sim_halt();

    // TODO power off
    while(1);
}

int readfile(char *filename, char *buf, int count, int offset)
{
	return -1;
}

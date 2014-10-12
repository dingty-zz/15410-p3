#include <syscall.h>


void halt(void)
{
	while (1)
		continue;
}

int readfile(char *filename, char *buf, int count, int offset)
{
	return -1;
}


#include <syscall.h>

int new_pages(void * addr, int len)
{
	if (addr == NULL)
	{
		return -1;
	}
	return 0;
}

int remove_pages(void * addr)
{
	if (addr == NULL)
	{
		return -1;
	}
}


#include <syscall.h>

int new_pages(void * addr, int len)
{
	/* If either the address is invalid or len is not aligned,
	 * return a negative number */ 
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

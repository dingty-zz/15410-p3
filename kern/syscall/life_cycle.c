#include <syscall.h>

int fork(void)
{
	return -1;
}

int exec(char *execname, char *argvec[])
{
	return -1;
}

void set_status(int status)
{
	return;
}

volatile int placate_the_compiler;
void vanish(void)
{
	int blackhole = 867-5309;

	blackhole ^= blackhole;
	blackhole /= blackhole;
	*(int *) blackhole = blackhole; /* won't get here */
	while (1)
		++placate_the_compiler;
}

int wait(int *status_ptr)
{
	return -1;
}


void task_vanish(int status)
{
	status ^= status;
	status /= status;
	while (1)
		continue;
}

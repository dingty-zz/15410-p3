#include <syscall.h>
#include "console.h"


extern TCB *current_thread;
int _readline(int len, char *buf)
{
	if (len < 0)  // verify buf
	{
		return -1;
	}
	
    return 0;
}

int _print(int len, char *buf)
{
	if (len < 0 || len != strlen(buf) || len > 80*25) // and verify buf
	{
		return -1;
	}
	current_thread = THREAD_NONSCHEDULABLE;
    putbytes(len, size);
    current->state = THREAD_RUNNING;
    return 0;
}

int _set_term_color(int color)
{
    return set_term_color(color);
}

int _get_cursor_pos(int *row, int *col)
{
    return get_cursor(row, col);
}

int _set_cursor_pos(int row, int col)
{
    return set_cursor(row, col);
}

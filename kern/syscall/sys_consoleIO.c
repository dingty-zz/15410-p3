#include <syscall.h>
#include "console.h"
#include "hardware/keyboard.h"
#include "control_block.h"
#include "string.h"
#include "simics.h"


extern TCB *current_thread;
int sys_readline(int len, char *buf)
{

    if (current_read_thread < 0)
        current_read_thread = current_thread->tid;
    else
    {
        list_insert_last(&readline_queue, &current_thread->readline_node);

        while (next_read_thread!=current_thread->tid)
        {
            yield(current_read_thread);
        }
        current_read_thread = current_thread->tid;
        next_read_thread = -1;
    }
    	

    if (len < 0)  // verify buf
	{
		return -1;
	}
    int count = 0;
	int c;
    while (1) {
        while ((c = readchar()) == -1);
        buf[count] = c;
        count ++;
        
        if (c == '\b') {
            count --;
            if (count > 0) count--;
        }
        if (count == len - 1) {
            buf[count] = '\0';
            break;
        }
        if (c == '\n') {
            buf[count-1] = '\0';
            count--;
            break;
        }
    }
    lprintf("readline done, the buf: %s",buf);
   
    
    node* next = list_delete_first(&readline_queue);
    if (next == NULL)
        next_read_thread = -1;
    else
        next_read_thread = list_entry(next, TCB, readline_node);
    return count;
}

int sys_print(int len, char *buf)
{
	if (len < 0 || len != strlen(buf) || len > 80*25) // and verify buf
	{
		return -1;
	}

    putbytes(buf, len);

    return 0;
}

// int sys_set_term_color(int color)
// {
//     return set_term_color(color);
// }

int sys_get_cursor_pos(int *row, int *col)
{
    get_cursor(row, col);
    return 0;
}

int sys_set_cursor_pos(int row, int col)
{
    return set_cursor(row, col);
}

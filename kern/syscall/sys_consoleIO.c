#include <syscall.h>
#include "console.h"
#include "hardware/keyboard.h"
#include "control_block.h"
#include "string.h"
#include "simics.h"
#include "stddef.h"
#include <asm.h>
#include "process/scheduler.h"
#include "memory/vm_routines.h"

extern TCB *current_thread;
int sys_readline(int len, char *buf)
{
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;
    disable_interrupts();
    current_thread -> state = THREAD_READLINE;
    enable_interrupts();
    schedule(-1);
    lprintf("in readline again");
    // MAGIC_BREAK;
    disable_interrupts();

    if (len < 0)  // verify buf
	{
		return -1;
	}
    int count = 0;
	int c;
    while ((c = readchar()) != -1) 
    {
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
    total_num = total_num - count - 1;
    lprintf("the total num is %d", total_num);
    enable_interrupts();

    return count;
}

int sys_print(int len, char *buf)
{
    lprintf("len is: %d",len);
    lprintf("strlen is: %d",strlen(buf));
    lprintf("buf is: %s",buf);
 //    MAGIC_BREAK;
	// if (len < 0 || len != strlen(buf) || len > 80*25) // and verify buf
	// {
	// 	return -1;
	// }
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;

    putbytes(buf, len);
    // bzero(buf, len);

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

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

#define MAX_READ_LEN 4096  // Default buffer length, to hold 512 scan codes in a queue

int sys_readline(int len, char *buf)
{
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;
    if (len > MAX_READ_LEN) return -1;
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> state = THREAD_READLINE;
    mutex_unlock(&current_thread -> tcb_mutex);
    schedule(-1);
    disable_interrupts();
    lprintf("in readline again");
    lprintf("the total num is %d", total_num);
    lprintf("the len is %d", len);
    if (len < 0)  // verify buf
	{
		return -1;
	}
    int count = 0;
	char c;
    lprintf("very original count is: %d",count);
    while ((c = readchar()) != -1) 
    {
        lprintf("i have read %c",c);
        buf[count] = c;
        count ++;
        lprintf("original count is: %d",count);

        if (c == '\b') {
            lprintf("this is bbb");
            count --;
            if (count > 0) count--;
        }

        if (count == len) {
            lprintf("wtf");
            buf[count-1] = '\0';
            break;
        }
        
        if (c == '\n') {
            lprintf("this is nnn");
            buf[count-1] = '\n';
            break;
        }
    }
    // MAGIC_BREAK;
    lprintf("readline done, the buf: %s",buf);
    total_num = total_num - count;
    lprintf("the total num is %d", total_num);
    enable_interrupts();

    return count;
}

int sys_print(int len, char *buf)
{
    mutex_lock(&print_lock);
    lprintf("len is: %d",len);
    lprintf("strlen is: %d",strlen(buf));
    lprintf("buf is: %s",buf);
	// if (len < 0 || len != strlen(buf) || len > 80*25) // and verify buf
	// {
	// 	return -1;
	// }
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;

    putbytes(buf, len);
    // bzero(buf, len);
    mutex_unlock(&print_lock);
    return 0;
}

// int sys_set_term_color(int color)
// {
//     return set_term_color(color);
// }

int sys_get_cursor_pos(int *row, int *col)
{
    if (!is_user_addr(row) || !addr_has_mapping(row)) return -1;
    if (!is_user_addr(col) || !addr_has_mapping(col)) return -1;   
    get_cursor(row, col);
    return 0;
}

int sys_set_cursor_pos(int row, int col)
{
    return set_cursor(row, col);
}

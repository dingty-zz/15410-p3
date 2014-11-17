/** @file sys_consoleIO.c
 *
 *  @brief This file provides the implementation for readline, print,
 *         get_cursor_pos and set_cursor_pos system calls
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
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

#define MAX_READ_LEN 4096  // Default buffer length, to hold 512 scan codes in a queue

/** @brief Read line from the console and store it into the buf. The
 *         length is also specified
 *
 *  @param len the number of bytes read to the buffer
 *  @param buf the buffer to be filled in
 *  @return int the number of actual bytes read, -1 otherwise
 **/
int sys_readline(int len, char *buf)
{
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;
    if (len > MAX_READ_LEN) return -1;
    // Block the current thread no matter what, let the keyboard 
    // wakes it up when there are new characters
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> state = THREAD_READLINE;
    mutex_unlock(&current_thread -> tcb_mutex);
    schedule(-1);
    disable_interrupts();

    if (len < 0)  // verify buf
	{
		return -1;
	}
    int count = 0;
	char c;
    // Continuously get chars
    while ((c = readchar()) != -1) 
    {
        buf[count] = c;
        count ++;

        // If the character contains a backspace
        if (c == '\b') {
            count --;
            if (count > 0) count--;
        }

        // If we have filled in the buffer
        if (count == len) {
            buf[count-1] = '\0';
            break;
        }
        // If a new line character is read, we return
        if (c == '\n') {
            buf[count-1] = '\n';
            break;
        }
    }
    // readline done
    total_num = total_num - count;
    enable_interrupts();

    return count;
}

/** @brief Print the bytes from buf to the console, len bytes
 *         are printed
 *
 *  @param len the number of bytes to be printed
 *  @param buf the buffer that contains characters
 *  @return int 0 on success, -1 otherwise
 **/
int sys_print(int len, char *buf)
{
    // The print routine will not be interleaved
    mutex_lock(&print_lock);
	if (len < 0 || len > 80*25) // verify the length
	{
		return -1;
	}
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;

    putbytes(buf, len);
    mutex_unlock(&print_lock);
    return 0;
}

// Implementation please see console.c
// int sys_set_term_color(int color)
// {
//     return set_term_color(color);
// }

/** @brief Get current terminal cursor position
 *
 *  @param row the pointer stores the cursor row
 *  @param col the pointer stores the cursor col
 *  @return int 0 on success, -1 otherwise
 **/    
int sys_get_cursor_pos(int *row, int *col)
{
    if (!is_user_addr(row) || !addr_has_mapping(row)) return -1;
    if (!is_user_addr(col) || !addr_has_mapping(col)) return -1;   
    get_cursor(row, col);
    return 0;
}

/** @brief Set current terminal cursor position
 *
 *  @param row new cursor row
 *  @param col new cursor col
 *  @return int 0 on success, -1 otherwise
 **/  
int sys_set_cursor_pos(int row, int col)
{
    return set_cursor(row, col);
}

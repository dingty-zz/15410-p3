/** @file keyboard.c
 *
 *  @brief Allocates a keyboard buffer to hold the scan codes with a
 *  default size of 512. Treat this buffer as a queue. The start represents
 *  the start of the queue and the tail represents the tail of the
 *  queue. The tail always points to the place where the next element (a char)
 *  should be enqueued. The start points to the place where the element
 *  should be dequeued. We also designed the buffer to be a ring buffer always
 *  with the static length of 512, when over 512 chars, the tail will go back
 *  to the beginning of the buffer
 *
 *  @author Tianyuan Ding (tding)
 *  @author Jonathan Xianqi Zeng (xianqiz)
 *  @bug No known bugs
 */

#include <multiboot.h>
#include <stdio.h>
#include <seg.h>
#include <asm.h>
#include <malloc.h>
#include "simics.h"
#include "keyhelp.h"
#include "interrupt_defines.h"
#include <assert.h>
#include "keyboard.h"
#include "console.h"
#include "control_block.h"

/* Default buffer length, to hold 512 scan codes in a queue */
#define BUF_LEN 512  

/* Keyboard buffer with default length being 512 */
static uint8_t* s_queue;  
static int start;
static int tail;

static int queue_empty(uint8_t *q);
static void enqueue(uint8_t *q, uint8_t s);
static uint8_t dequeue(uint8_t *q);


/** @brief read a char from the char queue
 *
 *  @param nothing
 *  @return the char in the queue
 **/
int readchar(void)
{
    if (queue_empty(s_queue)) return -1;

    char c  = dequeue(s_queue);
    return c;
}

/** @brief the keyboard handler after keyboard interrupt
 *
 *  The handler first checks if the char has data, if not checks if 
 *  the data is enter or not for waking up readline
 *
 *  @param nothing
 *  @return nothing
 **/
void keyboard_handler()
{
    // lprintf("keyboard is called");
    uint8_t scancode = inb(KEYBOARD_PORT);
    kh_type aug_char = process_scancode(scancode);
    char real_char;
    TCB* next_readline_thread = NULL;
    node *n;
    /* found flag to specify if a waiting thread on readline is found */
    int found_flag = 0;
    /* When aug_char has data, go and extract it's char value */
    if (!KH_HASDATA(aug_char) || !KH_ISMAKE(aug_char))
    {
        // lprintf("keyboard done1");
        outb(INT_CTL_PORT, INT_ACK_CURRENT);
        return;
    }
    else
    {
        real_char = KH_GETCHAR(aug_char);
        // lprintf("keyboard 2");
        /*c heck if the key is \n */
        if (real_char == '\n')
        {
            /* look for a readline thread waiting on enter */
            for (n = list_begin(&blocked_queue); n != NULL; n = n -> next)
            {
                next_readline_thread = list_entry(n, TCB, thread_list_node);
                if (next_readline_thread -> state == THREAD_READLINE)
                {
                    next_readline_thread -> state = THREAD_RUNNABLE;
                    list_delete(&blocked_queue, &next_readline_thread->thread_list_node);
                    list_insert_last(&runnable_queue, &next_readline_thread->thread_list_node);
                    /* Found one! */
                    found_flag = 1;
                    break;
                }
            }
            if (!found_flag) 
            {
                putbyte(real_char);
                outb(INT_CTL_PORT, INT_ACK_CURRENT);
                return;
            }
        }
        else if (real_char == '\b')
        {
            /* if backspace, and there is no previous char entered on screen, 
             * then simply ignore the backspace */
            if (total_num==0)
            {
                outb(INT_CTL_PORT, INT_ACK_CURRENT);
                return;
            }
            else
            {
                 total_num = total_num-2;
            }
        }
        total_num++;
        // lprintf("the total num is %d", total_num);
        enqueue(s_queue, real_char);
        // lprintf("I read %x", (unsigned int)aug_char);
        /*signal to next_readline_node that it could read the line*/
        putbyte(real_char);
        outb(INT_CTL_PORT, INT_ACK_CURRENT);
        return;
    }
}

/** @brief the function to set up the keyboard queue
 *
 *  @param nothing
 *  @return nothing
 **/
void setup_keyboard()
{
    s_queue = (uint8_t *)calloc(sizeof(uint8_t), BUF_LEN);
    if (s_queue == NULL) panic("Memory allocation fails!");
    start = 0;
    tail = 0;
    return;
}

/** @brief Determine if the given queue is empty
 *
 *  If start == tail, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
static int queue_empty(uint8_t *q)
{
    if (start == tail) return 1;
    else return 0;
}

/** @brief Enqueue the given element into the queue
 *
 *  If the buffer is full, in other words, tail will reach
 *  outside of the queue, but enqueue is called, we calculate
 *  the number of elements in the queue and reallocates a queue with
 *  double size, and enqueue. If start == tail, this means that we 
 *  ran out of space. We need to round the ring buffer by mod the 
 *  pointer index by the buffer length
 *
 *  @param q The pointer to the queue
 *  @param s The element to be enqueued
 *  @return void
 **/
static void enqueue(uint8_t *q, uint8_t s)
{
    q[tail] = s;
    tail++;   // Increment the tail index
    tail = tail % BUF_LEN;
    if (start == tail)
    {
        start++;
        start = start % BUF_LEN;
    }
    return;
}

/** @brief Dequeue the element out of the queue
 *
 *  Return the element pointed by start and add 1 to start. Note
 *  we don't need to check if queue is empty here because first
 *  this function is only used in this file, and second, it's caller,
 *  readchar will do the checking in line 40
 *
 *  @param q The pointer to the queue
 *  @return The element at the front of the queue
 **/
static uint8_t dequeue(uint8_t *q)
{
    uint8_t res = q[start];
    start++;  // Increment the start index
    start = start % BUF_LEN;
    return res;
}
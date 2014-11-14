/** @file keyboard.c
 *
 *  @brief Allocates a keyboard buffer to hold the scan codes with a
 *  default size 100. Treat this buffer as a queue. The top represents
 *  the top of the queue and the bottom represents the bottom of the
 *  queue. The bottom always points to the place where the next element
 *  should be enqueued. The top points to the place where the element
 *  should be dequeued. The queue is implemented as an unbounded array in
 *  15-122 when resizing.
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#include <multiboot.h>
// #include <s.h>
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

#define BUF_LEN 100   // Default buffer length, to hold 100 scan codes in a queue

static uint8_t *s_queue;           // Keyboard buffer
static int top;
static int bottom;
static int size = BUF_LEN;         // The size of the current queue

static int queue_empty(uint8_t *q);
static void enqueue(uint8_t *q, uint8_t s);
static uint8_t dequeue(uint8_t *q);

int readchar(void)
{
    if (queue_empty(s_queue)) return -1;

    char c  = dequeue(s_queue);
    // kh_type aug_char = process_scancode(k);

    // /* When aug_char has data, go and extract it's char value */
    // if (KH_HASDATA(aug_char))
    // {
    //      When key released, we know a key press event is done, so we
    //        will return the respective character 
    //     if (!KH_ISMAKE(aug_char))
    return c;
    // }
    // // Note that we don't have to worry about the state of the modifier keys
    // return -1;
}

void keyboard_handler()
{
    // disable_interrupts();   // Defer other interrupts
    lprintf("keyboard is called");
    uint8_t scancode = inb(KEYBOARD_PORT);
    kh_type aug_char = process_scancode(scancode);
    char real_char;
    /* When aug_char has data, go and extract it's char value */
    if (!KH_HASDATA(aug_char) || !KH_ISMAKE(aug_char))
    {
        lprintf("keyboard done1");
        outb(INT_CTL_PORT, INT_ACK_CURRENT);
        return;
    }
    else
    {
        lprintf("I read %x", (unsigned int)aug_char);
        real_char = KH_GETCHAR(aug_char);
        enqueue(s_queue, real_char);
        putbyte(real_char);
        outb(INT_CTL_PORT, INT_ACK_CURRENT);
        lprintf("keyboard done2");
        return;
    }
    // enable_interrupts();   // After we enqueued the scancode, we enable interrupts
}

void setup_keyboard()
{
    s_queue = (uint8_t *)calloc(sizeof(uint8_t), size);
    if (s_queue == NULL) panic("Memory allocation fails!");
    top = 0;
    bottom = 0;
}

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
static int queue_empty(uint8_t *q)
{
    if (top == bottom) return 1;
    else return 0;
}

/** @brief Enqueue the given element into the queue
 *
 *  If the buffer is full, in other words, bottom will reach
 *  outside of the queue, but enqueue is called, we calculate
 *  the number of elements in the queue and reallocates a queue with
 *  double size, and enqueue. If top == bottom but still bottom will
 *  reach outside of the queue, which is often the case because the
 *  application in this project will often call readchar, causing
 *  the queue becomes empty, we reallocate a queue with BUF_LEN size
 *  and reset top and bottom. After all, we want to avoid the case when
 *  bottom will point outside of the queue.
 *
 *  @param q The pointer to the queue
 *  @param s The element to be enqueued
 *  @return void
 **/
static void enqueue(uint8_t *q, uint8_t s)
{

    /* When the buffer is full */
    if (bottom + 1 == size)
    {
        size = bottom - top;
        /* The bottom will reach out of the buffer but nothing is in
         * the queue, then we reallocate a 100 sized queue to avoid
         * index out of bounds for future enqueue */
        if (size == 0)
        {
            size = BUF_LEN;
            free(s_queue);  // Free the previous queue
            s_queue = (uint8_t *)calloc(sizeof(uint8_t), size);
            if (s_queue == NULL) panic("Memory allocation fails!");
            bottom = 0;   // Reset top and bottom
            top = 0;
        }
        else
        {
            int i;
            bottom = 0;
            size *= 2;   // We will allocate a queue with twice the size
            uint8_t *new_queue = (uint8_t *)calloc(sizeof(uint8_t), size);
            if (new_queue == NULL) panic("Memory allocation fails!");
            for (i = 0; i < size / 2; ++i)  // copy from the old queue to new queue
            {
                new_queue[i] = s_queue[top + i];
                bottom ++;
            }
            top = 0;
            free(s_queue);  // Free the previous queue
            s_queue = new_queue;
        }
    }
    q[bottom] = s;
    bottom++;   // Increment the bottom index
}

/** @brief Dequeue the element out of the queue
 *
 *  Return the element pointed by top and add 1 to top. Note
 *  we don't need to check if queue is empty here because first
 *  this function is only used in this file, and second, it's caller,
 *  readchar will do the checking in line 40
 *
 *  @param q The pointer to the queue
 *  @return The element at the front of the queue
 **/
static uint8_t dequeue(uint8_t *q)
{
    uint8_t res = q[top];
    top++;  // Increment the top index
    return res;
}


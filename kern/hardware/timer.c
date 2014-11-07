/** @file timer.c
 *
 *  @brief This file includes a timer handler and a setup funtion
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

// #include <p1kern.h>
#include <stdio.h>
#include <seg.h>
#include <asm.h>
#include "simics.h"
#include "interrupt_defines.h"
#include "timer.h"

static void (*callback)(unsigned int);

unsigned int numTicks = 0;  // Number of total ticks

int timer_handler()
{
    numTicks++;
    outb(INT_CTL_PORT, INT_ACK_CURRENT);  // Send ack back
    if (callback != NULL)
        (*callback)(numTicks);

    return 0;
}

void setup_timer(void (*tickback)(unsigned int))
{
    callback = tickback;
}

unsigned int sys_get_ticks()
{
    return numTicks;
}
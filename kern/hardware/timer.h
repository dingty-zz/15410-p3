/** @file timer.h
 *
 *  @brief This file includes a timer handler and a setup function.
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#ifndef __timer_h_
#define __timer_h_

/** @brief The timer handler
 *	
 *	It calls the callback function, if it's not NULL, with number of total ticks
 *  and then sends INT_ACK_CURRENT to one of the PIC's I/O ports
 *
 *  @return void
 **/
int timer_handler();

/** @brief Save the address of the provided callback funtion
 *  @param tickback The address of the callback function
 *  @return void
 **/
void setup_timer(void (*tickback)(unsigned int));

unsigned int sys_get_ticks();
#endif
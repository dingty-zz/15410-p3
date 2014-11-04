/** @file keyboard.h
 *
 *  @brief This file includes keyboard handler and setup function.
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#ifndef __keyboard_h_
#define __keyboard_h_

/** @brief The keyboard handler
 *	
 *	It calls the callback function, if it's not NULL, with number of total ticks
 *  and then sends INT_ACK_CURRENT to one of the PIC's I/O ports. We defer and 
 *  then enable the interrupts when this handler is called to ensure there are 
 *  no interrupt-related concurrency problems.
 *
 *  @return void
 **/
void keyboard_handler();


/** @brief Sets up the keyboard handler
 *	
 *	It allocates a keyboard buffer to store scan codes with size BUF_LEN = 100. The
 *	buffer is implemented as a queue. After allocation, it sets the queue top and 
 *	bottom to 0
 *
 *  @return void
 **/
void setup_keyboard();

int readchar(void);

#endif
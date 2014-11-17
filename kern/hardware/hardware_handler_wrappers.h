/** @file handler_wrappers.h
 *
 *  @brief This file includes timer handler and keyboard handler wrappers.
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#ifndef HANDLER_WRAPPERS
#define HANDLER_WRAPPERS

/** @brief Wraps the timer handler
 *	
 *	It first pushs all the general purpose registers, plus %ebp and other
 *  related registers to save the current states. Then it calls the timer_handler
 *	function in timer.h. Finally, it does a pop, to load the saved state, and return.
 *
 *	@param nothing
 *
 *  @return void
 **/

 /** @brief The function set the color code into the pointer argument
 *
 *  @param the pointer to integer to be set to the color of current terminal
 *  @return nothing
 */

void timer_wrapper();

/** @brief Wraps the keyboard handler
 *	
 *	It first pushs all the general purpose registers, plus %ebp and other
 *  related registers to save the current states. Then it calls the keyboard_handler
 *	function in timer.h. Finally, it does a pop, to load the saved state, and return.
 *  
 *  @param nothing
 *
 *  @return void
 **/
void keyboard_wrapper();

#endif
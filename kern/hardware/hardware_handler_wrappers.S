/** @file handler_wrappers.S
 *
 *  @brief This file includes timer handler and keyboard handler wrappers.
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
 
.global timer_wrapper
.global keyboard_wrapper

.extern timer_handler
.extern keyboard_handler

timer_wrapper:

	pusha
	pushl 	%ds
	pushl 	%es
	pushl 	%fs
	pushl 	%gs
    call    timer_handler
    popl	%gs	
    popl	%fs	
    popl	%es	
    popl	%ds	
    popa
    iret

keyboard_wrapper:

	pusha
    call    keyboard_handler
    popa
    iret
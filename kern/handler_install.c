/** @file handler_install.c 
 *
 *  @brief Initializes the timer by sending clock rates to port.
 *		   Setup timer and keyboard handlers
 *		   Install timer and keyboard wrapper handlers
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#include <p1kern.h>
#include <stdio.h>
#include <seg.h>
#include <asm.h>
#include <timer_defines.h>
#include <keyhelp.h>
#include <simics.h>
#include "handler_wrappers.h"
#include "timer.h"
#include "keyboard.h"

/* Configure the timer to generate interrupts every 10 milliseconds. */
#define FREQ 100

extern void timer_wrapper();
extern void keyboard_wrapper();
extern void setup_timer(void (*tickback)(unsigned int));
extern void setup_keyboard();

static void install_handler(int idt_entry, void (*handler)());

int handler_install(void (*tickback)(unsigned int))
{
	/* Initialize the timer */
	uint32_t period = TIMER_RATE / FREQ;
	outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
	outb(TIMER_PERIOD_IO_PORT, period & 0xFF);
	outb(TIMER_PERIOD_IO_PORT, (period >> 8) & 0xFF);

	/* Setup timer handler and pass the callback function */
	setup_timer(tickback);
	
	/* Setup keyboard handler */
	setup_keyboard();

	/* Install timer handlers */
	install_handler(TIMER_IDT_ENTRY, timer_wrapper);
	install_handler(KEY_IDT_ENTRY, keyboard_wrapper);
  	return 0;
}

/** @brief Install the hander to the respective idt entry.
 *	
 *	It first builds the trap gate and then inserts it into IDT.
 *
 *  @params idt_entry The index into the IDT for the interrupt handler.
 *	@params handler The wrapped handler.
 *  
 *  @return void
 **/
static void install_handler(int idt_entry, void (*handler)()) {

	/* Build the trap gate */ 
	unsigned int* idtbase = (unsigned int *)idt_base();
    unsigned int offset = (unsigned int)handler;
	unsigned int reserved = 0x8F00;  // DPL = 0, P = 1, D = 1, reserved = 0
	unsigned int offset3116 = offset & 0xFFFF0000;
	unsigned int offset150 = offset & 0x0000FFFF;

	/* Fill in the idt entry. The idt_base is the unsigned int pointer but 
	 * each gate is 8 bytes long, so we have to time 2 to get the correct
	 * position */
	idtbase[idt_entry * 2] = (SEGSEL_KERNEL_CS << 16) | offset150;
	idtbase[idt_entry * 2 + 1] = offset3116 | reserved;

}
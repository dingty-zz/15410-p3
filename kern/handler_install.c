/** @file handler_install.c
 *
 *  @brief Initializes the timer by sending clock rates to port.
 *         Setup timer and keyboard handlers
 *         Install timer and keyboard wrapper handlers
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#include <syscall_int.h>
#include "syscall.h"
#include <stdio.h>
#include <seg.h>
#include <asm.h>
#include <timer_defines.h>
#include <keyhelp.h>
#include <simics.h>
#include "handler_install.h"
#include "hardware/hardware_handler_wrappers.h"
#include "hardware/timer.h"
#include "hardware/keyboard.h"
#include "exception/exception_handler_wrappers.h"
#include "syscall_handler.h"
#include <ureg.h>

/* Configure the timer to generate interrupts every 10 milliseconds. */
#define FREQ 100

int handler_install(void (*tickback)(unsigned int))
{
    /* Initialize the fault handlers */

    _handler_install(SWEXN_CAUSE_DIVIDE, DE);   //no error code
    _handler_install(SWEXN_CAUSE_DEBUG, DB);    //no error code
    _handler_install(SWEXN_CAUSE_BREAKPOINT, BP);      //no error code
    _handler_install(SWEXN_CAUSE_OVERFLOW, OF);        //no error code
    _handler_install(SWEXN_CAUSE_BOUNDCHECK, BR);      //no error code
    _handler_install(SWEXN_CAUSE_OPCODE, UD);          //no error code
    _handler_install(SWEXN_CAUSE_NOFPU, NM);           //no error code

    _handler_install(SWEXN_CAUSE_SEGFAULT, NP);            //error code: yes
    _handler_install(SWEXN_CAUSE_STACKFAULT, SS);          //error code: yes
    _handler_install(SWEXN_CAUSE_PROTFAULT, GP);           //error code: yes
    _handler_install(SWEXN_CAUSE_PAGEFAULT, PF);   //error code: yes

    _handler_install(SWEXN_CAUSE_FPUFAULT, MF);  //no error code
    _handler_install(SWEXN_CAUSE_ALIGNFAULT, AC);  //error code: yes, 0
    _handler_install(SWEXN_CAUSE_SIMDFAULT, XF);  //no error code

    /* Initialize the hardware handlers */

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
    _interrupt_install(TIMER_IDT_ENTRY, timer_wrapper);
    _interrupt_install(KEY_IDT_ENTRY, keyboard_wrapper);
    /* initialize system call handlers */

    _handler_install(EXEC_INT, (void *)exec);
    _handler_install(FORK_INT, (void *)fork);
    _handler_install(GETTID_INT, (void *)gettid);
    _handler_install(WAIT_INT, (void *)wait);
    _handler_install(SET_STATUS_INT, (void *)set_status);
    _handler_install(VANISH_INT, (void *)vanish);
    _handler_install(PRINT_INT, (void *)print);
    _handler_install(READFILE_INT, (void *)readfile);
    _handler_install(SLEEP_INT, (void *)sleep);
    _handler_install(DESCHEDULE_INT, (void *)deschedule);
    _handler_install(HALT_INT, (void *)halt);
    _handler_install(READLINE_INT, (void *)readline);
    _handler_install(NEW_PAGES_INT, (void *)new_pages);
    _handler_install(REMOVE_PAGES_INT, (void *)remove_pages);
    _handler_install(SWEXN_INT, (void *)sys_swexn_wrapper);    
    _handler_install(THREAD_FORK_INT, (void *)thread_fork_wrapper);
    _handler_install(GET_TICKS_INT, (void *)get_ticks);
    _handler_install(YIELD_INT, (void *)yield);
    _handler_install(MAKE_RUNNABLE_INT, (void *)make_runnable);
    _handler_install(SET_TERM_COLOR_INT, (void *)set_term_color);
    _handler_install(GET_CURSOR_POS_INT, (void *)get_cursor_pos);
    _handler_install(SET_CURSOR_POS_INT, (void *)set_cursor_pos);
    return 0;
}

/** @brief Install the handler to the respective idt entry.
 *
 *  It first builds the TRAP GATE and then inserts it into IDT.
 *
 *  @params idt_entry The index into the IDT for the interrupt handler.
 *  @params handler The wrapped handler.
 *
 *  @return void
 **/
void _handler_install(int idt_entry, void (*handler)())
{
    /* Build the trap gate */
    unsigned int *idtbase = (unsigned int *)idt_base();

    unsigned int offset = (unsigned int)handler;
    unsigned int reserved = 0xef00;   // P = 1, DPL = 1 1 , reserved = 0;
    unsigned int offset3116 = offset & 0xFFFF0000;
    unsigned int offset150 = offset & 0x0000FFFF;

    /* Fill in the idt entry. The idt_base is the unsigned int pointer but
     * each gate is 8 bytes long, so we have to time 2 to get the correct
     * position */
    idtbase[idt_entry * 2] = (SEGSEL_KERNEL_CS << 16) | offset150;
    idtbase[idt_entry * 2 + 1] = offset3116 | reserved;
}


/** @brief Install the interrupt handler to the respective idt entry.
 *
 *  It first builds the INTERRUPT GATE and then inserts it into IDT.
 *
 *  @params idt_entry The index into the IDT for the interrupt handler.
 *  @params handler The wrapped handler.
 *
 *  @return void
 **/
void _interrupt_install(int idt_entry, void (*handler)())
{
    /* Build the trap gate */
    unsigned int *idtbase = (unsigned int *)idt_base();

    unsigned int offset = (unsigned int)handler;
    unsigned int reserved = 0x8e00;  // P = 1, DPL = 0 0, reserved = 0;
    unsigned int offset3116 = offset & 0xFFFF0000;
    unsigned int offset150 = offset & 0x0000FFFF;

    /* Fill in the idt entry. The idt_base is the unsigned int pointer but
     * each gate is 8 bytes long, so we have to time 2 to get the correct
     * position */
    idtbase[idt_entry * 2] = (SEGSEL_KERNEL_CS << 16) | offset150;
    idtbase[idt_entry * 2 + 1] = offset3116 | reserved;
}
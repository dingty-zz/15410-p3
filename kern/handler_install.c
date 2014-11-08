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
#include "hardware/hardware_handler_wrappers.h"
#include "hardware/timer.h"
#include "hardware/keyboard.h"
// #include "exception/exception_handler_wrappers.h"
/* Configure the timer to generate interrupts every 10 milliseconds. */
#define FREQ 100

static void _handler_install(int idt_entry, void (*handler)());

int handler_install(void (*tickback)(unsigned int))
{
    /* Initialize the fault handlers */

    // for instance,
    // _handler_install(0x0, DE);
    // _handler_install(0x1, DB);
    // _handler_install(0x3, BP);
    // _handler_install(0x4, OF);
    // _handler_install(0x5, BR);
    // _handler_install(0x6, UD);
    // _handler_install(0x7, NM);
    // _handler_install(0x8, DF);
    // _handler_install(0x10, TS);
    // _handler_install(0x11, NP);
    // _handler_install(0x12, SS);
    // _handler_install(0X13, GP);
    // _handler_install(0x14, PF);
    // _handler_install(0x16, MF);
    // _handler_install(0x17, AC);
    // _handler_install(0x18, MC);
    // _handler_install(0x19, XF);



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
    _handler_install(TIMER_IDT_ENTRY, timer_wrapper);
    _handler_install(KEY_IDT_ENTRY, keyboard_wrapper);
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



    return 0;
}

/** @brief Install the hander to the respective idt entry.
 *
 *  It first builds the trap gate and then inserts it into IDT.
 *
 *  @params idt_entry The index into the IDT for the interrupt handler.
 *  @params handler The wrapped handler.
 *
 *  @return void
 **/
static void _handler_install(int idt_entry, void (*handler)())
{

    /* Build the trap gate */
    unsigned int *idtbase = (unsigned int *)idt_base();

    unsigned int offset = (unsigned int)handler;
    unsigned int reserved = 0xef00;  // DPL = 0, P = 1, D = 1, reserved = 0
    unsigned int offset3116 = offset & 0xFFFF0000;
    unsigned int offset150 = offset & 0x0000FFFF;

    /* Fill in the idt entry. The idt_base is the unsigned int pointer but
     * each gate is 8 bytes long, so we have to time 2 to get the correct
     * position */
    idtbase[idt_entry * 2] = (SEGSEL_KERNEL_CS << 16) | offset150;
    idtbase[idt_entry * 2 + 1] = offset3116 | reserved;


}
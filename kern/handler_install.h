 /**
 * @file do_switch.h
 *
 * @brief This file defines two status of the spinlock and 
 *		  several lock manipulation functions.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _HANDLER_INSTALL_H
#define _HANDLER_INSTALL_H

#define PF_INT 0xE

int handler_install(void (*tickback)(unsigned int));
void _handler_install(int idt_entry, void (*handler)());

#endif /* _HANDLER_INSTALL_H */

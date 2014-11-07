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

#ifndef _PROCESS_H
#define _PROCESS_H
#include <elf/elf_410.h>
#include "control_block.h"
void process_init();


unsigned int program_loader(simple_elf_t se_hdr, PCB *process);


int process_create(const char *filename, int run);


int process_exit();


#endif /* _PROCESS_H */

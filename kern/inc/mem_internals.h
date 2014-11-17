/** @file mem_internals.h
 *
 *  @brief This file includes paging handling routines
*          1. General design, PD, PT descrptions
           2. How free list works
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#ifndef _MEM_INTERNALS_H
#define _MEM_INTERNALS_H
#include <stdint.h>
#include "datastructure/linked_list.h"

#define PT_SIZE 1024
#define PD_SIZE 1024


/* a node that keeps track of allocated physical frames;
 * There is a global list containing these nodes;
 */
typedef struct kernel_frame
{
	unsigned int refcount;	
	struct kernel_frame *next;
} KF;


/* a node that keeps track of information of allocated virtual pages;
 * Each process has its own va list in PCB to track its allocated 
 * virtual addresses
 */
typedef struct addr_info
{
	uint32_t virtual_addr;
	//multiple of page size, specifying the length 
	//of allocated area from the virtual address as base
	uint32_t len;
	node va_node; 
} VA_INFO;



#endif /* _MEM_INTERNALS_H */
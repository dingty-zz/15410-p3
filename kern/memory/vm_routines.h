/**
 * @file vm_routines.h
 *
 * @brief A atomic exchange function to achieve mutual exclusion.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _VM_ROUTINES_H
#define _VM_ROUTINES_H
#include <stdint.h>
#include <types.h>
 
void mm_init();

int virtual_map_physical(uint32_t *PD, uint32_t pd_index, uint32_t pt_index);

int virtual_unmap_physical(uint32_t *PD, uint32_t pd_index, uint32_t pt_index);

int allocate_pages(uint32_t *pd, uint32_t virtual_addr, size_t size);

int free_pages(uint32_t *pd, uint32_t virtual_addr, size_t size);

uint32_t *init_pd();

void copy_page_directory(uint32_t *pd);

void copy_page_table(uint32_t *pt);

void destroy_page_directory(uint32_t *pd);

void destroy_page_table(uint32_t pt);

void map_readonly(uint32_t *pd, uint32_t virtual_addr, size_t size);

void map_readonly_pages(uint32_t *PD, uint32_t pd_index, uint32_t pt_index);

void init_free_frame();

uint32_t acquire_free_frame();

void release_free_frame(uint32_t address);

int is_user_addr(void *addr);

int addr_has_mapping(void *addr);

/* Some address manipulation macro*/
#define DEFLAG_ADDR(x)           (x & 0xfffff000)
#define ADDFLAG(x,flag)          (x | flag)
#define GET_FLAG(x)              (x & 0xfff)

#endif /*_VM_ROUTINES_H*/
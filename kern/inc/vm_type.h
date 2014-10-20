/** @file vm_type.h
 *  @brief This file defines the type for vm system.
 */

#ifndef _VM_H
#define _VM_H
#define PGT_SIZE 512

//allocated_frames
typedef struct alloc_frames_node
{
    unsigned long addr;
    struct alloc_frames *next;
    struct alloc_frames *prev;
} frame_node;

typedef struct alloc_frames_head
{
    struct alloc)frames *head;
    int num;
} alloc_frames;

typedef struct page_table_entry
{
    int virtual_addr;
    int phys_addr;
    struct page_table_entry* left;
    struct page_table_entry* node;
} pgt_entry;

//binary serch tree to implement the mapping
typedef struct page_table
{
    pgt_entry* head;
} pgt;

typedef struct page_directory
{
    pgt *[4] pgt_array;
} pgd;

pgd glb_PD;

#endif /* _VM_H */

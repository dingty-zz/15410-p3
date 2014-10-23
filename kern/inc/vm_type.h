/** @file vm_type.h
 *  @brief This file defines the type for vm system.
 */

#ifndef _VM_H
#define _VM_H
#define PT_SIZE 1024
#define PD_SIZE 1024

//allocated_frames
// typedef struct alloc_frames_node
// {
//     unsigned long addr;
//     struct alloc_frames *next;
//     struct alloc_frames *prev;
// } frame_node;

// typedef struct alloc_frames_head
// {
//     struct alloc)frames *head;
//     int num;
// } alloc_frames;

//Page table is essentially an array of physical addresses;
typedef struct PT
{
	void* pt[PT_SIZE];
} PT;

//Page directory is an array of page tables;
typedef struct PD
{
	PT* pd[PD_SIZE];
} PD;

#endif /* _VM_H */

#ifndef _MEM_INTERNALS_H
#define _MEM_INTERNALS_H
#include <stdint.h>
#include "linked_list.h"
#define PT_SIZE 1024
#define PD_SIZE 1024

// typedef struct mem_type
// {
// 	uint32_t	start;
// 	uint32_t	end;
// 	int status;
// 	uint32_t size;
// } mem_t;

// typedef struct page_t
// {
// 	int valid;
// 	void *address;
// } page;

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


// a node that keeps track of allocated physical frames
typedef struct kernel_frame
{
	void *address;
	int flag;
} KF;



#endif /* _MEM_INTERNALS_H */
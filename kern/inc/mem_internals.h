#ifndef _MEM_INTERNALS_H
#define _MEM_INTERNALS_H
#include <stdint.h>
#include "linked_list.h"

typedef struct mem_type
{
	uint32_t	start;
	uint32_t	end;
	int status;
	uint size;
} mem_t;

typedef struct page_directory
{
	PT page_tables[1024];
} PD ;

typedef struct page_table_t
{
	uint32_t entry;
} PT;

typedef struct page_t
{
	int valid;
	void *address;
} page;

// a node that keeps track of allocated physical frames
typedef struct kernel_frame
{
	void *address;
	uint8_t flag;
} KF;


#endif /* _MEM_INTERNALS_H */
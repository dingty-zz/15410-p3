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

struct page_table
{
	uint index
	uint32_t first10bit;
};

struct page_directory
{
	uint index;
	uint32_t first10bit;
};

struct page_entry
{
	/* data */
};

#endif /* _MEM_INTERNALS_H */
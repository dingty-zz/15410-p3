#ifndef _MEM_INTERNALS_H
#define _MEM_INTERNALS_H
#include <stdint.h>

typedef struct mem_type
{
	uint32_t	start;
	uint32_t	end;
	int status;
	uint size;
} mem_t;


#endif /* _MEM_INTERNALS_H */
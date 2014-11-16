#include <syscall.h>
#include "simics.h"
#include "loader.h"
#include "control_block.h"
#include "console.h"
#include <string.h>
#include <exec2obj.h>
#include "memory/vm_routines.h"

#define LEN_MIN(x,y) ((x) < (y) ? (x) : (y))


/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
void sys_halt(void)
{
	clear_console();        
    lprintf("Shutting down...");
    sim_halt();

    // TODO power off
    while(1);
}

int sys_readfile(char *filename, char *buf, int size, int offset)
{
	// verify filename ,buf
	// lprintf("I am readfile! filename: %s",filename);
	// lprintf("I am readfile! buf: %s",buf);
	// lprintf("I am readfile! size: %d",size);
	// lprintf("I am readfile! offset: %d",offset);
    if (!is_user_addr(filename) || !addr_has_mapping(filename)) return -1;   
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;   

	if (size < 0 || offset < 0)
	{
		lprintf("size, offset error");
		return -1;
	}

	int i = 0;
	int realsize = 0;
	char* real_starting_point;
    for (i = 0; i < exec2obj_userapp_count; i++)
    {   
        // If we find this filename
        if (!strcmp(exec2obj_userapp_TOC[i].execname , filename))
        {
        	real_starting_point = (char*)(
        					    (unsigned int)exec2obj_userapp_TOC[i].execbytes 
        						+ (unsigned int)offset);
        	realsize = LEN_MIN(size,exec2obj_userapp_TOC[i].execlen - offset);
            if (realsize < 0) return -1;
            // realsize = 200;
            memcpy(buf, (void*)real_starting_point,realsize);
        	lprintf("the real string is: %s", real_starting_point);
            lprintf("found a file. its length is: %d",realsize+1);
            return realsize;
        }
    }

    //not found;
    lprintf("readfile not found file");
    return -1;
}


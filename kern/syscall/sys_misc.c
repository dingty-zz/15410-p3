/** @file sys_misc.c
 *
 *  @brief This file provides the implementation for halt and readfile
 *          system calls
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
#include "simics.h"
#include "loader.h"
#include "control_block.h"
#include "console.h"
#include <string.h>
#include <exec2obj.h>
#include "memory/vm_routines.h"

#define LEN_MIN(x,y) ((x) < (y) ? (x) : (y))

/** @brief Shutdown the entire system
 *
 **/
void sys_halt(void)
{
	clear_console();        
    lprintf("Shutting down...");
    sim_halt();
}

/** @brief Fill the user-specified buffer buf with count bytes 
 *   starting offset bytes from the beginning of the RAM disk file 
 *     specified by filename.
 *
 *  @param filename the filename
 *  @param buf the buffer to be filled in
 *  @param size the number of bytes to be copied
 *  @param offset the offset from the file
 *  @return 0 on success, -1 otherwise
 **/
int sys_readfile(char *filename, char *buf, int size, int offset)
{

    if (!is_user_addr(filename) || !addr_has_mapping(filename)) return -1;   
    if (!is_user_addr(buf) || !addr_has_mapping(buf)) return -1;   

	if (size < 0 || offset < 0)
	{
		// size or offset error
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
            return realsize;
        }
    }

    // file not found
    return -1;
}


/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 * Functions for the loading
 * of user programs from binary
 * files should be written in
 * this file. The function
 * elf_load_helper() is provided
 * for your use.
 */
/*@{*/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>
#include "simics.h"

/* --- Local function prototypes --- */



/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on succes; -1 on failure
 */

int getbytes( const char *filename, int offset, int size, char *buf )
{

    int i = 0;
    for (i = 0; i < exec2obj_userapp_count; i++)
    {   
        // If we find this filename
        if (!strcmp(exec2obj_userapp_TOC[i].execname , filename))
        {
            memcpy(buf, (void *)exec2obj_userapp_TOC[i].execbytes + offset, size);
            return size;
        }
    }
    return -1;
}


/*@}*/

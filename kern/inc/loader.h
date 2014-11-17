/** @file loader.h
 *
 *  @brief This file includes paging handling routines
*          1. General design, PD, PT descrptions
           2. How free list works
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#ifndef _LOADER_H
#define _LOADER_H
     

/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );

/*
 * Declare your loader prototypes here.
 */

#endif /* _LOADER_H */
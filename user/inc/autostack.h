/**
* @file autostack.h
*
* @brief Defines the struct for stack information. For 
*	     Please refer to README.dox part I for stack design description
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
* @bugs No known bugs
*
*/
#ifndef _AUTOSTACK_H
#define _AUTOSTACK_H
#include <ureg.h>

typedef struct {
    int is_single_threaded;  // If the program runs in the single threaded mode
    int global_root_tid;	 // global tid for root task	
} stackinfo_t;

/* root stack information, modified by thread library */
stackinfo_t global_stackinfo;

#endif
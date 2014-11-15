/**
* @file malloc.c
*
* @brief Thread safe malloc library functions
*
* @author Xianqi Zeng (xianqiz)
* @author Tianyuan Ding (tding)
*
*/
#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include "mutex_type.h"

/* @brief Global mutex to allow only one thread to allocate the element
 * on the heap each time. */
mutex_t malloc_mutex;

/** @brief The function to initialize the malloc_mutex, called by thr_init
 *
 *  @return 0 on success and -1 an error (unlikely)
 */
int malloc_init()
{
    return mutex_init(&malloc_mutex);
}

void *malloc(size_t __size)
{
    void *temp = NULL;

    mutex_lock(&malloc_mutex);
    temp = _malloc(__size);
    mutex_unlock(&malloc_mutex);

    return temp;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
    void *temp = NULL;

    mutex_lock(&malloc_mutex);
    temp = _calloc(__nelt, __eltsize);
    mutex_unlock(&malloc_mutex);

    return temp;
}

void *realloc(void *__buf, size_t __new_size)
{
    void *temp = NULL;

    mutex_lock(&malloc_mutex);
    temp = _realloc(__buf, __new_size);
    mutex_unlock(&malloc_mutex);

    return temp;
}

void free(void *__buf)
{
    mutex_lock(&malloc_mutex);
    _free(__buf);
    mutex_unlock(&malloc_mutex);
}

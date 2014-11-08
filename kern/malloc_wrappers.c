#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include "locks/mutex_type.h"


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

/* safe versions of malloc functions */
void *malloc(size_t size)
{

    mutex_lock(&malloc_mutex);
    void *temp = _malloc(size);
    mutex_unlock(&malloc_mutex);

    return temp;
}
void *memalign(size_t alignment, size_t size)
{
    mutex_lock(&malloc_mutex);
    void *result = _memalign(alignment, size);
    mutex_unlock(&malloc_mutex);

    return result;
}

void *calloc(size_t nelt, size_t eltsize)
{
    mutex_lock(&malloc_mutex);
    void *result = _calloc(nelt, eltsize);
    mutex_unlock(&malloc_mutex);

    return result;
}

void *realloc(void *buf, size_t new_size)
{
    mutex_lock(&malloc_mutex);
    void *result = _realloc(buf, new_size);
    mutex_unlock(&malloc_mutex);

    return result;
}

void free(void *__buf)
{
    mutex_lock(&malloc_mutex);
    _free(__buf);
    mutex_unlock(&malloc_mutex);
}


void *smalloc(size_t size)
{
    mutex_lock(&malloc_mutex);
    void *result = smalloc(size);
    mutex_unlock(&malloc_mutex);

    return result;
}

void *smemalign(size_t alignment, size_t size)
{
    mutex_lock(&malloc_mutex);
    void *result = _smemalign(alignment, size);
    mutex_unlock(&malloc_mutex);
    return result;
}

void sfree(void *buf, size_t size)
{
    mutex_lock(&malloc_mutex);
    _sfree(buf, size);
    mutex_unlock(&malloc_mutex);
    return;
}



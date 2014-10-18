#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{
    void *result = _malloc(size);
    return result;
}

void *memalign(size_t alignment, size_t size)
{
    void *result = _memalign(alignment, size);
    return result;
}

void *calloc(size_t nelt, size_t eltsize)
{
    void *result = _calloc(nelt, eltsize);
    return result;
}

void *realloc(void *buf, size_t new_size)
{
    void *result = _realloc(buf, new_size);
    return result;
}

void free(void *buf)
{
    _free(buf);
    return;
}

void    *smalloc(size_t size)
{
    void *result = smalloc(size);
    return result;
}

void *smemalign(size_t alignment, size_t size)
{
    void *result = _smemalign(alignment, size);
    return result;
}

void sfree(void *buf, size_t size)
{
    _sfree(buf, size);
    return;
}



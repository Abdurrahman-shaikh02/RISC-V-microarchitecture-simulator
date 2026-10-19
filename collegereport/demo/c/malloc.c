#include <stddef.h>
extern char __heap_start;
extern char __heap_end;

static char *end = &__heap_start;   // must ensure __heap_start is word aligned !!!

void *malloc(unsigned int size)
{
    size = (size + 3) & ~3;

    if (end + size > &__heap_end)
        return 0;

    void *ptr = end;
    end += size;

    return ptr;
}

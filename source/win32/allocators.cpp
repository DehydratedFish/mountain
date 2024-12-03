

#ifndef NO_C_LIB_ALLOCATOR

#include <cstdlib>

INTERNAL void *cstd_alloc_func(void *, s64 size, void *old, s64 old_size) {
    void *result = 0;

    if (old) {
        if (size) {
            result = realloc(old, size);
        } else {
            free(old);
        }
    } else {
        result = calloc(1, size);
    }

    return result;
}
Allocator CStdAllocator = {cstd_alloc_func, 0};

#endif



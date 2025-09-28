#pragma once

#include "definitions.h"


// TODO: Are these ok as variables or is it better to have functions returning them?
extern Allocator DefaultAllocator;
extern thread_local Allocator TempAllocator;


// TODO: Making {} equal the DefaultAllocator does actually lead to unwanted allocations.
//       If a stored Allocator is {} and we specify the default one for the new allocation
//       the old one will be deallocated first. Which is not necessary.
inline void *allocate(Allocator alloc, s64 bytes) {
    if (bytes == 0) return 0;
    if (alloc.allocate == 0) alloc = DefaultAllocator;

    return alloc.allocate(alloc.data, bytes, 0, 0);
}
inline void *reallocate(Allocator alloc, s64 bytes, void *old, s64 old_bytes) {
    if (alloc.allocate == 0) alloc = DefaultAllocator;

    return alloc.allocate(alloc.data, bytes, old, old_bytes);
}
inline void deallocate(Allocator alloc, void *ptr, s64 old_size) {
    if (!ptr) return;
    if (alloc.allocate == 0) alloc = DefaultAllocator;

    alloc.allocate(alloc.data, 0, ptr, old_size);
}

#define ALLOC(alloc, type, count)  (type*)allocate((alloc), (count) * sizeof(type))
#define REALLOC(alloc, ptr, old_count, new_count) (decltype(ptr))reallocate((alloc), (new_count) * sizeof(*ptr), (ptr), (old_count) * sizeof(*ptr))
#define DEALLOC(alloc, ptr, count) deallocate((alloc), (ptr), (count) * sizeof(*ptr))

s64  temp_storage_mark();
void temp_storage_rewind(s64 mark);
void reset_temp_storage();

//===============================================
// This macro is intended to be used at the start of a function or scope (hence the name...).
// This way the temporary storage gets reset to the state it was before the scope entry.
// Useful if you need multiple or big temporary allocations which would lead the temporary
// storage to overflow on following code. Also it frees all allocated things at once.
//===============================================
#define SCOPE_TEMP_STORAGE() SCOPE_TEMP_STORAGE_IMPL(CONCAT(TMP_STORAGE_REWIND_MARK, __COUNTER__))
#define SCOPE_TEMP_STORAGE_IMPL(name) auto name = temp_storage_mark(); DEFER(temp_storage_rewind(name));


#define INIT_STRUCT(ptr) zero_memory(ptr, sizeof(*ptr))
inline void zero_memory(void *data, u64 bytes) {
    u8 *tmp = (u8*)data;
    for (u64 i = 0; i < bytes; i += 1) {
        tmp[i] = 0;
    }
}

inline void copy_memory(void *dest, void const *src, u64 size) {
    u8 *d = (u8*)dest;
    u8 *s = (u8*)src;

    if (d < s) {
        for (u64 i = 0; i < size; i += 1) {
            d[i] = s[i];
        }
    } else {
        for (u64 i = size; i > 0; i -= 1) {
            d[i - 1] = s[i - 1];
        }
    }
}


template<class Type>
Array<Type> array_allocate(s64 size, Allocator alloc = DefaultAllocator) {
    Array<Type> result;
    result.data = ALLOC(alloc, Type, size);
    result.size = size;

    return result;
}

template<class Type>
void array_destroy(Array<Type> *arr, Allocator alloc = DefaultAllocator) {
    DEALLOC(alloc, arr->data, arr->size);
    INIT_STRUCT(arr);
}


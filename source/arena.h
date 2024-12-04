#pragma once

#include "memory.h"


struct MemoryArena {
    Allocator allocator;
    u8 *memory;
    s64 used;
    s64 alloc;
};

inline void destroy(MemoryArena *arena) {
    DEALLOC(arena->allocator, arena->memory, arena->alloc);

    arena->memory = 0;
    arena->used   = 0;
    arena->alloc  = 0;
}

inline void init(MemoryArena *arena, s64 size, Allocator alloc = DefaultAllocator) {
    if (arena->allocator.allocate != alloc.allocate) {
        destroy(arena);
        arena->allocator = alloc;
    }

    arena->memory = ALLOC(arena->allocator, u8, size);
    arena->used   = 0;
    arena->alloc  = size;
}

inline void *allocate_from_arena(MemoryArena *arena, s64 size, void *old, s64 old_size) {
    u32 const alignment = alignof(void*);

    u8 *current_ptr = arena->memory + arena->used;

    if (old) {
        if (size) {
            if (current_ptr - old_size == old) {
                s64 additional_size = size - old_size;
                if (arena->used + additional_size > arena->alloc) die("Could not allocate from arena.");

                arena->used += additional_size;
                
                return old;
            } else {
                s64 space = arena->alloc - arena->used;

#pragma warning( suppress : 4146 )
                s64 padding = -(u64)current_ptr & (alignment - 1);

                if (size > (space - padding)) die("Could not allocate from arena.");

                void *result = arena->memory + padding + arena->used;
                zero_memory(result, size);
                arena->used += padding + size;

                copy_memory(result, old, old_size);

                return result;
            }
        } else {
            if (current_ptr - old_size == old) {
                arena->used -= old_size;
            }

            return 0;
        }
    }

    s64 space = arena->alloc - arena->used;

#pragma warning( suppress : 4146 )
    s64 padding = -(u64)current_ptr & (alignment - 1);

    if (size > (space - padding)) die("Could not allocate from arena.");

    void *result = arena->memory + padding + arena->used;
    zero_memory(result, size);
    arena->used += padding + size;

    return result;
}

inline Allocator make_arena_allocator(MemoryArena *arena) {
    return {(AllocatorFunc*)allocate_from_arena, arena};
}


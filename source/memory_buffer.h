#pragma once

#include "memory.h"


template<class Type>
struct MemoryBuffer {
    Allocator allocator;

    Type *data;
    s64   size;
    s64   alloc;
};


template<class Type>
void init_memory_buffer(MemoryBuffer<Type> *buffer, s64 size, Allocator alloc = DefaultAllocator) {
    if (buffer->allocator.allocate != alloc.allocate) {
        destroy(buffer);
        buffer->allocator = alloc;
    }

    buffer->data  = ALLOC(buffer->allocator, u8, size);
    buffer->size  = 0;
    buffer->alloc = size;
}

template<class Type>
void destroy(MemoryBuffer<Type> *buffer) {
    DEALLOC(buffer->allocator, buffer->data, buffer->alloc);

    buffer->data  = 0;
    buffer->size  = 0;
    buffer->alloc = 0;
}


#pragma once

#include "definitions.h"
#include "string2.h"


#ifndef STRING_BUILDER_BLOCK_SIZE
#define STRING_BUILDER_BLOCK_SIZE KILOBYTES(4)
#endif

struct StringBuilder {
    Allocator allocator;

    struct StringBuilderBlock {
        StringBuilderBlock *next;

        u8 buffer[STRING_BUILDER_BLOCK_SIZE];
        s64 used;
    };

    StringBuilderBlock  first;
    StringBuilderBlock *current;

    s64 total_size;
};


inline void reset(StringBuilder *builder) {
    auto *block = &builder->first;

    while (block) {
        block->used = 0;
        block = block->next;
    }

    builder->total_size = 0;
}

inline void append(StringBuilder *builder, u8 c, Allocator alloc = DefaultAllocator) {
    if (builder->current == 0) builder->current = &builder->first;
    if (builder->allocator.allocate == 0) builder->allocator = alloc;

    if (builder->current->used + 1 > STRING_BUILDER_BLOCK_SIZE) {
        if (builder->current->next == 0) {
            builder->current->next = ALLOC(builder->allocator, StringBuilder::StringBuilderBlock, 1);
        } else {
            // NOTE: buffer not initialized to zero for speed
            builder->current->next->used = 0;
        }
        builder->current = builder->current->next;
    }

    builder->current->buffer[builder->current->used] = c;
    builder->current->used += 1;
    builder->total_size    += 1;
}

inline void append(StringBuilder *builder, String str, Allocator alloc = DefaultAllocator) {
    if (builder->current == 0) builder->current = &builder->first;
    if (builder->allocator.allocate == 0) builder->allocator = alloc;

    s64 space = STRING_BUILDER_BLOCK_SIZE - builder->current->used;
    while (space < str.size) {
        copy_memory(builder->current->buffer + builder->current->used, str.data, space);
        builder->current->used += space;
        builder->total_size    += space;
        str = shrink_front(str, space);

        if (builder->current->next == 0) {
            builder->current->next = ALLOC(builder->allocator, StringBuilder::StringBuilderBlock, 1);
        } else {
            builder->current->next->used = 0;
        }
        builder->current = builder->current->next;

        space = STRING_BUILDER_BLOCK_SIZE;
    }

    copy_memory(builder->current->buffer + builder->current->used, str.data, str.size);
    builder->current->used += str.size;
    builder->total_size    += str.size;
}

inline void append_raw(StringBuilder *builder, void *buffer, s64 size, Allocator alloc = DefaultAllocator) {
    String str = {(u8*)buffer, size};
    append(builder, str, alloc);
}

inline void destroy(StringBuilder *builder) {
    auto *next = builder->first.next;

    while (next) {
        auto *tmp = next->next;
        DEALLOC(builder->allocator, next, 1);

        next = tmp;
    }
}

inline String to_allocated_string(StringBuilder *builder, Allocator alloc = DefaultAllocator) {
    String result = allocate_string(builder->total_size, alloc);

    s64 size = 0;
    auto *block = &builder->first;
    while (block) {
        copy_memory(result.data + size, block->buffer, block->used);
        size += block->used;

        block = block->next;
    }

    return result;
}

inline String temp_string(StringBuilder *builder) {
    assert(builder->total_size < STRING_BUILDER_BLOCK_SIZE);

    return {builder->first.buffer, builder->first.used};
}



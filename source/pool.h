//================================================
// A MemoryPool is meant to be a form of an dynamic Array
// where the pointers remain stable after growing.
//================================================
#pragma once

#include "definitions.h"
#include "memory.h"


struct MemoryPool {
    Allocator allocator;

    struct Block {
        Block *next;

        s64 used;
        u8 *data;
    };

    s64 block_size;

    Block  first_block;
    Block *current_block;
};


inline void destroy(MemoryPool *pool) {
    DEALLOC(pool->allocator, pool->first_block.data, pool->block_size);

    MemoryPool::Block *block = pool->first_block.next;
    while (block) {
        MemoryPool::Block *old = block;
        block = block->next;

        // NOTE: The block and data allocation was consolidated.
        deallocate(pool->allocator, old, sizeof(MemoryPool::Block) + pool->block_size);
    }

    INIT_STRUCT(pool);
}

inline void init(MemoryPool *pool, s64 block_size, Allocator alloc = DefaultAllocator) {
    if (pool->allocator.allocate != alloc.allocate) {
        destroy(pool);
        pool->allocator = alloc;
    }

    pool->first_block.data = ALLOC(pool->allocator, u8, block_size);

    pool->block_size    = block_size;
    pool->current_block = &pool->first_block;
}

inline b32 maybe_grow(MemoryPool *pool, s64 size) {
    MemoryPool::Block *block = pool->current_block;

    s64 space = pool->block_size - block->used;
    if (size > space) {
        return false;
    }

    // NOTE: Consolidated allocation.
    u8 *mem = (u8*)allocate(pool->allocator, sizeof(MemoryPool::Block) + pool->block_size);
    MemoryPool::Block *new_block = (MemoryPool::Block*)mem;
    new_block->data = mem + sizeof(MemoryPool::Block);

    pool->current_block->next = new_block;
    pool->current_block = new_block;

    return true;
}

inline void align_block(MemoryPool *pool, s32 alignment) {
    MemoryPool::Block *block = pool->current_block;

#pragma warning( suppress : 4146 )
    s64 padding = -(u64)(block->data + block->used) & (alignment - 1);

    block->used += padding;
    if (block->used > pool->block_size) block->used = pool->block_size;
}

inline void *allocate(MemoryPool *pool, s64 size, s32 alignment = alignof(void*)) {
    align_block(pool, alignment);
    if (!maybe_grow(pool, size)) {
        die("Allocation from MemoryPool is bigger than the block size.");
    }

    MemoryPool::Block *block = pool->current_block;
    void *result = block->data + block->used;
    block->used += size;

    return result;
}

inline void *allocate_from_pool(MemoryPool *pool, s64 size, void *old, s64 old_size) {
    if (old && size) {
        return 0;
    }

    return allocate(pool, size);
}

inline Allocator make_pool_allocator(MemoryPool *pool) {
    return {(AllocatorFunc*)allocate_from_pool, pool};
}


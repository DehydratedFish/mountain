#pragma once

#include "memory.h"


inline bool is_any(u8 c, String delimiters) {
    for (s64 i = 0; i < delimiters.size; i += 1) {
        if (c == delimiters[i]) return true;
    }

    return false;
}

inline bool equal(String lhs, String rhs) {
    if (lhs.size == rhs.size) {
        for (s64 i = 0; i < lhs.size; i += 1) {
            if (lhs[i] != rhs[i]) return false;
        }

        return true;
    }

    return false;
}

inline u8 lower_char(u8 c) {
    if (c >= 'A' && c <= 'Z') {
        return c | 32;
    }

    return c;
}

inline bool caseless_equal(String lhs, String rhs) {
    if (lhs.size == rhs.size) {
        for (s64 i = 0; i < lhs.size; i += 1) {
           if (lower_char(lhs[i]) != lower_char(rhs[i])) return false;
        }

        return true;
    }

    return false;
}

inline bool operator==(String lhs, String rhs) {
    return equal(lhs, rhs);
}

inline bool operator!=(String lhs, String rhs) {
    return !(equal(lhs, rhs));
}

inline bool contains(String str, String search) {
    for (s64 i = 0; i < str.size; i += 1) {
        if (i + search.size > str.size) return false;

        String tmp = {str.data + i, search.size};
        if (equal(tmp, search)) return true;
    }

    return false;
}

inline b32 starts_with(String str, String begin) {
    if (str.size < begin.size) return false;

    str.size = begin.size;
    return str == begin;
}

inline bool ends_with(String str, String search) {
    if (str.size < search.size) return false;

    str.data += str.size - search.size;
    str.size  = search.size;

    return str == search;
}

inline String next_line(String str, s64 *offset) {
    String result = {};

    s64 pos = *offset;
    result.data = &str[pos];

    while (pos < str.size) {
        if (is_any(str[pos], "\n\r")) {
            result.size = pos - *offset;
            break;
        }

        pos += 1;
    }
    while (pos < str.size) {
        if (!is_any(str[pos], "\n\r")) break;

        pos += 1;
    }

    *offset = pos;

    return result;
}

inline s64 find_last(String str, u8 c) {
    for (s64 i = str.size; i > 0; i -= 1) {
        s64 index = i - 1;
        if (str[index] == c) return index;
    }

    return -1;
}

inline String to_lower_case(String str) {
    for (s64 i = 0; i < str.size; i += 1) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] |= 32;
        }
    }

    return str;
}

inline String shrink_front(String str, s64 amount = 1) {
#ifdef BOUNDS_CHECKING
    if (str.size < amount) die("String is too small to shrink.");
#endif
    str.data += amount;
    str.size -= amount;

    return str;
}

inline String shrink_back(String str, s64 amount = 1) {
#ifdef BOUNDS_CHECKING
    if (str.size < amount) die("String is too small to shrink.");
#endif
    str.size -= amount;

    return str;
}

inline String shrink(String str, s64 amount = 1) {
#ifdef BOUNDS_CHECKING
    if (str.size < amount * 2) die("String is too small to shrink.");
#endif
    return shrink_back(shrink_front(str, amount), amount);
}

inline String trim(String str) {
    s64 num_whitespaces = 0;

    for (s64 i = 0; i < str.size; i += 1) {
        if (str[i] > ' ') break;

        num_whitespaces += 1;
    }
    str.data += num_whitespaces;
    str.size -= num_whitespaces;

    num_whitespaces = 0;
    for (s64 i = str.size; i > 0; i -= 1) {
        if (str[i - 1] > ' ') break;

        num_whitespaces += 1;
    }
    str.size -= num_whitespaces;

    return str;
}

inline String sub_string(String buffer, s64 offset, s64 size) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + size) die("String read out of bounds.");
#endif
    String result = {buffer.data + offset, size};

    return result;
}

inline void destroy(String *str, Allocator alloc = DefaultAllocator) {
    DEALLOC(alloc, str->data, str->size);
    INIT_STRUCT(str);
}

inline String allocate_string(s64 size, Allocator alloc = DefaultAllocator) {
    if (size == 0) return {};

    String result = {};
    result.data = ALLOC(alloc, u8, size);
    result.size = size;

    return result;
}

inline String allocate_string(String str, Allocator alloc = DefaultAllocator) {
    String result = allocate_string(str.size, alloc);

    copy_memory(result.data, str.data, str.size);

    return result;
}

inline char *c_string_copy(String str, Allocator alloc = DefaultAllocator) {
    char *result = ALLOC(alloc, char, str.size + 1);
    copy_memory(result, str.data, str.size);

    return result;
}



#ifndef STRING_BUILDER_BLOCK_SIZE
#define STRING_BUILDER_BLOCK_SIZE KILOBYTES(4)
#endif

struct StringBuilderBlock {
    StringBuilderBlock *next;

    u8 buffer[STRING_BUILDER_BLOCK_SIZE];
    s64 used;
};
struct StringBuilder {
    Allocator allocator;

    StringBuilderBlock  first;
    StringBuilderBlock *current;

    s64 total_size;
};

inline void reset(StringBuilder *builder) {
    StringBuilderBlock *block = &builder->first;

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
            builder->current->next = ALLOC(builder->allocator, StringBuilderBlock, 1);
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
            builder->current->next = ALLOC(builder->allocator, StringBuilderBlock, 1);
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
#define APPEND_RAW(builder, value) append_raw((builder), (void*)&(value), sizeof(value))

inline void destroy(StringBuilder *builder) {
    StringBuilderBlock *next = builder->first.next;

    while (next) {
        StringBuilderBlock *tmp = next->next;
        DEALLOC(builder->allocator, next, 1);

        next = tmp;
    }
}

inline String to_allocated_string(StringBuilder *builder, Allocator alloc = DefaultAllocator) {
    String result = allocate_string(builder->total_size, alloc);

    s64 size = 0;
    StringBuilderBlock *block = &builder->first;
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


inline String path_part(String path) {
    for (s64 i = path.size; i > 0; i -= 1) {
        s64 index = i - 1;
        if (path[index] == '/' || path[index] == '\\') {
            return {path.data, index};
        }
    }

    return "";
}

inline String path_file(String path) {
    for (s64 i = path.size; i > 0; i -= 1) {
        s64 index = i - 1;
        if (path[index] == '/' || path[index] == '\\') {
            return {path.data + i, path.size - i};
        }
    }

    return path;
}

inline String path_file_without_ext(String path) {
    path = path_file(path);

    for (s64 i = path.size; i > 0; i -= 1) {
        s64 index = i - 1;
        if (path[index] == '.') {
            path.size = index;

            break;
        }
    }

    return path;
}

inline String path_extension(String path) {
    s64 dot = find_last(path, '.');
    if (dot) {
        dot += 1;

        return {path.data + dot, path.size - dot};
    }

    return "";
}

inline String path_next_item(String path, String current) {
    if (current.data == 0) {
        current.data = path.data;
        current.size = 0;
    }

#ifdef BOUNDS_CHECKING
    if (current.data < begin(path) || current.data > end(path)) die("path_next_item out of bounds.");
#endif

    current.data += current.size;
    current.size  = 0;

    if (current.data == end(path)) return current;

    // TODO: Is a '\' needed as well?
    while (current.data != end(path) && (*current.data == '/' || *current.data == '\\')) {
        current.data += 1;
    }

    while (end(current) != end(path) && *end(current) != '/' && *current.data != '\\') {
        current.size += 1;
    }

    return current;
}


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


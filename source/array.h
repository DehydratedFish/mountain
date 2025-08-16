#pragma once

#include "definitions.h"



template<class Type>
Array<Type> slice(Array<Type> array, s64 pos, s64 size) {
    assert(pos >= 0 && size >= 0);
#if BOUNDS_CHECKING
    if (array.size < pos || array.size < pos + size) die("Slice out of bounds.");
#endif

    Array<Type> result = {};
    result.data = array.data + pos;
    result.size = size;

    return result;
}

template<class Type>
void reverse(Array<Type> array) {
    s64 mid = array.size / 2;
    for (s64 i = 0; i < mid; i += 1) {
        s64 j = array.size - i - 1;
        Type tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

template<class Type>
b32 contains(Array<Type> array, Type element) {
    for (s64 i = 0; i < array.size; i += 1) {
        if (array[i] == element) return true;
    }

    return false;
}


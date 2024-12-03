#pragma once

#include "definitions.h"


template<typename Type>
void reverse(Array<Type> array) {
    s64 mid = array.size / 2;
    for (s64 i = 0; i < mid; i += 1) {
        s64 j = array.size - i - 1;
        Type tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}


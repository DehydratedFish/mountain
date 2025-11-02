//================================================
// List is a growable array similar to std::vector.
//
// The name is not really to my liking but Array is
// taken by the view like things as it's use is broader
// and suits well. DynArray or something like that
// looks and feels not great. And since you often
// add stuff to a list it is ok for now.
// Maybe it is a bit misleading in terms of linked lists
// but I think that is ok as a tradeoff.
//================================================
#pragma once

#include "definitions.h"
#include "memory.h"


template<class Type>
struct List {
    Allocator allocator;

    Type *data;
    s64   size;
    s64   alloc;

    Type &operator[](s64 index) {
        if (index < 0) index = size + index;
        BOUNDS_CHECK(0, size - 1, index, "List indexing out of bounds");

        return data[index];
    }

    // NOTE: Automatically create a "view" if passing it to functions and such.
    operator Array<Type> () {
        return {data, size};
    }
};

template<class Type>
Type *begin(List<Type> list) { return list.data; }
template<class Type>
Type *end  (List<Type> list) { return list.data + list.size; }


template<class Type>
void init(List<Type> *list, s64 size, Allocator alloc = DefaultAllocator) {
    if (list->allocator.allocate != alloc.allocate) {
        destroy(list);
        list->allocator = alloc;
    }

    if (list->alloc < size) {
        list->data  = REALLOC(list->allocator, list->data, list->alloc, size);
        list->alloc = size;
    }
    list->size = 0;
}

template<class Type>
void init(List<Type> *list, Type *buffer, s64 size, Allocator alloc = DefaultAllocator) {
    prealloc(list, size, alloc);

    copy_memory(list->data, buffer, size * sizeof(Type));
}

template<class Type>
void destroy(List<Type> *list) {
    if (list->allocator.allocate) {
        deallocate(list->allocator, list->data, list->alloc * sizeof(Type));

        list->data  = 0;
        list->size  = 0;
        list->alloc = 0;
    }
}

template<class Type>
void prealloc(List<Type> *list, s64 size, Allocator alloc = DefaultAllocator) {
    init(list, size, alloc);
    list->size = size;
}

template<class Type>
void shrink(List<Type> *list) {
    list->data = REALLOC(list->allocator, list->data, list->alloc, list->size);
    list->alloc  = list->size;
}

template<class Type>
void maybe_grow(List<Type> *list, s64 needed_alloc) {
    s32 const growth = 2;

    if (needed_alloc > list->alloc) {
        s64 new_alloc = list->size * growth;
        if (needed_alloc > new_alloc) new_alloc = needed_alloc;

        list->data  = REALLOC(list->allocator, list->data, list->alloc, new_alloc);
        list->alloc = new_alloc;
    }
}

template<class Type>
void ensure_space(List<Type> *list, s64 size) {
    maybe_grow(list, list->size + size);
}

template<class Type>
Type *append(List<Type> *list, Type element) {
    maybe_grow(list, list->size + 1);

    list->data[list->size] = element;
    list->size += 1;

    return &list->data[list->size - 1];
}

template<class Type>
Type *append(List<Type> *list) {
    maybe_grow(list, list->size + 1);

    list->size += 1;

    return &list->data[list->size - 1];
}

template<class Type>
Array<Type> append(List<Type> *list, Array<Type> array) {
    maybe_grow(list, list->size + array.size);

    copy_memory(list->data + list->size, array.data, array.size * sizeof(Type));
    Array<Type> result = {list->data, array.size};
    list->size += array.size;

    return result;
}

template<class Type>
Type *insert(List<Type> *list, s64 index, Type element) {
    if (index < 0) index = list->size + index;
	BOUNDS_CHECK(0, list->size, index, "List insertion out of bounds.");

    maybe_grow(list, list->size + 1);

    copy_memory(list->data + index + 1, list->data + index, (list->size - index) * sizeof(Type));
    list->data[index] = element;
    list->size += 1;

    return &list->data[index];
}

template<class Type>
Array<Type> insert(List<Type> *list, s64 index, Array<Type> array) {
    if (index < 0) index = list->size + index;
	BOUNDS_CHECK(0, list->size, index, "List insertion out of bounds.");

    maybe_grow(list, list->size + array.size);

    copy_memory(list->data + index + array.size, list->data + index, (list->size - index) * sizeof(Type));
    copy_memory(list->data + index, array.data, array.size * sizeof(Type));
    Array<Type> result = {list->data + index, array.size};
    list->size += array.size;

    return result;
}

template<class Type>
void pop(List<Type> *list) {
    // TODO: Maybe just check if the list is > 0 and not error out?
    assert(list->size > 0);
    list->size -= 1;
}

template<class Type>
void move_to_front(List<Type> *list, s64 index) {
    if (list->size < 2) return;

    if (index < 0) index = list->size + index;
    BOUNDS_CHECK(0, list->size - 1, index, "List remove out of bounds.");

    Type tmp = list->data[index];
    copy_memory(list->data + 1, list->data, index * sizeof(Type));

    list->data[0] = tmp;
}

// NOTE: does not preserve order
template<class Type>
void remove(List<Type> *list, s64 index) {
    if (list->size == 0) return;

    if (index < 0) index = list->size + index;
    BOUNDS_CHECK(0, list->size - 1, index, "List remove out of bounds.");

    list->size -= 1;
    list->data[index] = list->data[list->size];
}

template<class Type>
void stable_remove(List<Type> *list, s64 index) {
    if (list->size == 0) return;

    if (index < 0) index = list->size + index;
    BOUNDS_CHECK(0, list->size - 1, index, "List remove out of bounds.");

    copy_memory(list->data + index, list->data + index + 1, (list->size - (index + 1)) * sizeof(Type));
    list->size -= 1;
}

template<class Type>
void stable_remove(List<Type> *list, s64 index, s64 elements) {
    if (list->size == 0) return;

    if (index < 0) index = list->size + index;
    BOUNDS_CHECK(0, list->size, index + elements, "List remove out of bounds.");

    copy_memory(list->data + index, list->data + index + elements, (list->size - (index + elements)) * sizeof(Type));
    list->size -= elements;
}

template<class Type>
void copy_array(List<Type> *list, Array<Type> array) {
    ensure_space(list, array.size);

    copy_memory(list->data, array.data, array.size * sizeof(Type));
    list->size = array.size;
}

template<class Type>
Array<Type> create_array(List<Type> list, Allocator alloc = DefaultAllocator) {
    Array<Type> result = {};
    result.data = ALLOC(alloc, Type, list.size);
    result.size = list.size;

    copy_memory(result.data, list.data, list.size * sizeof(Type));

    return result;
}


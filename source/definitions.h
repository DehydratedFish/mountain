#pragma once

#include <cstdint>


#if defined(WIN32) || defined(_WIN32)
#define OS_WINDOWS
#elif defined(linux)
#define OS_LINUX
#else
#error "Platform not supported"
#endif


#define INTERNAL static

#define KILOBYTES(x) ((x) * 1024)
#define MEGABYTES(x) (KILOBYTES(x) * 1024)
#define GIGABYTES(x) (MEGABYTES(x) * 1024)

#define C_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define FOR(collection, it) for (auto it = begin(collection); it < end(collection); next_element(&it))
#define FOR_INDEX(collection, it) ((it) - begin(collection))
template<class PointerType> void next_element(PointerType **it) { *it += 1; }

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  r32;
typedef double r64;

typedef s8  b8;
typedef s32 b32;


struct V2i {
    union {
        struct {
            s32 x;
            s32 y;
        };
        struct {
            s32 width;
            s32 height;
        };
    };
};


void fire_assert(char const *msg, char const *func, char const *file, int line);
void die(const char *msg);


#ifdef DEVELOPER
#define assert(expr) (void)((expr) || (fire_assert(#expr, __func__, __FILE__, __LINE__),0))

#else
#define assert(expr)

#endif // DEVELOPER

#ifdef BOUNDS_CHECKING
#define BOUNDS_CHECK(low, high, index, msg) {if ((index) < (low) || (index) > (high)) die(msg);}

#else
#define BOUNDS_CHECK(low, high, index, msg)

#endif // BOUNDS_CHECKING


typedef void*(AllocatorFunc)(void *data, s64 size, void *old, s64 old_size);
struct Allocator {
    AllocatorFunc *allocate;
    void *data;
};

inline constexpr s64 c_string_length(char const *str) {
    if (str == 0) return 0;

    s64 size = 0;
    for (; str[size]; size += 1);

    return size;
}

struct String {
    u8 *data;
    s64 size;

    String() = default;
    String(u8 *buffer, s64 length) {
        data = buffer;
        size = length;
    }

    //NOTE: Reference are not touched and always copied before a change so this cast should be fine.
    constexpr String(char const *str)
    : data((u8*)str), size(c_string_length(str)) {}

    String &operator=(char const *str) {
        data = (u8*)str;
        size = c_string_length(str);

        return *this;
    }

    u8 &operator[](s64 index) {
        if (index < 0) index = size + index;
        BOUNDS_CHECK(0, size - 1, index, "String indexing out of bounds");

        return data[index];
    }
};

inline u8 *begin(String str) { return str.data; }
inline u8 *end  (String str) { return str.data + str.size; }


template<class Type>
struct Array {
    Type *data;
    s64   size;

    Type &operator[](s64 index) {
        if (index < 0) index = size + index;
        BOUNDS_CHECK(0, size - 1, index, "Array indexing out of bounds");

        return data[index];
    }
};

template<class Type>
Type *begin(Array<Type> array) { return array.data; }
template<class Type>
Type *end  (Array<Type> array) { return array.data + array.size; }


template<typename Functor>
struct DeferGuardBase {
    Functor functor;

    DeferGuardBase(Functor f): functor(f) {}
    ~DeferGuardBase() {functor();}
};

template<typename Functor>
DeferGuardBase<Functor> make_defer_guard_base(Functor f) {return f;}

#define CONCAT(l, r) CONCAT2(l, r)
#define CONCAT2(l, r) l ## r

#define UNIQUE_NAME(base) CONCAT(base, __COUNTER__)

#define DEFER(stuff) DEFER2(UNIQUE_NAME(DeferGuardUniqueName), stuff)
#define DEFER2(name, stuff) auto name = make_defer_guard_base([&](){stuff;});


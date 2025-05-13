#pragma once

#include "definitions.h"


struct V2 {
    union {
        struct {
            r32 x, y;
        };
        struct {
            r32 w, h;
        };
    };
};
static_assert(sizeof(V2) == 8, "V2 union stuff not working correctly.");

struct V2i {
    union {
        struct {
            s32 x, y;
        };
        struct {
            s32 w, h;
        };
    };
};
static_assert(sizeof(V2i) == 8, "V2i union stuff not working correctly.");

struct V3 {
    union {
        struct {
            r32 x, y, z;
        };
        struct {
            r32 r, g, b;
        };
        r32 value[3];
    };
};
static_assert(sizeof(V3) == 12, "V3 union stuff not working correctly.");

struct V3i {
    union {
        struct {
            s32 x, y, z;
        };
        s32 value[3];
    };
};
static_assert(sizeof(V3i) == 12, "V3i union stuff not working correctly.");

struct V4 {
    union {
        struct {
            r32 x, y, z, w;
        };
        struct {
            r32 r, g, b, a;
        };
        r32 value[4];
    };
};
static_assert(sizeof(V4) == 16, "V4 union stuff not working correctly.");
typedef V4 Color;

struct V4i {
    union {
        struct {
            s32 x, y, z, w;
        };
        s32 value[4];
    };
};
static_assert(sizeof(V4i) == 16, "V4i union stuff not working correctly.");


inline V2  operator+(V2  lhs, r32 val) { lhs.x += val;   lhs.y += val;   return lhs; }
inline V2i operator+(V2i lhs, s32 val) { lhs.x += val;   lhs.y += val;   return lhs; }
inline V2  operator+(V2  lhs, V2  rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline V2i operator+(V2i lhs, V2i rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline V2  operator-(V2  lhs, r32 val) { lhs.x -= val;   lhs.y -= val;   return lhs; }
inline V2i operator-(V2i lhs, s32 val) { lhs.x -= val;   lhs.y -= val;   return lhs; }
inline V2  operator-(V2  lhs, V2  rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline V2i operator-(V2i lhs, V2i rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }

inline V3  operator+(V3  lhs, r32 val) { lhs.x += val;   lhs.y += val;   lhs.z += val;   return lhs; }
inline V3  operator+(V3  lhs, V3  rhs) { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; return lhs; }
inline V3  operator-(V3  lhs, r32 val) { lhs.x -= val;   lhs.y -= val;   lhs.z -= val;   return lhs; }
inline V3  operator-(V3  lhs, V3  rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; return lhs; }


#pragma once

#include "definitions.h"

#include <cmath>


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
inline V3  operator*(V3  lhs, r32 val) { lhs.x *= val;   lhs.y *= val;   lhs.z *= val;   return lhs; }



inline r32 length(V3 vector) {
    return sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

inline V3 normalize(V3 vector) {
    r32 inverse_length = 1.0f / length(vector);

    return vector * inverse_length;
}

inline r32 dot(V3 lhs, V3 rhs) {
	return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

inline V3 cross(V3 lhs, V3 rhs) {
	V3 v;
	v.x = (lhs.y * rhs.z) - (rhs.y * lhs.z);
	v.y = (lhs.z * rhs.x) - (rhs.z * lhs.x);
	v.z = (lhs.x * rhs.y) - (rhs.x * lhs.y);

	return v;
}


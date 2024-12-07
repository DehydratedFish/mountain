#pragma once

#include "definitions.h"
#include "string_builder.h"


inline u8 read_byte(String buffer, s64 offset) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset) die("String read out of bounds.");
#endif

    return buffer[offset];
}
inline u8 read_byte(String buffer, s64 *offset) {
    u8 result = read_byte(buffer, *offset);
    *offset += 1;

    return result;
}

inline u16 read_le_u16(String buffer, s64 offset) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + (s64)sizeof(u16)) die("String read out of bounds.");
#endif
    u8 *data = buffer.data + offset;

    return data[0] | (data[1] << 8);
}
inline u16 read_le_u16(String buffer, s64 *offset) {
    u16 result = read_le_u16(buffer, *offset);
    *offset += sizeof(u16);

    return result;
}

inline u32 read_le_u32(String buffer, s64 offset) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + (s64)sizeof(u32)) die("String read out of bounds.");
#endif
    u8 *data = buffer.data + offset;

    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}
inline u32 read_le_u32(String buffer, s64 *offset) {
    u32 result = read_le_u32(buffer, *offset);
    *offset += sizeof(u32);

    return result;
}

inline u64 read_le_u64(String buffer, s64 offset) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + (s64)sizeof(u64)) die("String read out of bounds.");
#endif
    u8 *data = buffer.data + offset;

    u64 result = (data[0])            | (data[1] << 8)       | (data[2] << 16)      | (data[3] << 24) |
                 ((u64)data[4] << 32) | ((u64)data[5] << 40) | ((u64)data[6] << 48) | ((u64)data[7] << 56);

    return result;
}
inline u64 read_le_u64(String buffer, s64 *offset) {
    u64 result = read_le_u64(buffer, *offset);
    *offset += sizeof(u64);

    return result;
}

inline s64 read_le_s64(String buffer, s64 offset) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + (s64)sizeof(s64)) die("String read out of bounds.");
#endif
    u8 *data = buffer.data + offset;

    s64 result = (data[0])            | (data[1] << 8)       | (data[2] << 16)      | (data[3] << 24) |
                 ((s64)data[4] << 32) | ((s64)data[5] << 40) | ((s64)data[6] << 48) | ((s64)data[7] << 56);

    return result;
}

inline r32 read_le_r32(String buffer, s64 offset) {
#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + (s64)sizeof(r32)) die("String read out of bounds.");
#endif
    u32 tmp = read_le_u32(buffer, offset);

    return *(r32*)&tmp;
}
inline r32 read_le_r32(String buffer, s64 *offset) {
    r32 result = read_le_r32(buffer, *offset);
    *offset += sizeof(r32);

    return result;
}

// TODO: Change to varints?
inline String read_binary_string(String buffer, s64 offset) {
    u32 length = read_le_u32(buffer, &offset);

#ifdef BOUNDS_CHECKING
    if (buffer.size < offset + length) die("String read out of bounds.");
#endif
    return {buffer.data + offset, length};
}
inline String read_binary_string(String buffer, s64 *offset) {
    u32 length = read_le_u32(buffer, offset);

#ifdef BOUNDS_CHECKING
    if (buffer.size < *offset + length) die("String read out of bounds.");
#endif
    String result = {buffer.data + *offset, length};
    *offset += length;

    return result;
}


template<class Type>
inline void write_binary(StringBuilder *builder, Type value) {
    append_raw(builder, &value, sizeof(value));
}

// TODO: Write varints?
inline void write_binary_string(StringBuilder *builder, String string) {
    u32 length = (u32)string.size;

    write_binary(builder, length);
    append(builder, string);
}


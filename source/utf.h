//===============================================
// This file is a total mess... it needs updating
// as soon as I find out what functions are really
// useful and which are not needed.
//===============================================
#pragma once

#include "definitions.h"


enum {
    GET_OK    = 0x00,
    GET_EMPTY = 0x01,
    GET_ERROR = 0x02,
};

struct String16 {
    u16 *data;
    s64  size;
};

struct String32 {
    u32 *data;
    s64  size;
};


s64    utf8_string_length(String16 string);
String to_utf8(Allocator alloc, String16 string);
String to_utf8(Allocator alloc, String32 string);

String16 to_utf16(Allocator alloc, String string, b32 add_null_terminator = false);


struct UTF8Iterator {
    String string;

    s64 index;
    u32 utf8_size;
    u32 cp;
    b32 valid;
};

enum IterationStart {
    ITERATE_FROM_START,
    ITERATE_FROM_END
};
UTF8Iterator make_utf8_it(String string, IterationStart from = ITERATE_FROM_START);
void next(UTF8Iterator *it);
void prev(UTF8Iterator *it);

struct UTF8CharResult {
    u32 cp;
    u8  byte[4];
    s32 length;
    u32 status;
};
UTF8CharResult utf8_peek(String str);

u8 *next_utf8_sequence(String text, u8 *pos);
u8 *previous_utf8_sequence(String text, u8 *pos);

UTF8CharResult to_utf8(u32 cp);
u32 to_utf32(u16 *str);


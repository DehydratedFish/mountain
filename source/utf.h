//===============================================
// This file is a total mess... it needs updating
// as soon as I find out what functions are really
// useful and which are not needed.
//===============================================
#pragma once

#include "definitions.h"


enum UTFResult {
    UTF_OK               = 0x00,
    UTF_INVALID_SEQUENCE = 0x01,
    UTF_CONVERSION_ERROR = 0x02,
    UTF_BUFFER_TOO_SHORT = 0x03,
};
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

struct UTF8Info {
    u8  byte[4];
    s32 bytes;
    UTFResult status;
};

struct UTF16Info {
    u16 byte[2]; // TODO: Bad name.
    s32 bytes;
    UTFResult status;
};

// NOTE: No length check on string. Only use if you know your data is valid.
UTF8Info utf8_info(u8 *string);
UTF8Info utf8_info(String string);

s64       utf8_string_length(String16 string);
UTFResult to_utf8(String buffer, String16 string);

String to_utf8(Allocator alloc, String16 string);
String to_utf8(Allocator alloc, String32 string);


// NOTE: No length check on string. Only use if you know your data is valid.
UTF16Info utf16_info(u16 *string);
UTF16Info utf16_info(String16 string);

s64       utf16_string_length(String string);
UTFResult to_utf16(String16 buffer, String string);

// NOTE: I don't like the last parameter, but added it for conversion inside the
//       win32 platform layer. It should go away.
String16  to_utf16(Allocator alloc, String string, b32 add_null_terminator = false);


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

// NOTE: This is in codepoints.
s64 utf8_string_length(String text);

UTF8CharResult to_utf8(u32 cp);
u32 to_utf32(u16 *str);


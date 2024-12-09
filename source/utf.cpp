#include "utf.h"
#include "memory.h"
#include "string_builder.h"
#include "list.h"
        


struct UTF16GetResult {
    u16 value[2];
    b32 is_surrogate;
    b32 status;
};

INTERNAL UTF16GetResult get_utf16(String16 *string) {
    if (string->size == 0) return {{}, 0, GET_EMPTY};
    if (string->data[0] > 0xDBFF && string->data[0] < 0xE000) return {{}, 0, GET_ERROR};

    UTF16GetResult result = {};
    if (string->data[0] > 0xD7FF && string->data[0] < 0xDC00) {
        if (string->size < 2) return {{}, 0, GET_ERROR};

        result.value[0] = string->data[0];
        result.value[1] = string->data[1];
        result.is_surrogate = true;
        result.status   = GET_OK;

        string->data += 2;
        string->size -= 2;
    } else {
        result.value[0] = string->data[0];
        result.is_surrogate = false;
        result.status   = GET_OK;

        string->data += 1;
        string->size -= 1;
    }

    return result;
}

s64 utf8_string_length(String16 string) {
    s64 length = 0;

    UTF16GetResult c = get_utf16(&string);
    while (c.status == GET_OK) {
        if (c.is_surrogate) {
            length += 4;
        } else {
            if (c.value[0] < 0x80) {
                length += 1;
            } else if (c.value[0] < 0x800) {
                length += 2;
            } else {
                length += 3;
            }
        }

        c = get_utf16(&string);
    }

    return length;
}

UTFResult to_utf8(String buffer, String16 string) {
    UTF16Info info = utf16_info(string);

    s64 pos = 0;
    while (string.size) {
        if (info.status != UTF_OK) return info.status;

        if (info.bytes == 2) {
            if (pos + 3 >= buffer.size) return UTF_BUFFER_TOO_SHORT;

            u32 cp = ((info.byte[0] - 0xD800) >> 10) + (info.byte[1] - 0xDC00) + 0x10000;

            u8 byte;
            byte = 0xF0 | (cp >> 18);
            buffer[pos + 0] = byte;
            byte = 0x80 | ((cp >> 12) & 0x3F);
            buffer[pos + 1] = byte;
            byte = 0x80 | ((cp >> 6) & 0x3F);
            buffer[pos + 2] = byte;
            byte = 0x80 | (cp & 0x3F);
            buffer[pos + 3] = byte;

            pos += 4;
        } else {
            u16 cp = info.byte[0];

            u8 byte;
            if (cp < 0x80) {
                if (pos >= buffer.size) return UTF_BUFFER_TOO_SHORT;

                byte = (u8)cp;
                buffer[pos] = byte;

                pos += 1;
            } else if (cp < 0x800) {
                if (pos + 1 >= buffer.size) return UTF_BUFFER_TOO_SHORT;

                byte = 0xC0 | (cp >> 6);
                buffer[pos + 0] = byte;
                byte = 0x80 | (cp & 0x3F);
                buffer[pos + 1] = byte;

                pos += 2;
            } else {
                if (pos + 2 >= buffer.size) return UTF_BUFFER_TOO_SHORT;

                byte = 0xE0 | (cp >> 12);
                buffer[pos + 0] = byte;
                byte = 0x80 | ((cp >> 6) & 0x3F);
                buffer[pos + 1] = byte;
                byte = 0x80 | (cp & 0x3F);
                buffer[pos + 2] = byte;

                pos += 3;
            }
        }

        string.data += info.bytes;
        string.size -= info.bytes;
        info = utf16_info(string);
    }

    return info.status;
}

String to_utf8(Allocator alloc, String16 string) {
    if (string.size == 0) return {};

    List<u8> buffer = {};
    init(&buffer, utf8_string_length(string), alloc);

    UTF16GetResult c = get_utf16(&string);
    while (c.status == GET_OK) {
        if (c.is_surrogate) {
            u32 cp = ((c.value[0] - 0xD800) >> 10) + (c.value[1] - 0xDC00) + 0x10000;

            u8 byte;
            byte = 0xF0 | (cp >> 18);
            append(&buffer, byte);
            byte = 0x80 | ((cp >> 12) & 0x3F);
            append(&buffer, byte);
            byte = 0x80 | ((cp >> 6) & 0x3F);
            append(&buffer, byte);
            byte = 0x80 | (cp & 0x3F);
            append(&buffer, byte);
        } else {
            u16 cp = c.value[0];

            u8 byte;
            if (cp < 0x80) {
                byte = (u8)cp;
                append(&buffer, byte);
            } else if (cp < 0x800) {
                byte = 0xC0 | (cp >> 6);
                append(&buffer, byte);
                byte = 0x80 | (cp & 0x3F);
                append(&buffer, byte);
            } else {
                byte = 0xE0 | (cp >> 12);
                append(&buffer, byte);
                byte = 0x80 | ((cp >> 6) & 0x3F);
                append(&buffer, byte);
                byte = 0x80 | (cp & 0x3F);
                append(&buffer, byte);
            }
        }

        c = get_utf16(&string);
    }

    if (c.status == GET_ERROR) {
        die("Malformed utf16 string.");
    }

    return {buffer.data, buffer.size};
}

String to_utf8(Allocator alloc, String32 string) {
    if (string.size == 0) return {};

    StringBuilder builder = {};
    DEFER(destroy(&builder));

    for (s64 i = 0; i < string.size; i += 1) {
        u32 cp = string.data[i];

        u8 byte;
        if (cp < 0x80) {
            byte = cp;
            append(&builder, byte);
        } else if (cp < 0x800) {
            byte = 0xC0 | (cp >> 6);
            append(&builder, byte);
            byte = 0x80 | (cp & 0x3F);
            append(&builder, byte);
        } else {
            byte = 0xE0 | (cp >> 12);
            append(&builder, byte);
            byte = 0x80 | ((cp >> 6) & 0x3F);
            append(&builder, byte);
            byte = 0x80 | (cp & 0x3F);
            append(&builder, byte);
        }
    }

    return to_allocated_string(&builder, alloc);
}

INTERNAL s32 utf8_length(u8 c) {
    if      ((c & 0x80) == 0x00) return 1;
    else if ((c & 0xE0) == 0xC0) return 2;
    else if ((c & 0xF0) == 0xE0) return 3;
    else if ((c & 0xF8) == 0xF0) return 4;

    return -1;
}


// TODO: Maybe lookup table?
INTERNAL s32 utf8_length(String str, s64 offset) {
    if (str.size <= offset) return 0;

    return utf8_length(str[offset]);
}

// TODO: Remove.
struct UTF8GetResult {
    u8  byte[4];
    s32 size;
    b32 status;
};

INTERNAL UTF8GetResult get_utf8(String string, s64 offset) {
    UTF8GetResult result = {};

    s32 size = utf8_length(string, offset);
    if (size == -1) {
        String tmp = sub_string(string, offset, string.size - offset);
        result.status = GET_ERROR;

        return result;
    }
    if (size == 0) {
        result.status = GET_EMPTY;
    }

    copy_memory(result.byte, string.data + offset, size);
    result.size = size;

    return result;
}

INTERNAL UTF8GetResult get_utf8_backwards(String string, s64 offset) {
    UTF8GetResult result = {};

    if (offset - 1 < 0) {
        result.status = GET_EMPTY;

        return result;
    }

    while (offset >= 0) {
        offset -= 1;

        if ((string[offset] & 0xC0) != 0x80) {
            break;
        }
    }

    result = get_utf8(string, offset);

    return result;
}


UTF8Info utf8_info(u8 *string) {
    UTF8Info result = {};

    s32 length = utf8_length(string[0]);

    copy_memory(result.byte, string, length);
    result.bytes = length;

    return result;
}

UTF8Info utf8_info(String string) {
    UTF8Info result = {};

    if (string.size == 0) return result;

    s32 length = utf8_length(string[0]);
    if (length > string.size) {
        result.status = UTF_INVALID_SEQUENCE;
        return result;
    }

    copy_memory(result.byte, string.data, length);
    result.bytes = length;

    return result;
}

UTF16Info utf16_info(u16 *string) {
    UTF16Info result = {};

    s32 length = string[0] > 0xD7FF && string[0] < 0xDC00 ? 2 : 1;

    copy_memory(result.byte, string, length * sizeof(u16));
    result.bytes = length;

    return result;
}

UTF16Info utf16_info(String16 string) {
    UTF16Info result = {};

    if (string.size == 0) return result;

    s32 length = string.data[0] > 0xD7FF && string.data[0] < 0xDC00 ? 2 : 1;
    if (length > string.size) {
        result.status = UTF_INVALID_SEQUENCE;
        return result;
    }

    copy_memory(result.byte, string.data, length * sizeof(u16));
    result.bytes = length;

    return result;
}

/*
 * Note: Helpful stackoverflow question
 * https://stackoverflow.com/questions/73758747/looking-for-the-description-of-the-algorithm-to-convert-utf8-to-utf16
 */
s64 utf16_string_length(String string) {
    s64 offset = 0x00;
    u32 status = GET_OK;
    while (status == GET_OK) {
        UTF8GetResult c = get_utf8(string, offset);
        status = c.status;
        if (c.status != GET_OK) return -1;

        offset += c.size;
    }

    return offset;
}

UTFResult to_utf16(String16 buffer, String string) {
    UTF8Info info = utf8_info(string);

    s64 pos = 0;
    while (string.size) {
        if (info.status != UTF_OK) return info.status;

        if (info.bytes == 1) {
            if (pos >= buffer.size) return UTF_BUFFER_TOO_SHORT;

            u16 value = info.byte[0] & 0x7F;
            buffer.data[pos] = value;

            pos += 1;
        } else if (info.bytes == 2) {
            if (pos >= buffer.size) return UTF_BUFFER_TOO_SHORT;

            u16 value = ((info.byte[0] & 0x1F) << 6) | (info.byte[1] & 0x3F);
            buffer.data[pos] = value;

            pos += 1;
        } else if (info.bytes == 3) {
            if (pos >= buffer.size) return UTF_BUFFER_TOO_SHORT;

            u16 value = ((info.byte[0] & 0x0F) << 12) | ((info.byte[1] & 0x3F) << 6) | (info.byte[2] & 0x3F);
            buffer.data[pos] = value;

            pos += 1;
        } else if (info.bytes == 4) {
            if (pos + 1 >= buffer.size) return UTF_BUFFER_TOO_SHORT;
            u32 cp = ((info.byte[0] & 0x07) << 18) | ((info.byte[1] & 0x3F) << 12) | ((info.byte[2] & 0x3F) << 6) | (info.byte[3] & 0x3F);
            cp -= 0x10000;

            u16 high = 0xD800 + ((cp >> 10) & 0x3FF);
            u16 low  = 0xDC00 + (cp & 0x3FF);

            buffer.data[pos]     = high;
            buffer.data[pos + 1] = low;

            pos += 2;
        }

        shrink_front(string, info.bytes);
        info = utf8_info(string);
    }

    return info.status;
}

String16 to_utf16(Allocator alloc, String string, b32 add_null_terminator) {
    if (string.size == 0) return {};

    List<u16> buffer = {};
    buffer.allocator = alloc;

    s64 offset = 0x00;
    u32 status = GET_OK;
    while (status == GET_OK) {
        UTF8GetResult c = get_utf8(string, offset);
        status = c.status;
        if (c.status != GET_OK) break;

        if (c.size == 1) {
            u16 value = c.byte[0] & 0x7F;
            append(&buffer, value);
        } else if (c.size == 2) {
            u16 value = ((c.byte[0] & 0x1F) << 6) | (c.byte[1] & 0x3F);
            append(&buffer, value);
        } else if (c.size == 3) {
            u16 value = ((c.byte[0] & 0x0F) << 12) | ((c.byte[1] & 0x3F) << 6) | (c.byte[2] & 0x3F);
            append(&buffer, value);
        } else if (c.size == 4) {
            u32 cp = ((c.byte[0] & 0x07) << 18) | ((c.byte[1] & 0x3F) << 12) | ((c.byte[2] & 0x3F) << 6) | (c.byte[3] & 0x3F);
            cp -= 0x10000;

            u16 high = 0xD800 + ((cp >> 10) & 0x3FF);
            u16 low  = 0xDC00 + (cp & 0x3FF);

            append(&buffer, high);
            append(&buffer, low );
        }

        offset += c.size;
    }

    if (status == GET_ERROR) {
        // TODO: better error handling
        die("Malformed utf8");
    }

    if (add_null_terminator) {
        u16 n = L'\0';
        append(&buffer, n);
    }

    return {buffer.data, buffer.size};
}

UTF8Iterator make_utf8_it(String string, IterationStart from) {
    UTF8Iterator it = {};
    it.string = string;

    if (from == ITERATE_FROM_START) {
        next(&it);
    } else {
        it.index = string.size;
        prev(&it);
    }

    return it;
}

void next(UTF8Iterator *it) {
    assert(it);

    it->index += it->utf8_size;
    UTF8GetResult c = get_utf8(it->string, it->index);
    if (c.status != GET_OK) {
        INIT_STRUCT(it);

        return;
    }

    if (c.size == 1) {
        it->cp = c.byte[0] & 0x7F;
    } else if (c.size == 2) {
        it->cp = ((c.byte[0] & 0x1F) << 6) | (c.byte[1] & 0x3F);
    } else if (c.size == 3) {
        it->cp = ((c.byte[0] & 0x0F) << 12) | ((c.byte[1] & 0x3F) << 6) | (c.byte[2] & 0x3F);
    } else if (c.size == 4) {
        it->cp = ((c.byte[0] & 0x07) << 18) | ((c.byte[1] & 0x3F) << 12) | ((c.byte[2] & 0x3F) << 6) | (c.byte[3] & 0x3F);
    }
    
    it->utf8_size = c.size;
    it->valid = true;
}

void prev(UTF8Iterator *it) {
    assert(it);

    UTF8GetResult c = get_utf8_backwards(it->string, it->index);
    if (c.status != GET_OK) {
        assert(c.status == GET_EMPTY);
        INIT_STRUCT(it);

        return;
    }

    if (c.size == 1) {
        it->cp = c.byte[0] & 0x7F;
    } else if (c.size == 2) {
        it->cp = ((c.byte[0] & 0x1F) << 6) | (c.byte[1] & 0x3F);
    } else if (c.size == 3) {
        it->cp = ((c.byte[0] & 0x0F) << 12) | ((c.byte[1] & 0x3F) << 6) | (c.byte[2] & 0x3F);
    } else if (c.size == 4) {
        it->cp = ((c.byte[0] & 0x07) << 18) | ((c.byte[1] & 0x3F) << 12) | ((c.byte[2] & 0x3F) << 6) | (c.byte[3] & 0x3F);
    }
    
    it->index -= c.size;
    it->utf8_size = c.size;
    it->valid = true;
}

UTF8CharResult utf8_peek(String str) {
    UTF8CharResult result = {};

    if (str.size == 0) {
        result.status = GET_EMPTY;

        return result;
    }

    s32 length = utf8_length(str.data[0]);
    if (str.size < length) {
        result.status = GET_ERROR;

        return result;
    }

    if (length == 1) {
        result.cp = str[0] & 0x7F;
    } else if (length == 2) {
        result.cp = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
    } else if (length == 3) {
        result.cp = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
    } else if (length == 4) {
        result.cp = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
    }

    copy_memory(result.byte, str.data, length);
    result.length = length;
    result.status = GET_OK;

    return result;
}

u8 *next_utf8_sequence(String text, u8 *pos) {
    assert(pos >= begin(text) && pos <= end(text));

    if (pos == end(text)) return 0;

    return pos + utf8_length(*pos);
}

u8 *previous_utf8_sequence(String text, u8 *pos) {
    assert(pos >= begin(text) && pos <= end(text));

    u8 *prev = pos;
    for (;;) {
        prev -= 1;

        if (prev < begin(text)) break;
        if ((*prev & 0xC0) != 0x80 && pos - prev < 5) {
            return prev;
        }
    }

    return 0;
}


UTF8CharResult to_utf8(u32 cp) {
    UTF8CharResult result = {};
    result.cp = cp;

    if (cp < 0x80) {
        result.byte[0] = cp;
        result.length = 1;
    } else if (cp < 0x800) {
        result.byte[0] = 0xC0 | (cp >> 6);
        result.byte[1] = 0x80 | (cp & 0x3F);
        result.length = 2;
    } else if (cp < 0x10000) {
        result.byte[0] = 0xE0 |  (cp >> 12);
        result.byte[1] = 0x80 | ((cp >>  6) & 0x3F);
        result.byte[2] = 0x80 | (cp & 0x3F);
        result.length = 3;
    } else if (cp < 0x110000) {
        result.byte[0] = 0xF0 |  (cp >> 18);
        result.byte[1] = 0x80 | ((cp >> 12) & 0x3F);
        result.byte[2] = 0x80 | ((cp >>  6) & 0x3F);
        result.byte[3] = 0x80 | (cp & 0x3F);
        result.length = 4;
    } else {
        result.status = -1;
    }
    // TODO: Check for overlong stuff and so on.

    return result;
}

// TODO: Range and error check.
u32 to_utf32(u16 *str) {
    u32 result = 0;

    if (str[0] >= 0xD800 && str[0] <= 0xDFFF) {
        result  = (str[0] - 0xD800) * 0x400;
        result += (str[1] - 0xDC00);
        result += 0x10000;
    } else {
        result = str[0];
    }

    return result;
}


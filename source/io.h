#pragma once

#include <cstdarg>

#include "definitions.h"


enum MouseKey {
    MOUSE_LMB,
    MOUSE_RMB,
    MOUSE_MMB,

    MOUSE_MISC_1,
    MOUSE_MISC_2,

    MOUSE_KEY_COUNT,
};

enum ScanCode {
    SCAN_CODE_ESCAPE = 0x01,
    SCAN_CODE_1 = 0x02,
    SCAN_CODE_2 = 0x03,
    SCAN_CODE_3 = 0x04,
    SCAN_CODE_4 = 0x05,
    SCAN_CODE_5 = 0x06,
    SCAN_CODE_6 = 0x07,
    SCAN_CODE_7 = 0x08,
    SCAN_CODE_8 = 0x09,
    SCAN_CODE_9 = 0x0A,
    SCAN_CODE_0 = 0x0B,

    SCAN_CODE_MINUS = 0x0C,
    SCAN_CODE_EQUAL = 0x0D,

    SCAN_CODE_BACKSPACE = 0x0E,
    SCAN_CODE_TAB = 0x0F,

    SCAN_CODE_Q = 0x10,
    SCAN_CODE_W = 0x11,
    SCAN_CODE_E = 0x12,
    SCAN_CODE_R = 0x13,
    SCAN_CODE_T = 0x14,
    SCAN_CODE_Y = 0x15,
    SCAN_CODE_U = 0x16,
    SCAN_CODE_I = 0x17,
    SCAN_CODE_O = 0x18,
    SCAN_CODE_P = 0x19,
    SCAN_CODE_LEFT_BRACKET  = 0x1A,
    SCAN_CODE_RIGHT_BRACKET = 0x1A,
    SCAN_CODE_RETURN        = 0x1C,
    SCAN_CODE_LEFT_CONTROL  = 0x1D,
    SCAN_CODE_A = 0x1E,
    SCAN_CODE_S = 0x1F,
    SCAN_CODE_D = 0x20,
    SCAN_CODE_F = 0x21,
    SCAN_CODE_G = 0x22,
    SCAN_CODE_H = 0x23,
    SCAN_CODE_J = 0x24,
    SCAN_CODE_K = 0x25,
    SCAN_CODE_L = 0x26,
    SCAN_CODE_SEMICOLON    = 0x27,
    SCAN_CODE_SINGLE_QUOTE = 0x28,
    SCAN_CODE_GRAVE        = 0x29,
    SCAN_CODE_LEFT_SHIFT   = 0x2A,
    SCAN_CODE_BACKSLASH    = 0x2B,
    SCAN_CODE_Z = 0x2C,
    SCAN_CODE_X = 0x2D,
    SCAN_CODE_C = 0x2E,
    SCAN_CODE_V = 0x2F,
    SCAN_CODE_B = 0x30,
    SCAN_CODE_N = 0x31,
    SCAN_CODE_M = 0x32,
    SCAN_CODE_COMMA = 0x33,
    SCAN_CODE_DOT   = 0x34,
    SCAN_CODE_SLASH = 0x34,
    SCAN_CODE_RIGHT_SHIFT = 0x36,
    SCAN_CODE_LEFT_ALT = 0x38,
    SCAN_CODE_SPACE    = 0x39,
    SCAN_CODE_CAPSLOCK = 0x3A,

    SCAN_CODE_F1  = 0x3B,
    SCAN_CODE_F2  = 0x3B,
    SCAN_CODE_F3  = 0x3B,
    SCAN_CODE_F4  = 0x3B,
    SCAN_CODE_F5  = 0x3B,
    SCAN_CODE_F6  = 0x3B,
    SCAN_CODE_F7  = 0x3B,
    SCAN_CODE_F8  = 0x3B,
    SCAN_CODE_F9  = 0x3C,
    SCAN_CODE_F10 = 0x3D,
    SCAN_CODE_F11 = 0x57,
    SCAN_CODE_F12 = 0x58,

    SCAN_CODE_ARROW_UP    = 0xE048,
    SCAN_CODE_ARROW_LEFT  = 0xE04B,
    SCAN_CODE_ARROW_RIGHT = 0xE04D,
    SCAN_CODE_ARROW_DOWN  = 0xE050,

    SCAN_CODE_DELETE = 0xE053,
};

// TODO: Give all codes an explicit value.
enum KeyboardKeyCode {
    KEY_CODE_NONE,

    KEY_CODE_1 = 0x01,
    KEY_CODE_2,
    KEY_CODE_3,
    KEY_CODE_4,
    KEY_CODE_5,
    KEY_CODE_6,
    KEY_CODE_7,
    KEY_CODE_8,
    KEY_CODE_9,
    KEY_CODE_0,

    KEY_CODE_F1,
    KEY_CODE_F2,
    KEY_CODE_F3,
    KEY_CODE_F4,
    KEY_CODE_F5,
    KEY_CODE_F6,
    KEY_CODE_F7,
    KEY_CODE_F8,
    KEY_CODE_F9,
    KEY_CODE_F10,
    KEY_CODE_F11,
    KEY_CODE_F12,

    KEY_CODE_A,
    KEY_CODE_B,
    KEY_CODE_C,
    KEY_CODE_D,
    KEY_CODE_E,
    KEY_CODE_F,
    KEY_CODE_G,
    KEY_CODE_H,
    KEY_CODE_I,
    KEY_CODE_J,
    KEY_CODE_K,
    KEY_CODE_L,
    KEY_CODE_M,
    KEY_CODE_N,
    KEY_CODE_O,
    KEY_CODE_P,
    KEY_CODE_Q,
    KEY_CODE_R,
    KEY_CODE_S,
    KEY_CODE_T,
    KEY_CODE_U,
    KEY_CODE_V,
    KEY_CODE_W,
    KEY_CODE_X,
    KEY_CODE_Y,
    KEY_CODE_Z,

    KEY_CODE_ESCAPE,
    KEY_CODE_GRAVE,
    KEY_CODE_TAB,
    KEY_CODE_CAPSLOCK,
    KEY_CODE_LEFT_SHIFT,
    KEY_CODE_LEFT_CONTROL,
    KEY_CODE_LEFT_META,
    KEY_CODE_LEFT_ALT,

    KEY_CODE_SPACE,

    KEY_CODE_MINUS,
    KEY_CODE_EQUAL,
    KEY_CODE_BACKSPACE,
    KEY_CODE_LEFT_BRACKET,
    KEY_CODE_RIGHT_BRACKET,
    KEY_CODE_BACKSLASH,
    KEY_CODE_SEMICOLON,
    KEY_CODE_QUOTE,
    KEY_CODE_RETURN,
    KEY_CODE_COMMA,
    KEY_CODE_DOT,
    KEY_CODE_SLASH,
    KEY_CODE_RIGHT_SHIFT,
    KEY_CODE_RIGHT_ALT,
    KEY_CODE_RIGHT_META,
    KEY_CODE_RIGHT_CONTROL,

    KEY_CODE_INSERT,
    KEY_CODE_HOME,
    KEY_CODE_PAGE_UP,
    KEY_CODE_DELETE,
    KEY_CODE_END,
    KEY_CODE_PAGE_DOWN,

    KEY_CODE_ARROW_UP,
    KEY_CODE_ARROW_LEFT,
    KEY_CODE_ARROW_RIGHT,
    KEY_CODE_ARROW_DOWN,

    KEY_CODE_COUNT,
};

struct KeyboardState {
    b32 keys[KEY_CODE_COUNT];
};

enum KeyActionKind {
    KEY_ACTION_RELEASE,
    KEY_ACTION_PRESS,
    KEY_ACTION_REPEAT,
};
struct KeyAction {
    KeyActionKind kind;

    KeyboardKeyCode code;
    u32 code_point;
    // TODO: Virtual keys.
};

struct UserMouseInput {
    Point cursor_pos;
    b32 keys[MOUSE_KEY_COUNT];
    s32 scroll;
};

s32 const MaxKeyActions = 32;
struct KeyboardActions {
    KeyAction keys[MaxKeyActions];
    s32 used;
};

struct UserInput {
    UserMouseInput mouse;
    UserMouseInput last_mouse;
    Point cursor_relative;

    KeyboardActions actions;
    KeyboardState   keyboard;
};

b32 key_held_down(KeyAction const *key);


s64 to_s64(String str);
u64 to_u64(String str);
r32 to_r32(String str);

String convert_signed_to_string(u8 *buffer, s32 buffer_size, s64 signed_number, s32 base = 10, b32 uppercase = false, b32 keep_sign = false);
String convert_unsigned_to_string(u8 *buffer, s32 buffer_size, u64 number, s32 base = 10, b32 uppercase = false);
String convert_double_to_string(u8 *buffer, s32 size, r64 number, s32 precision = 6, b32 scientific = false, b32 hex = false, b32 uppercase = false, b32 keep_sign = false);
void convert_to_ptr_string(u8 *buffer, s32 buffer_size, void *address);

s64 print(char const *fmt, ...);
s64 print(String str);
s64 format(struct StringBuilder *builder, char const *fmt, ...);
s64 format(struct StringBuilder *builder, char const *fmt, va_list args);
s64 format(struct PlatformFile *file, char const *fmt, ...);
s64 format(struct PlatformFile *file, char const *fmt, va_list args);

String format(char const *fmt, ...);
String t_format(char const *fmt, ...);
String format(Allocator alloc, char const *fmt, ...);

b32 write_builder_to_file(StringBuilder *builder, PlatformFile *file);

void change_log_file(struct PlatformFile *file);
void log_internal(char const *func, char const *file, int line, char const *fmt, ...);
#define log_error(...) log_internal(__func__, __FILE__, __LINE__, __VA_ARGS__);


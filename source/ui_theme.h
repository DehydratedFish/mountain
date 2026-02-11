#pragma once

#include "definitions.h"

#define PACK_RGBA(r, g, b, a) (u32)((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define PACK_RGB(r, g, b) PACK_RGBA(r, g, b, 255)


enum UICursorShape {
    UI_CURSOR_BLOCK,
    UI_CURSOR_H_LINE,
};
struct UICursorTheme {
    u32 color;
    u32 shape;
};

struct UIFontIndex {
    s32 index;
};

struct UIFontStyle {
    UIFontIndex font;
    r32 height;
    u32 fg;
};

struct UIThemeCategory {
    UIFontStyle font_style;

    u32 background;
    u32 hover;
    u32 pressed;

    s32 padding;
    s32 margin;
};

struct UITheme {
    UIThemeCategory text;
    UIThemeCategory button;
};


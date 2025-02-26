#pragma once

#include "definitions.h"
#include "io.h"
#include "list.h"
#include "arena.h"
#include "vector.h"

#include "ui_theme.h"


struct UI;

struct UIHittest;

struct UIVertex {
    V3  pos;
    V2  uv;
    u32 color;
};

struct UIRect {
    s32 x, y, w, h;
};

struct UIGlyphInfo {
    r32 u0, v0, u1, v1;
    s32 x0, y0, x1, y1;

    s32 advance;
};

typedef V2i  UITextMetricCallback(void *data, String text, r32 height);
typedef void UIGlyphInfoCallback (void *data, UIGlyphInfo *glyph, u32 cp, r32 height);
struct UIFont {
    void *font_data;
    UITextMetricCallback *text_metrics;
    UIGlyphInfoCallback  *glyph_info;
};

struct UIFontStyle {
    s32 id;
    r32 height;
    u32 fg;
};

enum UITextAlign {
    UI_TEXT_ALIGN_LEFT,
    UI_TEXT_ALIGN_CENTER,
};

enum UITaskKind {
    UI_TASK_EMPTY,
    UI_TASK_CLIPPING,
    UI_TASK_GEOMETRY,
    UI_TASK_TEXT,
};
struct UITask {
    UITaskKind kind;
    s64 start;
    s64 count;

    union {
        struct {
            s32 font_id;
        } text;
        struct {
            UIRect area;
        } clipping;
    };
};


enum UIRegionKind {
    UI_REGION_NONE,
    UI_REGION_CUSTOM,
    UI_REGION_TEXT,
};

struct UIUserRegion {
    V2i old_widget_cursor;
};

struct UIWindow {
    void *id;
    UIRect region;

    V2i widget_cursor;
    List<UITask> tasks;
    List<UIHittest> hittests;

    UIRegionKind region_kind;
    UIUserRegion user_region;

    V2i next_widget_offset;
};

typedef void UIFrameFunc(UI *ui);
enum UIState {
    UI_STATE_IDLE,
    UI_STATE_INPUT,
    UI_STATE_DRAW,
};
struct UI {
    UIState state;

    UserInput *input;

    void *hover;
    void *focus;
    void *active;
    void *last_clicked;

    MemoryArena per_frame_memory;

    UIWindow  viewport;
    UIWindow *current_window;
    List<UIWindow> windows;

    List<UIFont> fonts;

    UIFrameFunc *frame_func;

    List<UIVertex> vertex_buffer;
};

inline r32 pt(r32 points) {
    r32 const factor = 1.0f / 0.75f;
    return points * factor;
}

void init(UI *ui, s64 memory, UIFrameFunc *initial_frame);

s32 add_font(UI *ui, UIFont font);

void do_frame(UI *ui, V2i window_size, UserInput *input);

void text_line(UI *ui, s32 font, r32 height, String text, u32 color);
s32  edit_line(UI *ui, UIFontStyle font, String text, s32 indent, s32 cursor);

b32 button(UI *ui, String text, UITheme *custom_theme = 0);
b32 button(UI *ui, void *id, String text, UITheme *custom_theme = 0);

typedef b32 UICustomKeyCallback(KeyAction *key);
b32  begin_custom_region(UI *ui, UIRect region, UICustomKeyCallback *key_callback = 0);
void end_custom_region(UI *ui);

b32  begin_text_edit_region(UI *ui, UIRect region, UICustomKeyCallback *callback);
void end_text_edit_region(UI *ui);

void offset_next_widget(UI *ui, V2i offset);
void offset_widgets(UI *ui, V2i offset);
s32  capture_scroll(UI *ui, void *id, UIRect region);


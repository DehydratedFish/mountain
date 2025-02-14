#pragma once

#include "definitions.h"
#include "io.h"
#include "list.h"
#include "arena.h"


struct UI;

typedef void UIFrameFunc(UI *ui);
typedef V2i  UITextMetricCallback(void *font_id, String text, u32 flags);

struct UIVertex;
struct UIHittest;

struct UIRect {
    s32 x, y, w, h;
};


//=============================================================================
// TODO(design):
// I am still not quite sure how I want to handle fonts.
// There are two options I can think of for now that would work for user
// specified font stuff.
// 1) Just have a small struct with the font data and a callback for the
//    metrics of a string/textblock.
//    This way the UI can ask the font to calculate the bounds, depending
//    on some options, for it.
//    This would simplify the UI code for text rendering and storing.
//    Also it seems quite flexible in terms of how text is stored and
//    rendered. No vertices are needed which means even a simple software
//    renderer can draw things.
//    The big downside of this is that the text can't be preprocessed and
//    needs to be copied into the draw command. Else it can't be
//    quaranteed that the strings are not modified in before they are drawn.
//    Using an Arena should mitigate the issue a lot though.
// 2) Adding an additional callback function to the struct that queries the
//    glyph metrics itself. This way the UI can generate the vertex data
//    directly and no additional allocations are needed.
//    The downside here would probably be flexibility in how text is drawn
//    and a lot of function pointer indirection.
//
// I go with option one for now as I never done it this way.
//=============================================================================
struct UIFont {
    void *font_data;
    UITextMetricCallback *metrics;
};

enum UITaskKind {
    UI_TASK_EMPTY,
    UI_TASK_TEXT,
};
struct UITask {
    UITaskKind kind;
    u32 flags;
    UIRect clip;

    union {
        struct {
            String content;
            s32 font_id;
            r32 size;
            u32 color;
        } text;
    };
};

struct UIWindow {
    void *id;
    UIRect region;

    V2i widget_cursor;
    List<UITask> tasks;
    List<UIHittest> hittests;
};

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
};

inline r32 pt(r32 points) {
    r32 const factor = 1.0f / 0.75f;
    return points * factor;
}

void init(UI *ui, s64 memory, UIFrameFunc *initial_frame);

s32 add_font(UI *ui, UIFont font);

void do_frame(UI *ui, V2i window_size, UserInput *input);

void text_line(UI *ui, s32 font, r32 size, u32 color, String text);


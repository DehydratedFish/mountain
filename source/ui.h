#pragma once

#include "definitions.h"
#include "io.h"
#include "list.h"
#include "arena.h"
#include "vector.h"

#include "ui_theme.h"


struct UI;
struct UIHittest;
struct Font;
struct GPUTexture;

struct UIVertex {
    V3  pos;
    V2  uv;
    u32 color;
};

struct UIRect {
    s32 x, y, w, h;
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
        GPUTexture *texture;
        struct {
            UIRect area;
        } clipping;
    };
};


struct UIFrameBuffer {
    s64 frame_number;
    Dimension frame_size;
    MemoryArena storage; // TODO: Does this need to grow in size?

    List<UIVertex> vertex_buffer;
    List<UITask>   tasks; // NOTE: Needs to be doubled and copied from each window for ordering reasons.

    u32 background;
};


struct UIWindow {
    void *id;

    V2i widget_cursor;
    V2i reset;
    List<UITask> tasks;
    List<UIHittest> hittests;
    List<UIRect> regions;

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

    List<Font*>      fonts;
    List<GPUTexture> font_textures;

    UIWindow  viewport;
    UIWindow *current_window;
    List<UIWindow> windows;

    List<UIRect> clip_stack;

    UIFrameFunc   *frame_func;
    UIFrameBuffer *frame_buffer;

    s64 frame_counter;
};


inline r32 pt(r32 points) {
    r32 const factor = 1.0f / 0.75f;
    return points * factor;
}

UIFrameBuffer create_frame_buffer(s64 size);

void init(UI *ui, UIFrameFunc *initial_frame);
void clear_background(UI *ui, u32 color);

UIFontIndex add_font(UI *ui, Font *font);

void do_frame(UI *ui, UIFrameBuffer *buffer, Dimension window_size, UserInput *input);

void v_split(UI *ui, r32 *split);

b32  begin_sub_window(UI *ui, UIRect region);
void end_sub_window(UI *ui);

b32 text_line_button(UI *ui, void *id, UIFontStyle font, String text);
b32 text_line_button(UI *ui, void *id, String text);

void image_view(UI *ui, GPUTexture *image, Dimension resize = {0, 0});


// TODO: Make it possible to specify the renderer?
void render_frame(UIFrameBuffer *frame_buffer);



b32 button(UI *ui, String text, UITheme *custom_theme = 0);
b32 button(UI *ui, void *id, String text, UITheme *custom_theme = 0);

s32  capture_scroll(UI *ui, void *id, UIRect region);


#pragma once

#include "definitions.h"
#include "io.h"
#include "list.h"


struct UI;

typedef void UIFrameFunc(UI *ui);
typedef V2i  UITextMetricCallback(void *font_id, String text);

struct UIVertex;
struct UIHittest;
struct UICommand;

struct UIRect {
    s32 x, y, w, h;
};

struct UIFont {
    void *font_data;
    UITextMetricCallback *metrics;
};

struct UIWindow {
    void *id;
    UIRect region;

    V2i widget_cursor;
    List<UICommand> commands;
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

    UIWindow  viewport;
    UIWindow *current_window;
    List<UIWindow> windows;

    UIFrameFunc *frame_func;
    List<UIVertex> vertex_buffer;
};


void init(UI *ui, UIFrameFunc *initial_frame);

void do_frame(UI *ui, V2i window_size, UserInput *input);


#include "ui.h"



INTERNAL String duplicate(UI *ui, String str) {
    if (str.size == 0) return {};

    u8 *tmp = (u8*)allocate_from_arena(&ui->per_frame_memory, str.size, 0, 0);
    copy_memory(tmp, str.data, str.size);
    str.data = tmp;

    return str;
}

INTERNAL void default_frame_func(UI *ui) {
}


void init(UI *ui, s64 memory, UIFrameFunc *initial_frame) {
    UI new_ui = {};
    new_ui.current_window = &new_ui.viewport;
    init(&new_ui.per_frame_memory, memory);

    if (initial_frame) {
        new_ui.frame_func = initial_frame;
    } else {
        new_ui.frame_func = default_frame_func;
    }

    *ui = new_ui;
}

s32 add_font(UI *ui, UIFont font) {
    s32 index = ui->fonts.size;
    append(&ui->fonts, font);

    return index;
}

INTERNAL void prepare_window(UIWindow *window) {
    window->tasks.size    = 0;
    window->hittests.size = 0;

    window->widget_cursor = {0, 0};
}

void do_frame(UI *ui, V2i window_size, UserInput *input) {
    assert(input);
    assert(ui->frame_func);

    ui->viewport.region.w = window_size.w;
    ui->viewport.region.h = window_size.h;

    ui->per_frame_memory.used = 0;

    ui->input = input;

    prepare_window(&ui->viewport);
    ui->current_window = &ui->viewport;

    {
        ui->state = UI_STATE_INPUT;
        ui->frame_func(ui);

        ui->state = UI_STATE_DRAW;
        ui->frame_func(ui);
    }

    ui->state = UI_STATE_IDLE;
    ui->input = 0;
}


INTERNAL UITask *create_new_task(UI *ui, UITaskKind kind) {
    UITask task = {};
    task.kind = kind;
    
    return append(&ui->current_window->tasks, task);
}

void text_line(UI *ui, s32 font, r32 size, u32 color, String text) {
    if (ui->state == UI_STATE_DRAW) {
        UITask *task = create_new_task(ui, UI_TASK_TEXT);

        // TODO: This duplicate can be potentially very large.
        task->text.content = duplicate(ui, text);
        task->text.font_id = font;
        task->text.size    = size;
        task->text.color   = color;
    }
}

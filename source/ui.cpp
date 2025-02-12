#include "ui.h"


// TODO: Maybe display a funny message on a DEVELOPER build?
INTERNAL void default_frame_func(UI *ui) {}


void init(UI *ui, UIFrameFunc *initial_frame) {
    UI new_ui = {};
    new_ui.current_window = &new_ui.viewport;

    if (initial_frame) {
        new_ui.frame_func = initial_frame;
    } else {
        new_ui.frame_func = default_frame_func;
    }

    *ui = new_ui;
}

void do_frame(UI *ui, V2i window_size, UserInput *input) {
    assert(input);
    assert(ui->frame_func);

    ui->viewport.region.w = window_size.w;
    ui->viewport.region.h = window_size.h;

    ui->input = input;

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


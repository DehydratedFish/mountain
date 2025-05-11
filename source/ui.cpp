#include "ui.h"

#include "utf.h"



INTERNAL UICursorTheme const DefaultCursor {
    0,
    UI_CURSOR_BLOCK,
};

struct UIHittest {
    void *id;
    UIRect rect;
};

enum ButtonState {
    BUTTON_IDLE,
    BUTTON_HOVER,
    BUTTON_PRESSED,
    BUTTON_ACTIVATE,
};

INTERNAL ButtonState click_info(UI *ui, void *id) {
    ButtonState state = BUTTON_IDLE;

    b32 lmb_now  = ui->input->mouse.keys[MOUSE_LMB];
    b32 lmb_last = ui->input->last_mouse.keys[MOUSE_LMB];
    if (ui->hover == id) {
        if (!lmb_last && lmb_now) {
            ui->last_clicked = id;
            state = BUTTON_PRESSED;
        } else if (lmb_last && !lmb_now && ui->last_clicked == id) {
            state = BUTTON_ACTIVATE;
        } else if (ui->last_clicked == id) {
            state = BUTTON_PRESSED;
        } else {
            state = BUTTON_HOVER;
        }
    } else if (ui->last_clicked == id) {
        state = BUTTON_PRESSED;
    }

    return state;
}

INTERNAL String duplicate(UI *ui, String str) {
    if (str.size == 0) return {};

    u8 *tmp = (u8*)allocate_from_arena(&ui->per_frame_memory, str.size, 0, 0);
    copy_memory(tmp, str.data, str.size);
    str.data = tmp;

    return str;
}

INTERNAL void default_frame_func(UI *ui) {
}
INTERNAL r32 DefaultFontHeight = pt(16.0f);
INTERNAL u32 DefaultFontColor  = PACK_RGB(210, 210, 210);


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

INTERNAL b32 cursor_in_rect(UI *ui, UIRect rect) {
    Point p = ui->input->mouse.cursor_pos;

    s32 x1 = rect.x + rect.w;
    s32 y1 = rect.y + rect.h;

    return p.x >= rect.x && p.y >= rect.y && p.x < x1 && p.y < y1;
}

INTERNAL void *hovered_element(UI *ui) {
    // TODO: Put last active window at the back?
    FOR (ui->windows, window) {
        for (s64 i = -1; i >= -window->hittests.size; i -= 1) {
            UIHittest *hit = &window->hittests[i];

            if (cursor_in_rect(ui, hit->rect)) return hit->id;
        }
    }

    for (s64 i = -1; i >= -ui->viewport.hittests.size; i -= 1) {
        UIHittest *hit = &ui->viewport.hittests[i];

        if (cursor_in_rect(ui, hit->rect)) return hit->id;
    }

    return 0;
}

// TODO: Concatenate tasks that have the same kind and texture.
INTERNAL UITask *create_new_task(UI *ui, UITaskKind kind) {
    UITask task = {};
    task.kind  = kind;
    task.start = ui->vertex_buffer.size;
    
    return append(&ui->current_window->tasks, task);
}

// TODO: This is problematic because you can't add vertices to two different
//       tasks at the same time or the indices will overlap.
INTERNAL UITask *create_new_task_before(UI *ui, UITaskKind kind) {
    UITask task = {};
    task.kind  = kind;
    task.start = ui->vertex_buffer.size;
    
    return insert(&ui->current_window->tasks, -1, task);
}

INTERNAL void generate_clip_task(UI *ui, UIRect rect) {
    UITask *task = create_new_task(ui, UI_TASK_CLIPPING);
    task->clipping.area = rect;
}


INTERNAL void prepare_window(UIWindow *window) {
    window->tasks.size    = 0;
    window->hittests.size = 0;

    window->widget_cursor = {0, 0};
}

INTERNAL void push_clipping(UI *ui, UIRect rect) {
    append(&ui->clip_stack, rect);

    generate_clip_task(ui, rect);
}

INTERNAL void pop_clipping(UI *ui) {
    assert(ui->clip_stack.size > 0);
    pop(&ui->clip_stack);

    if (ui->clip_stack.size > 0) {
        generate_clip_task(ui, ui->clip_stack[-1]);
    } else {
        generate_clip_task(ui, ui->viewport.region);
    }
}

void do_frame(UI *ui, V2i window_size, UserInput *input) {
    assert(input);
    assert(ui->frame_func);
    assert(ui->clip_stack.size == 0);
    // TODO: Always load a default font.
    assert(ui->fonts.size > 0);

    ui->viewport.region.w = window_size.w;
    ui->viewport.region.h = window_size.h;

    ui->per_frame_memory.used = 0;
    ui->vertex_buffer.size = 0;

    ui->input = input;

    ui->hover = hovered_element(ui);
    ui->current_window = &ui->viewport;

    prepare_window(&ui->viewport);

    {
        ui->state = UI_STATE_INPUT;
        ui->frame_func(ui);

        ui->state = UI_STATE_DRAW;
        ui->frame_func(ui);
    }

    if (!ui->input->mouse.keys[MOUSE_LMB]) {
        ui->last_clicked = 0;
    }

    ui->state = UI_STATE_IDLE;
    ui->input = 0;
}



INTERNAL b32 size_from_layout(UI *ui, UIRect *rect) {
    // TODO: Check if a layout is set or else just return false.
    rect->x = ui->current_window->widget_cursor.x;
    rect->y = ui->current_window->widget_cursor.y;

    return false;
}

INTERNAL b32 widget_sizing(UI *ui, UIRect *rect, UITheme const *theme = 0) {
    UIWindow *window = ui->current_window;
    s32 margin = theme ? theme->margin : 0;

    rect->x = window->widget_cursor.x + margin + window->next_widget_offset.x;
    rect->y = window->widget_cursor.y + margin + window->next_widget_offset.y;
    window->next_widget_offset = {};

    // TODO: Calculate width and height from layout.
    return false;
}


INTERNAL void add_hittest(UI *ui, void *id, UIRect rect) {
    UIHittest hit = {
        id,
        rect,
    };

    append(&ui->current_window->hittests, hit);
}

INTERNAL void append(UI *ui, UITask *task, UIVertex vertex) {
    append(&ui->vertex_buffer, vertex);
    task->count += 1;
}

INTERNAL void draw_rect(UI *ui, UITask *task, UIRect rect, u32 color) {
    r32 x0 = (r32)rect.x;
    r32 y0 = (r32)rect.y;
    r32 x1 = x0 + rect.w;
    r32 y1 = y0 + rect.h;

    append(ui, task, {{x0, y0}, {}, color});
    append(ui, task, {{x1, y0}, {}, color});
    append(ui, task, {{x0, y1}, {}, color});
               
    append(ui, task, {{x0, y1}, {}, color});
    append(ui, task, {{x1, y0}, {}, color});
    append(ui, task, {{x1, y1}, {}, color});
}

INTERNAL void draw_rect(UI *ui, UIRect rect, u32 color) {
    UITask *task = create_new_task(ui, UI_TASK_GEOMETRY);
    draw_rect(ui, task, rect, color);
}

INTERNAL void glyph_info(UIFont *font, UIGlyphInfo *info, u32 cp, r32 height) {
    assert(font->glyph_info);
    font->glyph_info(font->font_data, info, cp, height);
}

INTERNAL void draw_character(UI *ui, UITask *task, UIGlyphInfo *glyph, V2i pos, r32 height, u32 color) {
    r32 x0 = (r32)(pos.x + glyph->x0);
    r32 y0 = (r32)(pos.y + glyph->y0);
    r32 x1 = (r32)(pos.x + glyph->x1);
    r32 y1 = (r32)(pos.y + glyph->y1);

    append(ui, task, {{x0, y0}, {glyph->u0, glyph->v0}, color});
    append(ui, task, {{x1, y0}, {glyph->u1, glyph->v0}, color});
    append(ui, task, {{x0, y1}, {glyph->u0, glyph->v1}, color});

    append(ui, task, {{x0, y1}, {glyph->u0, glyph->v1}, color});
    append(ui, task, {{x1, y0}, {glyph->u1, glyph->v0}, color});
    append(ui, task, {{x1, y1}, {glyph->u1, glyph->v1}, color});
}

INTERNAL V2i text_metrics(UIFont *font, String text, r32 height) {
    assert(font->text_metrics);
    return font->text_metrics(font->font_data, text, height);
}

// TODO: Additional selectable text function.
INTERNAL void draw_text(UI *ui, s32 font_id, UIRect rect, r32 height, String text, u32 color, UITextAlign align = UI_TEXT_ALIGN_LEFT) {
    UITask *task = create_new_task(ui, UI_TASK_TEXT);

    UIFont *font = &ui->fonts[font_id];

    V2i cursor = {rect.x, rect.y};
    if (align == UI_TEXT_ALIGN_CENTER) {
        V2i metrics = text_metrics(font, text, height);

        s32 x_offset = (rect.x + (rect.w / 2)) - (metrics.w / 2);
        s32 y_offset = (rect.y + (rect.h / 2)) - (metrics.h / 2);

        cursor.x = x_offset;
        cursor.y = y_offset;
    }

    UIGlyphInfo glyph = {};
    for (auto it = make_utf8_it(text); it.valid; next(&it)) {
        glyph_info(font, &glyph, it.cp, height);
        draw_character(ui, task, &glyph, cursor, height, color);

        cursor.x += glyph.advance;
    }
}

void text_line(UI *ui, s32 font, r32 height, String text, u32 color) {
    if (ui->state == UI_STATE_DRAW) {
        UIRect rect = {
            ui->current_window->widget_cursor.x,
            ui->current_window->widget_cursor.y,
        }; // NOTE: Width and height currently irrelevant.

        rect.x += ui->current_window->next_widget_offset.x;
        rect.y += ui->current_window->next_widget_offset.y;

        ui->current_window->next_widget_offset = {};

        draw_text(ui, font, rect, height, text, color);
        ui->current_window->widget_cursor.y += (s32)height;
    }
}

s32 edit_line(UI *ui, UIFontStyle font, String text, s32 indent, s32 cursor) {
    if (ui->state == UI_STATE_DRAW) {
        UIRect rect = {};
        if (!widget_sizing(ui, &rect)) {
            ui->current_window->widget_cursor.y += (s32)font.height;
        }
    }

    return -1;
}

INTERNAL UITheme const ButtonTheme = {
    0,
    DefaultFontHeight,
    DefaultFontColor,

    PACK_RGB(60, 60, 60),
    PACK_RGB(80, 80, 80),
    PACK_RGB(100, 100, 100),

    10,
    5,
};

b32 button(UI *ui, String text, UITheme *custom_theme) {
    return button(ui, text.data, text, custom_theme);
}

b32 button(UI *ui, void *id, String text, UITheme *custom_theme) {
    b32 clicked = false;

    ButtonState state = click_info(ui, id);
    if (ui->state == UI_STATE_DRAW) {
        UITheme const *theme = &ButtonTheme;
        if (custom_theme) theme = custom_theme;

        UIRect rect = {};
        if (!size_from_layout(ui, &rect)) {
            // TODO: Size is calculated later in rendering again. Maybe cache it somehow?
            UIFont *font = &ui->fonts[theme->font_id];
            V2i text_size = font->text_metrics(font->font_data, text, theme->font_height);
            rect.w = text_size.w + theme->padding;
            rect.h = text_size.h + theme->padding;
        }

        rect.x += theme->margin;
        rect.y += theme->margin;

        add_hittest(ui, id, rect);

        u32 color = theme->background;
        if (state == BUTTON_HOVER) color   = theme->hover;
        if (state == BUTTON_PRESSED) color = theme->pressed;
        draw_rect(ui, rect, color);

        draw_text(ui, theme->font_id, rect, theme->font_height, text, theme->font_color, UI_TEXT_ALIGN_CENTER);
    } else if (ui->state == UI_STATE_INPUT) {
        if (state == BUTTON_ACTIVATE) {
            clicked = true;
        }
    }

    return clicked;
}

b32 begin_custom_region(UI *ui, UIRect region, u32 background) {
    b32 open = false;

    UIWindow *window = ui->current_window;

    // TODO: Maybe stack nested regions?
    if (window->region_kind != UI_REGION_NONE) return open;

    open = true;

    window->region_kind = UI_REGION_CUSTOM;
    window->user_region.old_widget_cursor = window->widget_cursor;

    window->widget_cursor.x = region.x;
    window->widget_cursor.y = region.y;

    if (ui->state == UI_STATE_DRAW && background) {
        draw_rect(ui, region, background);
    }

    return open;
}

void end_custom_region(UI *ui) {
    ui->current_window->region_kind = UI_REGION_NONE;
}

void offset_next_widget(UI *ui, V2i offset) {
    UIWindow *window = ui->current_window;
    assert(window->region_kind != UI_REGION_NONE);

    window->next_widget_offset = offset;
}

void offset_widgets(UI *ui, V2i offset) {
    UIWindow *window = ui->current_window;
    assert(window->region_kind != UI_REGION_NONE);

    window->widget_cursor.x += offset.x;
    window->widget_cursor.y += offset.y;

    window->reset.x += offset.x;
    window->reset.y += offset.y;
}

s32 capture_scroll(UI *ui, void *id, UIRect region) {
    s32 scroll = 0;

    if (ui->state == UI_STATE_INPUT) {
        add_hittest(ui, id, region);

        if (ui->hover == id) {
            scroll = ui->input->mouse.scroll;
        }
    }

    return scroll;
}

b32 begin_text_edit_region(UI *ui, UIRect region, UICustomKeyCallback *callback) {
    b32 open = false;

    UIWindow *window = ui->current_window;

    // TODO: Maybe stack nested regions?
    if (window->region_kind != UI_REGION_NONE) return open;

    open = true;

    window->region_kind = UI_REGION_TEXT;
    window->user_region.old_widget_cursor = window->widget_cursor;
    window->user_region.old_reset = window->reset;
    window->user_region.region = region;

    window->widget_cursor.x = region.x;
    window->widget_cursor.y = region.y;

    window->reset = window->widget_cursor;

    push_clipping(ui, region);

    if (ui->state == UI_STATE_INPUT && callback) {
        for (s32 i = 0; i < ui->input->actions.used; i += 1) {
            callback(&ui->input->actions.keys[i]);
        }
    }

    return open;
}

void end_text_edit_region(UI *ui) {
    UIWindow *window = ui->current_window;
    assert(window->region_kind == UI_REGION_TEXT);

    window->region_kind   = UI_REGION_NONE;
    window->widget_cursor = window->user_region.old_widget_cursor;
    window->reset         = window->user_region.old_reset;

    pop_clipping(ui);
}

s32 text_piece(UI *ui, String text, UIFontStyle style, u8 *cursor_pos) {
    s32 hovered_character = -1;

    // TODO: The hover calculation should actually happen in the input state.
    //       But that means the string would be processed twice per frame
    //       and I think that is unnecesarry.
    if (ui->state == UI_STATE_DRAW) {
        UITask *task = create_new_task(ui, UI_TASK_TEXT);

        UIFont *font = &ui->fonts[style.id];

        UIRect rect = {};
        widget_sizing(ui, &rect);
        V2i cursor = {rect.x, rect.y};

        Point pointer = ui->input->mouse.cursor_pos;

        b32 draw_cursor = false;
        V2i cursor_bg   = {};
        s32 cursor_width = 0;

        s32 i = 0;
        UIGlyphInfo glyph = {};
        for (auto it = make_utf8_it(text); it.valid; next(&it)) {
            glyph_info(font, &glyph, it.cp, style.height);

            if (&text[it.index] == cursor_pos) {
                draw_cursor = true;
                cursor_bg   = cursor;
                cursor_width = glyph.advance;
                draw_character(ui, task, &glyph, cursor, style.height, PACK_RGB(15, 15, 15));
            } else {
                draw_character(ui, task, &glyph, cursor, style.height, style.fg);
            }

            s32 new_x = cursor.x + glyph.advance;
            if (cursor.x < pointer.x && new_x >= pointer.x) {
                hovered_character = i;
            }

            cursor.x = new_x;
            i += 1;
        }

        if (end(text) == cursor_pos) {
            glyph_info(font, &glyph, ' ', style.height);
            draw_cursor = true;
            cursor_bg   = cursor;
            cursor_width = glyph.advance;
        }

        if (draw_cursor) {
            UIRect  cursor_quad = {cursor_bg.x, cursor_bg.y, cursor_width, (s32)style.height};
            UITask *cursor_task = create_new_task_before(ui, UI_TASK_GEOMETRY);

            u32 color = DefaultCursor.color;
            if (color == 0) color = style.fg;
            draw_rect(ui, cursor_task, cursor_quad, color);
        }

        if (pointer.y < cursor.y || pointer.y >= (cursor.y + style.height)) {
            hovered_character = -1;
        }

        ui->current_window->widget_cursor.x = cursor.x;
    }

    return hovered_character;
}

b32 advance_text_line(UI *ui, s32 line_height) {
    b32 keep_advancing = false;

    UIWindow *window = ui->current_window;
    if (window->region_kind != UI_REGION_TEXT) return keep_advancing;

    s32 region_end = window->user_region.region.y + window->user_region.region.h;
    if (window->widget_cursor.y < region_end) {
        window->widget_cursor.y += line_height;
        window->widget_cursor.x  = window->reset.x;
        keep_advancing = true;
    }

    return keep_advancing;
}

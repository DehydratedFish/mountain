#include "ui.h"

#include "utf.h"
#include "font.h"
#include "renderer.h"



INTERNAL s32 const DefaultPadding = 5;
INTERNAL s32 const DefaultMargin  = 5;

INTERNAL r32 DefaultFontHeight = pt(16.0f);
INTERNAL u32 DefaultFontColor  = PACK_RGB(210, 210, 210);
INTERNAL u32 DefaultFontColorHover = PACK_RGB(75, 25, 160);

INTERNAL UIThemeCategory const DefaultTextTheme = {
    0,
    DefaultFontHeight,
    DefaultFontColor,

    DefaultFontColor,
    DefaultFontColorHover,
    DefaultFontColor,

    DefaultPadding,
    DefaultMargin,
};

INTERNAL UIThemeCategory const DefaultButtonTheme = {
    DefaultTextTheme.font_style,

    PACK_RGB(60, 60, 60),
    PACK_RGB(80, 80, 80),
    PACK_RGB(100, 100, 100),

    DefaultPadding,
    DefaultMargin,
};


INTERNAL UICursorTheme const DefaultCursor {
    0,
    UI_CURSOR_BLOCK,
};


INTERNAL b32 TheThemeInitialized = false;
INTERNAL UITheme TheTheme;

// NOTE: Default font is always at index zero.
// TODO: Make sure it is also loaded at all time.
INTERNAL void init_default_theme() {
    TheTheme.text = DefaultTextTheme;
    TheTheme.button = DefaultButtonTheme;
}


INTERNAL Font DefaultFont;


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

    u8 *tmp = (u8*)allocate_from_arena(&ui->frame_buffer->storage, str.size, 0, 0);
    copy_memory(tmp, str.data, str.size);
    str.data = tmp;

    return str;
}

INTERNAL void default_frame_func(UI *ui) {
}

UIFrameBuffer create_frame_buffer(s64 size) {
    UIFrameBuffer frame_buffer = {};
    init(&frame_buffer.storage, KILOBYTES(4));

    return frame_buffer;
}

void init(UI *ui, UIFrameFunc *initial_frame) {
    if (!TheThemeInitialized) {
        init_default_theme();
    }

    UI new_ui = {};
    new_ui.current_window = &new_ui.viewport;
    append(&new_ui.viewport.regions);

    String liberation_font_file = "fonts/LiterationMonoNerdFontMono-Regular.ttf";
    s32 const font_atlas_size = 2048;
    if (!init(&DefaultFont, liberation_font_file, 64.0f, font_atlas_size)) {
        print("Could not load font %S.\n", liberation_font_file);
        die("Aborting application...");
    }

    add_font(&new_ui, &DefaultFont);

    if (initial_frame) {
        new_ui.frame_func = initial_frame;
    } else {
        new_ui.frame_func = default_frame_func;
    }

    *ui = new_ui;
}

void clear_background(UI *ui, u32 color) {
    ui->frame_buffer->background = color;
}

UIFontIndex add_font(UI *ui, Font *font) {
    UIFontIndex result = {(s32)ui->fonts.size};
    append(&ui->fonts, font);

    GPUTexture *tex = append(&ui->font_textures);
    upload_gpu_texture(tex, {font->atlas_size, font->atlas_size}, 1, font->atlas);

    return result;
}

Font *font_from_index(UI *ui, UIFontIndex font) {
    BOUNDS_CHECK(0, ui->fonts.size, font.index, "Font index out of bounds. Did you forget to load a font earlier?");

    return ui->fonts[font.index];
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
    task.start = ui->frame_buffer->vertex_buffer.size;
    
    return append(&ui->current_window->tasks, task);
}

// TODO: This is problematic because you can't add vertices to two different
//       tasks at the same time or the indices will overlap.
INTERNAL UITask *create_new_task_before(UI *ui, UITaskKind kind) {
    UITask task = {};
    task.kind  = kind;
    task.start = ui->frame_buffer->vertex_buffer.size;
    
    return insert(&ui->current_window->tasks, -1, task);
}

INTERNAL void generate_clip_task(UI *ui, UIRect rect) {
    UITask *task = create_new_task(ui, UI_TASK_CLIPPING);
    task->clipping.area = rect;
}


INTERNAL void prepare_window(UIWindow *window) {
    window->tasks.size    = 0;
    window->hittests.size = 0;

    window->widget_cursor = {window->regions[-1].x, window->regions[-1].y};
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
        generate_clip_task(ui, ui->viewport.regions[-1]);
    }
}

void do_frame(UI *ui, UIFrameBuffer *buffer, Dimension window_size, UserInput *input) {
    assert(input);
    assert(buffer);
    assert(ui->frame_func);
    assert(ui->clip_stack.size == 0);
    // TODO: Always load a default font.
    assert(ui->fonts.size > 0);

    ui->viewport.regions[-1].w = window_size.w;
    ui->viewport.regions[-1].h = window_size.h;

    buffer->storage.used = 0;
    buffer->vertex_buffer.size = 0;
    buffer->tasks.size   = 0;
    buffer->frame_number = ui->frame_counter;
    buffer->frame_size   = window_size;
    ui->frame_buffer = buffer;

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
    ui->frame_counter += 1;

    FOR (ui->windows, window) {
        Array<UITask> tasks = window->tasks;
        append(&ui->frame_buffer->tasks, tasks);
    }

    {
        Array<UITask> tasks = ui->viewport.tasks;
        append(&ui->frame_buffer->tasks, tasks);
    }

    for (s64 i = 0; i < ui->fonts.size; i += 1) {
        Font *font = ui->fonts[i];
        if (ui->fonts[i]->is_dirty) {
            upload_gpu_texture(&ui->font_textures[i], {font->atlas_size, font->atlas_size}, 1, font->atlas);
        }
    }

    assert(ui->viewport.regions.size == 1);
}



INTERNAL b32 size_from_layout(UI *ui, UIRect *rect, s32 margin) {
    // TODO: Check if a layout is set or else just return false.
    rect->x = ui->current_window->widget_cursor.x + margin;
    rect->y = ui->current_window->widget_cursor.y + margin;

    return false;
}

INTERNAL b32 widget_sizing(UI *ui, UIRect *rect, UIThemeCategory const *theme = 0) {
    UIWindow *window = ui->current_window;
    s32 margin = theme ? theme->margin : 0;

    rect->x = window->widget_cursor.x + margin + window->next_widget_offset.x;
    rect->y = window->widget_cursor.y + margin + window->next_widget_offset.y;
    window->next_widget_offset = {};

    // TODO: Calculate width and height from layout.
    return false;
}

INTERNAL void advance_widget_cursor(UI *ui, UIRect rect, s32 margin) {
    UIWindow *window = ui->current_window;

    window->widget_cursor.x  = window->regions[-1].x;
    window->widget_cursor.y += margin + rect.h;
}


INTERNAL void add_hittest(UI *ui, void *id, UIRect rect) {
    UIHittest hit = {
        id,
        rect,
    };

    append(&ui->current_window->hittests, hit);
}

INTERNAL void append(UI *ui, UITask *task, UIVertex vertex) {
    append(&ui->frame_buffer->vertex_buffer, vertex);
    task->count += 1;
}

INTERNAL void draw_rect(UI *ui, UITask *task, UIRect rect, u32 color) {
    r32 x0 = (r32)rect.x;
    r32 y0 = (r32)rect.y;
    r32 x1 = x0 + rect.w;
    r32 y1 = y0 + rect.h;

    append(ui, task, {{x0, y0}, {0.0f, 0.0f}, color});
    append(ui, task, {{x1, y0}, {1.0f, 0.0f}, color});
    append(ui, task, {{x0, y1}, {0.0f, 1.0f}, color});
               
    append(ui, task, {{x0, y1}, {0.0f, 1.0f}, color});
    append(ui, task, {{x1, y0}, {1.0f, 0.0f}, color});
    append(ui, task, {{x1, y1}, {1.0f, 1.0f}, color});
}

INTERNAL void draw_rect(UI *ui, UIRect rect, u32 color) {
    UITask *task = create_new_task(ui, UI_TASK_GEOMETRY);
    draw_rect(ui, task, rect, color);
}

INTERNAL void draw_rect(UI *ui, UIRect rect, GPUTexture *tex) {
    UITask *task = create_new_task(ui, UI_TASK_GEOMETRY);
    draw_rect(ui, task, rect, PACK_RGB(255, 255, 255));
    task->texture = tex;
}

INTERNAL void draw_character(UI *ui, UITask *task, GlyphInfo *glyph, V2i pos, r32 height, u32 color) {
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


// TODO: Additional selectable text function.
INTERNAL void draw_text(UI *ui, UIFontIndex font_index, UIRect rect, r32 height, String text, u32 color, UITextAlign align = UI_TEXT_ALIGN_LEFT) {
    UITask *task = create_new_task(ui, UI_TASK_TEXT);

    Font *font = ui->fonts[font_index.index];
    task->texture = &ui->font_textures[font_index.index];

    V2i cursor = {rect.x, rect.y};
    if (align == UI_TEXT_ALIGN_CENTER) {
        FontDimensions metrics = text_dimensions(font, text, height);
        Dimension m = {
            (s32)ceil(metrics.width),
            (s32)ceil(metrics.height)
        };

        s32 x_offset = (rect.x + (rect.w / 2)) - (m.w / 2);
        s32 y_offset = (rect.y + (rect.h / 2)) - (m.h / 2);

        cursor.x = x_offset;
        cursor.y = y_offset;
    }

    for (auto it = make_utf8_it(text); it.valid; next(&it)) {
        GlyphInfo glyph = get_glyph(font, it.cp, height);
        draw_character(ui, task, &glyph, cursor, height, color);

        cursor.x += (s32)glyph.advance;
    }
}

void v_split(UI *ui, r32 *split) {
    assert(split);
    void *id = split;

    if (ui->state == UI_STATE_DRAW) {
        UIWindow *window = ui->current_window;
        UIRect rect = {(s32)(window->regions[-1].w * *split), window->regions[-1].y, 1, window->regions[-1].h};
        // TODO: Put color into theme.
        draw_rect(ui, rect, PACK_RGB(150, 150, 150));

        rect.w += 3;
        add_hittest(ui, id, rect);
    } else if (ui->state == UI_STATE_INPUT) {
        ButtonState state = click_info(ui, id);
    }
}


b32 begin_sub_window(UI *ui, UIRect region) {
    UIWindow *window = ui->current_window;
    append(&window->regions, region);

    window->widget_cursor = {region.x, region.y};

    // TODO: Check if window is outside of bounds.
    return true;
}

void end_sub_window(UI *ui) {
    UIWindow *window = ui->current_window;
    pop(&window->regions);

    window->widget_cursor = {window->regions[-1].x, window->regions[-1].y};
}


void text_line(UI *ui, UIFontStyle style, String text) {
    if (ui->state == UI_STATE_DRAW) {
        UIRect rect = {
            ui->current_window->widget_cursor.x,
            ui->current_window->widget_cursor.y,
        }; // NOTE: Width and height currently irrelevant.

        rect.x += ui->current_window->next_widget_offset.x;
        rect.y += ui->current_window->next_widget_offset.y;

        ui->current_window->next_widget_offset = {};

        draw_text(ui, style.font, rect, style.height, text, style.fg);
        ui->current_window->widget_cursor.y += (s32)style.height;
    }
}

b32 text_line_button(UI *ui, void *id, UIFontStyle *font_style, String text) {
    b32 clicked = false;

    ButtonState state = click_info(ui, id);
    if (ui->state == UI_STATE_DRAW) {
        UIRect rect = {};
        if (!size_from_layout(ui, &rect, 0)) {
            // TODO: Size is calculated later in rendering again. Maybe cache it somehow?
            Font *font = font_from_index(ui, font_style->font);
            FontDimensions text_size = text_dimensions(font, text, font_style->height);
            rect.w = (s32)ceil(text_size.width);
            rect.h = (s32)ceil(text_size.height);
        }

        add_hittest(ui, id, rect);

        advance_widget_cursor(ui, rect, 0);

        UIFontStyle style = TheTheme.text.font_style;
        if (state == BUTTON_HOVER) style.fg = TheTheme.text.hover;

        draw_text(ui, style.font, rect, style.height, text, style.fg, UI_TEXT_ALIGN_CENTER);
    } else if (ui->state == UI_STATE_INPUT) {
        if (state == BUTTON_ACTIVATE) {
            clicked = true;
        }
    }

    return clicked;
}

b32 text_line_button(UI *ui, void *id, String text) {
    return text_line_button(ui, id, &TheTheme.text.font_style, text);
}



void image_view(UI *ui, GPUTexture *image, Dimension resize) {
    if (ui->state == UI_STATE_DRAW) {
        UIRect rect = {};
        if (!size_from_layout(ui, &rect, 0)) {
            rect.w = image->size.w;
            rect.h = image->size.h;

            if (resize.w) {
                if (resize.h == 0) {
                    r32 aspect = (r32)rect.w / (r32)rect.h;
                    rect.h = (s32)(resize.w / aspect);
                } else {
                    rect.h = resize.h;
                }
                rect.w = resize.w;
            } else if (resize.h) {
                r32 aspect = (r32)rect.w / (r32)rect.h;
                rect.w = (s32)(resize.h * aspect);
                rect.h = resize.h;
            }
        }

        advance_widget_cursor(ui, rect, 0);

        draw_rect(ui, rect, image);
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



b32 button(UI *ui, String text, UITheme *custom_theme) {
    return button(ui, text.data, text, custom_theme);
}

b32 button(UI *ui, void *id, String text, UITheme *custom_theme) {
    b32 clicked = false;

    ButtonState state = click_info(ui, id);
    if (ui->state == UI_STATE_DRAW) {
        UIRect rect = {};
        if (!size_from_layout(ui, &rect, TheTheme.button.margin)) {
            // TODO: Size is calculated later in rendering again. Maybe cache it somehow?
            Font *font = font_from_index(ui, TheTheme.button.font_style.font);
            FontDimensions text_size = text_dimensions(font, text, TheTheme.button.font_style.height);
            rect.w = (s32)ceil(text_size.width)  + TheTheme.button.padding;
            rect.h = (s32)ceil(text_size.height) + TheTheme.button.padding;
        }

        rect.x += TheTheme.button.margin;
        rect.y += TheTheme.button.margin;

        add_hittest(ui, id, rect);

        u32 color = TheTheme.button.background;
        if (state == BUTTON_HOVER)   color = TheTheme.button.hover;
        if (state == BUTTON_PRESSED) color = TheTheme.button.pressed;
        draw_rect(ui, rect, color);

        draw_text(ui, TheTheme.button.font_style.font, rect, TheTheme.button.font_style.height, text, TheTheme.button.font_style.fg, UI_TEXT_ALIGN_CENTER);
    } else if (ui->state == UI_STATE_INPUT) {
        if (state == BUTTON_ACTIVATE) {
            clicked = true;
        }
    }

    return clicked;
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


INTERNAL Renderer UIRenderer;
INTERNAL GPUBuffer UIVertexBuffer;
INTERNAL GPUShader UITextShader;
INTERNAL GPUShader UIShapeShader;
INTERNAL b32 UIRendererInitialized;
INTERNAL void init_ui_renderer() {
    init_renderer(&UIRenderer);
    set_shader_folder(&UIRenderer, "shader");
    UITextShader  = load_gpu_shader(&UIRenderer, "ui_text");
    UIShapeShader = load_gpu_shader(&UIRenderer, "ui_shape");

    VertexBinding bindings[] = {
        {VERTEX_COMPONENT_V3, ShaderPositionLocation, (u32)STRUCT_OFFSET(UIVertex, pos)},
        {VERTEX_COMPONENT_V2, ShaderUVLocation      , (u32)STRUCT_OFFSET(UIVertex, uv)},
        {VERTEX_COMPONENT_PACKED_COLOR, ShaderForegroundColorLocation, (u32)STRUCT_OFFSET(UIVertex, color)},
    };
    create_gpu_buffer(&UIVertexBuffer, sizeof(UIVertex), 0, 0, {bindings, C_ARRAY_SIZE(bindings)});

    UIRendererInitialized = true;
}

void render_frame(UIFrameBuffer *frame_buffer) {
    if (!UIRendererInitialized) init_ui_renderer();

    if (frame_buffer->background) {
        clear_background(&UIRenderer, frame_buffer->background);
    }

    update_render_area(&UIRenderer, frame_buffer->frame_size);

    update_gpu_buffer(&UIVertexBuffer, frame_buffer->vertex_buffer.data, frame_buffer->vertex_buffer.size);
    glBindVertexArray(UIVertexBuffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, UIVertexBuffer.vbo);
    glEnable(GL_SCISSOR_TEST);

    FOR (frame_buffer->tasks, task) {
        if (task->kind == UI_TASK_GEOMETRY) {
            glUseProgram(UIShapeShader.id);
            
            if (task->texture) glBindTexture(GL_TEXTURE_2D, task->texture->id);
            else               glBindTexture(GL_TEXTURE_2D, 0);

            glDrawArrays(GL_TRIANGLES, task->start, task->count);
        } else if (task->kind == UI_TASK_TEXT) {
            glUseProgram(UITextShader.id);
            glBindTexture(GL_TEXTURE_2D, task->texture->id);
            glDrawArrays(GL_TRIANGLES, task->start, task->count);
        } else if (task->kind == UI_TASK_CLIPPING) {
            UIRect r = task->clipping.area;
            r.y = (r.y + r.h) - frame_buffer->frame_size.h;
            glScissor(r.x, r.y, r.w, r.h);
        }
    }

    glDisable(GL_SCISSOR_TEST);
}


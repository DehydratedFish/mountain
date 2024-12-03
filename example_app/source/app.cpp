#include "platform.h"

#include "opengl.h"

#include "font.h"


u32 const UpdateFlags = 
   PLATFORM_UPDATE_TIME |
   PLATFORM_UPDATE_WINDOW_SIZE |
   PLATFORM_UPDATE_MOUSE_INPUT;

s32 application_main(Array<String> args) {
    (void)args;

    Font font = {};
    if (!init(&font, t_format("fonts/%s", "LiberationSans-Regular.ttf"), 20.0f, 2048)) {
        return -1;
    }
    DEFER(destroy(&font));

    GlyphInfo glyph = get_glyph(&font, 'a');

    PlatformWindow *window = platform_create_window();
    DEFER(platform_delete_window(window));
    platform_change_title(window, "The Answer is 42");

    PlatformOpenGLContext *context = platform_opengl_create_context(window);
    DEFER(platform_opengl_delete_context(context));

    if (!load_opengl_functions(context)) {
        return -1;
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    while (platform_is_running(window)) {
        platform_update();

        PlatformUpdateInfo info = {};
        platform_window_updates(window, &info, UpdateFlags);

        glClear(GL_COLOR_BUFFER_BIT);

        platform_swap_buffers(window);
    }

    return 0;
}


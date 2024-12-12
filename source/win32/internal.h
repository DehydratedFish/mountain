#pragma once

#include "win33.h"
#include "io.h"


struct PlatformWindow {
    HWND handle;
    HDC  dc;

    b32 running;

    b32 size_changed;
    V2i size;

    UserMouseInput mouse;
    UserMouseInput last_mouse;
    V2i mouse_relative;

    KeyboardActions key_actions;
};

#ifndef PLATFORM_NO_OPENGL

struct PlatformOpenGLContext {
    HMODULE gl_lib;
    HGLRC handle;
};

#endif // #ifndef PLATFORM_NO_OPENGL


#ifdef PLATFORM_OPENGL_INTEGRATION

#include "platform.h"
#include "internal.h"
#include "opengl.h"
#include "string2.h"


#define WGL_DRAW_TO_WINDOW_ARB  0x2000
#define WGL_SUPPORT_OPENGL_ARB  0x2010
#define WGL_DOUBLE_BUFFER_ARB   0x2011
#define WGL_PIXEL_TYPE_ARB      0x2013
#define WGL_COLOR_BITS_ARB      0x2014
#define WGL_DEPTH_BITS_ARB      0x2022
#define WGL_TYPE_RGBA_ARB       0x202B

#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_FLAGS_ARB            0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB        0x0001

typedef void (__stdcall *GLDEBUGPROCARB)(GLenum, GLenum, GLuint, GLenum, GLsizei, char const*, void const*);
#define GL_DEBUG_OUTPUT             0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DONT_CARE                0x1100


INTERNAL void *load_gl_function(HMODULE lib, char const *name) {
    // NOTE: This is a known issue with wglGetProcAddress. It can return 5 different error codes.
    //       In the past it could not load fixed pipeline functions from the driver and needed
    //       the regular GetProcAddress. Maybe was fixed but I doubt it.
    void *result = (void*)wglGetProcAddress(name);
    if (result == (void*)0 ||
        result == (void*)1 ||
        result == (void*)2 ||
        result == (void*)3 ||
        result == (void*)-1)
    {
        result = (void*)GetProcAddress(lib, name);
    }

    return result;
}

#define LOAD(name) name = (decltype(name))load_gl_function(gl_lib, #name)

INTERNAL void __stdcall debug_output(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, char const *msg, void const *user) {
    (void)source;
    (void)type;
    (void)severity;
    (void)length;
    (void)user;

    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    print("------------------------------\n");
    print("Debug message (%u): %s\n", id, msg);
}

PlatformOpenGLContext *platform_opengl_create_context (PlatformWindow *window) {
    PlatformOpenGLContext *result = 0;

    HMODULE gl_lib = LoadLibraryW(L"opengl32.dll");
    if (!gl_lib) {
        log_error("Could not load opengl32.dll.");
        return result;
    }

    b32   (__stdcall *wglChoosePixelFormatARB)   (HDC, int const *, r32 const*, u32, int*, u32*);
    HGLRC (__stdcall *wglCreateContextAttribsARB)(HDC, HGLRC, int const*);

    HGLRC context = {};

    { // NOTE: Dummy context creation. Under Windows you can't load proper GL functions without a context.
        u32 format = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

        PIXELFORMATDESCRIPTOR desc = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1, format,
            PFD_TYPE_RGBA,
            32,
            0, 0, 0, 0, 0, 0,
            0, 0, 0,
            0, 0, 0, 0,
            24,
            8,
            0,
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        int pixel_format = ChoosePixelFormat(window->dc, &desc);
        if (pixel_format == 0) {
            log_error("Could not find suitable win32 pixel format.");
            FreeLibrary(gl_lib);
            return result;
        }

        if (!SetPixelFormat(window->dc, pixel_format, &desc)) {
            log_error("Could not set win32 pixel format.");
            FreeLibrary(gl_lib);
            return result;
        }

        HGLRC context = wglCreateContext(window->dc);
        if (!context) {
            log_error("Could not create win32 OpenGL context.");
            FreeLibrary(gl_lib);
            return result;
        }

        if (!wglMakeCurrent(window->dc, context)) {
            log_error("Could not set win32 OpenGL context current.");
            FreeLibrary(gl_lib);
            return result;
        }

        LOAD(wglChoosePixelFormatARB);
        LOAD(wglCreateContextAttribsARB);

        wglMakeCurrent(window->dc, 0);
        wglDeleteContext(context);
    }

    if (wglChoosePixelFormatARB == 0) {
        log_error("Could not load OpenGL function wglChoosePixelFormatARB.");
        FreeLibrary(gl_lib);
        return result;
    }
    if (wglCreateContextAttribsARB == 0) {
        log_error("Could not load OpenGL function wglCreateContextAttribsARB.");
        FreeLibrary(gl_lib);
        return result;
    }

    int pixel_format;
    u32 format_count;

    int pixel_attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 32,
        0
    };
    if (!wglChoosePixelFormatARB(window->dc, pixel_attributes, 0, 1, &pixel_format, &format_count) || format_count == 0) {
        log_error("Could not choose win32 pixel format.");
        FreeLibrary(gl_lib);
        return result;
    }

    int create_args[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef DEVELOPER
        WGL_CONTEXT_FLAGS_ARB,	       WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };
    context = wglCreateContextAttribsARB(window->dc, 0, create_args);
    if (!context) {
        log_error("Could not create win32 Opengl context.");
        FreeLibrary(gl_lib);
        return result;
    }

    wglMakeCurrent(window->dc, context);

#ifdef DEVELOPER
    {
        void (__stdcall *glEnable)(GLenum);
        void (__stdcall *glDebugMessageCallbackARB)(GLDEBUGPROCARB, void const*);
        void (__stdcall *glDebugMessageControlARB)(GLenum, GLenum, GLenum, GLsizei, GLuint const*, GLboolean);

        LOAD(glEnable);
        LOAD(glDebugMessageCallbackARB);
        LOAD(glDebugMessageControlARB);

        if (glEnable && glDebugMessageCallbackARB && glDebugMessageControlARB) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallbackARB(debug_output, 0);
            glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        } else {
            log_error("Could not initialize OpenGL debug functionality.");
        }
    }
#endif

    result = ALLOC(DefaultAllocator, PlatformOpenGLContext, 1);
    result->gl_lib = gl_lib;
    result->handle = context;

    return result;
}

void platform_opengl_delete_context(PlatformOpenGLContext *context) {
    FreeLibrary(context->gl_lib);
    wglDeleteContext(context->handle);

    DEALLOC(DefaultAllocator, context, 1);
}


INTERNAL void *public_function_loader(PlatformOpenGLContext *context, String name) {
    SCOPE_TEMP_STORAGE();
    return load_gl_function(context->gl_lib, c_string_copy(name, TempAllocator));
}

PlatformGLLoaderFunc *platform_gl_loader() {
    return public_function_loader;
}


// NOTE: This header is user specified. I would like it to be able for
//       applications to only load the OpenGL pointers they need. As this reduces
//       startup time a lot. Often you only need maybe 30 or so functions and loading
//       the entire OpenGL Spec for 4.6 with all extensions is just taking quite long.
// TODO: Better naming and option to specify the location of the header or if none
//       is specified use a default one.
#include "opengl_functions.h"

// NOTE: Declare the function pointer variables.
#define LOAD_OPENGL_FUNC(name) name##_proc *name;
OPENGL_FUNCTIONS_TO_LOAD
#undef LOAD_OPENGL_FUNC

// NOTE: Load the specified functions.
#define LOAD_OPENGL_FUNC(name) name = (decltype(name))loader(context, #name); if (name == 0) return { true, #name };
MessageResult platform_opengl_load_functions(PlatformOpenGLContext *context) {
    auto loader = platform_gl_loader();

    OPENGL_FUNCTIONS_TO_LOAD

    return {
        false,
        {}
    };
}
#undef LOAD_OPENGL_FUNC

#endif // #ifdef PLATFORM_OPENGL_INTEGRATION


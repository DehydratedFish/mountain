#pragma once

#include "definitions.h"
#include "memory_buffer.h"
#include "io.h"


struct PlatformFileOptions {
    u8 read:  1;
    u8 write: 1;
    u8 file_must_exist: 1;
    u8 truncate_file: 1;
};
struct PlatformFile {
    void *handle;
    MemoryBuffer<u8> read_buffer;
    MemoryBuffer<u8> write_buffer;

    b32 open;
};

PlatformFileOptions const PlatformFileRead = {
    true,
    false,
    true,
    false,
};

PlatformFileOptions const PlatformFileUpdate = {
    false,
    true,
    false,
    false,
};

PlatformFileOptions const PlatformFileOverride = {
    false,
    true,
    false,
    true,
};

PlatformFile platform_file_open(String filename, PlatformFileOptions options = PlatformFileRead);
void   platform_file_close(PlatformFile *file);
s64    platform_file_size(PlatformFile *file);
b32    platform_file_exists(String file);
String platform_read(PlatformFile *file, u64 offset, void *buffer, s64 size);
s64    platform_write(PlatformFile *file, void const *buffer, s64 size);
s64    platform_write(PlatformFile *file, u64 offset, void const *buffer, s64 size);
b32    platform_flush_write_buffer(PlatformFile *file);

// NOTE: Does also delete folders.
b32  platform_delete_file(String path);
void platform_delete_folder_content(String path);
b32  platform_create_folder(String name);
b32  platform_create_all_folders(String names);
b32  platform_rename_file(String from, String to);

String platform_line_ending();


struct PlatformTerminal {
    PlatformFile *out;
};

extern PlatformTerminal Console;


enum {
    PLATFORM_READ_OK,
    PLATFORM_READ_ERROR = 0x01,
    PLATFORM_FILE_NOT_FOUND = 0x02,
};
struct PlatformReadResult {
    String content;
    s32 error;
};
PlatformReadResult platform_read_entire_file(String file, Allocator alloc = DefaultAllocator);


s64 platform_timestamp();


struct PlatformDynamicLibrary;
PlatformDynamicLibrary *platform_open_dynamic_library (String name);
void                    platform_close_dynamic_library(PlatformDynamicLibrary *lib);
void *platform_load_function(PlatformDynamicLibrary *lib);


// TODO: Make it possible to specify if the update should wait for messages
//       and/or signals.
void platform_update();

struct PlatformWindow;
PlatformWindow *platform_create_window();
void            platform_delete_window(PlatformWindow *window);

b32 platform_is_running(PlatformWindow *window);
void platform_change_title(PlatformWindow *window, String title);
void platform_swap_buffers(PlatformWindow *window);

Dimension platform_size(PlatformWindow *window);


enum PlatformUpdateFlags {
    PLATFORM_UPDATE_WINDOW_SIZE    = 0x01,
    PLATFORM_UPDATE_TIME           = 0x02,
    PLATFORM_UPDATE_KEYBOARD_INPUT = 0x04,
    PLATFORM_UPDATE_MOUSE_INPUT    = 0x08,
};
struct PlatformUpdateInfo {
    b32 window_size_changed;
    Dimension window_size;

    UserInput input;

    r32 dt_in_seconds;
};
void platform_window_updates(PlatformWindow *window, PlatformUpdateInfo *info, u32 flags);

struct PlatformExecutionContext {
    b32    error;
    String output;
    s32    exit_code;
};

PlatformExecutionContext platform_execute(String command);

String platform_current_folder(Allocator alloc = TempAllocator);
String platform_config_folder(Allocator alloc = TempAllocator);
String platform_home_folder(Allocator alloc = TempAllocator);

s32 application_main(Array<String> args);



#ifdef PLATFORM_OPENGL_INTEGRATION

struct PlatformOpenGLContext;
PlatformOpenGLContext *platform_opengl_create_context (PlatformWindow *window);
void                   platform_opengl_delete_context(PlatformOpenGLContext *context);

// TODO: Should the name be just a c-string?
//       As a regular String it needs to be copied first because of the missing 0 terminator.
typedef void*(PlatformGLLoaderFunc)(PlatformOpenGLContext *context, String name);
PlatformGLLoaderFunc *platform_gl_loader();

#endif // #ifdef PLATFORM_OPENGL_INTEGRATION


#include "platform.h"
#include "internal.h"

#include "definitions.h"
#include "arena.h"
#include "list.h"
#include "string2.h"
#include "utf.h"

#include <cwchar>


#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "opengl32.lib")


INTERNAL void *ProcessHandle;

PlatformTerminal Console;

#ifndef PLATFORM_CONSOLE_BUFFER_SIZE
#define PLATFORM_CONSOLE_BUFFER_SIZE 4096
#endif

#include "allocators.cpp"


// TODO: On thread creation this has to be allocated.
thread_local MemoryArena TempStorage;
thread_local Allocator TempAllocator;
Allocator DefaultAllocator;


s64 temp_storage_mark() {
    return TempStorage.used;
}

void temp_storage_rewind(s64 mark) {
    TempStorage.used = mark;
}

void reset_temp_storage() {
    TempStorage.used = 0;
}


INTERNAL b32 setup_raw_input() {
    // NOTE: If support for generic HID's is needed the code processing the WM_INPUT message needs
    //       to be updated as well.
    RAWINPUTDEVICE device = {};
    device.usage_page = HID_USAGE_PAGE_GENERIC;
    device.usage      = HID_USAGE_GENERIC_MOUSE;

    if (!RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE))) {
        log_error("Could not register raw input devices.\n");
        return false;
    }

    return true;
}

INTERNAL PlatformFile StandardOutHandle;
INTERNAL b32 setup_terminal() {
    StandardOutHandle.handle = GetStdHandle(STD_OUTPUT_HANDLE);
    init_memory_buffer(&StandardOutHandle.write_buffer, PLATFORM_CONSOLE_BUFFER_SIZE);
    StandardOutHandle.open = true;

    Console.out = &StandardOutHandle;

    return true;
}
INTERNAL List<String> process_command_line() {
    wchar_t *cmd_line = GetCommandLineW();

    int cmd_arg_count;
    wchar_t **cmd_args = CommandLineToArgvW(cmd_line, &cmd_arg_count);
    DEFER(LocalFree(cmd_args));

    List<String> args = {};
    prealloc(&args, cmd_arg_count);

    for (int i = 0; i < cmd_arg_count; i += 1) {
        String16 arg = {(u16*)cmd_args[i], (s64)wcslen(cmd_args[i])};
        args[i] = to_utf8(DefaultAllocator, arg);
    }

    return args;
}


INTERNAL s64 QPCFrequency;
INTERNAL int main_main() {
    // IMPORTANT: Set the allocators as soon as possible.
    DefaultAllocator = CStdAllocator;
    init_arena(&TempStorage, KILOBYTES(32));
    TempAllocator = make_arena_allocator(&TempStorage);

    ProcessHandle = GetCurrentProcess();
    QueryPerformanceFrequency(&QPCFrequency);

    if (!setup_raw_input()) return -1;
    if (!setup_terminal())  return -1;
    change_log_file(Console.out);

    List<String> args = process_command_line();

    // NOTE: The entry point for the user application.
    s32 status = application_main(args);

    FOR (args, arg) {
        destroy(arg);
    }
    destroy(&args);

    platform_flush_write_buffer(Console.out);
    destroy(&Console.out->write_buffer);

    destroy(&TempStorage);

    return status;
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, char*, int) {
    return main_main();
}

int main() {
    return main_main();
}


struct WideString {
    wchar_t *data;
    s64      size;
};

INTERNAL WideString widen(String str, Allocator alloc = TempAllocator) {
    String16 result = to_utf16(alloc, str, true);

    return {(wchar_t*)result.data, result.size};
}


INTERNAL void convert_backslash_to_slash(wchar_t *buffer, s64 size) {
    for (s64 i = 0; i < size; i += 1) {
        if (buffer[i] == L'\\') buffer[i] = L'/';
    }
}
INTERNAL void convert_slash_to_backslash(wchar_t *buffer, s64 size) {
    for (s64 i = 0; i < size; i += 1) {
        if (buffer[i] == L'/') buffer[i] = L'\\';
    }
}
INTERNAL void convert_slash_to_backslash(WideString str) {
    return convert_slash_to_backslash(str.data, str.size);
}

INTERNAL RECT absolute_client_rect(HWND window) {
    RECT rect = {};
    GetClientRect(window, &rect);

    POINT points[2] = {};
    points[0].x = rect.left;
    points[0].y = rect.top;

    points[1].x = rect.right;
    points[1].y = rect.bottom;

    MapWindowPoints(window, 0, points, 2);

    rect.left   = points[0].x;
    rect.top    = points[0].y;
    rect.right  = points[1].x;
    rect.bottom = points[1].y;

    return rect;
}


PlatformFile platform_file_open(String filename, PlatformFileOptions options) {
    u32 mode = 0;
    if (options.read)  mode |= GENERIC_READ;
    if (options.write) mode |= GENERIC_WRITE;
    
    u32 open_mode = OPEN_ALWAYS;
    if (options.file_must_exist) open_mode = OPEN_EXISTING; 

    PlatformFile result = {};

    WideString wide_filename = widen(filename);
    convert_slash_to_backslash(wide_filename);

    void *handle = CreateFileW(wide_filename.data, mode, 0, 0, open_mode, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        return result;
    }

    result.handle = handle;
    result.open   = true;

    return result;
}

void platform_file_close(PlatformFile *file) {
    if (file->handle != INVALID_HANDLE_VALUE && file->handle != 0) {
        CloseHandle(file->handle);
    }

    INIT_STRUCT(file);
}

s64 platform_file_size(PlatformFile *file) {
    s64 size;
    GetFileSizeEx(file->handle, &size);

    return size;
}

String platform_read(PlatformFile *file, u64 offset, void *buffer, s64 size) {
    // TODO: Split reads if they are bigger than 32bit.
    assert(size <= INT_MAX);

    if (!file->open) return {};

    OVERLAPPED ov = {};
    ov.offset      = offset & 0xFFFFFFFF;
    ov.offset_high = (offset >> 32) & 0xFFFFFFFF;

    u32 bytes_read = 0;
    if (!ReadFile(file->handle, buffer, (u32)size, &bytes_read, &ov)) {
        print("Read error: %d\n", GetLastError());
    }

    return {(u8*)buffer, bytes_read};
}

s64 platform_write(PlatformFile *file, u64 offset, void const *buffer, s64 size) {
    // TODO: Split writes if they are bigger than 32bit.
    assert(size <= INT_MAX);

    if (!file->open) return 0;

    OVERLAPPED ov = {};
    ov.offset      = offset & 0xFFFFFFFF;
    ov.offset_high = (offset >> 32) & 0xFFFFFFFF;

    u32 bytes_written = 0;
    WriteFile(file->handle, buffer, (u32)size, &bytes_written, &ov);

    return bytes_written;
}

s64 platform_write(PlatformFile *file, void const *buffer, s64 size) {
    return platform_write(file, ULLONG_MAX, buffer, size);
}

b32 platform_flush_write_buffer(PlatformFile *file) {
    if (platform_write(file, file->write_buffer.data, file->write_buffer.size) != file->write_buffer.size) return false;
    file->write_buffer.size = 0;

    return true;
}

PlatformReadResult platform_read_entire_file(String file, Allocator alloc) {
    PlatformReadResult result = {};

    WideString wide_file = widen(file);
    void *handle = CreateFileW(wide_file.data, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        result.error = PLATFORM_READ_ERROR;

        return result;
    }
    DEFER(CloseHandle(handle));

    s64 size;
    GetFileSizeEx(handle, &size);

    String content = {};

    if (size) {
        content = allocate_string(size, alloc);

        s64 total = 0;
        while (total != content.size) {
            u32 bytes_read = 0;

            if (!ReadFile(handle, content.data + total, (u32)size, &bytes_read, 0)) {
                result.error = PLATFORM_READ_ERROR;

                destroy(&content, alloc);
                return result;
            }

            total += bytes_read;
            size -= bytes_read;
        }
    }
    result.content = content;

    return result;
}


b32 platform_file_exists(String file) {
    WideString wide_file = widen(file);

    return PathFileExistsW(wide_file.data);
}


s64 platform_timestamp() {
    s64 counter;
    QueryPerformanceCounter(&counter);

    return counter;
}


u32 const STACK_TRACE_SIZE = 64;
u32 const SYMBOL_NAME_LENGTH = 1024;

INTERNAL void print_stack_trace() {
    u8 InfoStorage[sizeof(SYMBOL_INFO) + SYMBOL_NAME_LENGTH];

    void *stack[STACK_TRACE_SIZE];
    HPROCESS process = GetCurrentProcess();

    if (!SymInitialize(process, 0, true)) {
        print("Could not retrieve stack trace.");
        return;
    }

    s32 const skipped_frames = 2;
    u16 frames = RtlCaptureStackBackTrace(skipped_frames, STACK_TRACE_SIZE, stack, 0);
    SYMBOL_INFO *info = (SYMBOL_INFO *)InfoStorage;
    info->size_of_struct = sizeof(SYMBOL_INFO);
    info->max_name_len   = SYMBOL_NAME_LENGTH - 1;

    u32 displacement;
    IMAGEHLP_LINE64 line = {};
    line.size_of_struct = sizeof(line);

    for (int i = 0; i < frames - 8; i += 1) {
        if (SymFromAddr(process, (u64)stack[i], 0, info)) {
            SymGetLineFromAddr64(process, (u64)stack[i], &displacement, &line);
            print("%s(%d): %s\n", line.file_name, line.line_number, info->name);
        } else {
            print("0x000000000000: ???");
        }
    }

    SymCleanup(process);
}

void die(char const *msg) {
    print("Fatal Error: %s\n\n", msg);
    print_stack_trace();

    DebugBreak();
    ExitProcess(-1);
}

void fire_assert(char const *msg, char const *func, char const *file, int line) {
    print("Assertion failed: %s\n", msg);
    print("\t%s\n\t%s:%d\n\n", file, func, line);

    print_stack_trace();

    DebugBreak();
    ExitProcess(-1);
}

struct PlatformDynamicLibrary {
    HMODULE handle;
};

PlatformDynamicLibrary *platform_open_dynamic_library (String name) {
    PlatformDynamicLibrary *result = 0;

    WideString wide_name = widen(name);
    HMODULE handle = LoadLibraryW(wide_name.data);
    if (handle) {
        result = ALLOC(DefaultAllocator, PlatformDynamicLibrary, 1);
    }

    return result;
}

void platform_close_dynamic_library(PlatformDynamicLibrary *lib) {
    assert(lib);

    // TODO: Make sure if the handle is 0 or INVALID_HANDLE_VALUE it still works.
    FreeLibrary(lib->handle);
}

void *platform_load_function(PlatformDynamicLibrary *lib, String name) {
    assert(lib);

    return (void*)GetProcAddress(lib->handle, c_string_copy(name, TempAllocator));
}


INTERNAL s64 LastTime;
INTERNAL s64 TimeSinceLastUpdate;
void platform_update() {
    MSG msg;
    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    s64 time = platform_timestamp();
    TimeSinceLastUpdate = time - LastTime;
    LastTime = time;
}

INTERNAL sPtr CALLBACK main_window_callback(HWND window, u32 msg, uPtr w_param, sPtr l_param);

INTERNAL b32 WindowClassRegistered;
PlatformWindow *platform_create_window() {
    wchar_t const *class_name = L"GruftiMainWindowClass";

    PlatformWindow *result = 0;

    if (!WindowClassRegistered) {
        WNDCLASSW wnd_class = {
            CS_OWNDC,
            main_window_callback,
            0,
            sizeof(PlatformWindow),
            GetModuleHandleW(0),
            0,
            LoadCursorW(0, IDC_ARROW),
            0, 0,
            class_name
        };

        if (!RegisterClassW(&wnd_class)) {
            log_error("Could not create win32 window class.");
            return result;
        }
    }

    result = ALLOC(DefaultAllocator, PlatformWindow, 1);
    result->handle = CreateWindowExW(0, class_name, L"",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     0, 0, GetModuleHandleW(0), result);

    if (!result->handle) {
        log_error("Could not create win32 window.");
        DEALLOC(DefaultAllocator, result, 1);
        return result;
    }

    result->dc = GetDC(result->handle);
    result->running = true;

    return result;
}

// TODO: Take window handle by double pointer?
//       Dangling reference could be dangerous.
void platform_delete_window(PlatformWindow *window) {
    assert(window);

    if (window->handle != 0) DestroyWindow(window->handle);
    window->dc = 0;

    DEALLOC(DefaultAllocator, window, 1);
}

b32 platform_is_running(PlatformWindow *window) {
    return window->running;
}


// TODO: The last flags passed to the update function could be stored into
//       the window handle. That means on receiving Windows messages they
//       can be filtered based on the stored flags.
//       But that would only help much if there exist a lot of windows
//       simultaniously. Maybe it is worth the hastle though?
void platform_window_updates(PlatformWindow *window, PlatformUpdateInfo *info, u32 flags) {
    if (!window->running) return;

    if (flags & PLATFORM_UPDATE_TIME) {
        // TODO: Is it better to make the dt per window instead of the general update?
        info->dt_in_seconds = TimeSinceLastUpdate * (1.0f / QPCFrequency);
    }
    if (flags & PLATFORM_UPDATE_WINDOW_SIZE) {
        info->window_size_changed = window->size_changed;
        info->window_size         = window->size;
    }
    if (flags & PLATFORM_UPDATE_MOUSE_INPUT) {
        info->input.mouse      = window->mouse;
        info->input.last_mouse = window->last_mouse;
        info->input.cursor_relative = window->mouse_relative;

        window->last_mouse     = window->mouse;
        window->mouse.scroll   = 0;
        window->mouse_relative = {};
    }
    /*
    if (flags & PLATFORM_UPDATE_KEYBOARD_INPUT) {
        copy_memory(info->input.key_changes, InternalUserInput.key_changes, sizeof(info->input.key_changes));
        info->input.key_changes_used = InternalUserInput.key_changes_used;
        info->input.keyboard    = InternalUserInput.keyboard;
    }
    InternalUserInput.key_changes_used = 0;
    */

    window->size_changed = false;
}

void platform_change_title(PlatformWindow *window, String title) {
    WideString wide_title = widen(title);
    SetWindowTextW(window->handle, wide_title.data);
}

void platform_swap_buffers(PlatformWindow *window) {
    SwapBuffers(window->dc);
}


INTERNAL u32 process_scan_code(sPtr l_param) {
    u32 scan_code = GET_SCAN_CODE(l_param);
    u32 extended  = GET_EXTENDED_CODE(l_param);

    if (extended) {
        if (scan_code != 0x45) {
            scan_code |= 0xE000;
        }
    } else {
        if (scan_code == 0x45) {
            scan_code = 0xE11D45;
        } else if (scan_code == 0x54) {
            scan_code = 0xE037;
        }
    }

    return scan_code;
}

INTERNAL KeyboardKeyCode ScanCodeMappingTable[] = {
    KEY_CODE_ESCAPE,
    KEY_CODE_1,
    KEY_CODE_2,
    KEY_CODE_3,
    KEY_CODE_4,
    KEY_CODE_5,
    KEY_CODE_6,
    KEY_CODE_7,
    KEY_CODE_8,
    KEY_CODE_9,
    KEY_CODE_0,
    KEY_CODE_MINUS,
    KEY_CODE_EQUAL,
    KEY_CODE_BACKSPACE,
    KEY_CODE_TAB,
    KEY_CODE_Q,
    KEY_CODE_W,
    KEY_CODE_E,
    KEY_CODE_R,
    KEY_CODE_T,
    KEY_CODE_Y,
    KEY_CODE_U,
    KEY_CODE_I,
    KEY_CODE_O,
    KEY_CODE_P,
    KEY_CODE_LEFT_BRACKET,
    KEY_CODE_RIGHT_BRACKET,
    KEY_CODE_RETURN,
    KEY_CODE_LEFT_CONTROL,
    KEY_CODE_A,
    KEY_CODE_S,
    KEY_CODE_D,
    KEY_CODE_F,
    KEY_CODE_G,
    KEY_CODE_H,
    KEY_CODE_J,
    KEY_CODE_K,
    KEY_CODE_L,
    KEY_CODE_SEMICOLON,
    KEY_CODE_QUOTE,
    KEY_CODE_GRAVE,
    KEY_CODE_LEFT_SHIFT,
    KEY_CODE_BACKSLASH,
    KEY_CODE_Z,
    KEY_CODE_X,
    KEY_CODE_C,
    KEY_CODE_V,
    KEY_CODE_B,
    KEY_CODE_N,
    KEY_CODE_M,
    KEY_CODE_COMMA,
    KEY_CODE_DOT,
    KEY_CODE_SLASH,
    KEY_CODE_RIGHT_SHIFT,
    KEY_CODE_NONE, // NUMPAD MUL
    KEY_CODE_LEFT_ALT,
    KEY_CODE_SPACE,
    KEY_CODE_CAPSLOCK,
    KEY_CODE_F1,
    KEY_CODE_F2,
    KEY_CODE_F3,
    KEY_CODE_F4,
    KEY_CODE_F5,
    KEY_CODE_F6,
    KEY_CODE_F7,
    KEY_CODE_F8,
    KEY_CODE_F9,
    KEY_CODE_F10,
    KEY_CODE_NONE, // NUMLOCK
    KEY_CODE_NONE, // SCROLL LOCK
    KEY_CODE_NONE, // NUM7
    KEY_CODE_NONE, // NUM8
    KEY_CODE_NONE, // NUM9
    KEY_CODE_NONE, // NUM SUB
    KEY_CODE_NONE, // NUM4
    KEY_CODE_NONE, // NUM5
    KEY_CODE_NONE, // NUM6
    KEY_CODE_NONE, // NUM ADD
    KEY_CODE_NONE, // NUM1
    KEY_CODE_NONE, // NUM2
    KEY_CODE_NONE, // NUM3
    KEY_CODE_NONE, // NUM0
    KEY_CODE_NONE, // NUM DOT
    KEY_CODE_NONE, // ALT PRINTSCREEN
    KEY_CODE_NONE, // QWERTZ < >
    KEY_CODE_F11,
    KEY_CODE_F12,
};
INTERNAL KeyboardKeyCode scan_code_to_key_code(u32 scan_code) {
    KeyboardKeyCode result = KEY_CODE_NONE;

    // TODO: Complete the lookup table.
    if (scan_code < 0x59) {
        result = ScanCodeMappingTable[scan_code];
    } else {
        switch (scan_code) {
        case 0xE038: result = KEY_CODE_RIGHT_ALT; break;
        case 0xE047: result = KEY_CODE_HOME; break;
        case 0xE048: result = KEY_CODE_ARROW_UP; break;
        case 0xE049: result = KEY_CODE_PAGE_UP; break;
        case 0xE04B: result = KEY_CODE_ARROW_LEFT; break;
        case 0xE04D: result = KEY_CODE_ARROW_RIGHT; break;
        case 0xE04F: result = KEY_CODE_END; break;
        case 0xE050: result = KEY_CODE_ARROW_DOWN; break;
        case 0xE051: result = KEY_CODE_PAGE_DOWN; break;
        case 0xE052: result = KEY_CODE_INSERT; break;
        case 0xE053: result = KEY_CODE_DELETE; break;
        case 0xE05B: result = KEY_CODE_LEFT_META; break;
        case 0xE05C: result = KEY_CODE_RIGHT_META; break;
        }
    }
    
    return result;
}

INTERNAL PlatformWindow *get_platform_window(HWND window) {
    return (PlatformWindow*)GetWindowLongPtrW(window, 0);
}

INTERNAL sPtr CALLBACK main_window_callback(HWND hwnd, u32 msg, uPtr w_param, sPtr l_param) {
    switch (msg) {
    case WM_CLOSE:
    case WM_DESTROY: {
        PostQuitMessage(0);
        PlatformWindow *window = get_platform_window(hwnd);
        window->running = false;
    } break;


    case WM_CREATE: {
        CREATESTRUCTW *cs = (CREATESTRUCTW*)l_param;
        SetWindowLongPtrW(hwnd, 0, (sPtr)cs->create_params);
    } break;

    case WM_MOUSEMOVE: {
        s32 x = GET_X_LPARAM(l_param);
        s32 y = GET_Y_LPARAM(l_param);

        PlatformWindow *window = get_platform_window(hwnd);
        window->mouse.cursor_pos.x = x;
        window->mouse.cursor_pos.y = y;
    } break;

    case WM_LBUTTONDOWN: {
        PlatformWindow *window = get_platform_window(hwnd);
        window->mouse.keys[MOUSE_LMB] = true;
    } break;

    case WM_LBUTTONUP: {
        PlatformWindow *window = get_platform_window(hwnd);
        window->mouse.keys[MOUSE_LMB] = false;
    } break;

    case WM_MOUSEWHEEL: {
        PlatformWindow *window = get_platform_window(hwnd);
        window->mouse.scroll += GET_WHEEL_DELTA_WPARAM(w_param) / WHEEL_DELTA;
    } break;

        /*
    case WM_KEYUP: {
        u32 scan_code = process_scan_code(l_param);

        if (InternalUserInput.keyboard.keys[scan_code]) {
            KeyAction action = {};
            action.kind = KEY_ACTION_RELEASE;
            action.scan_code = (ScanCode)scan_code;
            append(InternalUserInput.key_changes, action);
        }
    } break;

    case WM_KEYDOWN: {
        u32 scan_code = process_scan_code(l_param);
        KeyboardKeyCode code = scan_code_to_key_code(scan_code);

        // TODO: This could be slow to query on every keystroke.
        u8 kb[256];
        GetKeyboardState(kb);
        
        wchar_t utf16[2];
        s32 length = ToUnicode((u32)w_param, 0, kb, utf16, 2, 0);

        u32 code_point = 0;
        if (length > 0) {
            u32 cp = to_utf32((u16*)utf16);
            if (cp < 0x20 || cp == 0x7F) cp = 0;

            code_point = cp;
        }

        KeyAction action = {};
        action.code_point = code_point;
        action.code       = code;

        if (InternalUserInput.keyboard.keys[code]) {
            action.kind = KEY_ACTION_REPEAT;
        } else {
            action.kind = KEY_ACTION_PRESS;
        }

        if (InternalUserInput.key_changes_used != KeyBufferSize) {
            InternalUserInput.key_changes[InternalUserInput.key_changes_used] = action;
            InternalUserInput.key_changes_used += 1;
        }
    } break;
    */

    case WM_INPUT: {
        RAWINPUT input = {};
        u32 size = sizeof(input);

        // NOTE: We don't care about generic HID's so no dynamic allocation is required.
        u32 check = GetRawInputData((HRAWINPUT)l_param, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER));
        if (check != size) break;

        // NOTE: Only using raw input for the relative mouse movement.
        //       The cursor position is read from WM_MOUSEMOVE to keep the cursor synced with the system.
        if (input.header.type == RIM_TYPEMOUSE && (input.data.mouse.flags & MOUSE_MOVE_ABSOLUT) == 0) {
            PlatformWindow *window = get_platform_window(hwnd);
            window->mouse_relative.x += input.data.mouse.last_x;
            window->mouse_relative.y += input.data.mouse.last_y;
        }
    } break;

    case WM_SIZE: {
        PlatformWindow *window = get_platform_window(hwnd);
        window->size_changed = true;
        window->size.width   = LOWORD(l_param);
        window->size.height  = HIWORD(l_param);
    } break;

    default:
        return DefWindowProcW(hwnd, msg, w_param, l_param);
    }

    return 0;
}


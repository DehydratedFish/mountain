#include "platform.h"
#include "definitions.h"
#include "list.h"


#include "allocators.cpp"
#include "memory.h"
#include "arena.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include <csignal>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "fcntl.h"
#include "pwd.h"
#include "wait.h"
#include "execinfo.h"

#include "sys/stat.h"

#include "string2.h"
#include "string_builder.h"


#define STORE_FD(fd) ((void*)(s64)fd)
#define AS_FD(ptr)   ((s32)(s64)ptr)


Allocator DefaultAllocator;
thread_local MemoryArena TempStorage;
thread_local Allocator   TempAllocator;

PlatformTerminal Console;

#ifndef PLATFORM_CONSOLE_BUFFER_SIZE
#define PLATFORM_CONSOLE_BUFFER_SIZE 4096
#endif


s64 temp_storage_mark() {
    return TempStorage.used;
}

void temp_storage_rewind(s64 mark) {
    TempStorage.used = mark;
}

void reset_temp_storage() {
    TempStorage.used = 0;
}


INTERNAL void print_stack_trace() {
    s32 const trace_size = 64;
    void *buffer[trace_size];
    s32 call_stack = backtrace(buffer, trace_size);
    char **symbols = backtrace_symbols(buffer, call_stack);

    for (s32 i = 0; i < call_stack; i += 1) {
        print("%s(%p)\n", symbols[i], buffer[i]);
    }
}

void fire_assert(char const *msg, char const *func, char const *file, int line) {
    print("Assertion failed: %s\n", msg);
    print("\t%s\n\t%s:%d\n\n", file, func, line);

    print_stack_trace();

    raise(SIGTRAP);
    exit(-1);
}

void die(char const *msg) {
    print("Fatal Error: %s\n\n", msg);
    print_stack_trace();

    // TODO: This should work like a breakpoint. Needs testing.
    __builtin_trap();
    exit(-1);
}


struct CString {
    char *data;
    s64   size;
};

// TODO: Maybe use a thread local arena allocator or something like an array pool?
INTERNAL CString alloc_c_string(String str, String optional = "") {
    CString result = {};

    s64 size = str.size + optional.size;

    result.data = ALLOC(TempAllocator, char, size + 1);
    result.size = size;

    copy_memory(result.data, str.data, str.size);
    copy_memory(result.data + str.size, optional.data, optional.size);
    result.data[result.size] = '\0';

    return result;
}

b32 platform_create_folder(String name) {
    SCOPE_TEMP_STORAGE();

    CString c_name = alloc_c_string(name);

    int status = mkdir(c_name.data, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status != 0) {
        // TODO: Log or make a proper error reporter.
    }

    return true;
}

b32 platform_create_all_folders(String names) {
    b32 result = false;

    String path = {names.data, 0};
    for (s64 pos = 0; pos < names.size; pos += 1) {
        if (names[pos] == '/' || names[pos] == '\\') {
            path.size = pos;

            // NOTE: This does not break the loop on false, because for absolute paths
            //       the first C:\ has a different error code and would not work otherwise.
            result = platform_create_folder(path);
        }
    }

    result = platform_create_folder(names);

    return result;
}

String platform_home_folder(Allocator alloc) {
    passwd *pw = getpwuid(getuid());

    s64 length = c_string_length(pw->pw_dir);
    String result = allocate_string(length, alloc);
    copy_memory(result.data, pw->pw_dir, length);

    return result;
}

String platform_current_folder(Allocator alloc) {
    // TODO: Static buffer size query possible?
    char *name = get_current_dir_name();

    s64 length = c_string_length(name);
    String result = allocate_string(length, alloc);
    copy_memory(result.data, name, length);

    free(name);

    return result;
}

PlatformReadResult platform_read_entire_file(String file, Allocator alloc) {
    SCOPE_TEMP_STORAGE();

    PlatformReadResult result = {};

    CString c_file = alloc_c_string(file);
    int fd = open(c_file.data, O_RDONLY);
    if (fd == -1) {
        // TODO: Can also have different reason for error.
        result.error = PLATFORM_FILE_NOT_FOUND;
        return result;
    }
    DEFER(close(fd));

    struct stat info = {};
    if (fstat(fd, &info)) {
        result.error = PLATFORM_READ_ERROR;
        return result;
    }

    String content = {};
    if (info.st_size) {
        content = allocate_string(info.st_size);

        s64 total = 0;
        while (total != info.st_size) {
            ssize_t bytes_read = read(fd, content.data + total, info.st_size - total);
            if (bytes_read == -1) {
                result.error = PLATFORM_READ_ERROR;
                destroy(&content);

                return result;
            }

            total += bytes_read;
        }
    }
    result.content = content;

    return result;
}

PlatformFile platform_file_open(String filename, PlatformFileOptions options) {
    SCOPE_TEMP_STORAGE();

    u32 mode = 0;
    if (options.read) {
        mode = O_RDONLY;
        if (options.write) mode = O_RDWR;
    } else if (options.write) {
        mode = O_WRONLY;
    }
    
    // TODO: I don't know if this mode stuff behaves as expected correct;
    if (options.file_must_exist && options.truncate_file) mode |= O_TRUNC;
    else if (options.truncate_file)                       mode |= O_TRUNC | O_CREAT;

    PlatformFile result = {};

    CString c_filename = alloc_c_string(filename);

    int fd = open(c_filename.data, mode);
    if (fd == -1) {
        return result;
    }

    result.handle = STORE_FD(fd);
    result.open   = true;

    return result;
}

void platform_file_close(PlatformFile *file) {
    close(AS_FD(file->handle));

    INIT_STRUCT(file);
}

s64 platform_write(PlatformFile *file, u64 offset, void const *buffer, s64 size) {
    if (!file->open) return 0;

    u8 const* data = (u8 const*)buffer;

    s64 total = 0;
    while (total < size) {
        ssize_t bytes_written = pwrite(AS_FD(file->handle), data + total, size - total, offset + total);

        // TODO: Can this become an infinite loop if pwrite returns 0's constantly?
        if (bytes_written < 0) return -1;

        total += bytes_written;
    }

    return total;
}

s64 platform_write(PlatformFile *file, void const *buffer, s64 size) {
    if (!file->open) return 0;

    u8 const* data = (u8 const*)buffer;

    s64 total = 0;
    while (total < size) {
        ssize_t bytes_written = write(AS_FD(file->handle), data + total, size - total);

        // TODO: Can this become an infinite loop if pwrite returns 0's constantly?
        if (bytes_written < 0) return -1;

        total += bytes_written;
    }

    return total;
}

b32 platform_flush_write_buffer(PlatformFile *file) {
    if (platform_write(file, file->write_buffer.data, file->write_buffer.size) != file->write_buffer.size) return false;
    file->write_buffer.size = 0;

    return true;
}

PlatformExecutionContext platform_execute(String command) {
    SCOPE_TEMP_STORAGE();

    PlatformExecutionContext context = {};

    // TODO: Is this enough or should we do a proper redirect without shell stuff?
    CString c_command = alloc_c_string(command, " 2>&1");
    FILE *fd = popen(c_command.data, "r");
    if (fd == 0) {
        context.error = true;
        return context;
    }

    StringBuilder builder = {};
    DEFER(destroy(&builder));

    s32 const buffer_size = 4096;
    u8 buffer[buffer_size];

    size_t bytes_read = 0;
    while (bytes_read) {
        bytes_read = fread(buffer, 1, buffer_size, fd);
        append(&builder, {buffer, (s64)bytes_read});
    }

    int exit_code = pclose(fd);
    context.output    = to_allocated_string(&builder);
    context.exit_code = exit_code;

    return context;
}

INTERNAL PlatformFile StandardOutHandle;
INTERNAL PlatformFile StandardInHandle;
INTERNAL b32 setup_terminal() {
    StandardOutHandle.handle = STORE_FD(STDOUT_FILENO);
    init_memory_buffer(&StandardOutHandle.write_buffer, PLATFORM_CONSOLE_BUFFER_SIZE);
    StandardOutHandle.open = true;

    Console.out = &StandardOutHandle;

    StandardInHandle.handle = STORE_FD(STDIN_FILENO);
    init_memory_buffer(&StandardInHandle.read_buffer, PLATFORM_CONSOLE_BUFFER_SIZE);
    StandardInHandle.open = true;

    Console.in = &StandardInHandle;

    return true;
}

int main(int argc, char **argv) {
    // IMPORTANT: Set the allocators as soon as possible.
    DefaultAllocator = CStdAllocator;
    init(&TempStorage, KILOBYTES(32));
    TempAllocator = make_arena_allocator(&TempStorage);

    setup_terminal();

    List<String> args = {};
    init(&args, argc);
    DEFER(destroy(&args));

    for (int i = 0; i < argc; i += 1) {
        append(&args, {(u8*)argv[i], c_string_length(argv[i])});
    }

    s32 status = application_main(args);

    return status;
}


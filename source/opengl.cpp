#define OPENGL_DEFINITIONS
#include "opengl.h"

#include "platform.h"


#define LOAD(name) name = (decltype(name))loader(context, #name); if (name == 0) { log_error("Could not load OpenGL function pointer for %s.", name); return false; };

b32 load_opengl_functions(PlatformOpenGLContext *context) {
    auto loader = platform_gl_loader();

    LOAD(glGetIntegerv);
    LOAD(glEnable);
    LOAD(glDisable);
    LOAD(glClearColor);
    LOAD(glClear);
    LOAD(glViewport);
    LOAD(glBlendFunc);
    LOAD(glGenVertexArrays);
    LOAD(glDeleteVertexArrays);
    LOAD(glGenBuffers);
    LOAD(glDeleteBuffers);
    LOAD(glBindVertexArray);
    LOAD(glBindBuffer);
    LOAD(glBindBufferBase);
    LOAD(glBufferData);
    LOAD(glBufferSubData);
    LOAD(glVertexAttribPointer);
    LOAD(glEnableVertexAttribArray);
    LOAD(glUseProgram);
    LOAD(glUniformMatrix4fv);
    LOAD(glDrawArrays);
    LOAD(glCreateShader);
    LOAD(glDeleteShader);
    LOAD(glShaderSource);
    LOAD(glCompileShader);
    LOAD(glCreateProgram);
    LOAD(glDeleteProgram);
    LOAD(glAttachShader);
    LOAD(glLinkProgram);
    LOAD(glGetUniformLocation);
    LOAD(glGetAttribLocation);
    LOAD(glGetUniformBlockIndex);
    LOAD(glUniformBlockBinding);
    LOAD(glGetShaderiv);
    LOAD(glGetShaderInfoLog);
    LOAD(glGetProgramiv);
    LOAD(glGetProgramInfoLog);
    LOAD(glGenTextures);
    LOAD(glDeleteTextures);
    LOAD(glActiveTexture);
    LOAD(glBindTexture);
    LOAD(glTexImage2D);
    LOAD(glTexImage3D);
    LOAD(glTexParameteri);
    LOAD(glDepthFunc);
    LOAD(glScissor);
    LOAD(glDepthMask);
    LOAD(glGenFramebuffers);
    LOAD(glBindFramebuffer);
    LOAD(glFramebufferTexture);
    LOAD(glDrawBuffer);
    LOAD(glDrawBuffers);

    return true;
}


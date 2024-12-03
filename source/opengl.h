//================================================
// Currently doing this stuff by hand.
// I need to think more about a feasable way to
// auto generate all the functions from a specified
// list of functions wanted. But for now all options
// I can think of are either cumbersome or need
// third party software. I want to generate the needed
// header while building without needing any other 
// application.
//================================================
#pragma once

#include "definitions.h"


struct PlatformOpenGLContext;


typedef char GLchar;
typedef u8 GLboolean;
typedef s32 GLint;
typedef u32 GLuint;
typedef float GLfloat;
typedef s32 GLsizei;
typedef u32 GLenum;
typedef u32 GLbitfield;

#if defined(OS_WINDOWS) && !defined(_WIN64)
typedef s32 GLsizeiptr;
typedef s32 GLintptr;
#else
typedef s64 GLsizeiptr;
typedef s64 GLintptr;
#endif


#if defined(OS_WINDOWS) && !defined(_WIN64)
#define OPENGL_CALL __stdcall
#else
#define OPENGL_CALL
#endif // defined(OS_WINDOWS) && !defined(_WIN64)

#ifdef OPENGL_DEFINITIONS
#define OPENGL_FUNC(ret, name, ...) ret OPENGL_CALL (*name)(__VA_ARGS__);
#else
#define OPENGL_FUNC(ret, name, ...) extern ret OPENGL_CALL (*name)(__VA_ARGS__);
#endif // OPENGL_DEFINE_FUNCTIONS


#define GL_FALSE            0
#define GL_TRUE             1
#define GL_UNSIGNED_BYTE    0x1401
#define GL_UNSIGNED_SHORT   0x1403
#define GL_UNSIGNED_INT     0x1405
#define GL_INT              0x1404
#define GL_FLOAT            0x1406

#define GL_NO_ERROR         0
#define GL_LINES            0x0001
#define GL_LINE_STRIP       0x0003
#define GL_TRIANGLES        0x0004
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_ARRAY_BUFFER     0x8892
#define GL_STREAM_DRAW      0x88E0
#define GL_STATIC_DRAW      0x88E4
#define GL_DYNAMIC_DRAW     0x88E8
#define GL_FRAGMENT_SHADER  0x8B30
#define GL_GEOMETRY_SHADER  0x8DD9
#define GL_VERTEX_SHADER    0x8B31
#define GL_COMPILE_STATUS   0x8B81
#define GL_LINK_STATUS      0x8B82
#define GL_INFO_LOG_LENGTH  0x8B84
#define GL_TEXTURE_2D       0x0DE1
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_RED              0x1903
#define GL_GREEN            0x1904
#define GL_BLUE             0x1905
#define GL_ALPHA            0x1906
#define GL_RGB              0x1907
#define GL_RGBA             0x1908
#define GL_R8               0x8229
#define GL_BLEND            0x0BE2
#define GL_SRC_ALPHA        0x0302
#define GL_ONE_MINUS_SRC_ALPHA  0x0303
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_WRAP_S       0x2802
#define GL_TEXTURE_WRAP_T       0x2803
#define GL_CLAMP_TO_EDGE        0x812F
#define GL_TEXTURE_MAG_FILTER   0x2800
#define GL_TEXTURE_MIN_FILTER   0x2801
#define GL_REPEAT               0x2901
#define GL_NEAREST          0x2600
#define GL_LINEAR           0x2601
#define GL_CULL_FACE        0x0B44
#define GL_DEPTH_TEST       0x0B71
#define GL_SCISSOR_TEST     0x0C11
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF

#define GL_LESS             0x0201
#define GL_LEQUAL           0x0203
#define GL_RGBA8            0x8058
#define GL_RG               0x8227

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2

#define GL_UNIFORM_BUFFER   0x8A11

#define GL_TEXTURE0         0x84C0
#define GL_TEXTURE1         0x84C1
#define GL_TEXTURE2         0x84C2
#define GL_TEXTURE3         0x84C3
#define GL_TEXTURE4         0x84C4
#define GL_TEXTURE5         0x84C5
#define GL_TEXTURE6         0x84C6
#define GL_TEXTURE7         0x84C7
#define GL_TEXTURE8         0x84C8
#define GL_TEXTURE9         0x84C9
#define GL_TEXTURE10        0x84CA
#define GL_TEXTURE11        0x84CB
#define GL_TEXTURE12        0x84CC
#define GL_TEXTURE13        0x84CD
#define GL_TEXTURE14        0x84CE
#define GL_TEXTURE15        0x84CF
#define GL_TEXTURE16        0x84D0
#define GL_TEXTURE17        0x84D1
#define GL_TEXTURE18        0x84D2
#define GL_TEXTURE19        0x84D3
#define GL_TEXTURE20        0x84D4
#define GL_TEXTURE21        0x84D5
#define GL_TEXTURE22        0x84D6
#define GL_TEXTURE23        0x84D7
#define GL_TEXTURE24        0x84D8
#define GL_TEXTURE25        0x84D9
#define GL_TEXTURE26        0x84DA
#define GL_TEXTURE27        0x84DB
#define GL_TEXTURE28        0x84DC
#define GL_TEXTURE29        0x84DD
#define GL_TEXTURE30        0x84DE
#define GL_TEXTURE31        0x84DF

#define GL_FRAMEBUFFER        0x8D40
#define GL_BACK               0x0405

#define GL_COLOR_ATTACHMENT0  0x8CE0
#define GL_COLOR_ATTACHMENT1  0x8CE1
#define GL_COLOR_ATTACHMENT2  0x8CE2
#define GL_COLOR_ATTACHMENT3  0x8CE3
#define GL_COLOR_ATTACHMENT4  0x8CE4
#define GL_COLOR_ATTACHMENT5  0x8CE5
#define GL_COLOR_ATTACHMENT6  0x8CE6
#define GL_COLOR_ATTACHMENT7  0x8CE7
#define GL_COLOR_ATTACHMENT8  0x8CE8
#define GL_COLOR_ATTACHMENT9  0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF
#define GL_COLOR_ATTACHMENT16 0x8CF0
#define GL_COLOR_ATTACHMENT17 0x8CF1
#define GL_COLOR_ATTACHMENT18 0x8CF2
#define GL_COLOR_ATTACHMENT19 0x8CF3
#define GL_COLOR_ATTACHMENT20 0x8CF4
#define GL_COLOR_ATTACHMENT21 0x8CF5
#define GL_COLOR_ATTACHMENT22 0x8CF6
#define GL_COLOR_ATTACHMENT23 0x8CF7
#define GL_COLOR_ATTACHMENT24 0x8CF8
#define GL_COLOR_ATTACHMENT25 0x8CF9
#define GL_COLOR_ATTACHMENT26 0x8CFA
#define GL_COLOR_ATTACHMENT27 0x8CFB
#define GL_COLOR_ATTACHMENT28 0x8CFC
#define GL_COLOR_ATTACHMENT29 0x8CFD
#define GL_COLOR_ATTACHMENT30 0x8CFE
#define GL_COLOR_ATTACHMENT31 0x8CFF

#define GL_INVALID_INDEX    0xFFFFFFFF


OPENGL_FUNC(void,   glGetIntegerv, GLenum, GLint*);
OPENGL_FUNC(void,   glEnable, GLenum);
OPENGL_FUNC(void,   glDisable, GLenum);
OPENGL_FUNC(void,   glClearColor, GLfloat, GLfloat, GLfloat, GLfloat);
OPENGL_FUNC(void,   glClear, GLbitfield);
OPENGL_FUNC(void,   glViewport, GLint, GLint, GLsizei, GLsizei);
OPENGL_FUNC(void,   glBlendFunc, GLenum, GLenum);
OPENGL_FUNC(void,   glGenVertexArrays, GLsizei, GLuint*);
OPENGL_FUNC(void,   glDeleteVertexArrays, GLsizei, GLuint const*);
OPENGL_FUNC(void,   glGenBuffers, GLsizei, GLuint*);
OPENGL_FUNC(void,   glDeleteBuffers, GLsizei, GLuint const*);
OPENGL_FUNC(void,   glBindVertexArray, GLuint);
OPENGL_FUNC(void,   glBindBuffer, GLenum, GLuint);
OPENGL_FUNC(void,   glBindBufferBase, GLenum, GLuint, GLuint);
OPENGL_FUNC(void,   glBufferData, GLenum, GLsizeiptr, void const*, GLenum);
OPENGL_FUNC(void,   glBufferSubData, GLenum, GLintptr, GLsizeiptr, void const*);
OPENGL_FUNC(void,   glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, void const*);
OPENGL_FUNC(void,   glEnableVertexAttribArray, GLuint);
OPENGL_FUNC(void,   glUseProgram, GLuint);
OPENGL_FUNC(void,   glUniformMatrix4fv, GLint, GLsizei, GLboolean, GLfloat const*);
OPENGL_FUNC(void,   glDrawArrays, GLenum, GLint, GLsizei);

OPENGL_FUNC(GLuint, glCreateShader, GLenum);
OPENGL_FUNC(void,   glDeleteShader, GLuint);
OPENGL_FUNC(void,   glShaderSource, GLuint, GLsizei, GLchar const *const*, GLint const*);
OPENGL_FUNC(void,   glCompileShader, GLuint);
OPENGL_FUNC(GLuint, glCreateProgram, GLuint);
OPENGL_FUNC(void,   glDeleteProgram, GLuint);
OPENGL_FUNC(void,   glAttachShader, GLuint, GLuint);
OPENGL_FUNC(void,   glLinkProgram, GLuint);
OPENGL_FUNC(GLint,  glGetUniformLocation, GLuint, GLchar const*);
OPENGL_FUNC(GLint,  glGetAttribLocation, GLuint, GLchar const*);
OPENGL_FUNC(GLuint, glGetUniformBlockIndex, GLuint, GLchar const*);
OPENGL_FUNC(void,   glUniformBlockBinding, GLuint, GLuint, GLuint);
OPENGL_FUNC(void,   glGetShaderiv, GLuint, GLenum, GLint*);
OPENGL_FUNC(void,   glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*);
OPENGL_FUNC(void,   glGetProgramiv, GLuint, GLenum, GLint);
OPENGL_FUNC(void,   glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*);
OPENGL_FUNC(void,   glGenTextures, GLsizei, GLuint*);
OPENGL_FUNC(void,   glDeleteTextures, GLsizei, GLuint const*);
OPENGL_FUNC(void,   glActiveTexture, GLenum);
OPENGL_FUNC(void,   glBindTexture, GLenum, GLuint);
OPENGL_FUNC(void,   glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, void const*);
OPENGL_FUNC(void,   glTexImage3D, GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, void const*);
OPENGL_FUNC(void,   glTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, void const*);
OPENGL_FUNC(void,   glTexStorage2D, GLenum, GLsizei, GLenum, GLsizei, GLsizei);
OPENGL_FUNC(void,   glTexStorage3D, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);;
OPENGL_FUNC(void,   glTexParameteri, GLenum, GLenum, GLint);
OPENGL_FUNC(void,   glGenerateMipmap, GLenum);

OPENGL_FUNC(void,   glDepthFunc, GLenum);
OPENGL_FUNC(void,   glScissor, GLint, GLint, GLsizei, GLsizei);

OPENGL_FUNC(void,   glDepthMask, GLboolean);

OPENGL_FUNC(void,   glGenFramebuffers, GLsizei n, GLuint*);
OPENGL_FUNC(void,   glBindFramebuffer, GLenum, GLuint);
OPENGL_FUNC(void,   glFramebufferTexture, GLenum, GLenum, GLuint, GLint);
OPENGL_FUNC(void,   glDrawBuffer, GLenum);
OPENGL_FUNC(void,   glDrawBuffers, GLenum n, GLenum const*);

#undef OPENGL_FUNC


b32 load_opengl_functions(PlatformOpenGLContext *context);


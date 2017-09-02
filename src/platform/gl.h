// OpenGL 

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

#define GL_TEXTURE_2D                     0x0DE1

#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

#define GL_CLAMP                          0x2900
#define GL_REPEAT                         0x2901

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_TEXTURE_BORDER                 0x1005

#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B

#define GL_COLOR_INDEX                    0x1900
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_2_BYTES                        0x1407
#define GL_3_BYTES                        0x1408
#define GL_4_BYTES                        0x1409
#define GL_DOUBLE                         0x140A

#define GL_TRUE                           1
#define GL_FALSE                          0

#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2

#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308

#define GL_DEPTH_TEST                     0x0B71

#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_LIGHTING_BIT                   0x00000040
#define GL_FOG_BIT                        0x00000080
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_ACCUM_BUFFER_BIT               0x00000200
#define GL_STENCIL_BUFFER_BIT             0x00000400

#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

DLL_IMPORT const GLubyte* glGetString(GLenum name);

DLL_IMPORT void glDeleteTextures (GLsizei n, const GLuint *textures);
DLL_IMPORT void glTexParameteri (GLenum target, GLenum pname, GLint param);
DLL_IMPORT void glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
DLL_IMPORT void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);

DLL_IMPORT void glBlendFunc (GLenum sfactor, GLenum dfactor);

DLL_IMPORT void glEnable (GLenum cap);
DLL_IMPORT void glDisable (GLenum cap);

DLL_IMPORT void glClear (GLbitfield mask);
DLL_IMPORT void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
DLL_IMPORT void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

DLL_IMPORT void glDrawArrays (GLenum mode, GLint first, GLsizei count);
DLL_IMPORT void glDrawBuffer (GLenum mode);
DLL_IMPORT void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

// glext

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#define GL_DEBUG_SOURCE_OTHER             0x824B
#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#define GL_DEBUG_TYPE_OTHER               0x8251
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B

#define GL_MIRRORED_REPEAT                0x8370
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_CLAMP_TO_BORDER                0x812D

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_STREAM_DRAW                    0x88E0
#define GL_DYNAMIC_DRAW                   0x88E8

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

typedef void (*glDebugProc_t)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (*glDebugMessageCallback_t)(glDebugProc_t callback, const void *userParam);
typedef void (*glDebugMessageInsert_t)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message);
typedef void (*glDebugMessageControl_t)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);

typedef void (*glAttachShader_t)(GLuint program, GLuint shader);
typedef void (*glCompileShader_t)(GLuint shader);
typedef GLuint (*glCreateProgram_t)(void);
typedef GLuint (*glCreateShader_t)(GLenum type);
typedef void (*glDeleteProgram_t)(GLuint program);
typedef void (*glDeleteShader_t)(GLuint shader);
typedef void (*glLinkProgram_t)(GLuint program);
typedef void (*glShaderSource_t)(GLuint shader, GLsizei count, const GLchar* const* str, const GLint* length);
typedef void (*glUseProgram_t)(GLuint program);
typedef GLint (*glGetUniformLocation_t)(GLuint program, const GLchar *name);
typedef void (*glUniformMatrix4fv_t)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

typedef void (*glGenerateMipmap_t)(GLenum target);
typedef void (*glActiveTexture_t)(GLenum texture);
typedef void (*glCreateTextures_t)(GLenum target, GLsizei n, GLuint *textures);
typedef void (*glBindTextureUnit_t)(GLuint unit, GLuint texture);

typedef void (*glBindVertexArray_t)(GLuint array);
typedef void (*glDeleteVertexArrays_t)(GLsizei n, const GLuint *arrays);
typedef void (*glGenVertexArrays_t)(GLsizei n, GLuint *arrays);

typedef void (*glBindBuffer_t)(GLenum target, GLuint buffer);
typedef void (*glDeleteBuffers_t)(GLsizei n, const GLuint *buffers);
typedef void (*glGenBuffers_t)(GLsizei n, GLuint *buffers);
typedef void (*glBufferData_t)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);

typedef void (*glVertexAttribPointer_t)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (*glEnableVertexAttribArray_t)(GLuint index);

#pragma once

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

glDebugMessageCallback_t 	glDebugMessageCallback;
glDebugMessageInsert_t		glDebugMessageInsert;
glDebugMessageControl_t		glDebugMessageControl;

glAttachShader_t  			glAttachShader;
glCompileShader_t 			glCompileShader;
glCreateProgram_t 			glCreateProgram;
glCreateShader_t  			glCreateShader;
glDeleteProgram_t 			glDeleteProgram;
glDeleteShader_t  			glDeleteShader;
glLinkProgram_t   			glLinkProgram;
glShaderSource_t  			glShaderSource;
glUseProgram_t    			glUseProgram;
glGetUniformLocation_t 		glGetUniformLocation;
glUniformMatrix4fv_t   		glUniformMatrix4fv;

glGenerateMipmap_t			glGenerateMipmap;
glActiveTexture_t			glActiveTexture;
glCreateTextures_t			glCreateTextures;
glBindTextureUnit_t			glBindTextureUnit;

glBindVertexArray_t    		glBindVertexArray; 		
glDeleteVertexArrays_t 		glDeleteVertexArrays;
glGenVertexArrays_t    		glGenVertexArrays;

glBindBuffer_t				glBindBuffer;
glDeleteBuffers_t			glDeleteBuffers;
glGenBuffers_t				glGenBuffers;
glBufferData_t				glBufferData;

glVertexAttribPointer_t		glVertexAttribPointer;
glEnableVertexAttribArray_t glEnableVertexAttribArray;

typedef u32 shader_program_id;
typedef u32 texture_id;
typedef u32 context_id; // VAO

struct shader_source {
	string path;
	platform_file_attributes last_attrib;
	string source;
	allocator* alloc = NULL;
};

struct shader_program {
	shader_program_id id = 0;
	GLuint handle = 0;
	shader_source vertex;
	shader_source fragment;
	void (*set_uniforms)(shader_program*, render_command*, render_command_list*) = NULL;
	// tessellation control, evaluation, geometry
};

enum texture_wrap {
	wrap_repeat,
	wrap_mirror,
	wrap_clamp,
	wrap_clamp_border,
};

struct texture {
	texture_id id 		= 0;
	GLuint handle 		= 0;
	texture_wrap wrap 	= wrap_repeat;
	bool pixelated 		= false;;
};

struct ogl_draw_context {
	context_id id  = 0;
	GLuint vao     = 0;
	GLuint vbos[8] = {};
};

struct ogl_manager {
	map<shader_program_id, shader_program> programs;
	map<texture_id, texture> 			   textures;
	map<context_id, ogl_draw_context> 	   contexts;
	
	shader_program_id 	dbg_shader = 0;

	shader_program_id 	next_shader_id = 1;
	texture_id 			next_texture_id = 1;
	context_id 			next_context_id = 1;
	
	allocator* alloc = NULL;
	string version, renderer, vendor;
};

void ogl_load_global_funcs();

ogl_manager make_opengl(allocator* a);
void destroy_opengl(ogl_manager* ogl);

shader_program_id ogl_add_program(ogl_manager* ogl, string v_path, string f_path, void (*set_uniforms)(shader_program*, render_command*, render_command_list*));
shader_program* ogl_select_program(ogl_manager* ogl, shader_program_id id);
void ogl_try_reload_programs(ogl_manager* ogl);
void ogl_set_uniforms(shader_program* prog, render_command* rc, render_command_list* rcl);

texture_id ogl_add_texture(ogl_manager* ogl, asset_store* as, string name, texture_wrap wrap = wrap_repeat, bool pixelated = false);
texture* ogl_select_texture(ogl_manager* ogl, texture_id id);
texture_id ogl_add_texture_from_font(ogl_manager* ogl, asset_store* as, string name, texture_wrap wrap = wrap_repeat, bool pixelated = false);
texture_id ogl_add_texture_from_font(ogl_manager* ogl, asset* font, texture_wrap wrap = wrap_repeat, bool pixelated = false);
void ogl_destroy_texture(ogl_manager* ogl, texture_id id);

context_id ogl_add_draw_context(ogl_manager* ogl, void (*set_atribs)(ogl_draw_context* dc));
ogl_draw_context* ogl_select_draw_context(ogl_manager* ogl, context_id id);

void ogl_dbg_render_texture_fullscreen(ogl_manager* ogl, texture_id id);
void ogl_render_command_list(ogl_manager* ogl, render_command_list* rcl);

shader_source make_source(string path, allocator* a);
void load_source(shader_source* source);
void destroy_source(shader_source* source);
bool refresh_source(shader_source* source);

shader_program make_program(string vert, string frag, void (*set_uniforms)(shader_program*, render_command*, render_command_list*), allocator* a);
void compile_program(shader_program* prog);
bool refresh_program(shader_program* prog);
void destroy_program(shader_program* prog);

texture make_texture(texture_wrap wrap, bool pixelated);
void texture_load_bitmap(texture* tex, asset_store* as, string name);
void texture_load_bitmap_from_font(texture* tex, asset_store* as, string name);
void texture_load_bitmap_from_font(texture* tex, asset* font);
void destroy_texture(texture* tex);

void debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userPointer);

void ogl_mesh_2d_attribs(ogl_draw_context* dc);
void ogl_send_mesh_2d(ogl_manager* ogl, mesh_2d* m, context_id id);

void ogl_mesh_3d_attribs(ogl_draw_context* dc);
void ogl_send_mesh_3d(ogl_manager* ogl, mesh_3d* m, context_id id);

void ogl_uniforms_gui(shader_program* prog, render_command* rc, render_command_list* rcl);
void ogl_uniforms_dbg(shader_program* prog, render_command* rc, render_command_list* rcl) {};
void ogl_uniforms_3dtex(shader_program* prog, render_command* rc, render_command_list* rcl);

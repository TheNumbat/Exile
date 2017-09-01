
#pragma once

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
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static shader_source make(string path, allocator* a);
	void load();
	void destroy();
	bool refresh();
};

struct shader_program {
	shader_program_id id = 0;
	GLuint handle = 0;
	shader_source vertex;
	shader_source fragment;
	void (*set_uniforms)(shader_program*, render_command*, render_command_list*) = null;
	// tessellation control, evaluation, geometry

///////////////////////////////////////////////////////////////////////////////

	static shader_program make(string vert, string frag, void (*set_uniforms)(shader_program*, render_command*, render_command_list*), allocator* a);
	void compile();
	bool refresh();
	void destroy();
};

enum class texture_wrap {
	repeat,
	mirror,
	clamp,
	clamp_border,
};

struct texture {
	texture_id id 		= 0;
	GLuint handle 		= 0;
	texture_wrap wrap 	= texture_wrap::repeat;
	bool pixelated 		= false;

///////////////////////////////////////////////////////////////////////////////

	static texture make(texture_wrap wrap, bool pixelated);
	void destroy();

	void load_bitmap(asset_store* as, string name);
	void load_bitmap_from_font(asset_store* as, string name);
	void load_bitmap_from_font(asset* font);
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
	
	allocator* alloc = null;
	string version;
	string renderer;
	string vendor;

///////////////////////////////////////////////////////////////////////////////

	static ogl_manager make(allocator* a);
	void destroy();

	shader_program_id add_program(string v_path, string f_path, void (*set_uniforms)(shader_program*, render_command*, render_command_list*));
	shader_program* select_program(shader_program_id id);
	void try_reload_programs();

	texture_id add_texture(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture* select_texture(texture_id id);
	texture_id add_texture_from_font(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture_id add_texture_from_font(asset* font, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	void destroy_texture(texture_id id);

	context_id add_draw_context(void (*set_atribs)(ogl_draw_context* dc));
	ogl_draw_context* select_draw_context(context_id id);

	void dbg_render_texture_fullscreen(platform_window* win, texture_id id);
	void execute_command_list(platform_window* win, render_command_list* rcl);

	void send_mesh_3d(mesh_3d* m, context_id id);
	void send_mesh_2d(mesh_2d* m, context_id id);
};

void ogl_load_global_funcs();
void ogl_set_uniforms(shader_program* prog, render_command* rc, render_command_list* rcl);

void debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userPointer);

void ogl_mesh_2d_attribs(ogl_draw_context* dc);
void ogl_mesh_3d_attribs(ogl_draw_context* dc);

void ogl_uniforms_gui(shader_program* prog, render_command* rc, render_command_list* rcl);
void ogl_uniforms_dbg(shader_program* prog, render_command* rc, render_command_list* rcl) {};
void ogl_uniforms_3dtex(shader_program* prog, render_command* rc, render_command_list* rcl);

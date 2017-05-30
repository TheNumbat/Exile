
shader_source make_source(string path, allocator* a) {

	shader_source ret;

	ret.path = path;
	ret.alloc = a;

	load_source(&ret);

	return ret;
}

void load_source(shader_source* source) {

	platform_file source_file;
	platform_error err = global_state->api->platform_create_file(&source_file, source->path, open_file_existing);
	if(!err.good) {
		LOG_ERR_F("Failed to load shader source %s", source->path.c_str);
		global_state->api->platform_close_file(&source_file);
		return;
	}

	u32 len = global_state->api->platform_file_size(&source_file) + 1;
	source->source = make_string(len, source->alloc);
	global_state->api->platform_read_file(&source_file, (void*)source->source.c_str, len - 1);

	// TODO(max): do preprocessing somewhere around here
	source->source.c_str[len] = 0;

	global_state->api->platform_close_file(&source_file);

	global_state->api->platform_get_file_attributes(&source->last_attrib, source->path);
}

void destroy_source(shader_source* source) {

	free_string(source->source, source->alloc);
}

bool refresh_source(shader_source* source) {

	platform_file_attributes new_attrib;
	global_state->api->platform_get_file_attributes(&new_attrib, source->path);	

	if(global_state->api->platform_test_file_written(&source->last_attrib, &new_attrib)) {

		destroy_source(source);
		load_source(source);

		return true;
	}

	return false;
}

shader_program make_program(string vert, string frag, allocator* a) {

	shader_program ret;

	ret.vertex = make_source(vert, a);
	ret.fragment = make_source(frag, a);
	ret.handle = glCreateProgram();

	compile_program(&ret);

	return ret;
}

void compile_program(shader_program* prog) {

	GLuint vertex, fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertex, 1, &prog->vertex.source.c_str, NULL);
	glShaderSource(fragment, 1, &prog->fragment.source.c_str, NULL);

	glCompileShader(vertex);
	glCompileShader(fragment);

	glAttachShader(prog->handle, vertex);
	glAttachShader(prog->handle, fragment);
	glLinkProgram(prog->handle);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void refresh_program(shader_program* prog) {

	if(refresh_source(&prog->vertex) || refresh_source(&prog->fragment)) {

		glDeleteProgram(prog->handle);
		prog->handle = glCreateProgram();

		compile_program(prog);
	}
}

void destroy_program(shader_program* prog) {

	destroy_source(&prog->vertex);
	destroy_source(&prog->fragment);

	glDeleteProgram(prog->handle);
}

opengl make_opengl(allocator* a) {

	opengl ret;

	ret.alloc = a;
	ret.programs = make_map<string, shader_program>(8, a, &hash_string);

	ret.version 	= string_from_c_str((char*)glGetString(GL_VERSION));
	ret.renderer 	= string_from_c_str((char*)glGetString(GL_RENDERER));
	ret.vendor  	= string_from_c_str((char*)glGetString(GL_VENDOR));

	LOG_INFO_F("GL version : %s", ret.version.c_str);
	LOG_INFO_F("GL renderer: %s", ret.renderer.c_str);
	LOG_INFO_F("GL vendor  : %s", ret.vendor.c_str);

	return ret;
}

void destroy_opengl(opengl* ogl) {

	for(u32 i = 0; i < ogl->programs.contents.capacity; i++) {
		if(vector_get(&ogl->programs.contents, i)->occupied) {
			destroy_program(&vector_get(&ogl->programs.contents, i)->value);
		}
	}

	destroy_map(&ogl->programs);
}

void ogl_add_program(opengl* ogl, string name, string v_path, string f_path) {

	shader_program p = make_program(v_path, f_path, ogl->alloc);

	map_insert(&ogl->programs, name, p);
}

void ogl_select_program(opengl* ogl, string name) {

	shader_program* p = map_try_get(&ogl->programs, name);

	if(!p) {
		LOG_ERR_F("Failed to retrieve program %s", name.c_str);
		return;
	}
	
	glUseProgram(p->handle);
}

void debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up) {

	string message = string_from_c_str((char*)glmessage);
	string source, type;

	switch(glsource) {
	case GL_DEBUG_SOURCE_API:
		source = string_literal("OpenGL API");
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		source = string_literal("Window System");
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		source = string_literal("Shader Compiler");
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		source = string_literal("Third Party");
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		source = string_literal("Application");
		break;
	case GL_DEBUG_SOURCE_OTHER:
		source = string_literal("Other");
		break;
	}

	switch(gltype) {
	case GL_DEBUG_TYPE_ERROR:
		type = string_literal("Error");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type = string_literal("Deprecated");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type = string_literal("Undefined Behavior");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type = string_literal("Portability");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type = string_literal("Performance");
		break;
	case GL_DEBUG_TYPE_MARKER:
		type = string_literal("Marker");
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		type = string_literal("Push Group");
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		type = string_literal("Pop Group");
		break;
	case GL_DEBUG_TYPE_OTHER:
		type = string_literal("Other");
		break;
	}

	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_ERR_F("HIGH OpenGL: %s source: %s type: %s", message.c_str, source.c_str, type.c_str);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_WARN_F("MED OpenGL: %s source: %s type: %s", message.c_str, source.c_str, type.c_str);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		LOG_WARN_F("LOW OpenGL: %s source: %s type: %s", message.c_str, source.c_str, type.c_str);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_INFO_F("NOTF OpenGL: %s source: %s type: %s", message.c_str, source.c_str, type.c_str);
		break;
	}
}

void ogl_load_global_funcs() {

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback 	= (glDebugMessageCallback_t) global_state->api->platform_get_glproc(string_literal("glDebugMessageCallback"));
	glDebugMessageInsert 	= (glDebugMessageInsert_t) global_state->api->platform_get_glproc(string_literal("glDebugMessageInsert"));
	glDebugMessageControl 	= (glDebugMessageControl_t) global_state->api->platform_get_glproc(string_literal("glDebugMessageControl"));

	glAttachShader  = (glAttachShader_t)  global_state->api->platform_get_glproc(string_literal("glAttachShader"));
	glCompileShader = (glCompileShader_t) global_state->api->platform_get_glproc(string_literal("glCompileShader"));
	glCreateProgram = (glCreateProgram_t) global_state->api->platform_get_glproc(string_literal("glCreateProgram"));
	glCreateShader  = (glCreateShader_t)  global_state->api->platform_get_glproc(string_literal("glCreateShader"));
	glDeleteProgram = (glDeleteProgram_t) global_state->api->platform_get_glproc(string_literal("glDeleteProgram"));
	glDeleteShader  = (glDeleteShader_t)  global_state->api->platform_get_glproc(string_literal("glDeleteShader"));
	glLinkProgram   = (glLinkProgram_t)   global_state->api->platform_get_glproc(string_literal("glLinkProgram"));
	glShaderSource  = (glShaderSource_t)  global_state->api->platform_get_glproc(string_literal("glShaderSource"));
	glUseProgram    = (glUseProgram_t)    global_state->api->platform_get_glproc(string_literal("glUseProgram"));

	glDebugMessageCallback(debug_proc, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}

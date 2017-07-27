
shader_source make_source(string path, allocator* a) { PROF

	shader_source ret;

	ret.path = path;
	ret.alloc = a;

	load_source(&ret);

	return ret;
}

void load_source(shader_source* source) { PROF

	platform_file source_file;

	platform_error err;
	u32 itr = 0;
	do {
		itr++;
		err = global_state->api->platform_create_file(&source_file, source->path, platform_file_open_op::existing);
	} while (err.error == PLATFORM_SHARING_ERROR && itr < 100000);

	if(!err.good) {
		LOG_ERR_F("Failed to load shader source %", source->path);
		global_state->api->platform_close_file(&source_file);
		return;
	}

	u32 len = global_state->api->platform_file_size(&source_file) + 1;
	source->source = make_string(len, source->alloc);
	global_state->api->platform_read_file(&source_file, (void*)source->source.c_str, len - 1);

	source->source.c_str[len] = 0;

	global_state->api->platform_close_file(&source_file);

	global_state->api->platform_get_file_attributes(&source->last_attrib, source->path);
}

void destroy_source(shader_source* source) { PROF

	free_string(source->source, source->alloc);
}

bool refresh_source(shader_source* source) { PROF

	platform_file_attributes new_attrib;
	
	global_state->api->platform_get_file_attributes(&new_attrib, source->path);	
	
	if(global_state->api->platform_test_file_written(&source->last_attrib, &new_attrib)) {

		destroy_source(source);
		load_source(source);

		return true;
	}

	return false;
}

shader_program make_program(string vert, string frag, void (*set_uniforms)(shader_program*, render_command*, render_command_list*), allocator* a) { PROF

	shader_program ret;

	ret.vertex = make_source(vert, a);
	ret.fragment = make_source(frag, a);
	ret.handle = glCreateProgram();
	ret.set_uniforms = set_uniforms;

	compile_program(&ret);

	return ret;
}

void compile_program(shader_program* prog) { PROF

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

bool refresh_program(shader_program* prog) { PROF

	if(refresh_source(&prog->vertex) || refresh_source(&prog->fragment)) {

		glDeleteProgram(prog->handle);
		prog->handle = glCreateProgram();

		compile_program(prog);

		return true;
	}

	return false;
}

void destroy_program(shader_program* prog) { PROF

	destroy_source(&prog->vertex);
	destroy_source(&prog->fragment);

	glDeleteProgram(prog->handle);
}

void ogl_try_reload_programs(ogl_manager* ogl) { PROF
	FORMAP(ogl->programs,
		if(refresh_program(&it->value)) {
			LOG_INFO_F("Reloaded program % with files %, %", it->key, it->value.vertex.path, it->value.fragment.path);
		}
	)
}

ogl_manager make_opengl(allocator* a) { PROF

	ogl_manager ret;

	ret.alloc = a;
	ret.programs = make_map<shader_program_id, shader_program>(8, a);
	ret.textures = make_map<texture_id, texture>(32, a);
	ret.contexts = make_map<context_id, ogl_draw_context>(32, a);

	ret.version 	= string_from_c_str((char*)glGetString(GL_VERSION));
	ret.renderer 	= string_from_c_str((char*)glGetString(GL_RENDERER));
	ret.vendor  	= string_from_c_str((char*)glGetString(GL_VENDOR));

	ret.dbg_shader = ogl_add_program(&ret, string_literal("shaders/dbg.v"), string_literal("shaders/dbg.f"), &ogl_uniforms_dbg);

	LOG_INFO_F("GL version : %", ret.version);
	LOG_INFO_F("GL renderer: %", ret.renderer);
	LOG_INFO_F("GL vendor  : %", ret.vendor);

	return ret;
}

void destroy_opengl(ogl_manager* ogl) { PROF

	for(u32 i = 0; i < ogl->programs.contents.capacity; i++) {
		if(vector_get(&ogl->programs.contents, i)->occupied) {
			destroy_program(&vector_get(&ogl->programs.contents, i)->value);
		}
	}
	for(u32 i = 0; i < ogl->textures.contents.capacity; i++) {
		if(vector_get(&ogl->textures.contents, i)->occupied) {
			destroy_texture(&vector_get(&ogl->textures.contents, i)->value);
		}
	}
	for(u32 i = 0; i < ogl->contexts.contents.capacity; i++) {
		if(vector_get(&ogl->contexts.contents, i)->occupied) {
			glDeleteVertexArrays(1, &(vector_get(&ogl->contexts.contents, i))->value.vao);
			glDeleteBuffers(8, vector_get(&ogl->contexts.contents, i)->value.vbos);
		}
	}

	destroy_map(&ogl->programs);
	destroy_map(&ogl->textures);
	destroy_map(&ogl->contexts);
}

shader_program_id ogl_add_program(ogl_manager* ogl, string v_path, string f_path, void (*set_uniforms)(shader_program*, render_command*, render_command_list*)) { PROF

	shader_program p = make_program(v_path, f_path, set_uniforms, ogl->alloc);
	p.id = ogl->next_shader_id;

	map_insert(&ogl->programs, ogl->next_shader_id, p);

	LOG_DEBUG_F("Loaded shader from % and %", v_path, f_path);

	ogl->next_shader_id++;
	return ogl->next_shader_id - 1;
}

shader_program* ogl_select_program(ogl_manager* ogl, shader_program_id id) { PROF

	shader_program* p = map_try_get(&ogl->programs, id);

	if(!p) {
		LOG_ERR_F("Failed to retrieve program %", id);
		return NULL;
	}
	
	glUseProgram(p->handle);

	return p;
}

texture_id ogl_add_texture_from_font(ogl_manager* ogl, asset* font, texture_wrap wrap, bool pixelated) { PROF

	texture t = make_texture(wrap, pixelated);
	t.id = ogl->next_texture_id;

	texture_load_bitmap_from_font(&t, font);

	map_insert(&ogl->textures, ogl->next_texture_id, t);

	LOG_DEBUG_F("Created texture % from font %", ogl->next_texture_id, font->name);

	ogl->next_texture_id++;
	return ogl->next_texture_id - 1;
}

texture_id ogl_add_texture_from_font(ogl_manager* ogl, asset_store* as, string name, texture_wrap wrap, bool pixelated) { PROF

	texture t = make_texture(wrap, pixelated);
	t.id = ogl->next_texture_id;

	texture_load_bitmap_from_font(&t, as, name);

	map_insert(&ogl->textures, ogl->next_texture_id, t);

	LOG_DEBUG_F("Created texture % from font asset %", ogl->next_texture_id, name);

	ogl->next_texture_id++;
	return ogl->next_texture_id - 1;
}

texture_id ogl_add_texture(ogl_manager* ogl, asset_store* as, string name, texture_wrap wrap, bool pixelated) { PROF

	texture t = make_texture(wrap, pixelated);
	t.id = ogl->next_texture_id;

	texture_load_bitmap(&t, as, name);

	map_insert(&ogl->textures, ogl->next_texture_id, t);

	LOG_DEBUG_F("Created texture % from bitmap asset %", ogl->next_texture_id, name);

	ogl->next_texture_id++;
	return ogl->next_texture_id - 1;
}

void ogl_destroy_texture(ogl_manager* ogl, texture_id id) { PROF

	texture* t = map_try_get(&ogl->textures, id);

	if(!t) {
		LOG_ERR_F("Failed to find texture %", id);
		return;
	}

	glDeleteTextures(1, &t->handle);

	map_erase(&ogl->textures, id);
}

texture* ogl_select_texture(ogl_manager* ogl, texture_id id) { PROF

	texture* t = map_try_get(&ogl->textures, id);

	if(!t) {
		LOG_ERR_F("Failed to retrieve texture %", id);
		return NULL;
	}
	
	glBindTextureUnit(0, t->handle);

	return t;
}

texture make_texture(texture_wrap wrap, bool pixelated) { PROF

	texture ret;

	ret.wrap = wrap;
	ret.pixelated = pixelated;
	glCreateTextures(GL_TEXTURE_2D, 1, &ret.handle);

	glBindTextureUnit(0, ret.handle);
	
	switch(wrap) {
	case texture_wrap::repeat:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case texture_wrap::mirror:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	case texture_wrap::clamp:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case texture_wrap::clamp_border:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		f32 borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
		break;
	}

	if(pixelated) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindTextureUnit(0, 0);

	return ret;
}

void texture_load_bitmap_from_font(texture* tex, asset* font) { PROF

	LOG_DEBUG_ASSERT(font->type == asset_type::font);

	glBindTextureUnit(0, tex->handle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, font->font.width, font->font.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font->mem);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTextureUnit(0, 0);
}

void texture_load_bitmap_from_font(texture* tex, asset_store* as, string name) { PROF

	asset* a = get_asset(as, name);

	LOG_DEBUG_ASSERT(a->type == asset_type::font);

	glBindTextureUnit(0, tex->handle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, a->font.width, a->font.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a->mem);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTextureUnit(0, 0);
}

void texture_load_bitmap(texture* tex, asset_store* as, string name) { PROF

	asset* a = get_asset(as, name);

	LOG_DEBUG_ASSERT(a->type == asset_type::bitmap);

	glBindTextureUnit(0, tex->handle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, a->bitmap.width, a->bitmap.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a->mem);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTextureUnit(0, 0);
}

void destroy_texture(texture* tex) { PROF

	glDeleteTextures(1, &tex->handle);
}

context_id ogl_add_draw_context(ogl_manager* ogl, void (*set_atribs)(ogl_draw_context* dc)) { PROF

	ogl_draw_context d;
	glGenVertexArrays(1, &d.vao);
	glBindVertexArray(d.vao);
	glGenBuffers(8, d.vbos);
	d.id = ogl->next_context_id;

	set_atribs(&d);

	map_insert(&ogl->contexts, ogl->next_context_id, d);

	ogl->next_context_id++;

	return ogl->next_context_id - 1;
}

ogl_draw_context* ogl_select_draw_context(ogl_manager* ogl, context_id id) { PROF

	ogl_draw_context* d = map_try_get(&ogl->contexts, id);

	if(!d) {
		LOG_ERR_F("Failed to retrieve context %", id);
		return NULL;
	}
	
	glBindVertexArray(d->vao);

	return d;
}

void ogl_mesh_3d_attribs(ogl_draw_context* dc) { PROF

	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void ogl_send_mesh_3d(ogl_manager* ogl, mesh_3d* m, context_id id) { PROF

	ogl_draw_context* dc = ogl_select_draw_context(ogl, id);

	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, m->verticies.size * sizeof(v3), m->verticies.size ? m->verticies.memory : NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : NULL, GL_STREAM_DRAW);
}

void ogl_uniforms_3dtex(shader_program* prog, render_command* rc, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = mult(rc->model, rcl->view);
	transform = mult(transform, rcl->proj);

	glUniformMatrix4fv(loc, 1, GL_FALSE, transform.v);
}

void ogl_mesh_2d_attribs(ogl_draw_context* dc) { PROF

	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[2]);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dc->vbos[3]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void ogl_send_mesh_2d(ogl_manager* ogl, mesh_2d* m, context_id id) { PROF

	ogl_draw_context* dc = ogl_select_draw_context(ogl, id);

	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, m->verticies.size * sizeof(v2), m->verticies.size ? m->verticies.memory : NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, m->texCoords.size * sizeof(v3), m->texCoords.size ? m->texCoords.memory : NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, dc->vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, m->colors.size * sizeof(v4), m->colors.size ? m->colors.memory : NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dc->vbos[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : NULL, GL_STREAM_DRAW);
}

void ogl_uniforms_gui(shader_program* prog, render_command* rc, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = mult(rc->model, rcl->view);
	transform = mult(transform, rcl->proj);

	glUniformMatrix4fv(loc, 1, GL_FALSE, transform.v);
}

void ogl_set_uniforms(shader_program* prog, render_command* rc, render_command_list* rcl) { PROF

	prog->set_uniforms(prog, rc, rcl);
}

void ogl_render_command_list(ogl_manager* ogl, render_command_list* rcl) { PROF

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	for(u32 i = 0; i < rcl->commands.size; i++) {

		render_command* cmd = vector_get(&rcl->commands, i);

		ogl_select_draw_context(ogl, cmd->context);
		ogl_select_texture(ogl, cmd->texture);
		shader_program* prog = ogl_select_program(ogl, cmd->shader);

		ogl_set_uniforms(prog, cmd, rcl);

		glViewport(0, 0, global_state->window_w, global_state->window_h);

		if(cmd->cmd == render_command_type::mesh_2d) {

			// TODO(max): we don't want to send every frame, do we?
			ogl_send_mesh_2d(ogl, cmd->m2d, cmd->context);

			glDisable(GL_DEPTH_TEST);

			glDrawElements(GL_TRIANGLES, cmd->elements * 3, GL_UNSIGNED_INT, 0);

		} else if (cmd->cmd == render_command_type::mesh_3d) {

			ogl_send_mesh_3d(ogl, cmd->m3d, cmd->context);

			glEnable(GL_DEPTH_TEST);

			glDrawArrays(GL_TRIANGLES, 0, cmd->m3d->verticies.size);
		}
	}
}

// temporary and inefficient texture render
void ogl_dbg_render_texture_fullscreen(ogl_manager* ogl, texture_id id) { PROF

	GLfloat data[] = {
		-1.0f, -1.0f,	0.0f, 0.0f,
		-1.0f,  1.0f, 	0.0f, 1.0f,
		 1.0f, -1.0f,	1.0f, 0.0f,

		-1.0f,  1.0f, 	0.0f, 1.0f,
		 1.0f, -1.0f,	1.0f, 0.0f,
		 1.0f,  1.0f,	1.0f, 1.0f
	};

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	ogl_select_program(ogl, ogl->dbg_shader);
	ogl_select_texture(ogl, id);

	glViewport(0, 0, global_state->window_w, global_state->window_h);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up) { PROF

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
		LOG_ERR_F("HIGH OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_WARN_F("MED OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		LOG_WARN_F("LOW OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_OGL_F("NOTF OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	}
}

void ogl_load_global_funcs() { PROF

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback 	= (glDebugMessageCallback_t) global_state->api->platform_get_glproc(string_literal("glDebugMessageCallback"));
	glDebugMessageInsert 	= (glDebugMessageInsert_t) 	 global_state->api->platform_get_glproc(string_literal("glDebugMessageInsert"));
	glDebugMessageControl 	= (glDebugMessageControl_t)  global_state->api->platform_get_glproc(string_literal("glDebugMessageControl"));

	glAttachShader       = (glAttachShader_t)  global_state->api->platform_get_glproc(string_literal("glAttachShader"));
	glCompileShader      = (glCompileShader_t) global_state->api->platform_get_glproc(string_literal("glCompileShader"));
	glCreateProgram      = (glCreateProgram_t) global_state->api->platform_get_glproc(string_literal("glCreateProgram"));
	glCreateShader       = (glCreateShader_t)  global_state->api->platform_get_glproc(string_literal("glCreateShader"));
	glDeleteProgram      = (glDeleteProgram_t) global_state->api->platform_get_glproc(string_literal("glDeleteProgram"));
	glDeleteShader       = (glDeleteShader_t)  global_state->api->platform_get_glproc(string_literal("glDeleteShader"));
	glLinkProgram        = (glLinkProgram_t)   global_state->api->platform_get_glproc(string_literal("glLinkProgram"));
	glShaderSource       = (glShaderSource_t)  global_state->api->platform_get_glproc(string_literal("glShaderSource"));
	glUseProgram         = (glUseProgram_t)    global_state->api->platform_get_glproc(string_literal("glUseProgram"));
	glGetUniformLocation = (glGetUniformLocation_t) global_state->api->platform_get_glproc(string_literal("glGetUniformLocation"));
	glUniformMatrix4fv   = (glUniformMatrix4fv_t)   global_state->api->platform_get_glproc(string_literal("glUniformMatrix4fv"));

	glGenerateMipmap  = (glGenerateMipmap_t)  global_state->api->platform_get_glproc(string_literal("glGenerateMipmap"));
	glActiveTexture   = (glActiveTexture_t)   global_state->api->platform_get_glproc(string_literal("glActiveTexture"));
	glCreateTextures  = (glCreateTextures_t)  global_state->api->platform_get_glproc(string_literal("glCreateTextures"));
	glBindTextureUnit = (glBindTextureUnit_t) global_state->api->platform_get_glproc(string_literal("glBindTextureUnit"));

	glBindVertexArray    = (glBindVertexArray_t)    global_state->api->platform_get_glproc(string_literal("glBindVertexArray"));
	glDeleteVertexArrays = (glDeleteVertexArrays_t) global_state->api->platform_get_glproc(string_literal("glDeleteVertexArrays"));
	glGenVertexArrays    = (glGenVertexArrays_t)    global_state->api->platform_get_glproc(string_literal("glGenVertexArrays"));

	glBindBuffer    = (glBindBuffer_t)    global_state->api->platform_get_glproc(string_literal("glBindBuffer"));
	glDeleteBuffers = (glDeleteBuffers_t) global_state->api->platform_get_glproc(string_literal("glDeleteBuffers"));
	glGenBuffers    = (glGenBuffers_t)    global_state->api->platform_get_glproc(string_literal("glGenBuffers"));
	glBufferData	= (glBufferData_t)    global_state->api->platform_get_glproc(string_literal("glBufferData"));

	glVertexAttribPointer 	  = (glVertexAttribPointer_t) 	  global_state->api->platform_get_glproc(string_literal("glVertexAttribPointer"));
	glEnableVertexAttribArray = (glEnableVertexAttribArray_t) global_state->api->platform_get_glproc(string_literal("glEnableVertexAttribArray"));
	
	glDebugMessageCallback(debug_proc, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}

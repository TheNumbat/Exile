
shader_source shader_source::make(string path, allocator* a) { PROF

	shader_source ret;

	ret.path = string::make_copy(path, a);
	ret.alloc = a;

	ret.load();

	return ret;
}

void shader_source::load() { PROF

	platform_file source_file;

	platform_error error;
	u32 itr = 0;
	do {
		itr++;
		error = global_api->platform_create_file(&source_file, path, platform_file_open_op::existing);
	} while (error.error == PLATFORM_SHARING_ERROR && itr < 100000);

	if(!error.good) {
		LOG_ERR_F("Failed to load shader source %", path);
		CHECKED(platform_close_file, &source_file);
		return;
	}

	u32 len = global_api->platform_file_size(&source_file) + 1;
	source = string::make(len, alloc);
	CHECKED(platform_read_file, &source_file, (void*)source.c_str, len);

	CHECKED(platform_close_file, &source_file);

	CHECKED(platform_get_file_attributes, &last_attrib, path);
}

void shader_source::destroy() { PROF

	source.destroy(alloc);
	path.destroy(alloc);
}

bool shader_source::refresh() { PROF

	platform_file_attributes new_attrib;
	
	CHECKED(platform_get_file_attributes, &new_attrib, path);	
	
	if(global_api->platform_test_file_written(&last_attrib, &new_attrib)) {

		destroy();
		load();

		return true;
	}

	return false;
}

shader_program shader_program::make(string vert, string frag, _FPTR* uniforms, allocator* a) { PROF

	shader_program ret;

	ret.vertex = shader_source::make(vert, a);
	ret.fragment = shader_source::make(frag, a);
	ret.handle = glCreateProgram();
	ret.set_uniforms.set(uniforms);

	ret.compile();

	return ret;
}

void shader_program::compile() { PROF

	GLuint h_vertex, h_fragment;

	h_vertex = glCreateShader(gl_shader_type::vertex);
	h_fragment = glCreateShader(gl_shader_type::fragment);

	glShaderSource(h_vertex, 1, &vertex.source.c_str, null);
	glShaderSource(h_fragment, 1, &fragment.source.c_str, null);

	glCompileShader(h_vertex);
	glCompileShader(h_fragment);

	glAttachShader(handle, h_vertex);
	glAttachShader(handle, h_fragment);
	glLinkProgram(handle);

	glDeleteShader(h_vertex);
	glDeleteShader(h_fragment);
}

bool shader_program::refresh() { PROF

	if(vertex.refresh() || fragment.refresh()) {

		glDeleteProgram(handle);
		handle = glCreateProgram();

		compile();

		return true;
	}

	return false;
}

void shader_program::destroy() { PROF

	vertex.destroy();
	fragment.destroy();

	glDeleteProgram(handle);
}

void ogl_manager::try_reload_programs() { PROF
	FORMAP(it, programs) {
		if(it->value.refresh()) {
			LOG_INFO_F("Reloaded program % with files %, %", it->key, it->value.vertex.path, it->value.fragment.path);
		}
	}
	dbg_shader.refresh();
}

#define REGISTER_COMMAND(cmd) ret.add_program(render_command_type::cmd, string::literal("shaders/" #cmd ".v"), string::literal("shaders/" #cmd ".f"), FPTR(uniforms_##cmd)); \
							  ret.add_draw_context(render_command_type::cmd, FPTR(attribs_##cmd), FPTR(buffers_##cmd), FPTR(run_##cmd));

ogl_manager ogl_manager::make(allocator* a) { PROF

	ogl_manager ret;

	ret.alloc = a;
	ret.programs = map<render_command_type, shader_program>::make(8, a);
	ret.textures = map<texture_id, texture>::make(32, a);
	ret.contexts = map<render_command_type, draw_context>::make(32, a);

	ret.version 	= string::from_c_str((char*)glGetString(gl_info::version));
	ret.renderer 	= string::from_c_str((char*)glGetString(gl_info::renderer));
	ret.vendor  	= string::from_c_str((char*)glGetString(gl_info::vendor));

	REGISTER_COMMAND(mesh_2d_col);
	// REGISTER_COMMAND(mesh_2d_tex);
	// REGISTER_COMMAND(mesh_2d_tex_col);
	REGISTER_COMMAND(mesh_3d_tex);

	ret.dbg_shader = shader_program::make(string::literal("shaders/dbg.v"),string::literal("shaders/dbg.f"),FPTR(uniforms_dbg),a);

	LOG_INFO_F("GL version : %", ret.version);
	LOG_INFO_F("GL renderer: %", ret.renderer);
	LOG_INFO_F("GL vendor  : %", ret.vendor);

	return ret;
}

void ogl_manager::destroy() { PROF

	FORMAP(it, programs) {
		it->value.destroy();
	}
	FORMAP(it, textures) {
		it->value.destroy();
	}
	FORMAP(it, contexts) {
		glDeleteVertexArrays(1, &it->value.vao);
		glDeleteBuffers(8, it->value.vbos);
	}

	dbg_shader.destroy();

	programs.destroy();
	textures.destroy();
	contexts.destroy();
} 

void ogl_manager::add_program(render_command_type type, string v_path, string f_path, _FPTR* uniforms) { PROF

	shader_program p = shader_program::make(v_path, f_path, uniforms, alloc);

	programs.insert(type, p);

	LOG_DEBUG_F("Loaded shader from % and %", v_path, f_path);
}

shader_program* ogl_manager::select_program(render_command_type id) { PROF

	shader_program* p = programs.try_get(id);

	if(!p) {
		LOG_ERR_F("Failed to retrieve program %", id);
		return null;
	}
	
	glUseProgram(p->handle);

	return p;
}

texture_id ogl_manager::add_texture_from_font(asset* font, texture_wrap wrap, bool pixelated) { PROF

	texture t = texture::make(wrap, pixelated);
	t.id = next_texture_id;

	t.load_bitmap_from_font(font);

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created texture % from font %", next_texture_id, font->name);

	next_texture_id++;
	return next_texture_id - 1;
}

texture_id ogl_manager::add_texture_from_font(asset_store* as, string name, texture_wrap wrap, bool pixelated) { PROF

	texture t = texture::make(wrap, pixelated);
	t.id = next_texture_id;

	t.load_bitmap_from_font(as, name);

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created texture % from font asset %", next_texture_id, name);

	next_texture_id++;
	return next_texture_id - 1;
}

texture_id ogl_manager::add_texture(asset_store* as, string name, texture_wrap wrap, bool pixelated) { PROF

	texture t = texture::make(wrap, pixelated);
	t.id = next_texture_id;

	t.load_bitmap(as, name);

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created texture % from bitmap asset %", next_texture_id, name);

	next_texture_id++;
	return next_texture_id - 1;
}

void ogl_manager::destroy_texture(texture_id id) { PROF

	texture* t = textures.try_get(id);

	if(!t) {
		LOG_ERR_F("Failed to find texture %", id);
		return;
	}

	glDeleteTextures(1, &t->handle);

	textures.erase(id);
}

texture* ogl_manager::select_texture(texture_id id) { PROF

	if(id == -1) return null;

	texture* t = textures.try_get(id);

	if(!t) {
		LOG_ERR_F("Failed to retrieve texture %", id);
		return null;
	}
	
	glBindTextureUnit(0, t->handle);

	return t;
}

texture texture::make(texture_wrap wrap, bool pixelated) { PROF

	texture ret;

	ret.wrap = wrap;
	ret.pixelated = pixelated;
	glCreateTextures(gl_tex_target::_2D, 1, &ret.handle);

	glBindTextureUnit(0, ret.handle);
	
	switch(wrap) {
	case texture_wrap::repeat:
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::repeat);
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::repeat);
		break;
	case texture_wrap::mirror:
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::mirrored_repeat);
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::mirrored_repeat);
		break;
	case texture_wrap::clamp:
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::clamp_to_edge);
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::clamp_to_edge);
		break;
	case texture_wrap::clamp_border:
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::clamp_to_border);
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::clamp_to_border);
		f32 borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(gl_tex_target::_2D, gl_tex_param::border_color, borderColor);  
		break;
	}

	if(pixelated) {
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::min_filter, (GLint)gl_tex_filter::nearest);
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::mag_filter, (GLint)gl_tex_filter::nearest);
	} else {
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::min_filter, (GLint)gl_tex_filter::linear_mipmap_linear);
		glTexParameteri(gl_tex_target::_2D, gl_tex_param::mag_filter, (GLint)gl_tex_filter::linear);
	}

	glBindTextureUnit(0, 0);

	return ret;
}

void texture::load_bitmap_from_font(asset* font) { PROF

	LOG_DEBUG_ASSERT(font->type == asset_type::font);

	glBindTextureUnit(0, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, font->font.width, font->font.height, 0, gl_pixel_data_format::red, gl_pixel_data_type::unsigned_byte, font->mem);
	GLint swizzleMask[] = {(GLint)gl_tex_swizzle::red, (GLint)gl_tex_swizzle::red, (GLint)gl_tex_swizzle::red, (GLint)gl_tex_swizzle::red};
	glTexParameteriv(gl_tex_target::_2D, gl_tex_param::swizzle_rgba, swizzleMask);

	glGenerateMipmap(gl_tex_target::_2D);

	glBindTextureUnit(0, 0);
}

void texture::load_bitmap_from_font(asset_store* as, string name) { PROF

	asset* a = as->get(name);

	LOG_DEBUG_ASSERT(a->type == asset_type::font);

	glBindTextureUnit(0, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, a->font.width, a->font.height, 0, gl_pixel_data_format::red, gl_pixel_data_type::unsigned_byte, a->mem);
	GLint swizzleMask[] = {(GLint)gl_tex_swizzle::red, (GLint)gl_tex_swizzle::red, (GLint)gl_tex_swizzle::red, (GLint)gl_tex_swizzle::red};
	glTexParameteriv(gl_tex_target::_2D, gl_tex_param::swizzle_rgba, swizzleMask);

	glGenerateMipmap(gl_tex_target::_2D);

	glBindTextureUnit(0, 0);
}

void texture::load_bitmap(asset_store* as, string name) { PROF

	asset* a = as->get(name);

	LOG_DEBUG_ASSERT(a->type == asset_type::bitmap);

	glBindTextureUnit(0, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::red, gl_pixel_data_type::unsigned_byte, a->mem);
	
	glGenerateMipmap(gl_tex_target::_2D);

	glBindTextureUnit(0, 0);
}

void texture::destroy() { PROF

	glDeleteTextures(1, &handle);
}

void ogl_manager::add_draw_context(render_command_type type, _FPTR* attribs, _FPTR* buffers, _FPTR* run) { PROF

	draw_context d;
	glGenVertexArrays(1, &d.vao);
	glBindVertexArray(d.vao);
	glGenBuffers(8, d.vbos);

	d.set_attribs.set(attribs);
	d.set_buffers.set(buffers);
	d.run.set(run);

	d.set_attribs(&d);

	contexts.insert(type, d);
}

draw_context* ogl_manager::select_draw_context(render_command_type id) { PROF

	draw_context* d = contexts.try_get(id);

	if(!d) {
		LOG_ERR_F("Failed to retrieve context %", id);
		return null;
	}
	
	glBindVertexArray(d->vao);

	return d;
}

void ogl_manager::execute_command_list(platform_window* win, render_command_list* rcl) { PROF

	glEnable(gl_capability::blend);
	glBlendFunc(gl_blend_factor::one, gl_blend_factor::one_minus_src_alpha);

	FORVEC(cmd, rcl->commands) {

		glViewport(0, 0, win->w, win->h);

		cmd_set_settings(cmd);

		draw_context* d = select_draw_context(cmd->cmd);
		/*texture* t =*/ select_texture(cmd->texture);
		shader_program* s = select_program(cmd->cmd);

		d->set_buffers(d, cmd);
		s->set_uniforms(s, cmd, rcl);
		d->run(cmd);
	}
}

void ogl_manager::cmd_set_settings(render_command* cmd) {

}

// temporary and inefficient texture render
void ogl_manager::dbg_render_texture_fullscreen(platform_window* win, texture_id id) { PROF

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
	glBindBuffer(gl_buf_target::array, VBO);

	glBufferData(gl_buf_target::array, sizeof(data), data, gl_buf_usage::static_draw);

	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, 4 * sizeof(GLfloat), (void*)0);	
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUseProgram(dbg_shader.handle);
	select_texture(id);

	glViewport(0, 0, win->w, win->h);
	glEnable(gl_capability::blend);
	glBlendFunc(gl_blend_factor::one, gl_blend_factor::one_minus_src_alpha);

	glDrawArrays(gl_draw_mode::triangles, 0, 6);
	
	glBindBuffer(gl_buf_target::array, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void debug_proc(gl_debug_source glsource, gl_debug_type gltype, GLuint id, gl_debug_severity severity, GLsizei length, const GLchar* glmessage, const void* up) {

	string message = string::from_c_str((char*)glmessage);
	string source, type;

	switch(glsource) {
	case gl_debug_source::api:
		source = string::literal("OpenGL API");
		break;
	case gl_debug_source::window_system:
		source = string::literal("Window System");
		break;
	case gl_debug_source::shader_compiler:
		source = string::literal("Shader Compiler");
		break;
	case gl_debug_source::third_party:
		source = string::literal("Third Party");
		break;
	case gl_debug_source::application:
		source = string::literal("Application");
		break;
	case gl_debug_source::other:
		source = string::literal("Other");
		break;
	}

	switch(gltype) {
	case gl_debug_type::error:
		type = string::literal("Error");
		break;
	case gl_debug_type::deprecated_behavior:
		type = string::literal("Deprecated");
		break;
	case gl_debug_type::undefined_behavior:
		type = string::literal("Undefined Behavior");
		break;
	case gl_debug_type::portability:
		type = string::literal("Portability");
		break;
	case gl_debug_type::performance:
		type = string::literal("Performance");
		break;
	case gl_debug_type::marker:
		type = string::literal("Marker");
		break;
	case gl_debug_type::push_group:
		type = string::literal("Push Group");
		break;
	case gl_debug_type::pop_group:
		type = string::literal("Pop Group");
		break;
	case gl_debug_type::other:
		type = string::literal("Other");
		break;
	}

	switch(severity) {
	case gl_debug_severity::high:
		LOG_ERR_F("HIGH OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case gl_debug_severity::medium:
		LOG_WARN_F("MED OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case gl_debug_severity::low:
		LOG_WARN_F("LOW OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case gl_debug_severity::notification:
		// TODO(max): maybe re-enable when we stop updating the buffers every frame
		// LOG_OGL_F("NOTF OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	}
}

void ogl_load_global_funcs() { PROF

	glEnable(gl_capability::debug_output);
	glEnable(gl_capability::debug_output_synchronous);

	glDebugMessageCallback 	= (glDebugMessageCallback_t) global_api->platform_get_glproc(string::literal("glDebugMessageCallback"));
	glDebugMessageInsert 	= (glDebugMessageInsert_t) 	 global_api->platform_get_glproc(string::literal("glDebugMessageInsert"));
	glDebugMessageControl 	= (glDebugMessageControl_t)  global_api->platform_get_glproc(string::literal("glDebugMessageControl"));

	glAttachShader       = (glAttachShader_t)  global_api->platform_get_glproc(string::literal("glAttachShader"));
	glCompileShader      = (glCompileShader_t) global_api->platform_get_glproc(string::literal("glCompileShader"));
	glCreateProgram      = (glCreateProgram_t) global_api->platform_get_glproc(string::literal("glCreateProgram"));
	glCreateShader       = (glCreateShader_t)  global_api->platform_get_glproc(string::literal("glCreateShader"));
	glDeleteProgram      = (glDeleteProgram_t) global_api->platform_get_glproc(string::literal("glDeleteProgram"));
	glDeleteShader       = (glDeleteShader_t)  global_api->platform_get_glproc(string::literal("glDeleteShader"));
	glLinkProgram        = (glLinkProgram_t)   global_api->platform_get_glproc(string::literal("glLinkProgram"));
	glShaderSource       = (glShaderSource_t)  global_api->platform_get_glproc(string::literal("glShaderSource"));
	glUseProgram         = (glUseProgram_t)    global_api->platform_get_glproc(string::literal("glUseProgram"));
	glGetUniformLocation = (glGetUniformLocation_t) global_api->platform_get_glproc(string::literal("glGetUniformLocation"));
	glUniformMatrix4fv   = (glUniformMatrix4fv_t)   global_api->platform_get_glproc(string::literal("glUniformMatrix4fv"));

	glGenerateMipmap  = (glGenerateMipmap_t)  global_api->platform_get_glproc(string::literal("glGenerateMipmap"));
	glActiveTexture   = (glActiveTexture_t)   global_api->platform_get_glproc(string::literal("glActiveTexture"));
	glCreateTextures  = (glCreateTextures_t)  global_api->platform_get_glproc(string::literal("glCreateTextures"));
	glBindTextureUnit = (glBindTextureUnit_t) global_api->platform_get_glproc(string::literal("glBindTextureUnit"));
	glTexParameteriv  = (glTexParameteriv_t)  global_api->platform_get_glproc(string::literal("glTexParameterIiv"));

	glBindVertexArray    = (glBindVertexArray_t)    global_api->platform_get_glproc(string::literal("glBindVertexArray"));
	glDeleteVertexArrays = (glDeleteVertexArrays_t) global_api->platform_get_glproc(string::literal("glDeleteVertexArrays"));
	glGenVertexArrays    = (glGenVertexArrays_t)    global_api->platform_get_glproc(string::literal("glGenVertexArrays"));

	glBindBuffer    = (glBindBuffer_t)    global_api->platform_get_glproc(string::literal("glBindBuffer"));
	glDeleteBuffers = (glDeleteBuffers_t) global_api->platform_get_glproc(string::literal("glDeleteBuffers"));
	glGenBuffers    = (glGenBuffers_t)    global_api->platform_get_glproc(string::literal("glGenBuffers"));
	glBufferData	= (glBufferData_t)    global_api->platform_get_glproc(string::literal("glBufferData"));

	glVertexAttribPointer 	  = (glVertexAttribPointer_t) 	  global_api->platform_get_glproc(string::literal("glVertexAttribPointer"));
	glEnableVertexAttribArray = (glEnableVertexAttribArray_t) global_api->platform_get_glproc(string::literal("glEnableVertexAttribArray"));

	glDebugMessageCallback(debug_proc, null);
	glDebugMessageControl(gl_debug_source::dont_care, gl_debug_type::dont_care, gl_debug_severity::dont_care, 0, null, gl_bool::_true);
}

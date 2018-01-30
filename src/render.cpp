
v2 size_text(asset* font, string text_utf8, f32 point) { PROF

	v2 ret;

	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		glyph_data glyph = font->font.get_glyph(codepoint);
		ret.x += scale * glyph.advance;
	}

	ret.y = scale * font->font.linedist;
	return ret;
}

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

		source.destroy(alloc);
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
	ret.send_uniforms.set(uniforms);

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
	check_compile(vertex.path, h_vertex);

	glCompileShader(h_fragment);
	check_compile(fragment.path, h_fragment);

	glAttachShader(handle, h_vertex);
	glAttachShader(handle, h_fragment);
	glLinkProgram(handle);

	glDeleteShader(h_vertex);
	glDeleteShader(h_fragment);
}

bool shader_program::check_compile(string name, GLuint shader) { PROF

	GLint isCompiled = 0;
	glGetShaderiv(shader, gl_shader_param::compile_status, &isCompiled);
	if(isCompiled == (GLint)gl_bool::_false) {
		
		GLint len = 0;
		glGetShaderiv(shader, gl_shader_param::info_log_length, &len);

		char* msg = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &len, msg);

		LOG_WARN_F("Shader % failed to compile: %", name, string::from_c_str(msg));
		free(msg);

		return false;
	}

	return true;
}

bool shader_program::refresh() { PROF

	if(vertex.refresh() || fragment.refresh()) {

		glUseProgram(0);
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

	glUseProgram(0);
	glDeleteProgram(handle);
}

void ogl_manager::try_reload_programs() { PROF
	FORMAP(it, commands) {
		if(it->value.shader.refresh()) {
			LOG_DEBUG_F("Reloaded program % with files %, %", it->key, it->value.shader.vertex.path, it->value.shader.fragment.path);
		}
	}
	dbg_shader.refresh();
}

ogl_manager ogl_manager::make(platform_window* win, allocator* a) { PROF

	ogl_manager ret;

	ret.win = win;
	ret.alloc = a;
	ret.textures = map<texture_id, texture>::make(32, a);
	ret.commands = map<render_command_type, draw_context>::make(32, a);

	ret.load_global_funcs();
	ret.info = ogl_info::make(ret.alloc);
	ret.check_version_and_extensions();

	_type_info* info = TYPEINFO(render_command_type);
	PUSH_ALLOC(a) {
		for(u32 i = 0; i < info->_enum.member_count; i++) {
			
			string member = info->_enum.member_names[i];
			if(member == "none"_) continue;

			string buffer_func = string::makef("buffers_%"_, member);
			string run_func = string::makef("run_%"_, member);
			string uniforms_func = string::makef("uniforms_%"_, member);
			string vertex = string::makef("shaders/%.v"_, member);
			string fragment = string::makef("shaders/%.f"_, member);

			ret.add_command_ctx((render_command_type)info->_enum.member_values[i], FPTR_STR(buffer_func), FPTR_STR(run_func), vertex, fragment, FPTR_STR(uniforms_func));

			buffer_func.destroy();
			run_func.destroy();
			uniforms_func.destroy();
			vertex.destroy();
			fragment.destroy();
		}
	} POP_ALLOC();

	ret.dbg_shader = shader_program::make("shaders/dbg.v"_,"shaders/dbg.f"_,FPTR(uniforms_dbg),a);

	LOG_DEBUG_F("GL: %", ret.info);

	return ret;
}

void ogl_manager::destroy() { PROF

	FORMAP(it, commands) {
		it->value.shader.destroy();
	}
	FORMAP(it, textures) {
		it->value.destroy();
	}

	dbg_shader.destroy();
	textures.destroy();
	commands.destroy();
	info.destroy();

	check_leaked_handles();
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
	gl_tex_swizzle swizzle[] = {gl_tex_swizzle::red, gl_tex_swizzle::red, gl_tex_swizzle::red, gl_tex_swizzle::red};
	glTexParameteriv(gl_tex_target::_2D, gl_tex_param::swizzle_rgba, (GLint*)swizzle);

	glGenerateMipmap(gl_tex_target::_2D);

	glBindTextureUnit(0, 0);
}

void texture::load_bitmap_from_font(asset_store* as, string name) { PROF

	asset* a = as->get(name);

	LOG_DEBUG_ASSERT(a->type == asset_type::font);

	glBindTextureUnit(0, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, a->font.width, a->font.height, 0, gl_pixel_data_format::red, gl_pixel_data_type::unsigned_byte, a->mem);
	gl_tex_swizzle swizzle[] = {gl_tex_swizzle::red, gl_tex_swizzle::red, gl_tex_swizzle::red, gl_tex_swizzle::red};
	glTexParameteriv(gl_tex_target::_2D, gl_tex_param::swizzle_rgba, (GLint*)swizzle);

	glGenerateMipmap(gl_tex_target::_2D);

	glBindTextureUnit(0, 0);
}

void texture::load_bitmap(asset_store* as, string name) { PROF

	asset* a = as->get(name);

	LOG_DEBUG_ASSERT(a->type == asset_type::bitmap);

	glBindTextureUnit(0, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	
	glGenerateMipmap(gl_tex_target::_2D);

	glBindTextureUnit(0, 0);
}

void texture::destroy() { PROF

	glDeleteTextures(1, &handle);
}

void ogl_manager::add_command_ctx(render_command_type type, _FPTR* buffers, _FPTR* run, string v, string f, _FPTR* uniforms) { PROF

	draw_context d;

	d.send_buffers.set(buffers);
	d.run.set(run);
	d.shader = shader_program::make(v, f, uniforms, alloc);
	LOG_DEBUG_F("Loaded shader from % and %", v, f);

	commands.insert(type, d);
}

draw_context* ogl_manager::get_command_ctx(render_command_type id) { PROF

	draw_context* d = commands.try_get(id);

	if(!d) {
		LOG_ERR_F("Failed to retrieve context %", id);
		return null;
	}

	glUseProgram(d->shader.handle);
	
	return d;
}

void ogl_manager::execute_command_list(render_command_list* rcl) { PROF

	glEnable(gl_capability::blend);
	glEnable(gl_capability::scissor_test);
	glBlendFunc(gl_blend_factor::one, gl_blend_factor::one_minus_src_alpha);

	FORVEC(cmd, rcl->commands) {

		cmd_set_settings(cmd);

		draw_context* d = get_command_ctx(cmd->cmd);

		d->send_buffers(cmd);
		d->shader.send_uniforms(&d->shader, cmd, rcl);

		select_texture(cmd->texture);
		d->run(cmd);
	}

	glDisable(gl_capability::scissor_test);
}

void ogl_manager::cmd_set_settings(render_command* cmd) {

	ur2 viewport = roundR2(cmd->viewport), scissor = roundR2(cmd->scissor);

	if(viewport.w && viewport.h)
		glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
	else
		glViewport(0, 0, win->w, win->h);

	if(scissor.w && scissor.h)
		glScissor(scissor.x, win->h - scissor.y - scissor.h, scissor.w, scissor.h);
	else
		glScissor(0, 0, win->w, win->h);
}

// temporary and inefficient texture render
void ogl_manager::dbg_render_texture_fullscreen(texture_id id) { PROF

	f32 data[] = {
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

	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, 4 * sizeof(f32), (void*)0);	
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));

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
		source = "OpenGL API"_;
		break;
	case gl_debug_source::window_system:
		source = "Window System"_;
		break;
	case gl_debug_source::shader_compiler:
		source = "Shader Compiler"_;
		break;
	case gl_debug_source::third_party:
		source = "Third Party"_;
		break;
	case gl_debug_source::application:
		source = "Application"_;
		break;
	case gl_debug_source::other:
		source = "Other"_;
		break;
	}

	switch(gltype) {
	case gl_debug_type::error:
		type = "Error"_;
		break;
	case gl_debug_type::deprecated_behavior:
		type = "Deprecated"_;
		break;
	case gl_debug_type::undefined_behavior:
		type = "Undefined Behavior"_;
		break;
	case gl_debug_type::portability:
		type = "Portability"_;
		break;
	case gl_debug_type::performance:
		type = "Performance"_;
		break;
	case gl_debug_type::marker:
		type = "Marker"_;
		break;
	case gl_debug_type::push_group:
		type = "Push Group"_;
		break;
	case gl_debug_type::pop_group:
		type = "Pop Group"_;
		break;
	case gl_debug_type::other:
		type = "Other"_;
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

ogl_info ogl_info::make(allocator* a) { PROF

	ogl_info ret;

	ret.version  = string::from_c_str((char*)glGetString(gl_info::version));
	ret.renderer = string::from_c_str((char*)glGetString(gl_info::renderer));
	ret.vendor   = string::from_c_str((char*)glGetString(gl_info::vendor));
	
	ret.shader_version = string::from_c_str((char*)glGetString(gl_info::shading_language_version));
	
	ret.extensions = vector<string>::make(128, a);

	i32 num_extensions = 0;
	glGetIntegerv(gl_get::num_extensions, &num_extensions);

	for(i32 i = 0; i < num_extensions; i++) {

		string ext = string::from_c_str((char*)glGetStringi(gl_info::extensions, i));
		ret.extensions.push(ext);
	}

	glGetIntegerv(gl_get::major_version, &ret.major);
	glGetIntegerv(gl_get::minor_version, &ret.minor);
	glGetIntegerv(gl_get::max_texture_size, &ret.max_texture_size);
	glGetIntegerv(gl_get::max_array_texture_layers, &ret.max_texture_layers);	

	return ret;
}

void ogl_manager::check_version_and_extensions() {

	bool good = true;

	good = good && info.major >= 3 && info.minor >= 3;

	if(good) {
		LOG_INFO("OpenGL features supported!");
	} else {
		LOG_FATAL_F("Unsupported OpenGL features - info: %", info);
	}
}

void ogl_info::destroy() { PROF

	extensions.destroy();
}

void ogl_manager::load_global_funcs() { PROF

	#define GL_IS_LOAD(name) name = (glIs_t)global_api->platform_get_glproc(#name##_); \
							 if(!name) LOG_WARN_F("Failed to load GL function %", #name##_);
	#define GL_LOAD(name) name = (name##_t)global_api->platform_get_glproc(#name##_); \
						  if(!name) LOG_WARN_F("Failed to load GL function %", #name##_);

	GL_IS_LOAD(glIsTexture);
	GL_IS_LOAD(glIsBuffer);
	GL_IS_LOAD(glIsFramebuffer);
	GL_IS_LOAD(glIsRenderbuffer);
	GL_IS_LOAD(glIsVertexArray);
	GL_IS_LOAD(glIsShader);
	GL_IS_LOAD(glIsProgram);
	GL_IS_LOAD(glIsProgramPipeline);
	GL_IS_LOAD(glIsQuery);

	GL_LOAD(glDebugMessageCallback);
	GL_LOAD(glDebugMessageInsert);
	GL_LOAD(glDebugMessageControl);
	GL_LOAD(glAttachShader);
	GL_LOAD(glCompileShader);
	GL_LOAD(glCreateProgram);
	GL_LOAD(glCreateShader);
	GL_LOAD(glDeleteProgram);
	GL_LOAD(glDeleteShader);
	GL_LOAD(glLinkProgram);
	GL_LOAD(glShaderSource);
	GL_LOAD(glUseProgram);
	GL_LOAD(glGetUniformLocation);
	GL_LOAD(glUniformMatrix4fv);
	GL_LOAD(glGetShaderiv);
	GL_LOAD(glGetShaderInfoLog);
	GL_LOAD(glGenerateMipmap);
	GL_LOAD(glActiveTexture);
	GL_LOAD(glCreateTextures);
	GL_LOAD(glBindTextureUnit);
	GL_LOAD(glTexParameterIiv);
	GL_LOAD(glBindVertexArray);
	GL_LOAD(glDeleteVertexArrays);
	GL_LOAD(glGenVertexArrays);
	GL_LOAD(glBindBuffer);
	GL_LOAD(glDeleteBuffers);
	GL_LOAD(glGenBuffers);
	GL_LOAD(glBufferData);
	GL_LOAD(glVertexAttribPointer);
	GL_LOAD(glEnableVertexAttribArray);
	GL_LOAD(glGetShaderSource);
	GL_LOAD(glDrawElementsBaseVertex);
	GL_LOAD(glDrawElementsInstanced);
	GL_LOAD(glDrawElementsInstancedBaseVertex);
	GL_LOAD(glVertexAttribDivisor);
	GL_LOAD(glVertexAttribIPointer);

	GL_LOAD(glGetStringi);
	GL_LOAD(glGetInteger64v);
	GL_LOAD(glGetBooleani_v);
	GL_LOAD(glGetDoublei_v);
	GL_LOAD(glGetFloati_v);
	GL_LOAD(glGetIntegeri_v);
	GL_LOAD(glGetInteger64i_v);

	#undef GL_LOAD
	#undef GL_IS_LOAD

	glEnable(gl_capability::debug_output);
	glEnable(gl_capability::debug_output_synchronous);
	glDebugMessageCallback(debug_proc, null);
	glDebugMessageControl(gl_debug_source::dont_care, gl_debug_type::dont_care, gl_debug_severity::dont_care, 0, null, gl_bool::_true);
}

void ogl_manager::check_leaked_handles() {

	#define GL_CHECK(type) if(glIs##type(i) == gl_bool::_true) { LOG_WARN_F("Leaked OpenGL handle % of type %", i, #type##_); leaked = true;}

	bool leaked = false;
	for(GLuint i = 0; i < 10000; i++) {
		GL_CHECK(Texture);
		GL_CHECK(Buffer);
		GL_CHECK(Framebuffer);
		GL_CHECK(Renderbuffer);
		GL_CHECK(VertexArray);
		GL_CHECK(Program);
		GL_CHECK(ProgramPipeline);
		GL_CHECK(Query);

		if(glIsShader(i) == gl_bool::_true) {

			leaked = true;
			GLint shader_len = 0;
			glGetShaderiv(i, gl_shader_param::shader_source_length, &shader_len);

			GLchar* shader = (GLchar*)malloc(shader_len);
			glGetShaderSource(i, shader_len, null, shader);

			string shader_str = string::from_c_str(shader);
			
			LOG_WARN_F("Leaked OpenGL shader %, source %", i, shader_str); 

			free(shader);
		}
	}

	if(!leaked) {
		LOG_INFO("No OpenGL Objects Leaked!");
	}

	#undef GL_CHECK
}

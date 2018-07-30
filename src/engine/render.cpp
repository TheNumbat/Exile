
v2 size_text(asset* font, string text_utf8, f32 point) { PROF

	v2 ret;

	f32 scale = point / font->raster_font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		glyph_data glyph = font->raster_font.get_glyph(codepoint);
		ret.x += scale * glyph.advance;
	}

	ret.y = scale * font->raster_font.linedist;
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
		error = global_api->create_file(&source_file, path, platform_file_open_op::existing);
	} while (error.error == PLATFORM_SHARING_ERROR && itr < 100000);

	if(!error.good) {
		LOG_ERR_F("Failed to load shader source %"_, path);
		CHECKED(close_file, &source_file);
		return;
	}

	u32 len = global_api->file_size(&source_file);
	source = string::make(len + 1, alloc);
	source.c_str[len] = 0;
	CHECKED(read_file, &source_file, (void*)source.c_str, len);

	CHECKED(close_file, &source_file);

	CHECKED(get_file_attributes, &last_attrib, path);
}

void shader_source::destroy() { PROF

	if(source.c_str)
		source.destroy(alloc);
	if(path.c_str)
		path.destroy(alloc);
}

bool shader_source::try_refresh() { PROF

	if(!path.c_str) return false;

	platform_file_attributes new_attrib;
	
	CHECKED(get_file_attributes, &new_attrib, path);	
	
	if(global_api->test_file_written(&last_attrib, &new_attrib)) {

		source.destroy(alloc);
		load();

		return true;
	}

	return false;
}

shader_program shader_program::make(string vert, string frag, string geom, _FPTR* uniforms, allocator* a) { PROF

	shader_program ret;

	ret.vertex = shader_source::make(vert, a);
	ret.fragment = shader_source::make(frag, a);
	ret.handle = glCreateProgram();
	ret.send_uniforms.set(uniforms);

	if(geom.c_str)
		ret.geometry = shader_source::make(geom, a);

	ret.compile();

	return ret;
}

void shader_program::compile() { PROF

	bool do_geometry = geometry.path.c_str != null; // NOTE(max): fix?

	GLuint h_vertex = 0, h_fragment = 0, h_geometry = 0;

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

	if(do_geometry) {
		
		h_geometry = glCreateShader(gl_shader_type::geometry);

		glShaderSource(h_geometry, 1, &geometry.source.c_str, null);
		glCompileShader(h_geometry);

		check_compile(geometry.path, h_geometry);

		glAttachShader(handle, h_geometry);
	}

	glLinkProgram(handle);

	glDeleteShader(h_vertex);
	glDeleteShader(h_fragment);

	if(do_geometry) {
		glDeleteShader(h_geometry);
	}
}

bool shader_program::check_compile(string name, GLuint shader) { PROF

	GLint isCompiled = 0;
	glGetShaderiv(shader, gl_shader_param::compile_status, &isCompiled);
	if(isCompiled == (GLint)gl_bool::_false) {
		
		GLint len = 0;
		glGetShaderiv(shader, gl_shader_param::info_log_length, &len);

		char* msg = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &len, msg);

		LOG_WARN_F("Shader % failed to compile: %"_, name, string::from_c_str(msg));
		free(msg, len);

		return false;
	}

	return true;
}

void shader_program::gl_destroy() { PROF 

	glUseProgram(0);
	glDeleteProgram(handle);
	handle = 0;
}

void shader_program::recreate() { PROF 

	handle = glCreateProgram();
	compile();

	if(geometry.path)
		LOG_DEBUG_F("Recreated program % with files %, %, %"_, handle, vertex.path, geometry.path, fragment.path);
	else
		LOG_DEBUG_F("Recreated program % with files %, %"_, handle, vertex.path, fragment.path);
}

bool shader_program::try_refresh() { PROF

	if(vertex.try_refresh() || fragment.try_refresh() || geometry.try_refresh()) {

		gl_destroy();
		recreate();

		return true;
	}

	return false;
}

void shader_program::destroy() { PROF

	vertex.destroy();
	fragment.destroy();
	geometry.destroy();

	gl_destroy();
}

void ogl_manager::gl_begin_reload() { PROF

	FORMAP(it, objects) {
		it->value.destroy();
	}
	FORMAP(it, commands) {
		it->value.shader.gl_destroy();
	}
	dbg_shader.gl_destroy();
	FORMAP(it, textures) {
		it->value.gl_destroy();
	}

	info.destroy();
	check_leaked_handles();
}

void ogl_manager::gl_end_reload() { PROF

	load_global_funcs();
	info = ogl_info::make(alloc);

	FORMAP(it, commands) {
		if(!it->value.compat(&info)) {	
			LOG_WARN_F("Render command % failed compatibility check!!!"_, it->key);
			continue;
		}

		it->value.shader.recreate();
	}
	dbg_shader.recreate();
	FORMAP(it, textures) {
		it->value.recreate();
	}
	FORMAP(it, objects) {
		it->value.recreate();
	}

	glBlendFunc(gl_blend_factor::one, gl_blend_factor::one_minus_src_alpha);
	glDepthFunc(gl_depth_factor::lequal);
}

void ogl_manager::reload_texture_assets() { PROF

	FORMAP(it, textures) {
		it->value.reload_data();
	}
}

void ogl_manager::try_reload_programs() { PROF
	FORMAP(it, commands) {
		if(it->value.shader.try_refresh()) {

			if(it->value.shader.geometry.path)
				LOG_DEBUG_F("Reloaded program % with files %, %, %"_, it->key, it->value.shader.vertex.path, it->value.shader.geometry.path, it->value.shader.fragment.path);
			else
				LOG_DEBUG_F("Reloaded program % with files %, %"_, it->key, it->value.shader.vertex.path, it->value.shader.fragment.path);
		}
	}
	dbg_shader.try_refresh();
}

ogl_manager ogl_manager::make(platform_window* win, allocator* a) { PROF

	ogl_manager ret;

	ret.win = win;
	ret.alloc = a;
	
	ret.textures = map<texture_id, texture>::make(32, a);
	ret.objects = map<gpu_object_id, gpu_object>::make(256, a);

	ret.commands = map<u16, draw_context>::make(32, a);
	
	ret.command_settings = stack<cmd_settings>::make(4, a);
	ret.command_settings.push(cmd_settings());

	ret.load_global_funcs();
	ret.info = ogl_info::make(ret.alloc);
	LOG_DEBUG_F("GL %.% %"_, ret.info.major, ret.info.minor, ret.info.renderer);

	ret.dbg_shader = shader_program::make("shaders/dbg.v"_,"shaders/dbg.f"_, {}, FPTR(uniforms_dbg), a);

	glBlendFunc(gl_blend_factor::one, gl_blend_factor::one_minus_src_alpha);
	glDepthFunc(gl_depth_factor::lequal);

	return ret;
}

gpu_object gpu_object::make() { PROF

	gpu_object ret;
	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(5, ret.vbos);
	return ret;
}
 
void gpu_object::recreate() { PROF

	glGenVertexArrays(1, &vao);
	glGenBuffers(5, vbos);
	
	glBindVertexArray(vao);
	setup(this);
	update(this, data, true);

	LOG_DEBUG_F("Recreated gpu object %"_, id);
}

void gpu_object::destroy() { PROF

	glDeleteBuffers(5, vbos);
	glDeleteVertexArrays(1, &vao);
}

gpu_object_id ogl_manager::add_object(_FPTR* setup, _FPTR* update, void* cpu_data) { PROF

	gpu_object obj = gpu_object::make();
	
	obj.id = next_gpu_id;
	obj.data = cpu_data;
	obj.setup.set(setup);
	obj.update.set(update);

	glBindVertexArray(obj.vao);
	obj.setup(&obj);

	objects.insert(obj.id, obj);

	next_gpu_id++;
	return next_gpu_id - 1;
}

void ogl_manager::destroy_object(gpu_object_id id) { PROF

	gpu_object* obj = objects.try_get(id);
	LOG_DEBUG_ASSERT(obj);

	if(obj) {
		obj->destroy();
		objects.erase(id);
	}
}

gpu_object* ogl_manager::get_object(gpu_object_id id) { PROF
	return objects.try_get(id);
}

gpu_object* ogl_manager::select_object(gpu_object_id id) { PROF

	gpu_object* obj = objects.try_get(id);

	if(!obj) {
		LOG_WARN_F("Failed to find object ID %!!!"_, id);
		return null;
	}

	glBindVertexArray(obj->vao);
	obj->update(obj, obj->data, false);

	return obj;
}


void ogl_manager::destroy() { PROF

	FORMAP(it, commands) {
		it->value.shader.destroy();
	}
	FORMAP(it, textures) {
		it->value.destroy(alloc);
	}
	FORMAP(it, objects) {
		it->value.destroy();
	}

	dbg_shader.destroy();
	textures.destroy();
	commands.destroy();
	objects.destroy();
	info.destroy();
	command_settings.destroy();

	check_leaked_handles();
} 

texture_id ogl_manager::add_texture_from_font(asset_store* as, string name, texture_wrap wrap, bool pixelated) { PROF

	texture t = texture::make_rf(wrap, pixelated, settings.anisotropy);
	t.id = next_texture_id;

	t.rf_info.load(t.handle, as, name);

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created texture % from font asset %"_, next_texture_id, name);

	next_texture_id++;
	return next_texture_id - 1;
}

texture_id ogl_manager::add_texture(asset_store* as, string name, texture_wrap wrap, bool pixelated) { PROF

	texture t = texture::make_bmp(wrap, pixelated, settings.anisotropy);
	t.id = next_texture_id;

	t.bmp_info.load(t.handle, as, name);

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created texture % from bitmap asset %"_, next_texture_id, name);

	next_texture_id++;
	return next_texture_id - 1;
}

texture_id ogl_manager::add_cubemap(asset_store* as, string name) {

	texture t = texture::make_cube(texture_wrap::repeat, false, settings.anisotropy);
	t.id = next_texture_id;

	t.cube_info.load_single(t.handle, as, name);

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created cubemap %"_, next_texture_id);

	next_texture_id++;
	return next_texture_id - 1;
}

i32 ogl_manager::get_layers(texture_id tex) { PROF 

	texture* t = textures.try_get(tex);

	LOG_DEBUG_ASSERT(t);
	LOG_DEBUG_ASSERT(t->type == texture_type::array);

	return t->array_info.current_layer;
}

texture_id ogl_manager::begin_tex_array(iv3 dim, texture_wrap wrap, bool pixelated, u32 offset) { PROF

	texture t = texture::make_array(dim, offset, wrap, pixelated, settings.anisotropy, alloc);
	t.id = next_texture_id;

	textures.insert(next_texture_id, t);

	LOG_DEBUG_F("Created texture array %"_, next_texture_id);

	next_texture_id++;
	return next_texture_id - 1;
}

void ogl_manager::push_tex_array(texture_id tex, asset_store* as, string name) { PROF

	texture* t = textures.try_get(tex);

	LOG_DEBUG_ASSERT(t);
	LOG_DEBUG_ASSERT(t->type == texture_type::array);

	t->array_info.push(t->handle, as, name);

	return;
}

void ogl_manager::destroy_texture(texture_id id) { PROF

	texture* t = textures.try_get(id);

	if(!t) {
		LOG_ERR_F("Failed to find texture %"_, id);
		return;
	}

	glDeleteTextures(1, &t->handle);

	textures.erase(id);
}

texture* ogl_manager::select_texture(u32 unit, texture_id id) { PROF 

	if(id == -1) return null;

	texture* t = textures.try_get(id);

	if(!t) {
		LOG_ERR_F("Failed to retrieve texture %"_, id);
		return null;
	}

	glBindTextureUnit(unit, t->handle);

	return t;
}

void ogl_manager::select_textures(render_command* cmd) { PROF

	select_texture(0, cmd->texture0);
	select_texture(1, cmd->texture1);
}

texture texture::make_bmp(texture_wrap wrap, bool pixelated, f32 aniso) { PROF

	texture ret;

	ret.type = texture_type::bmp;
	ret.gl_type = gl_tex_target::_2D;
	ret.wrap = wrap;
	ret.pixelated = pixelated;
	ret.anisotropy = aniso;
	glGenTextures(1, &ret.handle);

	ret.set_params();

	return ret;
}

texture texture::make_cube(texture_wrap wrap, bool pixelated, f32 aniso) { PROF

	texture ret;

	ret.type = texture_type::cube;
	ret.gl_type = gl_tex_target::cube_map;
	ret.anisotropy = aniso;
	ret.wrap = wrap;
	ret.pixelated = pixelated;

	glGenTextures(1, &ret.handle);

	ret.set_params();

	return ret;	
}

texture texture::make_rf(texture_wrap wrap, bool pixelated, f32 aniso) { PROF

	texture ret;

	ret.type = texture_type::rf;
	ret.gl_type = gl_tex_target::_2D;
	ret.wrap = wrap;
	ret.pixelated = pixelated;
	ret.anisotropy = aniso;
	glGenTextures(1, &ret.handle);

	ret.set_params();

	return ret;
}

texture texture::make_array(iv3 dim, u32 offset, texture_wrap wrap, bool pixelated, f32 aniso, allocator* a) { PROF

	texture ret;

	ret.type = texture_type::array;
	ret.gl_type = gl_tex_target::_2D_array;
	ret.wrap = wrap;
	ret.pixelated = pixelated;
	ret.array_info.dim = dim;
	ret.array_info.layer_offset = offset;
	ret.array_info.current_layer = offset;
	ret.array_info.assets = array<asset_pair>::make(dim.z, a);
	ret.anisotropy = aniso;

	glGenTextures(1, &ret.handle);

	ret.set_params();

	return ret;
}

void texture::set_params() { PROF

	glBindTexture(gl_type, handle);

	if(gl_type == gl_tex_target::cube_map) {
		glTexParameteri(gl_type, gl_tex_param::min_filter, (GLint)gl_tex_filter::linear_mipmap_linear);
		glTexParameteri(gl_type, gl_tex_param::mag_filter, (GLint)gl_tex_filter::linear);

		glTexParameteri(gl_type, gl_tex_param::wrap_r, (GLint)gl_tex_wrap::clamp_to_edge);
		glTexParameteri(gl_type, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::clamp_to_edge);
		glTexParameteri(gl_type, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::clamp_to_edge);
		glBindTexture(gl_type, 0);
		return;
	}

	if(gl_type == gl_tex_target::_2D_array) {
		glTexStorage3D(gl_type, 1, gl_tex_format::rgba8, array_info.dim.x, array_info.dim.y, array_info.dim.z);
	}

	switch(wrap) {
	case texture_wrap::repeat:
		glTexParameteri(gl_type, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::repeat);
		glTexParameteri(gl_type, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::repeat);
		break;
	case texture_wrap::mirror:
		glTexParameteri(gl_type, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::mirrored_repeat);
		glTexParameteri(gl_type, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::mirrored_repeat);
		break;
	case texture_wrap::clamp:
		glTexParameteri(gl_type, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::clamp_to_edge);
		glTexParameteri(gl_type, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::clamp_to_edge);
		break;
	case texture_wrap::clamp_border:
		glTexParameteri(gl_type, gl_tex_param::wrap_s, (GLint)gl_tex_wrap::clamp_to_border);
		glTexParameteri(gl_type, gl_tex_param::wrap_t, (GLint)gl_tex_wrap::clamp_to_border);
		f32 borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(gl_type, gl_tex_param::border_color, borderColor);  
		break;
	}

	if(pixelated) {
		glTexParameteri(gl_type, gl_tex_param::min_filter, (GLint)gl_tex_filter::nearest);
		glTexParameteri(gl_type, gl_tex_param::mag_filter, (GLint)gl_tex_filter::nearest);
	} else {
		glTexParameteri(gl_type, gl_tex_param::min_filter, (GLint)gl_tex_filter::linear_mipmap_linear);
		glTexParameteri(gl_type, gl_tex_param::mag_filter, (GLint)gl_tex_filter::linear);
	}

	if(anisotropy) {
		glTexParameterf(gl_type, gl_tex_param::max_anisotropy, anisotropy);
	}

	glBindTexture(gl_type, 0);
}

void texture_cube_info::load_single(GLuint handle, asset_store* store, string name) { PROF 

	asset* a = store->get(name);

	info.name = name;
	info.store = store;

	LOG_DEBUG_ASSERT(a);
	LOG_DEBUG_ASSERT(a->type == asset_type::bitmap);

	glBindTexture(gl_tex_target::cube_map, handle);

	glTexImage2D(gl_tex_target::cube_map_negative_z, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	glTexImage2D(gl_tex_target::cube_map_positive_z, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	glTexImage2D(gl_tex_target::cube_map_positive_y, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	glTexImage2D(gl_tex_target::cube_map_negative_y, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	glTexImage2D(gl_tex_target::cube_map_negative_x, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	glTexImage2D(gl_tex_target::cube_map_positive_x, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);

	glGenerateMipmap(gl_tex_target::cube_map);

	glBindTexture(gl_tex_target::cube_map, 0);
}

void texture_rf_info::load(GLuint handle, asset_store* as, string name) { PROF

	asset* a = as->get(name);

	info.name = name;
	info.store = as;

	LOG_DEBUG_ASSERT(a);
	LOG_DEBUG_ASSERT(a->type == asset_type::raster_font);

	glBindTexture(gl_tex_target::_2D, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, a->raster_font.width, a->raster_font.height, 0, gl_pixel_data_format::red, gl_pixel_data_type::unsigned_byte, a->mem);
	gl_tex_swizzle swizzle[] = {gl_tex_swizzle::red, gl_tex_swizzle::red, gl_tex_swizzle::red, gl_tex_swizzle::red};
	glTexParameteriv(gl_tex_target::_2D, gl_tex_param::swizzle_rgba, (GLint*)swizzle);

	glGenerateMipmap(gl_tex_target::_2D);

	glBindTexture(gl_tex_target::_2D, 0);
}

void texture_bmp_info::load(GLuint handle, asset_store* as, string name) { PROF

	asset* a = as->get(name);
	
	info.name = name;
	info.store = as;

	LOG_DEBUG_ASSERT(a);
	LOG_DEBUG_ASSERT(a->type == asset_type::bitmap);

	glBindTexture(gl_tex_target::_2D, handle);

	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba8, a->bitmap.width, a->bitmap.height, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	
	glGenerateMipmap(gl_tex_target::_2D);

	glBindTexture(gl_tex_target::_2D, 0);
}

void texture_array_info::push(GLuint handle, asset_store* as, string name) {

	asset* a = as->get(name);

	LOG_DEBUG_ASSERT(a);
	LOG_DEBUG_ASSERT(a->type == asset_type::bitmap);
	LOG_DEBUG_ASSERT(dim.x == a->bitmap.width && dim.y == a->bitmap.height && dim.z != 0);
	LOG_DEBUG_ASSERT(current_layer < dim.z);

	glBindTexture(gl_tex_target::_2D_array, handle);

	glTexSubImage3D(gl_tex_target::_2D_array, 0, 0, 0, current_layer, a->bitmap.width, a->bitmap.height, 1, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, a->mem);
	
	assets.get(current_layer)->name = name;
	assets.get(current_layer)->store = as;
	current_layer++;

	glBindTexture(gl_tex_target::_2D_array, 0);
}

void texture::gl_destroy() { PROF
	
	glDeleteTextures(1, &handle);
}

void texture::recreate() { PROF

	glGenTextures(1, &handle);
	set_params();
	reload_data();

	LOG_DEBUG_F("Recreated texture %"_, id);
}

void texture::reload_data() {

	switch(type) {
	case texture_type::array: {
		array_info.current_layer = array_info.layer_offset;
		
		FORARR(it, array_info.assets) {
			if(it->name.c_str)
				array_info.push(handle, it->store, it->name);
		}
	} break;
	case texture_type::bmp: {
		bmp_info.load(handle, bmp_info.info.store, bmp_info.info.name);
	} break;
	case texture_type::rf: {
		rf_info.load(handle, rf_info.info.store, rf_info.info.name);
	} break;
	case texture_type::cube: {
		cube_info.load_single(handle, cube_info.info.store, cube_info.info.name);
	} break;
	}
}

void texture::destroy(allocator* a) { PROF

	if(type == texture_type::array) {
		array_info.assets.destroy();
	}

	gl_destroy();
}

void ogl_manager::add_command(u16 id, _FPTR* run, _FPTR* uniforms, _FPTR* compat, string v, string f, string g) { PROF

	if(commands.try_get(id)) {
		LOG_ERR_F("Render command id % already in use!!!"_, id);
		return;
	}

	draw_context d;
	d.compat.set(compat);

	if(!d.compat(&info)) {
		
		LOG_WARN_F("Render command % failed compatibility check!!!"_, id);
		return;
	}

	d.run.set(run);
	d.shader = shader_program::make(v, f, g, uniforms, alloc);
	
	if(g)
		LOG_DEBUG_F("Loaded shader from %, %, %"_, v, g, f);
	else 
		LOG_DEBUG_F("Loaded shader from %, %"_, v, f);

	commands.insert(id, d);
	return;
}

draw_context* ogl_manager::select_ctx(u16 id) { PROF

	draw_context* d = commands.try_get(id);

	if(!d) {
		LOG_ERR_F("Failed to retrieve context %"_, id);
		return null;
	}

	PROF_SEC("glUseProgram"_);
	glUseProgram(d->shader.handle);
	PROF_SEC_END();
	
	return d;
}

void ogl_manager::_cmd_push_settings() { PROF

	command_settings.push(cmd_settings());
}

void ogl_manager::_cmd_pop_settings() { PROF

	command_settings.pop();
}

void ogl_manager::_cmd_set_setting(render_setting setting, bool enable) { PROF

	cmd_settings* set = command_settings.top();

	switch(setting) {
	case render_setting::wireframe: set->polygon_line = enable; break;
	case render_setting::depth_test: set->depth_test = enable; break;
	case render_setting::aa_lines: set->line_smooth = enable; break;
	case render_setting::blend: set->blend = enable; break;
	case render_setting::scissor: set->scissor = enable; break;
	case render_setting::cull: set->cull_backface = enable; break;
	case render_setting::msaa: set->multisample = enable; break;
	case render_setting::aa_shading: set->sample_shading = enable; break;
	case render_setting::write_depth: set->depth_mask = enable; break;
	default: break;
	}
}

CALLBACK void ogl_apply(void* e) { PROF

	engine* eng = (engine*)e;
	if(ImGui::Button("Apply Settings")) {

		eng->ogl.apply_settings();
	}
}

void ogl_manager::apply_settings() { PROF

	if(settings.anisotropy != prev_settings.anisotropy) {

		if(settings.anisotropy < 1.0f) settings.anisotropy = 1.0f;
		if(settings.anisotropy > info.max_anisotropy) settings.anisotropy = info.max_anisotropy;

		FORMAP(it, textures) {
			it->value.anisotropy = settings.anisotropy;
			it->value.set_params();
		}

		LOG_DEBUG_F("Applied % anisotropy"_, settings.anisotropy);
	}

	prev_settings = settings;
}

void ogl_manager::_cmd_apply_settings() { PROF

	cmd_settings* set = command_settings.top();

	set->polygon_line 	? glPolygonMode(gl_poly::front_and_back, gl_poly_mode::line) : glPolygonMode(gl_poly::front_and_back, gl_poly_mode::fill);
	set->depth_test 	? glEnable(gl_capability::depth_test) : glDisable(gl_capability::depth_test);
	set->line_smooth 	? glEnable(gl_capability::line_smooth) : glDisable(gl_capability::line_smooth);
	set->blend 			? glEnable(gl_capability::blend) : glDisable(gl_capability::blend);
	set->scissor 		? glEnable(gl_capability::scissor_test) : glDisable(gl_capability::scissor_test);
	set->cull_backface 	? glEnable(gl_capability::cull_face) : glDisable(gl_capability::cull_face);
	set->multisample 	? glEnable(gl_capability::multisample) : glDisable(gl_capability::multisample);
	set->sample_shading	? glEnable(gl_capability::sample_shading) : glDisable(gl_capability::sample_shading);
	set->depth_mask 	? glDepthMask(gl_bool::_true) : glDepthMask(gl_bool::_false);

	if(set->sample_shading && info.check_version(4,0)) {
		glMinSampleShading(1.0f);
	}
}

void ogl_manager::execute_command_list(render_command_list* rcl) { PROF

	FORVEC(cmd, rcl->commands) {

		switch(cmd->cmd) {
		case cmd_push_settings: {
			_cmd_push_settings();
		} break;
		case cmd_pop_settings: {
			_cmd_pop_settings();
		} break;
		case cmd_setting: {
			_cmd_set_setting(cmd->setting, cmd->enable);
		} break;
		default: {
			_cmd_set_settings(cmd);

			select_textures(cmd);
			gpu_object* obj = select_object(cmd->object);

			draw_context* d = select_ctx(cmd->cmd);

			d->shader.send_uniforms(&d->shader, cmd);
			d->run(cmd, obj);
		} break;
		}

		if(cmd->callback) cmd->callback(cmd->param);
	}
}

void ogl_manager::_cmd_set_settings(render_command* cmd) { PROF

	_cmd_apply_settings();

	ur2 viewport = cmd->viewport.to_u(), scissor = cmd->scissor.to_u();

	if(viewport.w && viewport.h)
		glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
	else
		glViewport(0, 0, win->settings.w, win->settings.h);

	if(scissor.w && scissor.h)
		glScissor(scissor.x, win->settings.h - scissor.y - scissor.h, scissor.w, scissor.h);
	else
		glScissor(0, 0, win->settings.w, win->settings.h);
}

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
	select_texture(0, id);

	glViewport(0, 0, win->settings.w, win->settings.h);
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
		// LOG_ERR_F("HIGH OpenGL: % SOURCE: % TYPE: %"_, message, source, type);
		break;
	case gl_debug_severity::medium:
		// LOG_WARN_F("MED OpenGL: % SOURCE: % TYPE: %"_, message, source, type);
		break;
	case gl_debug_severity::low:
		LOG_WARN_F("LOW OpenGL: % SOURCE: % TYPE: %"_, message, source, type);
		break;
	case gl_debug_severity::notification:
		// LOG_OGL_F("NOTF OpenGL: % SOURCE: % TYPE: %"_, message, source, type);
		break;
	}
}

ogl_info ogl_info::make(allocator* a) { PROF

	ogl_info ret;

	ret.version  = string::from_c_str((char*)glGetString(gl_info::version));
	ret.renderer = string::from_c_str((char*)glGetString(gl_info::renderer));
	ret.vendor   = string::from_c_str((char*)glGetString(gl_info::vendor));
	
	ret.shader_version = string::from_c_str((char*)glGetString(gl_info::shading_language_version));
	
	i32 num_extensions = 0;
	glGetIntegerv(gl_get::num_extensions, &num_extensions);

	ret.extensions = vector<string>::make(num_extensions, a);

	for(i32 i = 0; i < num_extensions; i++) {

		string ext = string::from_c_str((char*)glGetStringi(gl_info::extensions, i));
		ret.extensions.push(ext);
	}

	glGetIntegerv(gl_get::major_version, &ret.major);
	glGetIntegerv(gl_get::minor_version, &ret.minor);
	glGetIntegerv(gl_get::max_texture_size, &ret.max_texture_size);
	glGetIntegerv(gl_get::max_array_texture_layers, &ret.max_texture_layers);	
	glGetFloatv(gl_get::max_texture_max_anisotropy, &ret.max_anisotropy);

	return ret;
}

void ogl_info::destroy() { PROF

	extensions.destroy();
}

bool ogl_info::check_version(i32 maj, i32 min) { PROF

	if(major > maj) return true;
	if(major == maj && minor >= min) return true;
	return false; 
}

void ogl_manager::load_global_funcs() { PROF

	#define GL_IS_LOAD(name) name = (glIs_t)global_api->get_glproc(#name##_); \
							 if(!name) LOG_WARN_F("Failed to load GL function %"_, #name##_);
	#define GL_LOAD(name) name = (name##_t)global_api->get_glproc(#name##_); \
						  if(!name) LOG_WARN_F("Failed to load GL function %"_, #name##_);

	GL_IS_LOAD(glIsTexture);
	GL_IS_LOAD(glIsBuffer);
	GL_IS_LOAD(glIsFramebuffer);
	GL_IS_LOAD(glIsRenderbuffer);
	GL_IS_LOAD(glIsVertexArray);
	GL_IS_LOAD(glIsShader);
	GL_IS_LOAD(glIsProgram);
	GL_IS_LOAD(glIsProgramPipeline);
	GL_IS_LOAD(glIsQuery);

	GL_LOAD(glMinSampleShading);
	GL_LOAD(glBlendEquation);
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
	GL_LOAD(glGetAttribLocation);
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
	GL_LOAD(glTexStorage3D);
	GL_LOAD(glTexSubImage3D);
	GL_LOAD(glUniform1f);
	GL_LOAD(glUniform1i);
	GL_LOAD(glUniform4fv);
	GL_LOAD(glBindSampler);

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

	#define GL_CHECK(type) if(glIs##type && glIs##type(i) == gl_bool::_true) { LOG_WARN_F("Leaked OpenGL handle % of type %"_, i, #type##_); leaked = true;}

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
			
			LOG_WARN_F("Leaked OpenGL shader %, source %"_, i, shader_str); 

			free(shader, shader_len);
		}
	}

	if(!leaked) {
		LOG_INFO("No OpenGL Objects Leaked!"_);
	}

	#undef GL_CHECK
}


render_command render_command::make(u16 type) {

	render_command ret;
	ret.cmd = type;
	return ret;
}

render_command render_command::make(u16 type, render_setting setting, bool enable) { PROF

	render_command ret;
	ret.cmd = type;
	ret.setting = setting;
	ret.enable = enable;
	return ret;
}

render_command render_command::make(u16 id, gpu_object_id gpu, u32 key) { PROF

	render_command ret;
	ret.object = gpu;
	ret.cmd = id;
	ret.sort_key = key;
	return ret;
}

bool operator<=(render_command& first, render_command& second) { PROF
	return first.sort_key <= second.sort_key;
}

render_command_list render_command_list::make(allocator* alloc, u32 cmds) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	render_command_list ret;

	ret.alloc = alloc;
	ret.commands = vector<render_command>::make(cmds, alloc);

	return ret;
}

void render_command_list::clear() { PROF

	commands.clear();
}

void render_command_list::destroy() { PROF

	commands.destroy();
	alloc = null;
}

void render_command_list::push_settings() {

	add_command(render_command::make(ogl_manager::cmd_push_settings));
}

void render_command_list::pop_settings() {

	add_command(render_command::make(ogl_manager::cmd_pop_settings));
}

void render_command_list::set_setting(render_setting setting, bool enable) {

	add_command(render_command::make(ogl_manager::cmd_setting, setting, enable));
}

void render_command_list::add_command(render_command rc) { PROF

	commands.push(rc);
}

void render_command_list::sort() { PROF

	commands.stable_sort();
}

void render_camera::update() {
	front.x = cos(RADIANS(pitch)) * cos(RADIANS(yaw));
	front.y = sin(RADIANS(pitch));
	front.z = sin(RADIANS(yaw)) * cos(RADIANS(pitch));
	front = norm(front);
	right = norm(cross(front, {0, 1, 0}));
	up = norm(cross(right, front));
}

void render_camera::move(i32 dx, i32 dy, f32 sens) {
	
	yaw   += dx * sens;
	pitch -= dy * sens;

	if (yaw > 360.0f) yaw -= 360.0f;
	else if (yaw < 0.0f) yaw += 360.0f;

	if (pitch > 89.0f) pitch = 89.0f;
	else if (pitch < -89.0f) pitch = -89.0f;

	update();
}

m4 render_camera::view() {

	switch(mode) {
	case camera_mode::first: {
		return lookAt(pos, pos + front, up);
	} break;
	case camera_mode::third: {
		return lookAt(pos - 2.0f * front + offset3rd, pos + reach * front, up);
	} break;
	}

	return m4::I;
}

m4 render_camera::view_no_translate() {

	switch(mode) {
	case camera_mode::first: {
		return lookAt({}, front, up);
	} break;
	case camera_mode::third: {
		return lookAt({}, reach * front - offset3rd, up);
	} break;
	}

	return m4::I;
}

m4 render_camera::view_pos_origin() {

	switch(mode) {
	case camera_mode::first: {
		return lookAt({}, front, up);
	} break;
	case camera_mode::third: {
		return lookAt(-2.0f * front + offset3rd, reach * front, up);
	} break;
	}

	return m4::I;
}

void render_camera::reset() {

	pos = {};
	pitch = 0.0f; yaw = -45.0f; fov = 60.0f;
	update();
}

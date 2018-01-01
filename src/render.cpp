
mesh_2d_col mesh_2d_col::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_col ret;

	ret.alloc = alloc;

	ret.vertices = vector<v2>::make(verts, alloc);
	ret.colors 	  =	vector<colorf>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(3, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, ret.vbos[1]);
	glVertexAttribPointer(1, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(gl_buf_target::element_array, ret.vbos[2]);

	glBindVertexArray(0);

	return ret;
}

void mesh_2d_col::destroy() { PROF

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(3, vbos);

	vertices.destroy();
	colors.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d_col::clear() { PROF
	vertices.clear();
	colors.clear();
	elements.clear();
}

void mesh_2d_col::push_tri(v2 p1, v2 p2, v2 p3, color c) { PROF

	u32 idx = vertices.size;
	
	vertices.push(p1);
	vertices.push(p2);
	vertices.push(p3);

	DO(3) colors.push(color_to_f(c));

	elements.push(V3u(idx, idx + 1, idx + 2));
}

void mesh_2d_col::push_cutrect(r2 r, f32 round, color c) { PROF

	u32 idx = vertices.size;

	vertices.push(V2(r.x, r.y + round));
	vertices.push(V2(r.x, r.y + r.h - round));
	vertices.push(V2(r.x + round, r.y + r.h));
	vertices.push(V2(r.x + r.w - round, r.y + r.h));
	vertices.push(V2(r.x + r.w, r.y + r.h - round));
	vertices.push(V2(r.x + r.w, r.y + round));
	vertices.push(V2(r.x + r.w - round, r.y));
	vertices.push(V2(r.x + round, r.y));

	colorf cf = color_to_f(c);
	DO(8) colors.push(cf);

	elements.push(V3u(idx, idx + 1, idx + 2));
	elements.push(V3u(idx, idx + 2, idx + 7));
	elements.push(V3u(idx + 7, idx + 2, idx + 3));
	elements.push(V3u(idx + 7, idx + 6, idx + 3));
	elements.push(V3u(idx + 3, idx + 4, idx + 5));
	elements.push(V3u(idx + 3, idx + 5, idx + 6));
}

void mesh_2d_col::push_rect(r2 r, color c) { PROF

	u32 idx = vertices.size;

	vertices.push(V2(r.x, r.y + r.h));	// BLC
	vertices.push(r.xy);				// TLC
	vertices.push(add(r.xy, r.wh));		// BRC
	vertices.push(V2(r.x + r.w, r.y));	// TRC

	colorf cf = color_to_f(c);
	DO(4) colors.push(cf);

	elements.push(V3u(idx, idx + 1, idx + 2));
	elements.push(V3u(idx + 1, idx + 2, idx + 3));
}


mesh_2d_tex mesh_2d_tex::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_tex ret;

	ret.alloc = alloc;

	ret.vertices = vector<v2>::make(verts, alloc);
	ret.texCoords =	vector<v2>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(3, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, ret.vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(gl_buf_target::element_array, ret.vbos[2]);

	glBindVertexArray(0);

	return ret;

}

void mesh_2d_tex::destroy() { PROF

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(3, vbos);

	vertices.destroy();
	texCoords.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d_tex::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
}

mesh_2d_tex_col mesh_2d_tex_col::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_tex_col ret;

	ret.alloc = alloc;

	ret.vertices = vector<v2>::make(verts, alloc);
	ret.texCoords =	vector<v2>::make(verts, alloc);
	ret.colors 	  = vector<colorf>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(4, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, ret.vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(gl_buf_target::array, ret.vbos[2]);
	glVertexAttribPointer(2, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(gl_buf_target::element_array, ret.vbos[3]);

	glBindVertexArray(0);

	return ret;
}

void mesh_2d_tex_col::destroy() { PROF

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(4, vbos);

	vertices.destroy();
	texCoords.destroy();
	colors.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d_tex_col::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
	colors.clear();
}

mesh_3d_tex mesh_3d_tex::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_3d_tex ret;

	ret.alloc = alloc;

	ret.vertices = vector<v3>::make(verts, alloc);
	ret.texCoords = vector<v2>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc);

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(3, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);
	glVertexAttribPointer(0, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, ret.vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(gl_buf_target::element_array, ret.vbos[2]);

	glBindVertexArray(0);

	return ret;
}

void mesh_3d_tex::destroy() { PROF

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(3, vbos);

	vertices.destroy();
	texCoords.destroy();
	elements.destroy();
	alloc = null;	
}

void mesh_3d_tex::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
}

render_command render_command::make(render_command_type type, void* data, u32 key) { PROF

	render_command ret;

	ret.cmd = type;
	ret.data = data;
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

void render_command_list::destroy() { PROF

	commands.destroy();
	alloc = null;
}

void render_command_list::add_command(render_command rc) { PROF

	commands.push(rc);
}

void render_command_list::sort() { PROF

	commands.stable_sort();
}

f32 mesh_2d_tex_col::push_text_line(asset* font, string text_utf8, v2 pos, f32 point, color c) { PROF

	colorf cf = color_to_f(c);
	f32 x = pos.x;
	f32 y = pos.y;
	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}
	y += scale * font->font.linedist;

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		u32 idx = vertices.size;
		glyph_data glyph = font->font.get_glyph(codepoint);

		f32 w = (f32)font->font.width;
		f32 h = (f32)font->font.height;
		v2 tlc = V2(glyph.x1/w, 1.0f - glyph.y1/h);
		v2 brc = V2(glyph.x2/w, 1.0f - glyph.y2/h);
		v2 trc = V2(glyph.x2/w, 1.0f - glyph.y1/h);
		v2 blc = V2(glyph.x1/w, 1.0f - glyph.y2/h);

		vertices.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff2)); 	// BLC
 		vertices.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff1));	// TLC
 		vertices.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff2));	// BRC
 		vertices.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff1));	// TRC

		texCoords.push(blc);
		texCoords.push(tlc);
		texCoords.push(brc);
		texCoords.push(trc);

		DO(4) colors.push(cf);

		elements.push(V3u(idx, idx + 1, idx + 2));
		elements.push(V3u(idx + 1, idx + 2, idx + 3));

		x += scale * glyph.advance;
	}

	return scale * font->font.linedist;
}

void mesh_3d_tex::push_cube(v3 pos, f32 len) {

	f32 len2 = len / 2.0f;
	vertices.push(pos + V3( len2,  len2,  len2));
	vertices.push(pos + V3(-len2,  len2,  len2));
	vertices.push(pos + V3( len2, -len2,  len2));
	vertices.push(pos + V3( len2,  len2, -len2));
	vertices.push(pos + V3(-len2, -len2,  len2));
	vertices.push(pos + V3( len2, -len2, -len2));
	vertices.push(pos + V3(-len2,  len2, -len2));
	vertices.push(pos + V3(-len2, -len2, -len2));

	texCoords.push(V2(0.0f, 0.0f));
	texCoords.push(V2(1.0f, 0.0f));
	texCoords.push(V2(0.0f, 1.0f));
	texCoords.push(V2(0.0f, 0.0f));
	texCoords.push(V2(1.0f, 0.0f));
	texCoords.push(V2(0.0f, 1.0f));
	texCoords.push(V2(1.0f, 0.0f));
	texCoords.push(V2(1.0f, 1.0f));	

	elements.push(V3u(0,2,5));
	elements.push(V3u(0,3,5));
	elements.push(V3u(0,3,6));
	elements.push(V3u(0,1,6));
	elements.push(V3u(1,4,7));
	elements.push(V3u(1,6,7));
	elements.push(V3u(4,2,5));
	elements.push(V3u(4,7,5));
	elements.push(V3u(7,5,3));
	elements.push(V3u(7,6,3));
	elements.push(V3u(0,2,4));
	elements.push(V3u(0,1,4));
}

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
	FORMAP(it, commands) {
		if(it->value.shader.refresh()) {
			LOG_DEBUG_F("Reloaded program % with files %, %", it->key, it->value.shader.vertex.path, it->value.shader.fragment.path);
		}
	}
	dbg_shader.refresh();
}

#define REGISTER_COMMAND(cmd) ret.add_command_ctx(render_command_type::cmd, FPTR(buffers_##cmd), FPTR(run_##cmd), string::literal("shaders/" #cmd ".v"), string::literal("shaders/" #cmd ".f"), FPTR(uniforms_##cmd));

ogl_manager ogl_manager::make(allocator* a) { PROF

	ogl_manager ret;

	ret.alloc = a;
	ret.textures = map<texture_id, texture>::make(32, a);
	ret.commands = map<render_command_type, draw_context>::make(32, a);

	ret.version  = string::from_c_str((char*)glGetString(gl_info::version));
	ret.renderer = string::from_c_str((char*)glGetString(gl_info::renderer));
	ret.vendor   = string::from_c_str((char*)glGetString(gl_info::vendor));

	REGISTER_COMMAND(mesh_2d_col);	
	REGISTER_COMMAND(mesh_2d_tex);
	REGISTER_COMMAND(mesh_2d_tex_col);
	REGISTER_COMMAND(mesh_3d_tex);

	ret.dbg_shader = shader_program::make("shaders/dbg.v"_,"shaders/dbg.f"_,FPTR(uniforms_dbg),a);

	LOG_DEBUG_F("GL version : %", ret.version);
	LOG_DEBUG_F("GL renderer: %", ret.renderer);
	LOG_DEBUG_F("GL vendor  : %", ret.vendor);

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

void ogl_manager::execute_command_list(platform_window* win, render_command_list* rcl) { PROF

	glEnable(gl_capability::blend);
	glEnable(gl_capability::scissor_test);
	glBlendFunc(gl_blend_factor::one, gl_blend_factor::one_minus_src_alpha);

	FORVEC(cmd, rcl->commands) {

		cmd_set_settings(win, cmd);

		draw_context* d = get_command_ctx(cmd->cmd);

		d->send_buffers(cmd);
		d->shader.send_uniforms(&d->shader, cmd, rcl);

		select_texture(cmd->texture);
		d->run(cmd);
	}

	glDisable(gl_capability::scissor_test);
}

void ogl_manager::cmd_set_settings(platform_window* win, render_command* cmd) {

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
void ogl_manager::dbg_render_texture_fullscreen(platform_window* win, texture_id id) { PROF

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

#define GL_LOAD(name) name = (name##_t)global_api->platform_get_glproc(#name##_);
void ogl_load_global_funcs() { PROF

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

	glEnable(gl_capability::debug_output);
	glEnable(gl_capability::debug_output_synchronous);
	glDebugMessageCallback(debug_proc, null);
	glDebugMessageControl(gl_debug_source::dont_care, gl_debug_type::dont_care, gl_debug_severity::dont_care, 0, null, gl_bool::_true);
}




CALLBACK void uniforms_mesh_2d_col(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.v);
}

CALLBACK void uniforms_mesh_2d_tex(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.v);
}

CALLBACK void uniforms_mesh_2d_tex_col(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.v);
}

CALLBACK void uniforms_mesh_3d_tex(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF
	
	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.v);
}

CALLBACK void uniforms_mesh_3d_tex_instanced(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF
	
	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.v);
}


CALLBACK void buffers_mesh_2d_col(render_command* cmd) { PROF

	mesh_2d_col* m = cmd->mesh_2d_col.mesh;
	if(!m->dirty) return;

	glBindVertexArray(m->vao);

	glBindBuffer(gl_buf_target::array, m->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::array, m->vbos[1]);
	glBufferData(gl_buf_target::array, m->colors.size * sizeof(v4), m->colors.size ? m->colors.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::element_array, m->vbos[2]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	glBindVertexArray(0);
	m->dirty = false;
}

CALLBACK void buffers_mesh_2d_tex(render_command* cmd) { PROF

	mesh_2d_tex* m = cmd->mesh_2d_tex.mesh;
	if(!m->dirty) return;

	glBindVertexArray(m->vao);

	glBindBuffer(gl_buf_target::array, m->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::array, m->vbos[1]);
	glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::element_array, m->vbos[2]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	glBindVertexArray(0);
	m->dirty = false;
}

CALLBACK void buffers_mesh_2d_tex_col(render_command* cmd) { PROF

	mesh_2d_tex_col* m = cmd->mesh_2d_tex_col.mesh;
	if(!m->dirty) return;

	glBindVertexArray(m->vao);

	glBindBuffer(gl_buf_target::array, m->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::array, m->vbos[1]);
	glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::array, m->vbos[2]);
	glBufferData(gl_buf_target::array, m->colors.size * sizeof(v4), m->colors.size ? m->colors.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::element_array, m->vbos[3]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	glBindVertexArray(0);
	m->dirty = false;
}

CALLBACK void buffers_mesh_3d_tex(render_command* cmd) { PROF

	mesh_3d_tex* m = cmd->mesh_3d_tex.mesh;
	if(!m->dirty) return;

	glBindVertexArray(m->vao);

	glBindBuffer(gl_buf_target::array, m->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v3), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::array, m->vbos[1]);
	glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::element_array, m->vbos[2]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	glBindVertexArray(0);
	m->dirty = false;
}

CALLBACK void buffers_mesh_3d_tex_instanced(render_command* cmd) { PROF

	mesh_3d_tex* m = cmd->mesh_3d_tex_instanced.data->parent;
	
	glBindVertexArray(m->vao);
	
	if(m->dirty) {

		glBindBuffer(gl_buf_target::array, m->vbos[0]);
		glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v3), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

		glBindBuffer(gl_buf_target::array, m->vbos[1]);
		glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

		glBindBuffer(gl_buf_target::element_array, m->vbos[2]);
		glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);
	}

	mesh_3d_tex_instance_data* data = cmd->mesh_3d_tex_instanced.data;
	if(data->dirty) {

		glBindBuffer(gl_buf_target::array, data->vbo);
		glBufferData(gl_buf_target::array, data->data.size * sizeof(v3), data->data.size ? data->data.memory : null, gl_buf_usage::dynamic_draw);
	}

	glBindVertexArray(0);
	m->dirty = false;
	data->dirty = false;
}



CALLBACK void run_mesh_2d_col(render_command* cmd) { PROF

	mesh_2d_col* m = cmd->mesh_2d_col.mesh;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);
	
	glBindVertexArray(0);
}

CALLBACK void run_mesh_2d_tex(render_command* cmd) { PROF

	mesh_2d_tex* m = cmd->mesh_2d_tex.mesh;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_2d_tex_col(render_command* cmd) { PROF

	mesh_2d_tex_col* m = cmd->mesh_2d_tex_col.mesh;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_3d_tex(render_command* cmd) { PROF

	mesh_3d_tex* m = cmd->mesh_3d_tex.mesh;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_3d_tex_instanced(render_command* cmd) { PROF

	mesh_3d_tex* m = cmd->mesh_3d_tex_instanced.data->parent;
	mesh_3d_tex_instance_data* data = cmd->mesh_3d_tex_instanced.data;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;

	glDrawElementsInstancedBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), data->instances, cmd->offset);

	glBindVertexArray(0);
}

mesh_3d_tex_instance_data mesh_3d_tex_instance_data::make(mesh_3d_tex* parent, u32 instances, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_3d_tex_instance_data ret;

	ret.parent = parent;
	ret.data = vector<v3>::make(instances, alloc);

	glGenBuffers(1, &ret.vbo);

	glBindVertexArray(parent->vao);

	glBindBuffer(gl_buf_target::array, ret.vbo);
	glVertexAttribPointer(2, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	return ret;
}

void mesh_3d_tex_instance_data::destroy() { PROF

	glDeleteBuffers(1, &vbo);

	data.destroy();
}

void mesh_3d_tex_instance_data::clear() { PROF

	data.clear();
	dirty = true;
}

mesh_2d_col mesh_2d_col::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_col ret;

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
}

void mesh_2d_col::clear() { PROF
	vertices.clear();
	colors.clear();
	elements.clear();
	dirty = true;
}

void mesh_2d_col::push_tri(v2 p1, v2 p2, v2 p3, color c) { PROF

	u32 idx = vertices.size;
	
	vertices.push(p1);
	vertices.push(p2);
	vertices.push(p3);

	DO(3) colors.push(color_to_f(c));

	elements.push(V3u(idx, idx + 1, idx + 2));

	dirty = true;
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

	dirty = true;
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

	dirty = true;
}


mesh_2d_tex mesh_2d_tex::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_tex ret;

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
}

void mesh_2d_tex::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();

	dirty = true;
}

mesh_2d_tex_col mesh_2d_tex_col::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_tex_col ret;

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
}

void mesh_3d_tex::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
}

render_command render_command::make(render_command_type type, u32 key) { PROF

	render_command ret;

	ret.cmd = type;
	ret.sort_key = key;

	return ret;
}

bool operator<=(render_command& first, render_command& second) { PROF
	return first.sort_key <= second.sort_key;
}

render_command_mesh_2d_col render_command_mesh_2d_col::make(mesh_2d_col* m) { PROF

	render_command_mesh_2d_col ret;
	ret.mesh = m;
	return ret;
}

render_command_mesh_2d_tex render_command_mesh_2d_tex::make(mesh_2d_tex* m) { PROF

	render_command_mesh_2d_tex ret;
	ret.mesh = m;
	return ret;
}

render_command_mesh_2d_tex_col render_command_mesh_2d_tex_col::make(mesh_2d_tex_col* m) { PROF

	render_command_mesh_2d_tex_col ret;
	ret.mesh = m;
	return ret;
}

render_command_mesh_3d_tex render_command_mesh_3d_tex::make(mesh_3d_tex* m) { PROF

	render_command_mesh_3d_tex ret;
	ret.mesh = m;
	return ret;
}

render_command_mesh_3d_tex_instanced render_command_mesh_3d_tex_instanced::make(mesh_3d_tex* mesh, mesh_3d_tex_instance_data* data) { PROF

	render_command_mesh_3d_tex_instanced ret;
	ret.data = data;
	return ret;
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

	dirty = true;
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

	dirty = true;
}

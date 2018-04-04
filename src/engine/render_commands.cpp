

CALLBACK void uniforms_mesh_2d_col(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_2d_tex(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_2d_tex_col(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_3d_tex(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF
	
	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_3d_tex_instanced(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF
	
	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_chunk(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}


CALLBACK void buffers_mesh_2d_col(render_command* cmd) { PROF

	mesh_2d_col* m = (mesh_2d_col*)cmd->mesh;
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

	mesh_2d_tex* m = (mesh_2d_tex*)cmd->mesh;
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

	mesh_2d_tex_col* m = (mesh_2d_tex_col*)cmd->mesh;
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

	mesh_3d_tex* m = (mesh_3d_tex*)cmd->mesh;
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

	mesh_3d_tex_instance_data* data = (mesh_3d_tex_instance_data*)cmd->mesh;
	mesh_3d_tex* m = data->parent;
	
	glBindVertexArray(m->vao);
	
	if(m->dirty) {

		glBindBuffer(gl_buf_target::array, m->vbos[0]);
		glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v3), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

		glBindBuffer(gl_buf_target::array, m->vbos[1]);
		glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

		glBindBuffer(gl_buf_target::element_array, m->vbos[2]);
		glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);
	}

	if(data->dirty) {

		glBindBuffer(gl_buf_target::array, data->vbo);
		glBufferData(gl_buf_target::array, data->data.size * sizeof(v3), data->data.size ? data->data.memory : null, gl_buf_usage::dynamic_draw);
	}

	glBindVertexArray(0);
	m->dirty = false;
	data->dirty = false;
}

CALLBACK void buffers_mesh_chunk(render_command* cmd) { PROF

	mesh_chunk* m = (mesh_chunk*)cmd->mesh;
	if(!m->dirty) return;

	glBindVertexArray(m->vao);

	glBindBuffer(gl_buf_target::array, m->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(chunk_vertex), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::element_array, m->vbos[1]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	glBindVertexArray(0);

	m->dirty = false;
}


CALLBACK void run_mesh_2d_col(render_command* cmd) { PROF

	mesh_2d_col* m = (mesh_2d_col*)cmd->mesh;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);
	
	glBindVertexArray(0);
}

CALLBACK void run_mesh_2d_tex(render_command* cmd) { PROF

	mesh_2d_tex* m = (mesh_2d_tex*)cmd->mesh;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_2d_tex_col(render_command* cmd) { PROF

	mesh_2d_tex_col* m = (mesh_2d_tex_col*)cmd->mesh;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_3d_tex(render_command* cmd) { PROF

	mesh_3d_tex* m = (mesh_3d_tex*)cmd->mesh;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_3d_tex_instanced(render_command* cmd) { PROF

	mesh_3d_tex_instance_data* data = (mesh_3d_tex_instance_data*)cmd->mesh;
	mesh_3d_tex* m = data->parent;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;

	glDrawElementsInstancedBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), data->instances, cmd->offset);

	glBindVertexArray(0);
}

CALLBACK void run_mesh_chunk(render_command* cmd) { PROF

	mesh_chunk* m = (mesh_chunk*)cmd->mesh;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);
	glEnable(gl_capability::cull_face);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glDisable(gl_capability::cull_face);
	glDisable(gl_capability::depth_test);

	glBindVertexArray(0);
}

CALLBACK bool compat_mesh_2d_col(ogl_info* info) { PROF
	return info->check_version(3, 2);
}
CALLBACK bool compat_mesh_2d_tex(ogl_info* info) { PROF
	return info->check_version(3, 2);
}
CALLBACK bool compat_mesh_2d_tex_col(ogl_info* info) { PROF
	return info->check_version(3, 2);
}
CALLBACK bool compat_mesh_3d_tex(ogl_info* info) { PROF
	return info->check_version(3, 2);
}
CALLBACK bool compat_mesh_3d_tex_instanced(ogl_info* info) { PROF
	return info->check_version(3, 3);
}
CALLBACK bool compat_mesh_chunk(ogl_info* info) { PROF
	return info->check_version(3, 2);
}


inline u32 hash(render_command_type key) { PROF

	return hash(*(u8*)&key);
}

chunk_vertex chunk_vertex::from_vec(v3 v, v3 uv, u8 ao) { PROF

	LOG_DEBUG_ASSERT(v.x >= 0 && v.x < 256);
	LOG_DEBUG_ASSERT(v.y >= 0 && v.y < 4096);
	LOG_DEBUG_ASSERT(v.z >= 0 && v.z < 256);
	LOG_DEBUG_ASSERT(uv.x >= 0 && uv.x < 256);
	LOG_DEBUG_ASSERT(uv.y >= 0 && uv.y < 256);
	LOG_DEBUG_ASSERT(uv.z >= 0 && uv.z < 1024);
	LOG_DEBUG_ASSERT(ao >= 0 && ao < 4);

	chunk_vertex ret;
	ret.x = (u8)v.x;
	ret.z = (u8)v.z;
	ret.y_norm |= (u16)v.y << 4;

	ret.u = (u8)uv.x;
	ret.v = (u8)uv.y;
	ret.ao_t = (u16)uv.z;
	ret.ao_t |= (u16)ao << 12;

	return ret;
}

mesh_chunk mesh_chunk::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_chunk ret;

	ret.vertices = vector<chunk_vertex>::make(verts, alloc);
	ret.elements = vector<uv3>::make(verts, alloc);

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(2, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);

	glVertexAttribIPointer(0, 2, gl_vert_attrib_type::unsigned_int, sizeof(chunk_vertex), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::element_array, ret.vbos[1]);

	glBindVertexArray(0);

	return ret;
}

mesh_chunk mesh_chunk::make_gpu() { PROF

	mesh_chunk ret;

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(2, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);

	glVertexAttribIPointer(0, 2, gl_vert_attrib_type::unsigned_int, sizeof(chunk_vertex), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::element_array, ret.vbos[1]);

	glBindVertexArray(0);

	return ret;
}

mesh_chunk mesh_chunk::make_cpu(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_chunk ret;

	ret.vertices = vector<chunk_vertex>::make(verts, alloc);
	ret.elements = vector<uv3>::make(verts, alloc);

	return ret;
}

void mesh_chunk::swap_mesh(mesh_chunk other) { PROF

	vertices.destroy();
	elements.destroy();

	vertices = other.vertices;
	elements = other.elements;

	dirty = true;
}

void mesh_chunk::destroy() { PROF

	glDeleteBuffers(2, vbos);
	glDeleteVertexArrays(1, &vao);

	vertices.destroy();
	elements.destroy();
}

void mesh_chunk::free_cpu() { PROF

	vertices.resize(0);
	elements.resize(0);
}

void mesh_chunk::clear() { PROF

	vertices.clear();
	elements.clear();

	dirty = true;
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
	instances = 0;
	dirty = true;
}

bool mesh_3d_tex_instance_data::empty() { PROF

	return !data.size;
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

bool mesh_2d_col::empty() { PROF

	return !vertices.size;
}

void mesh_2d_col::push_tri(v2 p1, v2 p2, v2 p3, color c) { PROF

	u32 idx = vertices.size;
	
	vertices.push(p1);
	vertices.push(p2);
	vertices.push(p3);

	DO(3) colors.push(c.to_f());

	elements.push(uv3(idx, idx + 1, idx + 2));

	dirty = true;
}

void mesh_2d_col::push_cutrect(r2 r, f32 round, color c) { PROF

	u32 idx = vertices.size;

	vertices.push(v2(r.x, r.y + round));
	vertices.push(v2(r.x, r.y + r.h - round));
	vertices.push(v2(r.x + round, r.y + r.h));
	vertices.push(v2(r.x + r.w - round, r.y + r.h));
	vertices.push(v2(r.x + r.w, r.y + r.h - round));
	vertices.push(v2(r.x + r.w, r.y + round));
	vertices.push(v2(r.x + r.w - round, r.y));
	vertices.push(v2(r.x + round, r.y));

	colorf cf = c.to_f();
	DO(8) colors.push(cf);

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx, idx + 2, idx + 7));
	elements.push(uv3(idx + 7, idx + 2, idx + 3));
	elements.push(uv3(idx + 7, idx + 6, idx + 3));
	elements.push(uv3(idx + 3, idx + 4, idx + 5));
	elements.push(uv3(idx + 3, idx + 5, idx + 6));

	dirty = true;
}

void mesh_2d_col::push_rect(r2 r, color c) { PROF

	u32 idx = vertices.size;

	vertices.push(v2(r.x, r.y + r.h));	// BLC
	vertices.push(r.xy);				// TLC
	vertices.push(r.xy + r.wh);		 	// BRC
	vertices.push(v2(r.x + r.w, r.y));	// TRC

	DO(4) colors.push(c.to_f());

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx + 1, idx + 2, idx + 3));

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

bool mesh_2d_tex::empty() { PROF

	return !vertices.size;
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

bool mesh_2d_tex_col::empty() { PROF

	return !vertices.size;
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

bool mesh_3d_tex::empty() { PROF

	return !vertices.size;
}

render_command render_command::make(render_command_type type, void* data, u32 key) { PROF

	render_command ret;

	ret.cmd = type;
	ret.sort_key = key;
	ret.mesh = data;

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

	colorf cf = c.to_f();
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
		v2 tlc (glyph.x1/w, 1.0f - glyph.y1/h);
		v2 brc (glyph.x2/w, 1.0f - glyph.y2/h);
		v2 trc (glyph.x2/w, 1.0f - glyph.y1/h);
		v2 blc (glyph.x1/w, 1.0f - glyph.y2/h);

		vertices.push(v2(x + scale*glyph.xoff1, y + scale*glyph.yoff2)); 	// BLC
 		vertices.push(v2(x + scale*glyph.xoff1, y + scale*glyph.yoff1));	// TLC
 		vertices.push(v2(x + scale*glyph.xoff2, y + scale*glyph.yoff2));	// BRC
 		vertices.push(v2(x + scale*glyph.xoff2, y + scale*glyph.yoff1));	// TRC

		texCoords.push(blc);
		texCoords.push(tlc);
		texCoords.push(brc);
		texCoords.push(trc);

		DO(4) colors.push(cf);

		elements.push(uv3(idx, idx + 1, idx + 2));
		elements.push(uv3(idx + 1, idx + 2, idx + 3));

		x += scale * glyph.advance;
	}

	dirty = true;
	return scale * font->font.linedist;
}

void mesh_chunk::quad(v3 p1, v3 p2, v3 p3, v3 p4, v3 uv_ext, bv4 ao) { PROF

	u32 idx = vertices.size;

	vertices.push(chunk_vertex::from_vec(p1, v3(0.0f, 0.0f, uv_ext.z), ao.x));
	vertices.push(chunk_vertex::from_vec(p2, v3(uv_ext.x, 0.0f, uv_ext.z), ao.y));
	vertices.push(chunk_vertex::from_vec(p3, v3(0.0f, uv_ext.y, uv_ext.z), ao.z));
	vertices.push(chunk_vertex::from_vec(p4, v3(uv_ext.x, uv_ext.y, uv_ext.z), ao.w));

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx + 3, idx + 2, idx + 1));

	dirty = true;
}

void mesh_chunk::quad16(v3 p1, v3 p2, v3 p3, v3 p4, v3 uv_ext, bv4 ao) { PROF

	u32 idx = vertices.size;

	vertices.push(chunk_vertex::from_vec(p1 * 16.0f, v3(0.0f, 0.0f, uv_ext.z), ao.x));
	vertices.push(chunk_vertex::from_vec(p2 * 16.0f, v3(uv_ext.x, 0.0f, uv_ext.z), ao.y));
	vertices.push(chunk_vertex::from_vec(p3 * 16.0f, v3(0.0f, uv_ext.y, uv_ext.z), ao.z));
	vertices.push(chunk_vertex::from_vec(p4 * 16.0f, v3(uv_ext.x, uv_ext.y, uv_ext.z), ao.w));

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx + 3, idx + 2, idx + 1));

	dirty = true;
}

void mesh_chunk::cube(v3 pos, f32 len) { PROF

	u32 idx = vertices.size;

	f32 len2 = len / 2.0f;
	pos += {len2, len2, len2};

	vertices.push(chunk_vertex::from_vec(pos + v3( len2,  len2,  len2), v3(0,0,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2,  len2,  len2), v3(1,0,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3( len2, -len2,  len2), v3(0,1,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3( len2,  len2, -len2), v3(0,0,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2, -len2,  len2), v3(1,0,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3( len2, -len2, -len2), v3(0,1,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2,  len2, -len2), v3(1,0,0), 0));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2, -len2, -len2), v3(1,1,0), 0));

	elements.push(uv3(idx + 0, idx + 3, idx + 5));
	elements.push(uv3(idx + 0, idx + 3, idx + 6));
	elements.push(uv3(idx + 0, idx + 1, idx + 6));
	elements.push(uv3(idx + 1, idx + 4, idx + 7));
	elements.push(uv3(idx + 1, idx + 6, idx + 7));
	elements.push(uv3(idx + 4, idx + 2, idx + 5));
	elements.push(uv3(idx + 4, idx + 7, idx + 5));
	elements.push(uv3(idx + 7, idx + 5, idx + 3));
	elements.push(uv3(idx + 7, idx + 6, idx + 3));
	elements.push(uv3(idx + 0, idx + 2, idx + 4));
	elements.push(uv3(idx + 0, idx + 1, idx + 4));

	dirty = true;
}

void mesh_3d_tex::push_cube(v3 pos, f32 len) {

	u32 idx = vertices.size;

	f32 len2 = len / 2.0f;
	vertices.push(pos + v3( len2,  len2,  len2));
	vertices.push(pos + v3(-len2,  len2,  len2));
	vertices.push(pos + v3( len2, -len2,  len2));
	vertices.push(pos + v3( len2,  len2, -len2));
	vertices.push(pos + v3(-len2, -len2,  len2));
	vertices.push(pos + v3( len2, -len2, -len2));
	vertices.push(pos + v3(-len2,  len2, -len2));
	vertices.push(pos + v3(-len2, -len2, -len2));

	texCoords.push(v2(0.0f, 0.0f));
	texCoords.push(v2(1.0f, 0.0f));
	texCoords.push(v2(0.0f, 1.0f));
	texCoords.push(v2(0.0f, 0.0f));
	texCoords.push(v2(1.0f, 0.0f));
	texCoords.push(v2(0.0f, 1.0f));
	texCoords.push(v2(1.0f, 0.0f));
	texCoords.push(v2(1.0f, 1.0f));	

	elements.push(uv3(idx + 0, idx + 3, idx + 6));
	elements.push(uv3(idx + 0, idx + 3, idx + 5));
	elements.push(uv3(idx + 0, idx + 1, idx + 6));
	elements.push(uv3(idx + 1, idx + 4, idx + 7));
	elements.push(uv3(idx + 1, idx + 6, idx + 7));
	elements.push(uv3(idx + 4, idx + 2, idx + 5));
	elements.push(uv3(idx + 4, idx + 7, idx + 5));
	elements.push(uv3(idx + 7, idx + 5, idx + 3));
	elements.push(uv3(idx + 7, idx + 6, idx + 3));
	elements.push(uv3(idx + 0, idx + 2, idx + 4));
	elements.push(uv3(idx + 0, idx + 1, idx + 4));

	dirty = true;
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

	return lookAt(pos, pos + front, up);
}

m4 render_camera::view_no_translate() {

	return lookAt(v3(0.0f, 0.0f, 0.0f), front, up);
}

void render_camera::reset() {

	pos = v3(-5.0f, 128.0f, -5.0f);

	pitch = 00.0f; yaw = -45.0f; fov = 60.0f;
	update();
}

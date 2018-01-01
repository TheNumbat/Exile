
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


CALLBACK void buffers_mesh_2d_col(render_command* cmd) { PROF

	mesh_2d_col* m = cmd->m2dc;
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

	mesh_2d_tex* m = cmd->m2dt;
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

	mesh_2d_tex_col* m = cmd->m2dtc;
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

	mesh_3d_tex* m = cmd->m3dt;
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


CALLBACK void run_mesh_2d_col(render_command* cmd) { PROF

	mesh_2d_col* m = cmd->m2dc;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->num_tris ? cmd->num_tris * 3 : m->elements.size * 3, gl_index_type::unsigned_int, (void*)(u64)(cmd->start_tri * 3));

	glBindVertexArray(0);
}

CALLBACK void run_mesh_2d_tex(render_command* cmd) { PROF

	mesh_2d_tex* m = cmd->m2dt;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->num_tris ? cmd->num_tris * 3 : m->elements.size * 3, gl_index_type::unsigned_int, (void*)(u64)(cmd->start_tri * 3));

	glBindVertexArray(0);
}

CALLBACK void run_mesh_2d_tex_col(render_command* cmd) { PROF

	mesh_2d_tex_col* m = cmd->m2dtc;

	glBindVertexArray(m->vao);

	glDisable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->num_tris ? cmd->num_tris * 3 : m->elements.size * 3, gl_index_type::unsigned_int, (void*)(u64)(cmd->start_tri * 3));

	glBindVertexArray(0);
}

CALLBACK void run_mesh_3d_tex(render_command* cmd) { PROF

	mesh_3d_tex* m = cmd->m3dt;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->num_tris ? cmd->num_tris * 3 : m->elements.size * 3, gl_index_type::unsigned_int, (void*)(u64)(cmd->start_tri * 3));

	glBindVertexArray(0);
}

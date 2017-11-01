
CALLBACK void attribs_mesh_2d_col(draw_context* dc) { PROF

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glVertexAttribPointer(1, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

CALLBACK void attribs_mesh_2d_tex(draw_context* dc) { PROF

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);	
}

CALLBACK void attribs_mesh_2d_tex_col(draw_context* dc) { PROF

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);
	glVertexAttribPointer(2, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[3]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);	
	glEnableVertexAttribArray(2);
}

CALLBACK void attribs_mesh_3d_tex(draw_context* dc) { PROF

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glVertexAttribPointer(0, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}


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


CALLBACK void buffers_mesh_2d_col(draw_context* dc, render_command* cmd) { PROF

	mesh_2d_col* m = cmd->m2dc;

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glBufferData(gl_buf_target::array, m->colors.size * sizeof(v4), m->colors.size ? m->colors.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::stream_draw);
}

CALLBACK void buffers_mesh_2d_tex(draw_context* dc, render_command* cmd) { PROF

	mesh_2d_tex* m = cmd->m2dt;

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::stream_draw);
}

CALLBACK void buffers_mesh_2d_tex_col(draw_context* dc, render_command* cmd) { PROF

	mesh_2d_tex_col* m = cmd->m2dtc;

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);
	glBufferData(gl_buf_target::array, m->colors.size * sizeof(v4), m->colors.size ? m->colors.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[3]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::stream_draw);
}

CALLBACK void buffers_mesh_3d_tex(draw_context* dc, render_command* cmd) { PROF

	mesh_3d_tex* m = cmd->m3dt;

	glBindBuffer(gl_buf_target::array, dc->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(v3), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::array, dc->vbos[1]);
	glBufferData(gl_buf_target::array, m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::stream_draw);
	glBindBuffer(gl_buf_target::element_array, dc->vbos[2]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::stream_draw);
}


CALLBACK void run_mesh_2d_col(render_command* cmd) { PROF

	glDisable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->m2dc->elements.size * 3, gl_index_type::unsigned_int, 0);
}

CALLBACK void run_mesh_2d_tex(render_command* cmd) { PROF

	glDisable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->m2dt->elements.size * 3, gl_index_type::unsigned_int, 0);
}

CALLBACK void run_mesh_2d_tex_col(render_command* cmd) { PROF

	glDisable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->m2dtc->elements.size * 3, gl_index_type::unsigned_int, 0);
}

CALLBACK void run_mesh_3d_tex(render_command* cmd) { PROF

	glEnable(gl_capability::depth_test);
	glDrawElements(gl_draw_mode::triangles, cmd->m3dt->elements.size * 3, gl_index_type::unsigned_int, 0);
}


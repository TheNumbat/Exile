
mesh_2d make_mesh_2d(allocator* alloc, u32 verts) {

	mesh_2d ret;

	ret.alloc = alloc;

	ret.verticies = make_vector<v2>(verts, alloc);
	ret.texCoords = make_vector<v2>(verts, alloc);
	ret.colors =	make_vector<v4>(verts, alloc);

	return ret;
}

void destroy_mesh_2d(mesh_2d* m) {

	destroy_vector(&m->verticies);
	destroy_vector(&m->texCoords);
	destroy_vector(&m->colors);
	m->alloc = NULL;
}

mesh_3d make_mesh_3d(allocator* alloc, u32 verts, u32 inds) {

	mesh_3d ret;

	ret.alloc = alloc;

	ret.verticies = make_vector<v3>(verts, alloc);
	ret.texCoords = make_vector<v2>(verts, alloc);

	return ret;
}

void destroy_mesh_3d(mesh_3d* m) {

	destroy_vector(&m->verticies);
	destroy_vector(&m->texCoords);

	m->alloc = NULL;	
}

render_command make_render_command(render_command_type type, void* data) {

	render_command ret;

	ret.cmd = type;
	ret.data = data;

	return ret;
}

render_command_list make_command_list(allocator* alloc, u32 cmds) {

	render_command_list ret;

	ret.alloc = alloc;

	ret.commands = make_vector<render_command>(cmds, alloc);

	return ret;
}

void destroy_command_list(render_command_list* rcl) {

	destroy_vector(&rcl->commands);
	rcl->alloc = NULL;
}

void render_add_command(render_command_list* rcl, render_command rc) {

	vector_push(&rcl->commands, rc);
}


chunk chunk::make(mesh_3d_tex* cube, allocator* a) { PROF

	chunk ret;

	ret.cube_data = mesh_3d_tex_instance_data::make(cube, 16, a);
	memset(ret.blocks, 16 * 16 * 256, 1);

	return ret;
}

void chunk::destroy() { PROF

	cube_data.destroy();
}

void chunk::build_data() { PROF

	cube_data.clear();

	for(u32 x = 0; x < 16; x++) {
		for(u32 z = 0; z < 16; z++) {
			for(u32 y = 0; y < 256; y++) {
				if(blocks[x][z][y] != block_type::air) {

					cube_data.data.push(V3f(x, y, z));
					cube_data.instances++;
				}
			}
		}
	}
}

void world_manager::init(game_state* st, allocator* a) { PROF

	state = st;
	alloc = a;

	cube = mesh_3d_tex::make(8, a);
	cube.push_cube(V3(0.0f, 0.0f, 0.0f), 1.0f);

	cube_tex = state->ogl.add_texture(&state->default_store, "numbat"_);
	the_chunk = chunk::make(&cube, a);
	the_chunk.build_data();
}

void world_manager::destroy() { PROF

	cube.destroy();
	the_chunk.destroy();
}

void world_manager::render() { PROF

	render_command_list rcl = render_command_list::make(alloc);
	render_command cmd = render_command::make(render_command_type::mesh_3d_tex_instanced);

	cmd.mesh_3d_tex_instanced.data = &the_chunk.cube_data;
	cmd.model = scale(V3(0.1f, 0.1f, 0.1f));
	cmd.texture = cube_tex;

	rcl.view = camera.view();
	rcl.proj = proj(60.0f, (f32)state->window.w / (f32)state->window.h, 0.1f, 100.0f);

	rcl.add_command(cmd);
	state->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

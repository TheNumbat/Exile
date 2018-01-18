
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

void exile::init(engine* st) { PROF

	state = st;
	alloc = MAKE_PLATFORM_ALLOCATOR("world");

	camera.reset();

	cube = mesh_3d_tex::make(8, &alloc);
	cube.push_cube(V3(0.0f, 0.0f, 0.0f), 1.0f);

	cube_tex = state->ogl.add_texture(&state->default_store, "numbat"_);
	the_chunk = chunk::make(&cube, &alloc);
	the_chunk.build_data();
}

void exile::update() { PROF

}

void exile::destroy() { PROF

	the_chunk.destroy();
	cube.destroy();
	alloc.destroy();
}

void exile::render() { PROF

	render_command_list rcl = render_command_list::make();
	render_command cmd = render_command::make(render_command_type::mesh_3d_tex_instanced);

	cmd.mesh_3d_tex_instanced.data = &the_chunk.cube_data;
	cmd.texture = cube_tex;

	rcl.view = camera.view();
	rcl.proj = proj(camera.fov, (f32)state->window.w / (f32)state->window.h, 0.001f, 1000.0f);

	rcl.add_command(cmd);
	state->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

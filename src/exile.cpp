
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

	p.camera.reset();

	cube = mesh_3d_tex::make(8, &alloc);
	cube.push_cube(V3(0.0f, 0.0f, 0.0f), 1.0f);

	cube_tex = state->ogl.add_texture(&state->default_store, "numbat"_);
	the_chunk = chunk::make(&cube, &alloc);
	the_chunk.build_data();

	handler = state->evt.add_handler(FPTR(default_evt_handle), this);
}

void exile::update() { PROF

	platform_perfcount now = global_api->platform_get_perfcount();

	p.update(now);
}

void exile::destroy() { PROF

	state->evt.rem_handler(handler);

	the_chunk.destroy();
	cube.destroy();
	alloc.destroy();
}

void exile::render() { PROF

	render_command_list rcl = render_command_list::make();
	render_command cmd = render_command::make(render_command_type::mesh_3d_tex_instanced);

	cmd.mesh_3d_tex_instanced.data = &the_chunk.cube_data;
	cmd.texture = cube_tex;

	rcl.view = p.camera.view();
	rcl.proj = proj(p.camera.fov, (f32)state->window.w / (f32)state->window.h, 0.001f, 1000.0f);

	rcl.add_command(cmd);
	state->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

void player::update(platform_perfcount now) { PROF

	platform_perfcount pdt = now - last;
	f32 dt = (f32)pdt / (f32)global_api->platform_get_perfcount_freq();

	camera.pos += velocity * dt;
	camera.update();

	last = now;
}

void player::forward() { PROF

	velocity = camera.front * max_speed;
}

void player::left() { PROF

	velocity = camera.right * -max_speed;
}

void player::right() { PROF

	velocity = camera.right * max_speed;
}

void player::back() { PROF

	velocity = camera.front * -max_speed;
}

CALLBACK bool default_evt_handle(void* param, platform_event evt) { PROF

	exile* game = (exile*)param;

	if(evt.type == platform_event_type::key) {

		if(evt.key.flags & (u16)platform_keyflag::press) {

			switch(evt.key.code) {
			case platform_keycode::escape: {

				game->state->running = false;

			} break;

			case platform_keycode::w: {

				game->p.forward();

			} break;

			case platform_keycode::a: {

				game->p.left();

			} break;

			case platform_keycode::s: {

				game->p.back();

			} break;

			case platform_keycode::d: {

				game->p.right();

			} break;									

			case platform_keycode::grave: {

				game->state->dbg.toggle_ui();

			} break;

			default: return false;
			}
		}

		return true;
	}

	else if(evt.type == platform_event_type::mouse) {
	
		if(evt.mouse.flags & (u16)platform_mouseflag::move) {
		
		}
	}

	return false;
}

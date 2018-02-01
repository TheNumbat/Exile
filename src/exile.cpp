
chunk chunk::make(allocator* a) { PROF

	chunk ret;

	ret.mesh = mesh_chunk::make(16, a);
	memset(ret.blocks, 16 * 16 * 256, 1);

	return ret;
}

void chunk::destroy() { PROF

	mesh.destroy();
}

void chunk::build_data() { PROF

	mesh.clear();

	for(u32 x = 0; x < 16; x += 2) {
		for(u32 y = 0; y < 16; y += 2) {
			for(u32 z = 0; z < 256; z += 2) {
				if(blocks[x][z][y] != block_type::air) {

					mesh.push_cube(V3f(x, y, z), 1.0f);
				}
			}
		}
	}
}

void exile::init(engine* st) { PROF

	state = st;
	alloc = MAKE_PLATFORM_ALLOCATOR("world");

	p.init();

	cube_tex = state->ogl.add_texture(&state->default_store, "numbat"_);
	the_chunk = chunk::make(&alloc);
	the_chunk.build_data();

	default_evt = state->evt.add_handler(FPTR(default_evt_handle), this);
	camera_evt = state->evt.add_handler(FPTR(camera_evt_handle), this);

	// global_api->platform_capture_mouse(&state->window);
}

void exile::update() { PROF

	platform_perfcount now = global_api->platform_get_perfcount();

	p.update(now);

	// global_api->platform_set_cursor_pos(&state->window, state->window.w / 2, state->window.h / 2);
}

void exile::destroy() { PROF

	state->evt.rem_handler(default_evt);
	state->evt.rem_handler(camera_evt);

	the_chunk.destroy();
	alloc.destroy();
}

void exile::render() { PROF

	render_command_list rcl = render_command_list::make();
	render_command cmd = render_command::make(render_command_type::mesh_chunk, &the_chunk.mesh);

	cmd.texture = cube_tex;

	rcl.view = p.camera.view();
	rcl.proj = proj(p.camera.fov, (f32)state->window.w / (f32)state->window.h, 0.001f, 1000.0f);

	rcl.add_command(cmd);
	state->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

void player::init() { PROF

	speed = 5.0f;
	last = 0;
	camera.reset();
}

void player::update(platform_perfcount now) { PROF

	platform_perfcount pdt = now - last;
	f32 dt = (f32)pdt / (f32)global_api->platform_get_perfcount_freq();

	camera.pos += velocity * dt;
	camera.update();

	gui_begin("Exile"_, R2(50.0f, 50.0f, 350.0f, 100.0f));
	gui_text(string::makef("pos: %"_, camera.pos));
	gui_text(string::makef("vel: %"_, velocity));
	gui_end();

	last = now;
}

CALLBACK bool default_evt_handle(void* param, platform_event evt) { PROF

	exile* game = (exile*)param;

	if(evt.type == platform_event_type::key) {
		if(evt.key.flags & (u16)platform_keyflag::press) {
			if(evt.key.code == platform_keycode::escape) {

				game->state->running = false;
				return true;
			}

			else if(evt.key.code == platform_keycode::grave) {

				game->state->dbg.toggle_ui();
				if(game->state->dbg.show_ui) {
					// global_api->platform_release_mouse();
				} else {
					// global_api->platform_capture_mouse(&game->state->window);
				}
			}
		}
	}

	return false;
}

CALLBACK bool camera_evt_handle(void* param, platform_event evt) { PROF

	exile* game = (exile*)param;
	player& p = game->p;

	if(evt.type == platform_event_type::key) {

		if(evt.key.flags & (u16)platform_keyflag::press) {

			switch(evt.key.code) {
			case platform_keycode::escape: {

				game->state->running = false;

			} break;

			case platform_keycode::w: {

				p.velocity += p.camera.front * p.speed;

			} break;

			case platform_keycode::a: {

				p.velocity += p.camera.right * -p.speed;

			} break;

			case platform_keycode::s: {

				p.velocity += p.camera.front * -p.speed;

			} break;

			case platform_keycode::d: {

				p.velocity += p.camera.right * p.speed;

			} break;	

			case platform_keycode::space: {

				p.velocity += p.camera.up * p.speed;

			} break;	

			case platform_keycode::lshift: {

				p.velocity += p.camera.up * -p.speed;

			} break;	

			default: return false;
			}
		}

		if(evt.key.flags & (u16)platform_keyflag::release) {

			switch(evt.key.code) {
			case platform_keycode::w: {

				p.velocity -= p.camera.front * p.speed;

			} break;

			case platform_keycode::a: {

				p.velocity -= p.camera.right * -p.speed;

			} break;

			case platform_keycode::s: {

				p.velocity -= p.camera.front * -p.speed;

			} break;

			case platform_keycode::d: {

				p.velocity -= p.camera.right * p.speed;

			} break;

			case platform_keycode::space: {

				p.velocity -= p.camera.up * p.speed;

			} break;	

			case platform_keycode::lshift: {

				p.velocity -= p.camera.up * -p.speed;

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

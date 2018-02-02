
u32 hash(chunk_pos key) { PROF

	return hash(key.x) ^ hash(key.y) ^ hash(key.z);
}

chunk_pos::chunk_pos(i32 _x, i32 _y, i32 _z) {
	x = _x; y = _y; z = _z;
}

bool operator==(chunk_pos l, chunk_pos r) { PROF

	return l.x == r.x && l.y == r.y && l.z == r.z;
}

chunk_pos chunk_pos::from_abs(v3 pos) { PROF

	chunk_pos ret;
	ret.x = (i32)(pos.x / chunk::xsz) - (pos.x < 0 ? 1 : 0);
	ret.y = (i32)(pos.y / chunk::ysz) - (pos.y < 0 ? 1 : 0);
	ret.z = (i32)(pos.z / chunk::zsz) - (pos.z < 0 ? 1 : 0);
	return ret;
}

chunk_pos chunk_pos::operator+(chunk_pos other) { PROF
	chunk_pos ret;
	ret.x = x + other.x;
	ret.y = y + other.y;
	ret.z = z + other.z;
	return ret;
}

chunk_pos chunk_pos::operator-(chunk_pos other) { PROF
	chunk_pos ret;
	ret.x = x - other.x;
	ret.y = y - other.y;
	ret.z = z - other.z;
	return ret;
}

chunk chunk::make(chunk_pos p, allocator* a) { PROF

	chunk ret;

	ret.pos = p;
	ret.mesh = mesh_chunk::make(16, a);

	return ret;
}

void chunk::destroy() { PROF

	mesh.destroy();
}

void chunk::gen() { PROF

	for(u32 x = 0; x < xsz; x++) {
		for(u32 z = 0; z < zsz; z++) {

			f32 fx = pos.x + x / (f32)xsz;
			f32 fz = pos.z + z / (f32)zsz;

			float val = stb_perlin_noise3(fx / 8.0f, 0, fz / 8.0f, 0, 0, 0);
			u32 height = (u32)(val * ysz / 2.0f + ysz / 2.0f);
			
			for(u32 y = 0; y < height; y++) {
				blocks[x][z][y] = block_type::numbat;
			}
		}
	}
}

void chunk::build_data() { PROF

	mesh.clear();

	for(u32 x = 0; x < xsz; x++) {
		for(u32 z = 0; z < zsz; z++) {
			for(u32 y = 0; y < ysz; y++) {
				if(blocks[x][z][y] != block_type::air) {

					mesh.push_cube(V3f(x * 16, y * 16, z * 16), 16.0f);
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

	default_evt = state->evt.add_handler(FPTR(default_evt_handle), this);
	camera_evt = state->evt.add_handler(FPTR(camera_evt_handle), this);

	chunks = map<chunk_pos, chunk>::make(256, &alloc);

	// global_api->platform_capture_mouse(&state->window);
}

void exile::update() { PROF

	platform_perfcount now = global_api->platform_get_perfcount();

	gui_begin("Exile"_, R2(50.0f, 50.0f, 350.0f, 100.0f));
	gui_int_slider("view: "_, &view_distance, 0, 6);

	p.update(now);

	gui_end();

	// global_api->platform_set_cursor_pos(&state->window, state->window.w / 2, state->window.h / 2);
}

void exile::destroy() { PROF

	state->evt.rem_handler(default_evt);
	state->evt.rem_handler(camera_evt);

	FORMAP(it, chunks) {
		it->value.destroy();
	}
	chunks.destroy();

	alloc.destroy();
}

void exile::populate_local_area() { PROF

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			
			if(!chunks.try_get(current)) {
				
				LOG_INFO_F("building chunk %", current);
				
				chunk* c = chunks.insert(current, chunk::make(current, &alloc));
				c->gen();
				c->build_data();
			}
		}
	}
}

void exile::render() { PROF

	render_command_list rcl = render_command_list::make();

	// NOTE(max): we need to do this first so command meshes pointers don't get moved while adding more chunks
	populate_local_area();

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			
			chunk* c = chunks.get(current);
			if(!c->mesh.dirty) {
				c->mesh.free_cpu(); 
			}

			render_command cmd = render_command::make(render_command_type::mesh_chunk, &c->mesh);
					
			cmd.texture = cube_tex;
			cmd.model = translate(V3f(current.x * chunk::xsz, current.y * chunk::ysz, current.z * chunk::zsz));

			rcl.add_command(cmd);
		}
	}

	rcl.view = p.camera.view();
	rcl.proj = proj(p.camera.fov, (f32)state->window.w / (f32)state->window.h, 0.001f, 1000.0f);

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

	gui_text(string::makef("pos: %"_, camera.pos));
	gui_text(string::makef("vel: %"_, velocity));
	gui_text(string::makef("chunk: %"_, chunk_pos::from_abs(camera.pos)));

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


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

			f32 val = stb_perlin_noise3(fx / 8.0f, 0, fz / 8.0f, 0, 0, 0);
			u32 height = (u32)(val * ysz / 2.0f + ysz / 2.0f);
			
			for(u32 y = 0; y < height; y++) {
				blocks[x][z][y] = block_type::numbat;
			}
		}
	}
}

void chunk::build_data() { PROF

	PUSH_PROFILE_PROF(false);
	mesh.clear();

	block_type slice[xsz * ysz];

	i32 xyz[] = {0, 0, 0};
	i32 max[] = {xsz, ysz, zsz};

	for (i32 i = 0; i < 6; i++) {

		i32 d0 = (i + 0) % 3;
		i32 d1 = (i + 1) % 3;
		i32 d2 = (i + 2) % 3;
		i32 backface = i / 3 * 2 - 1;

		// Traverse the chunk
		for (xyz[d0] = 0; xyz[d0] < max[d0]; xyz[d0]++) {

			// Fill in slice
			for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++) {
				for (xyz[d2] = 0; xyz[d2] < max[d2]; xyz[d2]++) {
					if(xyz[0] >= 0 && xyz[0] < xsz && xyz[1] >= 0 && xyz[1] < ysz && xyz[2] >=0 && xyz[2] < zsz) {
						block_type b = blocks[xyz[0]][xyz[2]][xyz[1]];

						// check for air
						if (b != block_type::air) {
							// Check neighbor
							xyz[d0] += backface;
							if(xyz[0] >= 0 && xyz[0] < xsz && xyz[1] >= 0 && xyz[1] < ysz && xyz[2] >=0 && xyz[2] < zsz) {
								if (blocks[xyz[0]][xyz[2]][xyz[1]] != block_type::air) {
									slice[xyz[d1] * max[d2] + xyz[d2]] = block_type::air;
								} else {
									slice[xyz[d1] * max[d2] + xyz[d2]] = b;
								}
							} else {
								slice[xyz[d1] * max[d2] + xyz[d2]] = b;
							}
							xyz[d0] -= backface;
						} else {
							slice[xyz[d1] * max[d2] + xyz[d2]] = block_type::air;
						}
					}
				}
			}

			// Mesh the slice
			for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++) {
				for (xyz[d2] = 0; xyz[d2] < max[d2];) {
					block_type type = slice[xyz[d1] * max[d2] + xyz[d2]];

					// check for air
					if (type == block_type::air) {
						xyz[d2]++;
						continue;
					}

					int width = 1;

					// Find the largest line
					for (int d22 = xyz[d2] + 1; d22 < max[d2]; d22++) {
						if (slice[xyz[d1] * max[d2] + d22] != type) break;
						width++;
					}

					int height = 1;

					// Find the largest rectangle
					bool done = false;
					for (int d11 = xyz[d1] + 1; d11 < max[d1]; d11++) {
						// Find lines of the same width
						for (int d22 = xyz[d2]; d22 < xyz[d2] + width; d22++) {
							if (slice[d11 * max[d2] + d22] != type) {
								done = true;
								break;
							}
						}
						if (done) break;
						height++;
					}

					f32 w[] = { 0, 0, 0 };
					w[d2] = (f32) width;
					f32 h[] = { 0, 0, 0 };
					h[d1] = (f32) height;

					v3 v = V3((f32) xyz[0], (f32) xyz[1], (f32) xyz[2]);

					// shift front faces by one block
					if (backface > 0) {
						f32 f[] = { 0, 0, 0 };
						f[d0] += 1.0f;
						v += V3(f[0], f[1], f[2]);
					}

					// emit quad
					switch (i) {
					case 0: // -X
						mesh.quad16(v, v + V3(w[0], w[1], w[2]), v + V3(h[0], h[1], h[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]));
						break;
					case 1: // -Y
						mesh.quad16(v, v + V3(w[0], w[1], w[2]), v + V3(h[0], h[1], h[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]));
						break;
					case 2: // -Z
						mesh.quad16(v + V3(h[0], h[1], h[2]), v, v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), v + V3(w[0], w[1], w[2]));
						break;
					case 3: // +X
						mesh.quad16(v + V3(w[0], w[1], w[2]), v, v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), v + V3(h[0], h[1], h[2]));
						break;
					case 4: // +Y
						mesh.quad16(v + V3(h[0], h[1], h[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), v, v + V3(w[0], w[1], w[2]));
						break;
					case 5: // +Z
						mesh.quad16(v, v + V3(h[0], h[1], h[2]), v + V3(w[0], w[1], w[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]));
						break;
					}

					// Zero the quad in the slice
					for (int d11 = xyz[d1]; d11 < xyz[d1] + height; d11++) {
						for (int d22 = xyz[d2]; d22 < xyz[d2] + width; d22++) {
							slice[d11 * max[d2] + d22] = block_type::air;
						}
					}

					// Advance search position for next quad
					xyz[d2] += width;
				}
			}
		}
	}

	POP_PROFILE_PROF();
}

void exile::init(engine* st) { PROF

	state = st;
	alloc = MAKE_PLATFORM_ALLOCATOR("world");

	p.init();

	cube_tex = state->ogl.add_texture(&state->default_store, "stone"_, texture_wrap::repeat, true);

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

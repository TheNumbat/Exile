
void world::init(asset_store* store, allocator* a) { PROF

	alloc = a;
	p.init();

	block_textures = eng->ogl.begin_tex_array(V3i(32, 32, (i32)NUM_BLOCKS), texture_wrap::repeat, true, 1);
	eng->ogl.push_tex_array(block_textures, store, "stone"_);
	eng->ogl.push_tex_array(block_textures, store, "numbat"_);

	chunks = map<chunk_pos, chunk*>::make(512, a);

	LOG_DEBUG_F("% logical cores % physical cores", global_api->get_num_cpus(), global_api->get_phys_cpus());

	thread_pool = threadpool::make(a, eng->platform->get_phys_cpus() - 1);
	thread_pool.start_all();
}

void world::destroy() { PROF

	FORMAP(it, chunks) {
		it->value->destroy();
		PUSH_ALLOC(it->value->alloc) {
			free(it->value);
		} POP_ALLOC();
	}
	chunks.destroy();
	thread_pool.stop_all();
	thread_pool.destroy();
}

void world::update(platform_perfcount now) { PROF

	gui_begin("Exile"_, R2(50.0f, 50.0f, 350.0f, 100.0f));
	gui_int_slider(string::makef("view: % "_, view_distance), &view_distance, 0, 32);

	p.update(now);

	gui_end();
}

void world::populate_local_area() { PROF

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			
			if(!chunks.try_get(current)) {
				
				chunk** c = chunks.insert(current, chunk::make_new(current, alloc));
				
				thread_pool.queue_job([](void* p) -> job_callback {
					chunk* c = (chunk*)p;

					c->gen();
					c->build_data();

					return null;
				}, *c, 1.0f / lengthsq(current.center() - p.camera.pos));
			}
		}
	}
}

CALLBACK void unlock_chunk(void* v) { PROF

	chunk* c = (chunk*)v;

	eng->platform->release_mutex(&c->swap_mut);
}

float check_pirority(job j, void* param) {

	// player* p = (player*)param;
	return 0.0f;
}

void world::render() { PROF

	render_command_list rcl = render_command_list::make();

	// NOTE(max): we need to do this first so command meshes pointers don't get moved while adding more chunks
	populate_local_area();
	thread_pool.renew_priorities(check_pirority, &p);

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			
			chunk* c = *chunks.get(current);

			eng->platform->aquire_mutex(&c->swap_mut);
			if(!c->mesh.dirty) {
				c->mesh.free_cpu(); 
			}

			render_command cmd = render_command::make(render_command_type::mesh_chunk, &c->mesh);

			cmd.texture = block_textures;
			cmd.model = translate(V3f(current.x * chunk::xsz, current.y * chunk::ysz, current.z * chunk::zsz));

			cmd.callback.set(FPTR(unlock_chunk));
			cmd.param = c;

			rcl.add_command(cmd);
		}
	}

	rcl.view = p.camera.view();
	rcl.proj = proj(p.camera.fov, (f32)eng->window.w / (f32)eng->window.h, 0.001f, 1000.0f);

	eng->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

void player::init() { PROF

	speed = 300.0f;
	last = 0;
	camera.reset();
}

void player::update(platform_perfcount now) { PROF

	platform_perfcount pdt = now - last;
	f32 dt = (f32)pdt / (f32)eng->platform->get_perfcount_freq();

	if(eng->platform->window_focused(&eng->window)) {

		velocity = V3(0.0f, 0.0f, 0.0f);
		if(eng->platform->keydown(platform_keycode::w)) {
			velocity += camera.front * speed;
		}
		if(eng->platform->keydown(platform_keycode::a)) {
			velocity += camera.right * -speed;
		}
		if(eng->platform->keydown(platform_keycode::s)) {
			velocity += camera.front * -speed;
		}
		if(eng->platform->keydown(platform_keycode::d)) {
			velocity += camera.right * speed;
		}
		if(eng->platform->keydown(platform_keycode::space)) {
			velocity.y += speed;
		}
		if(eng->platform->keydown(platform_keycode::lshift)) {
			velocity.y += -speed;
		}

		camera.pos += velocity * dt;
		camera.update();
	}

	gui_text(string::makef("pos: %"_, camera.pos));
	gui_text(string::makef("vel: %"_, velocity));
	gui_text(string::makef("chunk: %"_, chunk_pos::from_abs(camera.pos)));

	last = now;
}

inline u32 hash(chunk_pos key) { PROF

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

v3 chunk_pos::center() { PROF

	return V3(x * chunk::xsz + chunk::xsz / 2.0f, y * chunk::ysz + chunk::ysz / 2.0f, z * chunk::zsz + chunk::zsz / 2.0f);
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
	ret.alloc = a;
	ret.mesh = mesh_chunk::make(1024, a);
	eng->platform->create_mutex(&ret.swap_mut, false);

	return ret;
}

chunk* chunk::make_new(chunk_pos p, allocator* a) { PROF

	PUSH_ALLOC(a);

	chunk* ret = NEW(chunk);

	ret->pos = p;
	ret->alloc = a;
	ret->mesh = mesh_chunk::make(1024, a);
	eng->platform->create_mutex(&ret->swap_mut, false);

	POP_ALLOC();

	return ret;
}

void chunk::destroy() { PROF

	mesh.destroy();
	eng->platform->destroy_mutex(&swap_mut);
}

void chunk::gen() { PROF

	for(u32 x = 0; x < xsz; x++) {
		for(u32 z = 0; z < zsz; z++) {

			f32 fx = 4.0f * (pos.x + x / (f32)xsz);
			f32 fz = 4.0f * (pos.z + z / (f32)zsz);

			f32 val = stb_perlin_noise3(fx / 8.0f, 0, fz / 8.0f, 0, 0, 0);
			u32 height = (u32)(val * ysz / 2.0f + ysz / 2.0f);
			
			for(u32 y = 0; y < height - 2; y++) {
				blocks[x][z][y] = block_type::stone;
			}
			for(u32 y = height - 2; y < height; y++) {
				blocks[x][z][y] = block_type::numbat;
			}
		}
	}
}

void chunk::build_data() { PROF

	mesh_chunk new_mesh = mesh_chunk::make_cpu(1024, alloc);

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

					i32 width = 1;

					// Find the largest line
					for (i32 d22 = xyz[d2] + 1; d22 < max[d2]; d22++) {
						if (slice[xyz[d1] * max[d2] + d22] != type) break;
						width++;
					}

					i32 height = 1;

					// Find the largest rectangle
					bool done = false;
					for (i32 d11 = xyz[d1] + 1; d11 < max[d1]; d11++) {
						// Find lines of the same width
						for (i32 d22 = xyz[d2]; d22 < xyz[d2] + width; d22++) {
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
						new_mesh.quad(v, v + V3(w[0], w[1], w[2]), v + V3(h[0], h[1], h[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), V3f(width, height, (i32)type));
						break;
					case 1: // -Y
						new_mesh.quad(v, v + V3(w[0], w[1], w[2]), v + V3(h[0], h[1], h[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), V3f(width, height, (i32)type));
						break;
					case 2: // -Z
						new_mesh.quad(v + V3(h[0], h[1], h[2]), v, v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), v + V3(w[0], w[1], w[2]), V3f(height, width, (i32)type));
						break;
					case 3: // +X
						new_mesh.quad(v + V3(w[0], w[1], w[2]), v, v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), v + V3(h[0], h[1], h[2]), V3f(width, height, (i32)type));
						break;
					case 4: // +Y
						new_mesh.quad(v + V3(h[0], h[1], h[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), v, v + V3(w[0], w[1], w[2]), V3f(width, height, (i32)type));
						break;
					case 5: // +Z
						new_mesh.quad(v, v + V3(h[0], h[1], h[2]), v + V3(w[0], w[1], w[2]), v + V3(w[0] + h[0], w[1] + h[1], w[2] + h[2]), V3f(height, width, (i32)type));
						break;
					}

					// Zero the quad in the slice
					for (i32 d11 = xyz[d1]; d11 < xyz[d1] + height; d11++) {
						for (i32 d22 = xyz[d2]; d22 < xyz[d2] + width; d22++) {
							slice[d11 * max[d2] + d22] = block_type::air;
						}
					}

					// Advance search position for next quad
					xyz[d2] += width;
				}
			}
		}
	}

	LOG_DEBUG_F("Built chunk{%,%}", pos.x, pos.z);

	eng->platform->aquire_mutex(&swap_mut);
	mesh.swap_mesh(new_mesh);
	eng->platform->release_mutex(&swap_mut);
}


void world::init(asset_store* store, allocator* a) { PROF

	alloc = a;
	p.init();

	block_textures = eng->ogl.begin_tex_array(iv3(32, 32, (i32)NUM_BLOCKS), texture_wrap::repeat, true, 1);
	eng->ogl.push_tex_array(block_textures, store, "stone"_);

	chunks = map<chunk_pos, chunk*>::make(512, a);

	LOG_DEBUG_F("% logical cores % physical cores", global_api->get_num_cpus(), global_api->get_phys_cpus());

	thread_pool = threadpool::make(a, eng->platform->get_phys_cpus() - 1);
	thread_pool.start_all();
}

void world::destroy() { PROF

	thread_pool.stop_all();
	thread_pool.destroy();
	FORMAP(it, chunks) {
		it->value->destroy();
		PUSH_ALLOC(it->value->alloc) {
			free(it->value, sizeof(chunk));
		} POP_ALLOC();
	}
	chunks.destroy();
}

void world::update(u64 now) { PROF

	gui_begin("Exile"_, r2(50.0f, 50.0f, 350.0f, 100.0f));
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
				
				(*c)->job_state.set(work::in_flight);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;

					c->gen();
					c->build_data();
					c->job_state.set(work::done);

				}, *c, 1.0f / lensq(current.center_xz() - p.camera.pos));
			}
		}
	}
}

CALLBACK void unlock_chunk(void* v) { PROF

	chunk* c = (chunk*)v;

	eng->platform->release_mutex(&c->swap_mut);
}

float check_pirority(super_job* j, void* param) {

	world* w = (world*)param;
	player* p = &w->p;
	chunk* c = (chunk*)j->data;

	v3 center = c->pos.center_xz();

	if(abs(center.x - p->camera.pos.x) > (f32)(w->view_distance + 1) * chunk::xsz ||
	   abs(center.z - p->camera.pos.z) > (f32)(w->view_distance + 1) * chunk::zsz) {
		return -FLT_MAX;
	}

	return 1.0f / lensq(center - p->camera.pos);
}

CALLBACK void cancel_build(void* param) {

	chunk* c = (chunk*)param;
	c->job_state.set(work::none);
}

void world::render() { PROF

	render_command_list rcl = render_command_list::make();
	thread_pool.renew_priorities(check_pirority, this);

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			chunk** ch = chunks.try_get(current);

			auto build_job = [&]() {

				(*ch)->job_state.set(work::in_flight);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;

					c->gen();
					c->build_data();
					c->job_state.set(work::done);

				}, *ch, 1.0f / lensq(current.center_xz() - p.camera.pos), FPTR(cancel_build));
			};
			
			if(!ch) {
				ch = chunks.insert(current, chunk::make_new(current, alloc));
				build_job();
			}
			chunk* c = *ch;

			if(c->job_state.get() == work::none) {
				build_job();
			}

			if(!c->mesh.dirty) {
				c->mesh.free_cpu();
			}

			eng->platform->aquire_mutex(&c->swap_mut);
			render_command cmd = render_command::make(render_command_type::mesh_chunk, &c->mesh);

			cmd.num_tris = c->mesh_triangles;
			cmd.texture = block_textures;

			v3 chunk_pos = v3((f32)current.x * chunk::xsz, (f32)current.y * chunk::ysz, (f32)current.z * chunk::zsz);
			cmd.model = translate(chunk_pos - p.camera.pos);

			cmd.callback.set(FPTR(unlock_chunk));
			cmd.param = c;

			rcl.add_command(cmd);
		}
	}

	rcl.view = p.camera.view_no_translate();
	rcl.proj = proj(p.camera.fov, (f32)eng->window.w / (f32)eng->window.h, 0.01f, 2000.0f);

	eng->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

void player::init() { PROF

	speed = 30.0f;
	last = 0;
	camera.reset();
}

void player::update(u64 now) { PROF

	u64 pdt = now - last;
	f32 dt = (f32)pdt / (f32)eng->platform->get_perfcount_freq();

	if(eng->platform->window_focused(&eng->window)) {

		velocity = v3(0.0f, 0.0f, 0.0f);
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

v3 chunk_pos::center_xz() { PROF

	return v3(x * chunk::xsz + chunk::xsz / 2.0f, 0.0f, z * chunk::zsz + chunk::zsz / 2.0f);
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
	ret.mesh = mesh_chunk::make_gpu();
	eng->platform->create_mutex(&ret.swap_mut, false);

	return ret;
}

chunk* chunk::make_new(chunk_pos p, allocator* a) { PROF

	PUSH_ALLOC(a);

	chunk* ret = NEW(chunk);

	ret->pos = p;
	ret->alloc = a;
	ret->mesh = mesh_chunk::make_gpu();
	eng->platform->create_mutex(&ret->swap_mut, false);

	POP_ALLOC();

	return ret;
}

void chunk::destroy() { PROF

	mesh.destroy();
	eng->platform->destroy_mutex(&swap_mut);
}

i32 chunk::y_at(i32 x, i32 z) { PROF

	f32 val = perlin((f32)x / 32.0f, 0, (f32)z / 32.0f, 0, 0, 0);
	i32 height = (u32)(val * ysz / 2.0f + ysz / 2.0f);
	return height;
}

void chunk::gen() { PROF

	for(u32 x = 0; x < xsz; x++) {
		for(u32 z = 0; z < zsz; z++) {

			u32 height = y_at(pos.x * xsz + x, pos.z * zsz + z);
			memset(blocks[x][z], height, (u8)block_type::stone);
		}
	}
}

u8 chunk::ao_at(v3 vert) { 

	i32 x = (i32)vert.x, y = (i32)vert.y, z = (i32)vert.z;

	bool side1 = block_at(x-1,y,z) != block_type::air;
	bool side2 = block_at(x,y,z-1) != block_type::air;
	bool corner = block_at(x-1,y+1,z-1) != block_type::air;

	if(side1 && side2) {
		return 0;
	}
	return 3 - (side1 + side2 + corner);
}

block_type chunk::block_at(i32 x, i32 y, i32 z) { 

	if(x < 0 || x >= xsz || y < 0 || y >= ysz || z < 0 || z >= zsz) {

		// TODO(max): if the neighboring chunk exists, get a block from it
		return y_at(pos.x * xsz + x, pos.z * zsz + z) > y ? block_type::stone : block_type::air; 
	}

	return blocks[x][z][y];
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
					block_type b = block_at(xyz[0],xyz[1],xyz[2]);

					// check for air
					if (b != block_type::air) {
						// Check neighbor
						xyz[d0] += backface;

						if (block_at(xyz[0],xyz[1],xyz[2]) != block_type::air) {
							slice[xyz[d1] * max[d2] + xyz[d2]] = block_type::air;
						} else {
							slice[xyz[d1] * max[d2] + xyz[d2]] = b;
						}
						xyz[d0] -= backface;
					} else {
						slice[xyz[d1] * max[d2] + xyz[d2]] = block_type::air;
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
					for (i32 d22 = xyz[d2] + 1; d22 < max[d2] && width <= 255; d22++) {
						if (slice[xyz[d1] * max[d2] + d22] != type) break;
						width++;
					}

					i32 height = 1;

					// Find the largest rectangle
					bool done = false;
					for (i32 d11 = xyz[d1] + 1; d11 < max[d1] && height <= 255; d11++) {
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

					v3 w, h;
					w.a[d2] = (f32)width;
					h.a[d1] = (f32)height;

					v3 v_0 = v3((f32) xyz[0], (f32) xyz[1], (f32) xyz[2]);

					// shift front faces by one block
					if (backface > 0) {
						v3 f;
						f.a[d0] += 1.0f;
						v_0 += f;
					}

					// emit quad
					v3 v_1 = v_0 + w;
					v3 v_2 = v_0 + w + h;
					v3 v_3 = v_0 + h;
					v3 wht = v3((f32)width, (f32)height, (f32)type), hwt = v3((f32)height, (f32)width, (f32)type);
					u8 ao_0 = ao_at(v_0), ao_1 = ao_at(v_1), ao_2 = ao_at(v_2), ao_3 = ao_at(v_3);

					switch (i) {
					case 0: // -X
						new_mesh.quad(v_0, v_1, v_3, v_2, wht, bv4(ao_0,ao_1,ao_3,ao_2));
						break;
					case 1: // -Y
						new_mesh.quad(v_0, v_1, v_3, v_2, wht, bv4(ao_0,ao_1,ao_3,ao_2));
						break;
					case 2: // -Z
						new_mesh.quad(v_3, v_0, v_2, v_1, hwt, bv4(ao_3,ao_0,ao_2,ao_1));
						break;
					case 3: // +X
						new_mesh.quad(v_1, v_0, v_2, v_3, wht, bv4(ao_1,ao_0,ao_2,ao_3));
						break;
					case 4: // +Y
						new_mesh.quad(v_3, v_2, v_0, v_1, wht, bv4(ao_3,ao_2,ao_0,ao_1));
						break;
					case 5: // +Z
						new_mesh.quad(v_0, v_3, v_1, v_2, hwt, bv4(ao_0,ao_3,ao_1,ao_2));
						break;
					}

					// Zero the quad in the slice
					for (i32 d11 = xyz[d1]; d11 < xyz[d1] + height; d11++) {
						memset(&slice[d11 * max[d2] + xyz[d2]], width, (u8)block_type::air);
					}

					// Advance search position for next quad
					xyz[d2] += width;
				}
			}
		}
	}

	// LOG_DEBUG_F("Built chunk{%,%}", pos.x, pos.z);

	eng->platform->aquire_mutex(&swap_mut);
	mesh.swap_mesh(new_mesh);
	mesh_triangles = mesh.elements.size;
	eng->platform->release_mutex(&swap_mut);
}

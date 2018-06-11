
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

void world::destroy_chunks() { PROF 

	FORMAP(it, chunks) {
		it->value->destroy();
		PUSH_ALLOC(it->value->alloc) {
			free(it->value, sizeof(chunk));
		} POP_ALLOC();
	}
	chunks.destroy();
}

void world::destroy() { PROF

	thread_pool.stop_all();
	thread_pool.destroy();
	destroy_chunks();
}

void world::update(u64 now) { PROF

	gui_begin("Exile"_, r2(50.0f, 50.0f, 350.0f, 200.0f));
	gui_int_slider(string::makef("view: % "_, view_distance), &view_distance, 0, 32);
	gui_checkbox("Wireframe "_, &wireframe);
	gui_checkbox("Respect Cam "_, &respect_cam);
	
	if(gui_button("Regenerate"_)) {
		
		destroy_chunks();
		chunks = map<chunk_pos, chunk*>::make(512, alloc);
	}

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

			chunk_pos current = respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
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

	if(wireframe)
		glPolygonMode(gl_poly::front_and_back, gl_poly_mode::line);

	eng->ogl.execute_command_list(&rcl);
	glPolygonMode(gl_poly::front_and_back, gl_poly_mode::fill);

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
	gui_text(string::makef("look: %"_, camera.front));
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

	bool top0 = block_at(x-1,y,z) != block_type::air;
	bool top1 = block_at(x,y,z-1) != block_type::air;
	bool top2 = block_at(x,y,z) != block_type::air;
	bool top3 = block_at(x-1,y,z-1) != block_type::air;
	bool bot0 = block_at(x-1,y-1,z) != block_type::air;
	bool bot1 = block_at(x,y-1,z-1) != block_type::air;
	bool bot2 = block_at(x,y-1,z) != block_type::air;
	bool bot3 = block_at(x-1,y-1,z-1) != block_type::air;

	bool side0, side1, corner;

	if(!top0 && bot0) {
		side0 = top2;
		side1 = top3;
		corner = top1;
	} else if(!top1 && bot1) {
		side0 = top2;
		side1 = top3;
		corner = top0;
	} else if(!top2 && bot2) {
		side0 = top0;
		side1 = top1;
		corner = top3;
	} else if(!top3 && bot3) {
		side0 = top0;
		side1 = top1;
		corner = top2;
	} else {
		return 3;
	}

	if(side0 && side1) {
		return 0;
	}
	return 3 - side0 - side1 - corner;
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

	for(i32 x = 0; x < xsz; x++) {
		for(i32 z = 0; z < zsz; z++) {
			for(i32 y = 0; y < ysz; y++) {

				block_type type = block_at(x, y, z);
				if(type != block_type::air) {

					v3 _000 (x, y, z)             ; u8 ao_000 = ao_at(_000);
					v3 _001 (x, y, z + 1)         ; u8 ao_001 = ao_at(_001);
					v3 _010 (x, y + 1, z)         ; u8 ao_010 = ao_at(_010);
					v3 _100 (x + 1, y, z)         ; u8 ao_100 = ao_at(_100);
					v3 _011 (x, y + 1, z + 1)     ; u8 ao_011 = ao_at(_011);
					v3 _110 (x + 1, y + 1, z)     ; u8 ao_110 = ao_at(_110);
					v3 _101 (x + 1, y, z + 1)     ; u8 ao_101 = ao_at(_101);
					v3 _111 (x + 1, y + 1, z + 1) ; u8 ao_111 = ao_at(_111);
					v3 wht  (1, 1, (i32)type)     ;

					if(block_at(x - 1, y, z) == block_type::air) 
						new_mesh.quad(_000, _001, _010, _011, wht, bv4(ao_000, ao_001, ao_010, ao_011));
					if(block_at(x, y - 1, z) == block_type::air)
						new_mesh.quad(_000, _100, _001, _101, wht, bv4(ao_000, ao_100, ao_001, ao_101));
					if(block_at(x, y, z - 1) == block_type::air)
						new_mesh.quad(_000, _010, _100, _110, wht, bv4(ao_000, ao_010, ao_100, ao_110));
					if(block_at(x + 1, y, z) == block_type::air)
						new_mesh.quad(_101, _100, _111, _110, wht, bv4(ao_101, ao_100, ao_111, ao_110));
					if(block_at(x, y + 1, z) == block_type::air)
						new_mesh.quad(_110, _010, _111, _011, wht, bv4(ao_110, ao_010, ao_111, ao_011));
					if(block_at(x, y, z + 1) == block_type::air)
						new_mesh.quad(_011, _001, _111, _101, wht, bv4(ao_011, ao_001, ao_111, ao_101));
				}
			}
		}
	}

	eng->platform->aquire_mutex(&swap_mut);
	mesh.swap_mesh(new_mesh);
	mesh_triangles = mesh.elements.size;
	eng->platform->release_mutex(&swap_mut);
}

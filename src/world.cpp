
CALLBACK void world_debug_ui(world* w) { PROF

	if(ImGui::SmallButton("Regenerate"_)) {
		w->regenerate();
	}

	ImGui::SameLine();
	if(ImGui::SmallButton("Reset")) {
		w->p.reset();
	}
}

void world::regenerate() { PROF

	thread_pool.stop_all();
	destroy_chunks();
	chunks = map<chunk_pos, chunk*>::make(512, alloc);
	thread_pool.start_all();
}

CALLBACK void player_debug_ui(world* w) { PROF

	v3 intersection = w->raymarch(w->p.camera.pos, w->p.camera.front, w->p.camera.reach);
	ImGui::ViewAny("inter"_, intersection);
}

void world::init(asset_store* store, allocator* a) { PROF

	alloc = a;

	p.reset();

	LOG_INFO_F("units_per_voxel: %"_, chunk::units_per_voxel);
	
	{
		block_info = vector<block_meta>::make(8192, alloc);
		block_textures = eng->ogl.begin_tex_array(iv3(32, 32, eng->ogl.info.max_texture_layers), texture_wrap::repeat, true, 1);
		add_block(); // air

		init_blocks(this, store);
	}

	{
		sky.init();
		sky.push_dome({}, 1.0f, 64);
		sky_texture = eng->ogl.add_texture(store, "sky"_, texture_wrap::mirror);
		night_sky_texture = eng->ogl.add_texture(store, "night_sky"_);
	}

	{
		LOG_DEBUG_F("% logical cores % physical cores"_, global_api->get_num_cpus(), global_api->get_phys_cpus());
		chunks = map<chunk_pos, chunk*>::make(512, a);
		thread_pool = threadpool::make(a, eng->platform->get_phys_cpus() - 1);
		thread_pool.start_all();
	}

	{
		eng->dbg.store.add_var("world/settings"_, &settings);
		eng->dbg.store.add_var("world/time"_, &time);
		eng->dbg.store.add_ele("world/ui"_, FPTR(world_debug_ui), this);

		eng->dbg.store.add_var("player/cam"_, &p.camera);
		eng->dbg.store.add_var("player/speed"_, &p.speed);
		eng->dbg.store.add_var("player/enable"_, &p.enable);
		eng->dbg.store.add_var("player/noclip"_, &p.noclip);
		eng->dbg.store.add_ele("player/inter"_, FPTR(player_debug_ui), this);
	}

	{
		time.last_update = global_api->get_perfcount();
		time.absolute = global_api->get_perfcount();
	}
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

	sky.destroy();
	thread_pool.stop_all();
	thread_pool.destroy();
	destroy_chunks();
	block_info.destroy();
}

block_meta* world::add_block() {

	block_type id = next_block_type;
	
	next_block_type++;
	if(next_block_type >= block_info.capacity) {
		block_info.grow();
	}

	block_meta* ret = block_info.get(id);
	ret->type = id;

	return ret;
}

v3 world::raymarch(v3 origin, v3 max) { PROF
	return raymarch(origin, max, len(max));
}

v3 world::raymarch(v3 pos3, v3 dir3, f32 max) { PROF

	if (max == 0.0f) return pos3;

	chunk** start = chunks.try_get(chunk_pos::from_abs(pos3));
	if(!start) return pos3;
	chunk* c = *start;

	v4 pos; pos.xyz = pos3;
	v4 dir; dir.xyz = norm(dir3);

	f32 progress = 0.0f;

	while(progress < max) {

		v4 current = pos + dir * progress;
		iv3 current_vox = current;

		if(c->block_at(current_vox.x % chunk::xsz, current_vox.y % chunk::ysz, current_vox.z % chunk::zsz) != block_air) {
			return current.xyz;
		}
		
		v4 delta = (step({}, dir) - fract(current)) / dir;
		progress += max(min_reset(delta.xyz), 0.001f);
	}

	return pos.xyz + dir.xyz * max;
}

f32 world_time::day_01() { PROF 

	return (hour + (minute / 60.0f)) / 24.0f;
}

void world_time::update(u64 now) { PROF 

	if(enable) {

		u64 rel_abs = now - last_update;
		absolute += rel_abs;
		
		f64 rel_ms = 1000.0f * rel_abs / global_api->get_perfcount_freq();
		absolute_ms += rel_ms;
		minute_ms += rel_ms;

		f64 ms_per_min = 1000.0f / time_scale;

		while(minute_ms >= ms_per_min) {
			minute_ms -= ms_per_min;
			minute++;
		}
		while(minute >= 60) {
			minute -= 60;
			hour++;
		}
		while(hour >= 24) {
			hour -= 24;
			day++;
		}
	}
	last_update = now;
}

void world::update(u64 now) { PROF

	time.update(now);
	update_player(now);
}

void world::populate_local_area() { PROF

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -settings.view_distance; x <= settings.view_distance; x++) {
		for(i32 z = -settings.view_distance; z <= settings.view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			
			if(!chunks.try_get(current)) {
				
				chunk** c = chunks.insert(current, chunk::make_new(this, current, alloc));
				
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

CALLBACK void unlock_chunk(chunk* c) { PROF

	eng->platform->release_mutex(&c->swap_mut);
}

float check_pirority(super_job* j, void* param) {

	world* w = (world*)param;
	player* p = &w->p;
	chunk* c = (chunk*)j->data;

	v3 center = c->pos.center_xz();

	if(abs(center.x - p->camera.pos.x) > (f32)(w->settings.view_distance + 1) * chunk::xsz ||
	   abs(center.z - p->camera.pos.z) > (f32)(w->settings.view_distance + 1) * chunk::zsz) {
		return -FLT_MAX;
	}

	return 1.0f / lensq(center - p->camera.pos);
}

CALLBACK void cancel_build(chunk* c) {

	c->job_state.set(work::none);
}

void player::reset() { PROF

	camera.reset();
	camera.pos = {3.0f, 115.0f, 16.0f};
	speed = 5.0f;
	velocity = v3();
	last = global_api->get_perfcount();
}

void world::render() { PROF

	render_chunks();
	render_player();
	render_sky();
}

void world::render_sky() { PROF 

	render_command_list rcl = render_command_list::make();

	render_command cmd = render_command::make((u16)mesh_cmd::skydome, sky.gpu);

	cmd.uniform_info = &time;
	cmd.texture0 = sky_texture;
	cmd.texture1 = night_sky_texture;

	cmd.view = p.camera.view_no_translate();
	cmd.proj = proj(p.camera.fov, (f32)eng->window.settings.w / (f32)eng->window.settings.h, 0.01f, 2000.0f);

	rcl.add_command(cmd);

	eng->ogl.execute_command_list(&rcl);

	rcl.destroy();
}

void world::render_chunks() { PROF

	render_command_list rcl = render_command_list::make();
	thread_pool.renew_priorities(check_pirority, this);

	rcl.push_settings();
	if(settings.wireframe)
		rcl.set_setting(render_setting::wireframe, true);
	if(settings.cull_backface)
		rcl.set_setting(render_setting::cull, true);
	if(settings.sample_shading)
		rcl.set_setting(render_setting::aa_shading, true);

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -settings.view_distance; x <= settings.view_distance; x++) {
		for(i32 z = -settings.view_distance; z <= settings.view_distance; z++) {

			chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
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
				ch = chunks.insert(current, chunk::make_new(this, current, alloc));
				build_job();
			}
			chunk* c = *ch;

			if(c->job_state.get() == work::none) {
				build_job();
			}

			eng->platform->aquire_mutex(&c->swap_mut);
			if(!c->mesh.dirty) {
				c->mesh.free_cpu();
			}
			render_command cmd = render_command::make((u16)mesh_cmd::chunk, c->mesh.gpu);

			cmd.texture0 = block_textures;
			cmd.num_tris = c->mesh_faces;

			v3 chunk_pos = v3((f32)current.x * chunk::xsz, (f32)current.y * chunk::ysz, (f32)current.z * chunk::zsz);
			cmd.model = translate(chunk_pos - p.camera.pos);

			cmd.callback.set(FPTR(unlock_chunk));
			cmd.param = c;

			cmd.view = p.camera.view_pos_origin();
			cmd.proj = proj(p.camera.fov, (f32)eng->window.settings.w / (f32)eng->window.settings.h, 0.01f, 2000.0f);

			rcl.add_command(cmd);
		}
	}

	rcl.pop_settings();

	eng->ogl.execute_command_list(&rcl);
	rcl.destroy();
}

void world::render_player() { PROF

	render_camera& cam = p.camera;

	render_command_list rcl = render_command_list::make();

	{
		mesh_lines lines; lines.init();

		lines.push(cam.pos + cam.front, cam.pos + cam.reach * cam.front, colorf(0,0,1,1), colorf(0,1,0,1));
		lines.push(cam.pos, cam.pos + cam.front, colorf(1,0,0,1), colorf(0,0,1,1));

		v3 intersection = raymarch(cam.pos, cam.front, cam.reach);

		lines.push(cam.pos, intersection, colorf(0,0,0,1), colorf(0,0,0,1));

		render_command cmd = render_command::make((u16)mesh_cmd::lines, lines.gpu);

		cmd.view = cam.view();
		cmd.proj = proj(cam.fov, (f32)eng->window.settings.w / (f32)eng->window.settings.h, 0.01f, 2000.0f);
		rcl.add_command(cmd);

		eng->ogl.execute_command_list(&rcl);
		lines.destroy();
	}

	rcl.clear();

	{
		f32 w = (f32)eng->window.settings.w, h = (f32)eng->window.settings.h;

		mesh_2d_col crosshair; crosshair.init();

		crosshair.push_rect(r2(w / 2.0f - 5.0f, h / 2.0f - 1.0f, 10.0f, 2.0f), WHITE);
		crosshair.push_rect(r2(w / 2.0f - 1.0f, h / 2.0f - 5.0f, 2.0f, 10.0f), WHITE);

		render_command cmd = render_command::make((u16)mesh_cmd::_2d_col, crosshair.gpu);
		cmd.proj = ortho(0, w, h, 0, -1, 1);

		rcl.push_settings();
		rcl.set_setting(render_setting::depth_test, false);
		rcl.pop_settings();
		rcl.add_command(cmd);

		eng->ogl.execute_command_list(&rcl);
		crosshair.destroy();
	}

	rcl.destroy();
}

void world::update_player(u64 now) { PROF

	render_camera& cam = p.camera;

	u64 pdt = now - p.last;
	f64 dt = (f64)pdt / (f64)eng->platform->get_perfcount_freq();

	if(p.enable) {

		v3 accel = v3(0.0f, -settings.gravity, 0.0f);
		v3 mov_v;

		if(eng->platform->window_focused(&eng->window)) {

			if(eng->platform->keydown(platform_keycode::w)) {
				mov_v += cam.front * p.speed;
			}
			if(eng->platform->keydown(platform_keycode::a)) {
				mov_v += cam.right * -p.speed;
			}
			if(eng->platform->keydown(platform_keycode::s)) {
				mov_v += cam.front * -p.speed;
			}
			if(eng->platform->keydown(platform_keycode::d)) {
				mov_v += cam.right * p.speed;
			}
			mov_v.y = 0.0f;
			if(eng->platform->keydown(platform_keycode::space)) {
				mov_v.y += p.speed;
			}
			if(eng->platform->keydown(platform_keycode::lshift)) {
				mov_v.y += -p.speed;
			}
		}

		p.velocity += accel * (f32)dt;

		if(p.noclip) {
			cam.pos += (p.velocity + mov_v) * (f32)dt;
		} else {
			v3 dp = (p.velocity + mov_v) * (f32)dt;
			v3 collide = raymarch(cam.pos, dp);

			if(collide != cam.pos + dp) {
				p.velocity = {};
			}

			cam.pos = collide;
		}
		cam.update();
	}

	p.last = now;
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

void chunk::init(world* _w, chunk_pos p, allocator* a) { PROF

	w = _w;
	pos = p;
	alloc = a;
	mesh.init_gpu();
	eng->platform->create_mutex(&swap_mut, false);
}

chunk* chunk::make_new(world* w, chunk_pos p, allocator* a) { PROF

	PUSH_ALLOC(a);

	chunk* ret = NEW(chunk);

	ret->init(w, p, a);

	POP_ALLOC();

	return ret;
}

void chunk::destroy() { PROF

	mesh.destroy();
	eng->platform->destroy_mutex(&swap_mut);
}

i32 chunk::y_at(i32 x, i32 z) { PROF

	f32 val = perlin((f32)x / 32.0f, 0, (f32)z / 32.0f, 0, 0, 0);
	i32 height = (i32)(val * ysz / 2.0f + ysz / 2.0f) / 2;

	return height;
}

void chunk::gen() { PROF

	for(u32 x = 0; x < xsz; x++) {
		for(u32 z = 0; z < zsz; z++) {

			u32 height = y_at(pos.x * xsz + x, pos.z * zsz + z);

			blocks[x][z][0] = block_bedrock;
			for(u32 y = 1; y < height; y++) {
				blocks[x][z][y] = block_stone;
			}
			for(u32 y = height; y < height + 1; y++) {
				blocks[x][z][y] = block_path;
			}			
		}
	}
}

u8 chunk::ao_at(v3 vert) { 

	i32 x = (i32)vert.x, y = (i32)vert.y, z = (i32)vert.z;

	bool top0 = w->block_info.get(block_at(x-1,y,z))->does_ao;
	bool top1 = w->block_info.get(block_at(x,y,z-1))->does_ao;
	bool top2 = w->block_info.get(block_at(x,y,z))->does_ao;
	bool top3 = w->block_info.get(block_at(x-1,y,z-1))->does_ao;
	bool bot0 = w->block_info.get(block_at(x-1,y-1,z))->does_ao;
	bool bot1 = w->block_info.get(block_at(x,y-1,z-1))->does_ao;
	bool bot2 = w->block_info.get(block_at(x,y-1,z))->does_ao;
	bool bot3 = w->block_info.get(block_at(x-1,y-1,z-1))->does_ao;

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

block_type chunk::block_at(i32 x, i32 y, i32 z) { PROF

	if(y < 0) return block_air;

	if(x < 0 || x >= xsz || y >= ysz || z < 0 || z >= zsz) {

		// TODO(max): if the neighboring chunk exists, get a block from it
		i32 h = y_at(pos.x * xsz + x, pos.z * zsz + z);

		if(y < h) return block_stone;
		if(y >= h && y < h + 1) return block_path;
		return block_air; 
	}

	return blocks[x][z][y];
}

mesh_face chunk::build_face(block_type t, iv3 p, i32 dir) { PROF

	mesh_face ret;
	ret.info = *w->block_info.get(t);

	switch(dir) {
	case 0: {
		ret.ao[0] = ao_at(p);
		ret.ao[1] = ao_at(p + v3(0,1,0));
		ret.ao[2] = ao_at(p + v3(0,0,1));
		ret.ao[3] = ao_at(p + v3(0,1,1));
	} break;
	case 1: {
		ret.ao[0] = ao_at(p);
		ret.ao[1] = ao_at(p + v3(1,0,0));
		ret.ao[2] = ao_at(p + v3(0,0,1));
		ret.ao[3] = ao_at(p + v3(1,0,1));
	} break;
	case 2: {
		ret.ao[0] = ao_at(p);
		ret.ao[1] = ao_at(p + v3(1,0,0));
		ret.ao[2] = ao_at(p + v3(0,1,0));
		ret.ao[3] = ao_at(p + v3(1,1,0));
	} break;
	case 3: {
		ret.ao[0] = ao_at(p + v3(1,0,0));
		ret.ao[1] = ao_at(p + v3(1,1,0));
		ret.ao[2] = ao_at(p + v3(1,0,1));
		ret.ao[3] = ao_at(p + v3(1,1,1));
	} break;
	case 4: {
		ret.ao[0] = ao_at(p + v3(0,1,0));
		ret.ao[1] = ao_at(p + v3(1,1,0));
		ret.ao[2] = ao_at(p + v3(0,1,1));
		ret.ao[3] = ao_at(p + v3(1,1,1));
	} break;
	case 5: {
		ret.ao[0] = ao_at(p + v3(0,0,1));
		ret.ao[1] = ao_at(p + v3(1,0,1));
		ret.ao[2] = ao_at(p + v3(0,1,1));
		ret.ao[3] = ao_at(p + v3(1,1,1));
	} break;
	}

	return ret;
}

bool mesh_face::can_merge(mesh_face f1, mesh_face f2, i32 dir, bool h) { PROF

	dir += h ? 3 : 0;
	return f1.info.merge[dir] == f2.info.type && f1.info.type == f2.info.merge[dir] && f1.ao == f2.ao;
}

CALLBACK void slab_model(chunk* c, mesh_chunk* m, block_meta info, iv3 posi, i32 wi, i32 hi, i32 i) {

	i32 ortho_2d = i % 3;
	i32 u_2d = (i + 1) % 3;
	i32 v_2d = (i + 2) % 3;
	i32 backface_offset = i / 3 * 2 - 1;

	f32 w = (f32)wi;
	f32 h = (f32)hi;
	v3 pos = posi;

	if(u_2d == 1) {
		w /= 2.0f;
	} else if(v_2d == 1) {
		h /= 2.0f;
	} else if(i == 4) {
		pos -= v3(0.0f, 0.5f, 0.0f);
	}

	v3 width_offset, height_offset;
	width_offset[u_2d] = (f32)w;
	height_offset[v_2d] = (f32)h;

	v3 v_0 = pos;
	if(backface_offset > 0) {
		v_0[ortho_2d] += 1.0f;
	}

	v3 v_1 = v_0 + width_offset;
	v3 v_2 = v_0 + height_offset;
	v3 v_3 = v_2 + width_offset;
	v2 wh = v2(w, h), hw = v2(h, w);
	u8 ao_0 = c->ao_at(v_0), ao_1 = c->ao_at(v_1), ao_2 = c->ao_at(v_2), ao_3 = c->ao_at(v_3);

	const f32 units = (f32)chunk::units_per_voxel;
	v_0 *= units; v_1 *= units; v_2 *= units; v_3 *= units;
	wh *= units; hw *= units;

	i32 tex = info.textures[i];

	switch (i) {
	case 0: // -X
		m->quad(v_0, v_2, v_1, v_3, hw, tex, bv4(ao_0,ao_2,ao_1,ao_3));
		break;
	case 1: // -Y
		m->quad(v_2, v_3, v_0, v_1, wh, tex, bv4(ao_2,ao_3,ao_0,ao_1));
		break;
	case 2: // -Z
		m->quad(v_1, v_0, v_3, v_2, wh, tex, bv4(ao_1,ao_0,ao_3,ao_2));
		break;
	case 3: // +X
		m->quad(v_2, v_0, v_3, v_1, hw, tex, bv4(ao_2,ao_0,ao_3,ao_1));
		break;
	case 4: // +Y
		m->quad(v_0, v_1, v_2, v_3, wh, tex, bv4(ao_0,ao_1,ao_2,ao_3));
		break;
	case 5: // +Z
		m->quad(v_0, v_1, v_2, v_3, wh, tex, bv4(ao_0,ao_1,ao_2,ao_3));
		break;
	}
}

void chunk::build_data() { PROF

	PUSH_PROFILE_PROF(false);

	mesh_chunk new_mesh = mesh_chunk::make_cpu(1024, alloc);

	// Array to hold 2D block slice (sized for largest slice)
	block_type slice[xsz * ysz];

	iv3 max = {xsz, ysz, zsz};

	//  0  1  2  3  4  5 
	// -x -y -z +x +y +z
	for(i32 i = 0; i < 6; i++) {

		// Axes of 2D slice to mesh
		i32 ortho_2d = i % 3;
		i32 u_2d = (i + 1) % 3;
		i32 v_2d = (i + 2) % 3;
		i32 backface_offset = i / 3 * 2 - 1;

		// Iterate over orthogonal orthogonal to slice
		iv3 position;
		for(position[ortho_2d] = 0; position[ortho_2d] < max[ortho_2d]; position[ortho_2d]++) {
 	
			// Iterate over 2D slice blocks to filter culled faces before greedy step
			for(position[v_2d] = 0; position[v_2d] < max[v_2d]; position[v_2d]++) {
				for(position[u_2d] = 0; position[u_2d] < max[u_2d]; position[u_2d]++) {

					block_type block = blocks[position[0]][position[2]][position[1]];
					block_meta info0 = *w->block_info.get(block);
					
					i32 slice_idx = position[u_2d] + position[v_2d] * max[u_2d];

					// Only add the face to the slice if its opposing face is not opaque
					if(info0.renders) {
						
						iv3 backface = position;
						backface[ortho_2d] += backface_offset;

						block_type backface_block = block_at(backface[0],backface[1],backface[2]);
						block_meta info1 = *w->block_info.get(backface_block);

						if(!info1.renders || !info0.opaque[i] || !info1.opaque[(i + 3) % 6]) {
							slice[slice_idx] = block;
						} else {
							slice[slice_idx] = block_air;
						}
					} else {
						slice[slice_idx] = block_air;
					}
				}
			}

			// Iterate over slice filled with relevant faces
			for(i32 v = 0; v < max[v_2d]; v++) {
				for(i32 u = 0; u < max[u_2d];) {

					position[u_2d] = u;
					position[v_2d] = v;
					i32 slice_idx = u + v * max[u_2d];

					block_type single_type = slice[slice_idx];
					
					if(single_type != block_air) {

						mesh_face face_type = build_face(single_type, position, i);

						i32 width = 1, height = 1;

						// Combine same faces in +u_2d
						for(; u + width < max[u_2d] && width < 31; width++) {

							iv3 w_pos = position;
							w_pos[u_2d] += width;

							mesh_face merge = build_face(slice[slice_idx + width], w_pos, i);

							if(!mesh_face::can_merge(merge, face_type, ortho_2d, false)) break;
						}

						// Combine all-same face row in +v_2d
						bool done = false;
						for(; v + height < max[v_2d] && height < 31; height++) {
							for(i32 row_idx = 0; row_idx < width; row_idx++) {

								iv3 wh_pos = position;
								wh_pos[u_2d] += row_idx;
								wh_pos[v_2d] +=  height;

								mesh_face merge = build_face(slice[slice_idx + row_idx + height * max[u_2d]], wh_pos, i);

								if(!mesh_face::can_merge(merge, face_type, ortho_2d, true)) {
									done = true;
									break;
								}
							}
							if(done) {
								break;
							}
						}

						// Add quad (u,v,width,height) in 2D slice

						if(face_type.info.custom_model) {

							face_type.info.model(this, &new_mesh, face_type.info, position, width, height, i);

						} else {
							v3 width_offset, height_offset;
							width_offset[u_2d] = (f32)width;
							height_offset[v_2d] = (f32)height;

							v3 v_0 = position;
							if(backface_offset > 0) {
								v_0[ortho_2d] += 1.0f;
							}

							v3 v_1 = v_0 + width_offset;
							v3 v_2 = v_0 + height_offset;
							v3 v_3 = v_2 + width_offset;
							v2 wh = v2(width, height), hw = v2(height, width);
							u8 ao_0 = ao_at(v_0), ao_1 = ao_at(v_1), ao_2 = ao_at(v_2), ao_3 = ao_at(v_3);

							const f32 units = (f32)units_per_voxel;
							v_0 *= units; v_1 *= units; v_2 *= units; v_3 *= units;
							wh *= units; hw *= units;

							i32 tex = face_type.info.textures[i];

							switch (i) {
							case 0: // -X
								new_mesh.quad(v_0, v_2, v_1, v_3, hw, tex, bv4(ao_0,ao_2,ao_1,ao_3));
								break;
							case 1: // -Y
								new_mesh.quad(v_2, v_3, v_0, v_1, wh, tex, bv4(ao_2,ao_3,ao_0,ao_1));
								break;
							case 2: // -Z
								new_mesh.quad(v_1, v_0, v_3, v_2, wh, tex, bv4(ao_1,ao_0,ao_3,ao_2));
								break;
							case 3: // +X
								new_mesh.quad(v_2, v_0, v_3, v_1, hw, tex, bv4(ao_2,ao_0,ao_3,ao_1));
								break;
							case 4: // +Y
								new_mesh.quad(v_0, v_1, v_2, v_3, wh, tex, bv4(ao_0,ao_1,ao_2,ao_3));
								break;
							case 5: // +Z
								new_mesh.quad(v_0, v_1, v_2, v_3, wh, tex, bv4(ao_0,ao_1,ao_2,ao_3));
								break;
							}
						}

						// Erase quad area in slice
						for(i32 h = 0; h < height; h++) {
							_memset(&slice[slice_idx + h * max[u_2d]], sizeof(block_type) * width, 0);
						}

						u += width;
					} else {
						u++;
					}
				}
			}
		}
	}

	POP_PROFILE_PROF();

	eng->platform->aquire_mutex(&swap_mut);
	mesh.swap_mesh(new_mesh);
	mesh_faces = mesh.vertices.size;
	eng->platform->release_mutex(&swap_mut);
}




#include "exile.h"
#include "world.h"
#include <engine/imgui.h>
#include <engine/dbg.h>
#include <engine/util/threadstate.h>

CALLBACK void world_debug_ui(world* w) { 

	if(ImGui::SmallButton("Regenerate"_)) {
		w->regenerate();
	}

	ImGui::SameLine();
	if(ImGui::SmallButton("Reset")) {
		w->p.reset();
	}
}

void world::regenerate() { 

	thread_pool.stop_all();
	destroy_chunks();
	chunks = map<chunk_pos, chunk*>::make(512, alloc);
	thread_pool.start_all();
}

CALLBACK void player_debug_ui(world* w) { 

	v3 intersection = w->raymarch(w->p.camera.pos, w->p.camera.front, w->p.camera.reach);
	ImGui::ViewAny("inter"_, intersection);
	ImGui::ViewAny("block"_, w->block_at(w->p.camera.pos.to_i_t()));
}

block_id world::block_at(iv3 pos) {
	block_node node = world_to_canonical(pos);
	if(!node.owner) return block_id::none;
	return node.owner->block_at(node.pos);
}

void world::init(asset_store* store, allocator* a) { PROF_FUNC

	alloc = a;

	p.reset();

	LOG_INFO_F("units_per_voxel: %"_, chunk::units_per_voxel);

	env.init(store, a);
	init_blocks(store);

	{
		LOG_DEBUG_F("% logical cores % physical cores"_, global_api->get_num_cpus(), global_api->get_phys_cpus());
		chunks = map<chunk_pos, chunk*>::make(512, a);
		thread_pool = threadpool::make(a, exile->eng->platform->get_phys_cpus() - 1);
		thread_pool.start_all();
	}

	{
		player_sightline.init(alloc);
		chunk_corners.init(alloc);
		player_hud.init(alloc);
	}

	{
		exile->eng->dbg.store.add_var("world/settings"_, &settings);
		exile->eng->dbg.store.add_var("world/time"_, &time);
		exile->eng->dbg.store.add_ele("world/ui"_, FPTR(world_debug_ui), this);

		exile->eng->dbg.store.add_var("player/cam"_, &p.camera);
		exile->eng->dbg.store.add_var("player/speed"_, &p.speed);
		exile->eng->dbg.store.add_var("player/enable"_, &p.enable);
		exile->eng->dbg.store.add_var("player/noclip"_, &p.noclip);
		exile->eng->dbg.store.add_ele("player/info"_, FPTR(player_debug_ui), this);
	}

	{
		time.last_update = global_api->get_perfcount();
		time.absolute = global_api->get_perfcount();
	}
}

void world::destroy_chunks() {  

	FORMAP(it, chunks) {
		it->value->destroy();
		PUSH_ALLOC(it->value->alloc) {
			free(it->value, sizeof(chunk));
		} POP_ALLOC();
	}
	chunks.destroy();
}

void world::destroy() { 

	env.destroy();
	thread_pool.stop_all();
	thread_pool.destroy();
	destroy_chunks();
	block_info.destroy();
	player_sightline.destroy();
	chunk_corners.destroy();
	player_hud.destroy();
}

v3 world::raymarch(v3 origin, v3 max) { 
	return raymarch(origin, max, len(max));
}

v3 world::raymarch(v3 pos3, v3 dir3, f32 max) { 

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

		if(c->block_at(iv3(current_vox.x % chunk::wid, current_vox.y % chunk::hei, current_vox.z % chunk::wid)) != block_id::none) {
			return current.xyz;
		}
		
		v4 delta = (step({}, dir) - fract(current)) / dir;
		progress += max(min_reset(delta.xyz), 0.001f);
	}

	return pos.xyz + dir.xyz * max;
}

f32 world_time::day_01() {  

	return (hour + (minute / 60.0f)) / 24.0f;
}

void world_time::update(u64 now) { PROF_FUNC

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

void world::update(u64 now) { PROF_FUNC

	time.update(now);
	update_player(now);
}

void world::local_populate() { PROF_FUNC

	i32 min = -settings.view_distance - settings.max_light_propogation - 1;
	i32 max = settings.view_distance + settings.max_light_propogation + 1;

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = min; x <= max; x++) {
		for(i32 z = min; z <= max; z++) {

			chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
			current.y = 0;
			
			if(!chunks.try_get(current)) {
				
				chunk* c = chunk::make_new(this, current, alloc);
				chunks.insert(current, c);

				chunk** xn = chunks.try_get(current - chunk_pos(1,0,0));
				if (xn) { (*xn)->neighbors[0] = c; c->neighbors[1] = *xn; }
				chunk** xp = chunks.try_get(current + chunk_pos(1,0,0));
				if(xp) { (*xp)->neighbors[1] = c; c->neighbors[0] = *xp; }
				chunk** zn = chunks.try_get(current - chunk_pos(0,0,1));
				if(zn) { (*zn)->neighbors[2] = c; c->neighbors[3] = *zn; }
				chunk** zp = chunks.try_get(current + chunk_pos(0,0,1));
				if(zp) { (*zp)->neighbors[3] = c; c->neighbors[2] = *zp; }

				chunk** xnzn = chunks.try_get(current - chunk_pos(1,0,1));
				if (xnzn) { (*xnzn)->neighbors[4] = c; c->neighbors[7] = *xnzn; }
				chunk** xnzp = chunks.try_get(current - chunk_pos(1,0,-1));
				if (xnzp) { (*xnzp)->neighbors[5] = c; c->neighbors[6] = *xnzp; }
				chunk** xpzn = chunks.try_get(current + chunk_pos(1,0,-1));
				if (xpzn) { (*xpzn)->neighbors[6] = c; c->neighbors[5] = *xpzn; }
				chunk** xpzp = chunks.try_get(current + chunk_pos(1,0,1));
				if (xpzp) { (*xpzp)->neighbors[7] = c; c->neighbors[4] = *xpzp; }
			}
		}
	}
}

void world::local_generate() { PROF_FUNC

	i32 min = -settings.view_distance - settings.max_light_propogation - 1;
	i32 max = settings.view_distance + settings.max_light_propogation + 1;

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = min; x <= max; x++) {
		for(i32 z = min; z <= max; z++) {

			chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
			current.y = 0;
			
			chunk* c = *chunks.get(current);
			
			if(c->state.get() == chunk_stage::none) {
				
				c->state.set(chunk_stage::generating);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;
					c->do_gen();
					c->state.set(chunk_stage::lit);
				}, c, 1.0f / lensq(current.center_xz() - p.camera.pos), 2, FPTR(cancel_gen));
			}
		}
	}
}

void world::local_light() { PROF_FUNC

	i32 min = -settings.view_distance;
	i32 max = settings.view_distance;

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = min; x <= max; x++) {
		for(i32 z = min; z <= max; z++) {

			chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
			current.y = 0;
			
			chunk* c = *chunks.get(current);
			
			chunk_stage stage = c->state.get();
			if(stage == chunk_stage::lit || stage == chunk_stage::meshed) {
			
				if(c->lighting_updates.empty()) {
					continue;
				}

				bool ready = true;
				for(i32 i = 0; i < 8; i++) {
					if(!c->neighbors[i] || c->neighbors[i]->state.get() < chunk_stage::lit) {
						ready = false;
						break;
					}
				}
				if(!ready) continue;

				c->state.set(chunk_stage::lighting);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;
					c->do_light();
					c->state.set(chunk_stage::lit);
				}, c, 1.0f / lensq(current.center_xz() - p.camera.pos), 1, FPTR(cancel_light));
			}
		}
	}
}

void world::local_mesh() { PROF_FUNC

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -settings.view_distance; x <= settings.view_distance; x++) {
		for(i32 z = -settings.view_distance; z <= settings.view_distance; z++) {

			chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
			current.y = 0;
			
			chunk* c = *chunks.get(current);
			
			if(c->state.get() == chunk_stage::lit) {
				
				bool ready = true;
				for(i32 i = 0; i < 8; i++) {
					if(!c->neighbors[i] || c->neighbors[i]->state.get() < chunk_stage::lit) {
						ready = false;
						break;
					}
				}
				if(!ready) continue;

				c->state.set(chunk_stage::meshing);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;
					c->do_mesh();
					c->state.set(chunk_stage::meshed);
				}, c, 1.0f / lensq(current.center_xz() - p.camera.pos), 0, FPTR(cancel_mesh));
			}
		}
	}
}

CALLBACK void unlock_chunk(chunk* c) { 

	exile->eng->platform->release_mutex(&c->swap_mut);
}

float check_pirority(super_job* j, void* param) {

	world* w = (world*)param;
	player* p = &w->p;
	chunk* c = (chunk*)j->data;

	v3 center = c->pos.center_xz();

	if(absv(center.x - p->camera.pos.x) > (f32)(w->settings.view_distance + 1) * chunk::wid ||
	   absv(center.z - p->camera.pos.z) > (f32)(w->settings.view_distance + 1) * chunk::wid) {
		return -FLT_MAX;
	}

	return 1.0f / lensq(center - p->camera.pos);
}

CALLBACK void cancel_gen(chunk* c) {
	c->state.set(chunk_stage::none);
}
CALLBACK void cancel_light(chunk* c) {
	c->state.set(chunk_stage::lit);
}
CALLBACK void cancel_mesh(chunk* c) {
	c->state.set(chunk_stage::lit);
}

void player::reset() { 

	camera.reset();
	camera.pos = {3.0f, 50.0f, 16.0f};
	speed = 5.0f;
	velocity = v3();
	last = global_api->get_perfcount();
}

void world::render() { PROF_FUNC

	exile->ren.world_clear();
	
	env.render(&p, &time);
	render_chunks();
	render_player();
}

block_node world::world_to_canonical(iv3 pos) {

	chunk_pos cpos(pos.x / chunk::wid - (pos.x < 0 ? 1 : 0), 0, pos.z / chunk::wid - (pos.z < 0 ? 1 : 0));

	block_node ret;
	ret.pos = iv3(pos.x % chunk::wid + (pos.x < 0 ? chunk::wid : 0), pos.y, pos.z % chunk::wid + (pos.z < 0 ? chunk::wid : 0));

	chunk** c = chunks.try_get(cpos);
	ret.owner = c ? *c : null;
	return ret;
}

block_meta* world::get_info(block_id id) {

	return block_info.get((u32)id);
}

void world::player_break_block() {

	v3 intersection = raymarch(p.camera.pos, p.camera.front, p.camera.reach);

	if(intersection != p.camera.pos + p.camera.front * p.camera.reach) {
		
		set_block(intersection.to_i_t(), block_id::none);		
	}
}

void world::player_place_block() {

	v3 intersection = raymarch(p.camera.pos, p.camera.front, p.camera.reach);

	if(intersection != p.camera.pos + p.camera.front * p.camera.reach) {
		
		set_block(intersection.to_i_t(), block_id::stone);		
	}
}

void world::set_block(iv3 pos, block_id id) {

	block_node local = world_to_canonical(pos);
	if(local.owner)
		local.owner->set_block(local.pos, id);
}

void world::place_light(iv3 pos, u8 intensity) {

	block_node local = world_to_canonical(pos);
	if(local.owner) {
		local.owner->place_light(local.pos, intensity);
	}
}

void world::rem_light(iv3 pos) {

	block_node local = world_to_canonical(pos);
	if(local.owner)
		local.owner->rem_light(local.pos);
}

void world_environment::init(asset_store* store, allocator* a) { PROF_FUNC

	sky.init(a);
	sky.push_dome({}, 1.0f, 64);
	sky_texture = exile->eng->ogl.add_texture(store, "sky"_, texture_wrap::mirror);

	env_texture = exile->eng->ogl.add_texture(store, "env"_);

	stars.init(a);
	stars.push_points({}, 1.0f, 5000, 0.1f);
}

void world_environment::destroy() {

	sky.destroy();
	stars.destroy();
}

void world_environment::render(player* p, world_time* t) { PROF_FUNC

	m4 mproj = p->camera.proj((f32)exile->eng->window.settings.w / (f32)exile->eng->window.settings.h);
	m4 view_no_trans = p->camera.view_pos_origin();

	exile->ren.world_skydome(sky.gpu, t, sky_texture, view_no_trans, mproj);
	exile->ren.world_stars(stars.gpu, t, view_no_trans, mproj);
}

void world::render_chunks() { PROF_FUNC

	local_populate();
	local_generate();
	local_light();
	local_mesh();

	thread_pool.renew_priorities(check_pirority, this);

	exile->ren.world_begin_chunks(this, settings.draw_chunk_corners);

	{PROF_SCOPE("Build Render List"_);

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -settings.view_distance; x <= settings.view_distance; x++) {
		for(i32 z = -settings.view_distance; z <= settings.view_distance; z++) {

			chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
			current.y = 0;
			chunk* c = *chunks.try_get(current);;

			exile->eng->platform->aquire_mutex(&c->swap_mut);
			if(!c->mesh.dirty) {
				c->mesh.free_cpu();
			}

			v3 chunk_pos = v3((f32)current.x * chunk::wid, (f32)current.y * chunk::hei, (f32)current.z * chunk::wid);
			m4 model = translate(chunk_pos - p.camera.pos);
			m4 view = p.camera.view_pos_origin();
			m4 proj = p.camera.proj((f32)exile->eng->window.settings.w / (f32)exile->eng->window.settings.h);

			exile->ren.world_chunk(c, block_textures, env.sky_texture, model, view, proj);
		}
	}

	exile->ren.world_finish_chunks();

	if(settings.draw_chunk_corners) {
		
		chunk_corners.clear();

		for(i32 x = -settings.view_distance; x <= settings.view_distance + 1; x++) {
			for(i32 z = -settings.view_distance; z <= settings.view_distance + 1; z++) {

				chunk_pos current = settings.respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);

				f32 fx = (f32)current.x * chunk::wid;
				f32 fz = (f32)current.z * chunk::wid;
				chunk_corners.push(v3(fx, 0.0f, fz), v3(fx, (f32)chunk::hei, fz), colorf(1,0,0,1));
			}
		}

		m4 view = p.camera.view();
		m4 proj = p.camera.proj((f32)exile->eng->window.settings.w / (f32)exile->eng->window.settings.h);

		exile->ren.world_lines(chunk_corners.gpu, view, proj);
	}
	}
}

void world::render_player() { PROF_FUNC

	render_camera& cam = p.camera;

	if(cam.mode == camera_mode::third) {

		player_sightline.clear();
		player_sightline.push(cam.pos + cam.front, cam.pos + cam.reach * cam.front, colorf(0,0,1,1), colorf(0,1,0,1));
		player_sightline.push(cam.pos, cam.pos + cam.front, colorf(1,0,0,1), colorf(0,0,1,1));

		v3 intersection = raymarch(cam.pos, cam.front, cam.reach);

		player_sightline.push(cam.pos, intersection, colorf(0,0,0,1), colorf(0,0,0,1));

		m4 view = cam.view();
		m4 proj = cam.proj((f32)exile->eng->window.settings.w / (f32)exile->eng->window.settings.h);
		
		exile->ren.world_lines(player_sightline.gpu, view, proj);
	}

	{
		player_hud.clear();
		
		f32 w = (f32)exile->eng->window.settings.w, h = (f32)exile->eng->window.settings.h;

		player_hud.push_rect(r2(w / 2.0f - 5.0f, h / 2.0f - 1.0f, 10.0f, 2.0f), WHITE);
		player_hud.push_rect(r2(w / 2.0f - 1.0f, h / 2.0f - 5.0f, 2.0f, 10.0f), WHITE);

		exile->ren.hud_2D(player_hud.gpu);
	}
}

void world::update_player(u64 now) { PROF_FUNC

	render_camera& cam = p.camera;

	u64 pdt = now - p.last;
	f64 dt = (f64)pdt / (f64)exile->eng->platform->get_perfcount_freq();

	if(p.enable) {

		v3 accel = v3(0.0f, -settings.gravity, 0.0f);
		v3 mov_v;

		if(exile->eng->platform->window_focused(&exile->eng->window)) {

			if(exile->eng->platform->keydown(platform_keycode::w)) {
				mov_v += cam.front * p.speed;
			}
			if(exile->eng->platform->keydown(platform_keycode::a)) {
				mov_v += cam.right * -p.speed;
			}
			if(exile->eng->platform->keydown(platform_keycode::s)) {
				mov_v += cam.front * -p.speed;
			}
			if(exile->eng->platform->keydown(platform_keycode::d)) {
				mov_v += cam.right * p.speed;
			}
			mov_v.y = 0.0f;
			if(exile->eng->platform->keydown(platform_keycode::space)) {
				mov_v.y += p.speed;
			}
			if(exile->eng->platform->keydown(platform_keycode::lshift)) {
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

inline u32 hash(chunk_pos key) { 

	return hash(key.x) ^ hash(key.y) ^ hash(key.z);
}

chunk_pos::chunk_pos(i32 _x, i32 _y, i32 _z) {
	x = _x; y = _y; z = _z;
}

bool operator==(chunk_pos l, chunk_pos r) { 

	return l.x == r.x && l.y == r.y && l.z == r.z;
}

chunk_pos chunk_pos::from_abs(v3 pos) { 

	chunk_pos ret;
	ret.x = (i32)(pos.x / chunk::wid) - (pos.x < 0 ? 1 : 0);
	ret.y = (i32)(pos.y / chunk::hei) - (pos.y < 0 ? 1 : 0);
	ret.z = (i32)(pos.z / chunk::wid) - (pos.z < 0 ? 1 : 0);
	return ret;
}

v3 chunk_pos::offset() {

	return v3(x * chunk::wid, y * chunk::hei, z * chunk::wid);
}

v3 chunk_pos::center_xz() { 

	return v3(x * chunk::wid + chunk::wid / 2.0f, 0.0f, z * chunk::wid + chunk::wid / 2.0f);
}

chunk_pos chunk_pos::operator+(chunk_pos other) { 
	chunk_pos ret;
	ret.x = x + other.x;
	ret.y = y + other.y;
	ret.z = z + other.z;
	return ret;
}

chunk_pos chunk_pos::operator-(chunk_pos other) { 
	chunk_pos ret;
	ret.x = x - other.x;
	ret.y = y - other.y;
	ret.z = z - other.z;
	return ret;
}

void chunk::init(world* _w, chunk_pos p, allocator* a) { 

	w = _w;
	pos = p;
	alloc = a;

	mesh.init_gpu();
	
	exile->eng->platform->create_mutex(&swap_mut, false);
	lighting_updates = locking_queue<light_work>::make(4, alloc);
	lights = vector<dynamic_torch>::make(32, alloc);
}

void chunk::set_block(iv3 p, block_id id) {

	light_work u;
	u.type = light_update::block;
	u.pos = p;
	u.id = id;
	lighting_updates.push(u);

	block_meta* info = w->get_info(id);
	if(info->emit_light > 0) {
		u.type = light_update::add;
		u.intensity = info->emit_light;
		lighting_updates.push(u);
	}
}

void chunk::place_light(iv3 p, u8 i) { 

	light_work u;
	u.type = light_update::add;
	u.pos = p;
	u.intensity = i;

	lighting_updates.push(u);

	dynamic_torch t;
	t.pos = p.to_f() + v3(0.5f, 0.5f, 0.5f);
	t.col = v3((f32)i);

	lights.push(t);
}

void chunk::rem_light(iv3 p) { 

	light_work u;
	u.type = light_update::remove;
	u.pos = p;

	lighting_updates.push(u);
}

chunk* chunk::make_new(world* w, chunk_pos p, allocator* a) { 

	PUSH_ALLOC(a);

	chunk* ret = NEW(chunk);

	ret->init(w, p, a);

	POP_ALLOC();

	return ret;
}

void chunk::destroy() { 

	lights.destroy();
	lighting_updates.destroy();
	mesh.destroy();
	exile->eng->platform->destroy_mutex(&swap_mut);
}

i32 chunk::y_at(i32 x, i32 z) { 

	f32 y_max = 512;
	f32 val = perlin((f32)x / 32.0f, 0, (f32)z / 32.0f, 0, 0, 0);
	i32 height = (i32)(val * y_max / 2.0f + y_max / 2.0f) / 2;

	return height / 2;
}

void chunk::do_gen() { PROF_FUNC

	LOG_DEBUG_F("Generating chunk %"_, pos);

	for(u32 x = 0; x < wid; x++) {
		for(u32 z = 0; z < wid; z++) {

			u32 height = y_at(pos.x * wid + x, pos.z * wid + z);

			blocks[x][z][0] = block_id::bedrock;
			for(u32 y = 1; y < height; y++) {
				blocks[x][z][y] = block_id::stone;
			}

			if(x % 4 == 0 && z % 4 == 0) {
				blocks[x][z][height] = block_id::torch;
				place_light(iv3(x, height, z), 16);
			} else {
				// blocks[x][z][height] = block_id::stone_slab;
			}
		}
	}

	light_work sun;
	sun.type = light_update::gen_sun;
	lighting_updates.push(sun);
}

void chunk::light_rem_sun(light_work work) { PROF_FUNC

	block_light& first = light[work.pos.x][work.pos.z][work.pos.y];

	queue<light_rem_node> q = queue<light_rem_node>::make(2048, &this_thread_data.scratch_arena);

	light_rem_node begin;
	begin.pos = work.pos;
	begin.owner = this;
	begin.val = first.s0;
	first.s0 = 0;

	q.push(begin);

	while(!q.empty()) {

		light_rem_node cur = q.pop();
		u8 current_light = cur.val;

		for(i32 i = 0; i < 6; i++) {

			iv3 neighbor = cur.pos + g_directions[i];
			block_node node = cur.owner->canonical_block(neighbor);
			block_light nval = node.get_l();

			if(nval.s0 == 0) continue;

			u8 test = current_light + (i == 1 && current_light == 15 ? 1 : 0);
			if(nval.s0 < test) {
				
				node.set_s(0);
				light_rem_node new_node;
				new_node.pos = node.pos;
				new_node.owner = node.owner;
				new_node.val = nval.s0;
				q.push(new_node);

				if(node.owner->lighting_updates.empty()) {
					light_work t; t.type = light_update::trigger;
					node.owner->lighting_updates.push(t);
				}

			} else {
				light_work fill;
				fill.type = light_update::add_sun;
				fill.pos = node.pos;
				fill.intensity = nval.s0;
				node.owner->lighting_updates.push(fill);
			}
		}
	}

	RESET_ARENA(&this_thread_data.scratch_arena);
}

void chunk::light_add_sun(light_work work) { PROF_FUNC

	light[work.pos.x][work.pos.z][work.pos.y].s0 = work.intensity;

	queue<block_node> q = queue<block_node>::make(2048, &this_thread_data.scratch_arena);

	block_node begin;
	begin.pos = work.pos;
	begin.owner = this;
	q.push(begin);

	while(!q.empty()) {

		block_node cur = q.pop();
		u8 current_light = cur.owner->l_at(cur.pos).light.s0;

		for(i32 i = 0; i < 6; i++) {
			
			iv3 neighbor = cur.pos + g_directions[i];
			block_node node = cur.owner->canonical_block(neighbor);

			if(!node.owner) continue;

			u8 test = current_light - (i == 1 && current_light == 15 ? 0 : 1);

			if(node.get_l().s0 < test && !w->get_info(node.get_type())->opaque[(i + 3) % 6]) {

				node.set_s(test);
				q.push(node);

				if(node.owner->lighting_updates.empty()) {
					light_work t; t.type = light_update::trigger;
					node.owner->lighting_updates.push(t);
				}
			}
		}
	}

	RESET_ARENA(&this_thread_data.scratch_arena);
}

void chunk::light_add(light_work work) { PROF_FUNC

	light[work.pos.x][work.pos.z][work.pos.y].t = work.intensity;

	queue<block_node> q = queue<block_node>::make(2048, &this_thread_data.scratch_arena);

	block_node begin;
	begin.pos = work.pos;
	begin.owner = this;
	q.push(begin);

	while(!q.empty()) {

		block_node cur = q.pop();
		u8 current_light = cur.owner->l_at(cur.pos).light.t;

		for(i32 i = 0; i < 6; i++) {
			
			iv3 neighbor = cur.pos + g_directions[i];
			block_node node = cur.owner->canonical_block(neighbor);

			if(!node.owner) continue;
			if(node.get_l().t < current_light - 1 && !w->get_info(node.get_type())->opaque[(i + 3) % 6]) {

				node.set_l(current_light - 1);
				q.push(node);

				if(node.owner->lighting_updates.empty()) {
					light_work t; t.type = light_update::trigger;
					node.owner->lighting_updates.push(t);
				}
			}
		}
	}

	RESET_ARENA(&this_thread_data.scratch_arena);
}

void chunk::light_remove(light_work work) { PROF_FUNC

	block_light& first = light[work.pos.x][work.pos.z][work.pos.y];

	queue<light_rem_node> q = queue<light_rem_node>::make(2048, &this_thread_data.scratch_arena);

	light_rem_node begin;
	begin.pos = work.pos;
	begin.owner = this;
	begin.val = first.t;
	first.t = 0;

	q.push(begin);

	while(!q.empty()) {

		light_rem_node cur = q.pop();
		u8 current_light = cur.val;

		for(i32 i = 0; i < 6; i++) {
			
			iv3 neighbor = cur.pos + g_directions[i];
			block_node node = cur.owner->canonical_block(neighbor);
			block_light nval = node.get_l();

			if(nval.t == 0) continue;

			if(nval.t < current_light) {
				node.set_l(0);
				light_rem_node new_node;
				new_node.pos = node.pos;
				new_node.owner = node.owner;
				new_node.val = nval.t;
				q.push(new_node);
				
				u8 emit = w->get_info(node.get_type())->emit_light;
				if(emit > 0) {
					light_work fill;
					fill.type = light_update::add;
					fill.pos = node.pos;
					fill.intensity = emit;
					node.owner->lighting_updates.push(fill);
				} else if(node.owner->lighting_updates.empty()) {
					light_work t; t.type = light_update::trigger;
					node.owner->lighting_updates.push(t);
				}
				
			} else {
				light_work fill;
				fill.type = light_update::add;
				fill.pos = node.pos;
				fill.intensity = nval.t;
				node.owner->lighting_updates.push(fill);
			}
		}
	}

	RESET_ARENA(&this_thread_data.scratch_arena);
}

void chunk::do_light() { PROF_FUNC

	LOG_DEBUG_F("Lighting chunk %"_, pos);

	light_work work;
	while(lighting_updates.try_pop(&work)) {

		if(work.type == light_update::add_sun) {
	
			light_add_sun(work);

		} else if(work.type == light_update::remove_sun) {

			light_rem_sun(work);

		} else if(work.type == light_update::gen_sun) {

			for(i32 x = 0; x < wid; x++) {
				for(i32 z = 0; z < wid; z++) {
					for(i32 y = hei - 1; y >= 0; y--) {

						block_meta* info = w->get_info(block_at(iv3(x,y,z)));
						if(!info->opaque[4]) {
							light[x][z][y].s0 = 15;
						} else {
							light_work add;
							add.type = light_update::add_sun;
							add.pos = iv3(x,y,z);
							add.intensity = 15;
							light_add_sun(add);
							break;
						}
					}
				}
			}

		} else if(work.type == light_update::block) {

			blocks[work.pos.x][work.pos.z][work.pos.y] = work.id;

			light_work rem;
			rem.type = light_update::remove;
			rem.pos = work.pos;
			lighting_updates.push(rem);

			rem.type = light_update::remove_sun;
			lighting_updates.push(rem);

			for(i32 i = 0; i < 6; i++) {
				iv3 neighbor = work.pos + g_directions[i];
				block_node node = canonical_block(neighbor);
				if(node.owner->lighting_updates.empty()) {
					light_work t; t.type = light_update::trigger;
					node.owner->lighting_updates.push(t);
				}
			}

		} else if(work.type == light_update::add) {

			light_add(work);

		} else if(work.type == light_update::remove) {
		
			light_remove(work);
		}
	}
}

void block_node::set_l(u8 intensity) {

	if(owner)
		owner->light[pos.x][pos.z][pos.y].t = intensity;
}

void block_node::set_s(u8 intensity) {

	if(owner)
		owner->light[pos.x][pos.z][pos.y].s0 = intensity;	
}

block_id block_node::get_type() { 

	if (!owner) return block_id::none;
	return owner->blocks[pos.x][pos.z][pos.y];
}

block_light block_node::get_l() { 

	if(pos.y >= chunk::hei) {
		block_light l;
		l.s0 = 15;
		return l;
	}

	if (!owner) return {};
	
	return owner->light[pos.x][pos.z][pos.y];
}

bool block_node::propogate_light_through_vert(world* w, i32 dir) { 

	i32 x = pos.x, y = pos.y, z = pos.z;

	switch(dir) {
	case 0: {
		return !w->get_info(owner->block_at(iv3(x-1,y,z)))->opaque[3] ||
			   !w->get_info(owner->block_at(iv3(x-1,y-1,z)))->opaque[3] ||
			   !w->get_info(owner->block_at(iv3(x-1,y,z-1)))->opaque[3] ||
			   !w->get_info(owner->block_at(iv3(x-1,y-1,z-1)))->opaque[3];
	} break;
	case 1: {
		return !w->get_info(owner->block_at(iv3(x,y-1,z)))->opaque[4] ||
			   !w->get_info(owner->block_at(iv3(x-1,y-1,z)))->opaque[4] ||
			   !w->get_info(owner->block_at(iv3(x,y-1,z-1)))->opaque[4] ||
			   !w->get_info(owner->block_at(iv3(x-1,y-1,z-1)))->opaque[4];
	} break;
	case 2: {
		return !w->get_info(owner->block_at(iv3(x,y,z-1)))->opaque[5] ||
			   !w->get_info(owner->block_at(iv3(x-1,y,z-1)))->opaque[5] ||
			   !w->get_info(owner->block_at(iv3(x,y-1,z-1)))->opaque[5] ||
			   !w->get_info(owner->block_at(iv3(x-1,y-1,z-1)))->opaque[5];
	} break;
	case 3: {
		return !w->get_info(owner->block_at(iv3(x,y,z)))->opaque[0] ||
			   !w->get_info(owner->block_at(iv3(x,y-1,z)))->opaque[0] ||
			   !w->get_info(owner->block_at(iv3(x,y,z-1)))->opaque[0] ||
			   !w->get_info(owner->block_at(iv3(x,y-1,z-1)))->opaque[0];
	} break;
	case 4: {
		return !w->get_info(owner->block_at(iv3(x,y,z)))->opaque[1] ||
			   !w->get_info(owner->block_at(iv3(x-1,y,z)))->opaque[1] ||
			   !w->get_info(owner->block_at(iv3(x,y,z-1)))->opaque[1] ||
			   !w->get_info(owner->block_at(iv3(x-1,y,z-1)))->opaque[1];
	} break;
	case 5: {
		return !w->get_info(owner->block_at(iv3(x,y,z)))->opaque[2] ||
			   !w->get_info(owner->block_at(iv3(x-1,y,z)))->opaque[2] ||
			   !w->get_info(owner->block_at(iv3(x,y-1,z)))->opaque[2] ||
			   !w->get_info(owner->block_at(iv3(x-1,y-1,z)))->opaque[2];
	} break;
	}

	INVALID_PATH;
	return false;
}

block_node chunk::canonical_block(iv3 block) { 

	i32 x = block.x, y = block.y, z = block.z;

	block_node same;
	same.pos = block;

	if (y < 0 || y >= hei) {
		return same;
	}

	if(x < 0 || x >= wid || z < 0 || z >= wid) {

		chunk_pos offset(x / wid - (x < 0 ? 1 : 0), 0, z / wid - (z < 0 ? 1 : 0));
		// TODO(max): use offset to calculate index

		if(offset.x == 1 && offset.z == 0 && neighbors[0]) {
			same.owner = neighbors[0]; 
			same.pos = iv3(x - wid, y, z);
			return same;
		} else if(offset.x == -1 && offset.z == 0 && neighbors[1]) {
			same.owner = neighbors[1]; 
			same.pos = iv3(x + wid, y, z);
			return same;
		} else if(offset.x == 0 && offset.z == 1 && neighbors[2]) {
			same.owner = neighbors[2]; 
			same.pos = iv3(x, y, z - wid);
			return same;
		} else if(offset.x == 0 && offset.z == -1 && neighbors[3]) {
			same.owner = neighbors[3]; 
			same.pos = iv3(x, y, z + wid);
			return same;
		} else if(offset.x == 1 && offset.z == 1 && neighbors[4]) {
			same.owner = neighbors[4]; 
			same.pos = iv3(x - wid, y, z - wid);
			return same;
		} else if(offset.x == 1 && offset.z == -1 && neighbors[5]) {
			same.owner = neighbors[5]; 
			same.pos = iv3(x - wid, y, z + wid);
			return same;
		} else if(offset.x == -1 && offset.z == 1 && neighbors[6]) {
			same.owner = neighbors[6]; 
			same.pos = iv3(x + wid, y, z - wid);
			return same;
		} else if(offset.x == -1 && offset.z == -1 && neighbors[7]) {
			same.owner = neighbors[7]; 
			same.pos = iv3(x + wid, y, z + wid);
			return same;
		}

		return same;
	}

	same.owner = this;
	return same;
}

light_at chunk::l_at(iv3 block) {

	block_node node = canonical_block(block);	

	light_at ret;
	ret.solid = w->get_info(node.get_type())->solid;
	ret.light = node.get_l();

	return ret;
}

u8 block_light::first_u8() {

	return (s0 << 4) | (t >= 15 ? 15 : t);
}

void light_gather::operator+=(light_at l) {
	if(l.solid) return;
	t += l.light.t; 
	s0 += l.light.s0;
	contrib++;
}

bool operator==(light_gather l, light_gather r) {
	return l.t==r.t && l.s0==r.s0;
}

light_gather chunk::gather_l(iv3 vert) {

	light_gather g;

	g += l_at(vert);
	g += l_at(vert + iv3(-1,0,0));
	g += l_at(vert + iv3(0,0,-1));
	g += l_at(vert + iv3(-1,0,-1));
	g += l_at(vert + iv3(0,-1,0));
	g += l_at(vert + iv3(-1,-1,0));
	g += l_at(vert + iv3(0,-1,-1));
	g += l_at(vert + iv3(-1,-1,-1));

	return g;
}

u8 chunk::l_at_vert(iv3 vert) { 

	light_gather g = gather_l(vert);

	u8 div = g.contrib ? g.contrib : 1;

	u8 t = (u8)min(g.t / div, 15);
	u8 s = (u8)(g.s0 / div);

	return (s << 4) | t;
}

u8 chunk::ao_at_vert(iv3 vert) { 

	i32 x = vert.x, y = vert.y, z = vert.z;

	bool top0 = w->get_info(block_at(iv3(x-1,y,z)))->does_ao;
	bool top1 = w->get_info(block_at(iv3(x,y,z-1)))->does_ao;
	bool top2 = w->get_info(block_at(iv3(x,y,z)))->does_ao;
	bool top3 = w->get_info(block_at(iv3(x-1,y,z-1)))->does_ao;
	bool bot0 = w->get_info(block_at(iv3(x-1,y-1,z)))->does_ao;

	bool side0, side1, corner;

	if(!top0 && bot0) {
		side0 = top2;
		side1 = top3;
		corner = top1;
	} else {

	bool bot1 = w->get_info(block_at(iv3(x,y-1,z-1)))->does_ao;

	if(!top1 && bot1) {
		side0 = top2;
		side1 = top3;
		corner = top0;
	} else {
	
	bool bot2 = w->get_info(block_at(iv3(x,y-1,z)))->does_ao;

	if(!top2 && bot2) {
		side0 = top0;
		side1 = top1;
		corner = top3;
	} else {
	
	bool bot3 = w->get_info(block_at(iv3(x-1,y-1,z-1)))->does_ao;

	if(!top3 && bot3) {
		side0 = top0;
		side1 = top1;
		corner = top2;
	} else {

		return 3;

	}}}}

	if(side0 && side1) {
		return 0;
	}
	return 3 - side0 - side1 - corner;
}

block_id chunk::block_at(iv3 block) { 

	block_node node = canonical_block(block);	

	if(!node.owner) return block_id::none;

	return node.owner->blocks[node.pos.x][node.pos.z][node.pos.y];
}

mesh_face chunk::build_face(block_id t, iv3 p, i32 dir) { 

	mesh_face ret;
	ret.info = w->get_info(t);

	switch(dir) {
	case 0: {
		ret.l[0] = gather_l(p);
		ret.l[1] = gather_l(p + iv3(0,1,0));
		ret.l[2] = gather_l(p + iv3(0,0,1));
		ret.l[3] = gather_l(p + iv3(0,1,1));
	} break;
	case 1: {
		ret.l[0] = gather_l(p);
		ret.l[1] = gather_l(p + iv3(1,0,0));
		ret.l[2] = gather_l(p + iv3(0,0,1));
		ret.l[3] = gather_l(p + iv3(1,0,1));
	} break;
	case 2: {
		ret.l[0] = gather_l(p);
		ret.l[1] = gather_l(p + iv3(1,0,0));
		ret.l[2] = gather_l(p + iv3(0,1,0));
		ret.l[3] = gather_l(p + iv3(1,1,0));
	} break;
	case 3: {
		ret.l[0] = gather_l(p + iv3(1,0,0));
		ret.l[1] = gather_l(p + iv3(1,1,0));
		ret.l[2] = gather_l(p + iv3(1,0,1));
		ret.l[3] = gather_l(p + iv3(1,1,1));
	} break;
	case 4: {
		ret.l[0] = gather_l(p + iv3(0,1,0));
		ret.l[1] = gather_l(p + iv3(1,1,0));
		ret.l[2] = gather_l(p + iv3(0,1,1));
		ret.l[3] = gather_l(p + iv3(1,1,1));
	} break;
	case 5: {
		ret.l[0] = gather_l(p + iv3(0,0,1));
		ret.l[1] = gather_l(p + iv3(1,0,1));
		ret.l[2] = gather_l(p + iv3(0,1,1));
		ret.l[3] = gather_l(p + iv3(1,1,1));
	} break;
	}

	return ret;
}

bool mesh_face::can_merge(mesh_face f1, mesh_face f2, i32 dir) { 

	if(f1.info->type != f2.info->type) return false;

	if(!f1.info->merge[dir] || !f2.info->merge[dir]) return false;

	// NOTE(max): kind of average-case optimization here (for no or full lighting)
	// theoretically we could merge faces that support a smooth light transition
	// across the combined face, but this breaks non-smooth lighting and is more 
	// intensive to compute

	// TODO(max): do I really want to keep non-smooth lighting? I just got rid of 
	// its AO system. ALSO, do we even need to merge faces?? It's not much of an
	// optimization with jagged terrain.

	if(f1.l[0] == f1.l[1] && f1.l[0] == f1.l[2] && f1.l[0] == f1.l[3] &&
	   f1.l[0] == f2.l[0] && f1.l[0] == f2.l[1] && f1.l[0] == f2.l[2] && f1.l[0] == f2.l[3]) return true;

	return false;
}

void chunk::do_mesh() { PROF_FUNC

	// TODO(max): optimize this function
		// the blocks should only be traversed x z y 
		// there's definitely some way to SIMD-ize dimensions or patches

	LOG_DEBUG_F("Meshing chunk %"_, pos);

	mesh_chunk new_mesh = mesh_chunk::make_cpu(8192, alloc);

	// Array to hold 2D block slice (sized for largest slice)
	block_id slice[wid * hei];

	iv3 max = {wid, hei, wid};

	//  0  1  2  3  4  5 
	// -x -y -z +x +y +z
	for(i32 i = 0; i < 6; i++) {

		// Axes of 2D slice to mesh
		i32 ortho_2d = i % 3;
		i32 u_2d = (i + 1) % 3;
		i32 v_2d = (i + 2) % 3;
		i32 backface_offset = i / 3 * 2 - 1;

		// Iterate over orthogonal slice
		iv3 position;
		for(position[ortho_2d] = 0; position[ortho_2d] < max[ortho_2d]; position[ortho_2d]++) {
 	
 			{PROF_SCOPE("2D Slice"_);
				// Iterate over 2D slice blocks to filter culled faces before greedy step
				for(position[v_2d] = 0; position[v_2d] < max[v_2d]; position[v_2d]++) {
					for(position[u_2d] = 0; position[u_2d] < max[u_2d]; position[u_2d]++) {

						block_id block = blocks[position[0]][position[2]][position[1]];
						block_meta* info0 = w->get_info(block);
						
						i32 slice_idx = position[u_2d] + position[v_2d] * max[u_2d];

						// Only add the face to the slice if its opposing face is not opaque
						if(info0->renders) {
							
							iv3 backface = position;
							backface[ortho_2d] += backface_offset;

							block_id backface_block = block_at(backface);
							block_meta* info1 = w->get_info(backface_block);

							if(!info0->opaque[i] || !info1->renders || !info1->opaque[(i + 3) % 6]) {
								slice[slice_idx] = block;
							} else {
								slice[slice_idx] = block_id::none;
							}
						} else {
							slice[slice_idx] = block_id::none;
						}
					}
				}
			}

			// Iterate over slice filled with relevant faces
			for(i32 v = 0; v < max[v_2d]; v++) {
				for(i32 u = 0; u < max[u_2d];) {

					position[u_2d] = u;
					position[v_2d] = v;
					i32 slice_idx = u + v * max[u_2d];

					block_id single_type = slice[slice_idx];
					
					if(single_type != block_id::none) {

						mesh_face face_type = build_face(single_type, position, i);

						i32 width = 1, height = 1;

						{PROF_SCOPE("Merge"_);
							// Combine same faces in +u_2d
							for(; u + width < max[u_2d] && width < 31; width++) {

								iv3 w_pos = position;
								w_pos[u_2d] += width;

								mesh_face merge = build_face(slice[slice_idx + width], w_pos, i);

								if(!mesh_face::can_merge(merge, face_type, i)) break;
							}

							// Combine all-same face row in +v_2d
							bool done = false;
							for(; v + height < max[v_2d] && height < 31; height++) {
								for(i32 row_idx = 0; row_idx < width; row_idx++) {

									iv3 wh_pos = position;
									wh_pos[u_2d] += row_idx;
									wh_pos[v_2d] +=  height;

									mesh_face merge = build_face(slice[slice_idx + row_idx + height * max[u_2d]], wh_pos, i);

									if(!mesh_face::can_merge(merge, face_type, i)) {
										done = true;
										break;
									}
								}
								if(done) {
									break;
								}
							}
						}

						// Add quad (u,v,width,height) in 2D slice

						iv3 width_offset, height_offset;
						width_offset[u_2d] = width;
						height_offset[v_2d] = height;

						iv3 v_0 = position;
						if(backface_offset > 0) {
							v_0[ortho_2d] += 1;
						}

						iv3 v_1 = v_0 + width_offset;
						iv3 v_2 = v_0 + height_offset;
						iv3 v_3 = v_2 + width_offset;
						iv2 wh(width, height), hw(height, width);
						 	
						u8 l, l_0, l_1, l_2, l_3, ao_0, ao_1, ao_2, ao_3;
						{PROF_SCOPE("Light"_);
							l_0 = l_at_vert(v_0); l_1 = l_at_vert(v_1); l_2 = l_at_vert(v_2); l_3 = l_at_vert(v_3);
							ao_0 = ao_at_vert(v_0); ao_1 = ao_at_vert(v_1); ao_2 = ao_at_vert(v_2); ao_3 = ao_at_vert(v_3);

							iv3 facing = v_0;
							if(backface_offset < 0) facing[ortho_2d] -= 1;

							light_at face_light = l_at(facing);
							l = face_light.light.t;
							l = l >= 15 ? 15 : l;
							l |= face_light.light.s0 << 4;
						}

						v_0 *= units_per_voxel; v_1 *= units_per_voxel; v_2 *= units_per_voxel; v_3 *= units_per_voxel;
						wh *= units_per_voxel; hw *= units_per_voxel;

						i32 tex = face_type.info->textures[i];

						{PROF_SCOPE("Model"_);

							if(face_type.info->custom_model) {

								face_type.info->model(&new_mesh, face_type.info, i, v_0 / units_per_voxel, iv2(width, height), l, bv4(ao_0,ao_1,ao_2,ao_3), bv4(l_0,l_1,l_2,l_3));

							} else {

								switch (i) {
								case 0: // -X
									new_mesh.quad(v_0, v_2, v_1, v_3, hw, tex, l, bv4(ao_0,ao_2,ao_1,ao_3), bv4(l_0,l_2,l_1,l_3));
									break;
								case 1: // -Y
									new_mesh.quad(v_2, v_3, v_0, v_1, wh, tex, l, bv4(ao_2,ao_3,ao_0,ao_1), bv4(l_2,l_3,l_0,l_1));
									break;
								case 2: // -Z
									new_mesh.quad(v_1, v_0, v_3, v_2, wh, tex, l, bv4(ao_1,ao_0,ao_3,ao_2), bv4(l_1,l_0,l_3,l_2));
									break;
								case 3: // +X
									new_mesh.quad(v_2, v_0, v_3, v_1, hw, tex, l, bv4(ao_2,ao_0,ao_3,ao_1), bv4(l_2,l_0,l_3,l_1));
									break;
								case 4: // +Y
									new_mesh.quad(v_0, v_1, v_2, v_3, wh, tex, l, bv4(ao_0,ao_1,ao_2,ao_3), bv4(l_0,l_1,l_2,l_3));
									break;
								case 5: // +Z
									new_mesh.quad(v_0, v_1, v_2, v_3, wh, tex, l, bv4(ao_0,ao_1,ao_2,ao_3), bv4(l_0,l_1,l_2,l_3));
									break;
								}
							}
						}

						// Erase quad area in slice
						for(i32 h = 0; h < height; h++) {
							_memset(&slice[slice_idx + h * max[u_2d]], sizeof(block_id) * width, 0);
						}

						u += width;
					} else {
						u++;
					}
				}
			}
		}
	}

	exile->eng->platform->aquire_mutex(&swap_mut);
	mesh.swap_mesh(new_mesh);
	mesh_faces = mesh.quads.size;
	exile->eng->platform->release_mutex(&swap_mut);
}



CALLBACK void slab_model(mesh_chunk* m, block_meta* info, i32 dir, iv3 v__0, iv2 ex, u8 ql, bv4 ao, bv4 l) {

	i32 u_2d = (dir + 1) % 3;
	i32 v_2d = (dir + 2) % 3;

	f32 w = (f32)ex.x, h = (f32)ex.y;

	v3 v_0 = v__0.to_f();

	if(u_2d == 1) {
		w /= 2.0f;
	} else if(v_2d == 1) {
		h /= 2.0f;
	} else if(dir == 4) {
		v_0 -= v3(0.0f, 0.5f, 0.0f);
	}

	v3 woff, hoff;
	woff[u_2d] = (f32)w;
	hoff[v_2d] = (f32)h;
	v3 v_1 = v_0 + woff;
	v3 v_2 = v_0 + hoff;
	v3 v_3 = v_2 + woff;

	v2 wh = v2(w, h), hw = v2(h, w);

	const f32 units = (f32)chunk::units_per_voxel;
	v_0 *= units; v_1 *= units; v_2 *= units; v_3 *= units;
	wh *= units; hw *= units;

	i32 tex = info->textures[dir];

	switch (dir) {
	case 0: // -X
		m->quad(v_0.to_i(), v_2.to_i(), v_1.to_i(), v_3.to_i(), hw.to_i(), tex, ql, bv4(ao.x,ao.z,ao.y,ao.w), bv4(l.x,l.z,l.y,l.w));
		break;
	case 1: // -Y
		m->quad(v_2.to_i(), v_3.to_i(), v_0.to_i(), v_1.to_i(), wh.to_i(), tex, ql, bv4(ao.z,ao.w,ao.x,ao.y), bv4(l.z,l.w,l.x,l.y));
		break;
	case 2: // -Z
		m->quad(v_1.to_i(), v_0.to_i(), v_3.to_i(), v_2.to_i(), wh.to_i(), tex, ql, bv4(ao.y,ao.x,ao.w,ao.z), bv4(l.y,l.x,l.w,l.z));
		break;
	case 3: // +X
		m->quad(v_2.to_i(), v_0.to_i(), v_3.to_i(), v_1.to_i(), hw.to_i(), tex, ql, bv4(ao.z,ao.x,ao.w,ao.y), bv4(l.z,l.x,l.w,l.y));
		break;
	case 4: // +Y
		m->quad(v_0.to_i(), v_1.to_i(), v_2.to_i(), v_3.to_i(), wh.to_i(), tex, ql, bv4(ao.x,ao.y,ao.z,ao.w), bv4(l.x,l.y,l.z,l.w));
		break;
	case 5: // +Z
		m->quad(v_0.to_i(), v_1.to_i(), v_2.to_i(), v_3.to_i(), wh.to_i(), tex, ql, bv4(ao.x,ao.y,ao.z,ao.w), bv4(l.x,l.y,l.z,l.w));
		break;
	}
}

CALLBACK void torch_model(mesh_chunk* m, block_meta* info, i32 dir, iv3 v__0, iv2 ex, u8 ql, bv4 ao, bv4 l) {
	
	i32 o_2d = dir % 3;
	i32 u_2d = (dir + 1) % 3;
	i32 v_2d = (dir + 2) % 3;

	v3 v_0 = v__0.to_f();
	f32 w = (f32)ex.x, h = (f32)ex.y;

	if(o_2d == 1) {
		w /= 8.0f;
		h /= 8.0f;
	} else if(o_2d == 2) {
		w /= 8.0f;
		h /= 1.5f;
	} else {
		w /= 1.5f;
		h /= 8.0f;
	}

	switch (dir) {
	case 0: // -X
		v_0 += v3(0.4375f, 0.0f, 0.4375f);
		break;
	case 1: // -Y
		v_0 += v3(0.4375f, 0.0f, 0.4375f);
		break;
	case 2: // -Z
		v_0 += v3(0.4375f, 0.0f, 0.4375f);
		break;
	case 3: // +X
		v_0 += v3(-0.4375f, 0.0f, 0.4375f);
		break;
	case 4: // +Y
		v_0 += v3(0.4375f, -0.375f, 0.4375f);
		break;
	case 5: // +Z
		v_0 += v3(0.4375f, 0.0f, -0.4375f);
		break;
	}

	v3 woff, hoff;
	woff[u_2d] = (f32)w;
	hoff[v_2d] = (f32)h;
	v3 v_1 = v_0 + woff;
	v3 v_2 = v_0 + hoff;
	v3 v_3 = v_2 + woff;

	v2 wh = v2(w, h), hw = v2(h, w);

	const f32 units = (f32)chunk::units_per_voxel;
	v_0 *= units; v_1 *= units; v_2 *= units; v_3 *= units;
	wh *= units; hw *= units;

	i32 tex = info->textures[dir];

	switch (dir) {
	case 0: // -X
		m->quad(v_0.to_i(), v_2.to_i(), v_1.to_i(), v_3.to_i(), hw.to_i(), tex, ql, bv4(ao.x,ao.z,ao.y,ao.w), bv4(l.x,l.z,l.y,l.w));
		break;
	case 1: // -Y
		m->quad(v_2.to_i(), v_3.to_i(), v_0.to_i(), v_1.to_i(), wh.to_i(), tex, ql, bv4(ao.z,ao.w,ao.x,ao.y), bv4(l.z,l.w,l.x,l.y));
		break;
	case 2: // -Z
		m->quad(v_1.to_i(), v_0.to_i(), v_3.to_i(), v_2.to_i(), wh.to_i(), tex, ql, bv4(ao.y,ao.x,ao.w,ao.z), bv4(l.y,l.x,l.w,l.z));
		break;
	case 3: // +X
		m->quad(v_2.to_i(), v_0.to_i(), v_3.to_i(), v_1.to_i(), hw.to_i(), tex, ql, bv4(ao.z,ao.x,ao.w,ao.y), bv4(l.z,l.x,l.w,l.y));
		break;
	case 4: // +Y
		m->quad(v_0.to_i(), v_1.to_i(), v_2.to_i(), v_3.to_i(), wh.to_i(), tex, ql, bv4(ao.x,ao.y,ao.z,ao.w), bv4(l.x,l.y,l.z,l.w));
		break;
	case 5: // +Z
		m->quad(v_0.to_i(), v_1.to_i(), v_2.to_i(), v_3.to_i(), wh.to_i(), tex, ql, bv4(ao.x,ao.y,ao.z,ao.w), bv4(l.x,l.y,l.z,l.w));
		break;
	}
}

void world::init_blocks(asset_store* store) {

	block_info = vector<block_meta>::make((u32)block_id::total_blocks, alloc);
	block_textures = exile->eng->ogl.begin_tex_array(iv3(32, 32, exile->eng->ogl.info.max_texture_layers), texture_wrap::repeat, true, true, 1);

	texture_id tex = block_textures;
	i32 tex_idx = exile->eng->ogl.get_layers(tex);
	

	tex_idx = exile->eng->ogl.get_layers(tex);
	exile->eng->ogl.push_tex_array(tex, store, "bedrock"_);

	block_meta* bedrock = get_info(block_id::bedrock);
	*bedrock = {
		block_id::bedrock,
		{true, true, true, true, true, true}, true,
		{tex_idx, tex_idx, tex_idx, tex_idx, tex_idx, tex_idx},
		{true, true, true, true, true, true},
		0, true, true, false, {null}
	};


	tex_idx = exile->eng->ogl.get_layers(tex);
	exile->eng->ogl.push_tex_array(tex, store, "stone"_);

	block_meta* stone = get_info(block_id::stone);
	*stone = {
		block_id::stone,
		{true, true, true, true, true, true}, true,
		{tex_idx, tex_idx, tex_idx, tex_idx, tex_idx, tex_idx},
		{true, true, true, true, true, true},
		0, true, true, false, {null}
	};


	tex_idx = exile->eng->ogl.get_layers(tex);
	exile->eng->ogl.push_tex_array(tex, store, "path_side"_);
	exile->eng->ogl.push_tex_array(tex, store, "dirt"_);
	exile->eng->ogl.push_tex_array(tex, store, "path_top"_);
	
	block_meta* path = get_info(block_id::path);
	*path = {
		block_id::path,
		{true, true, true, true, true, true}, true,
		{tex_idx, tex_idx + 1, tex_idx, tex_idx, tex_idx + 2, tex_idx},
		{true, true, true, true, true, true},
		0, true, true, false, {null}
	};	


	tex_idx = exile->eng->ogl.get_layers(tex);
	exile->eng->ogl.push_tex_array(tex, store, "slab_side"_);
	exile->eng->ogl.push_tex_array(tex, store, "slab_top"_);

	block_meta* stone_slab = get_info(block_id::stone_slab);
	*stone_slab = {
		block_id::stone_slab,
		{false, true, false, false, false, false}, false,
		{tex_idx, tex_idx + 1, tex_idx, tex_idx, tex_idx + 1, tex_idx},
		{false, true, true, true, true, false},
		0, true, false, true, FPTR(slab_model)
	};	


	tex_idx = exile->eng->ogl.get_layers(tex);
	exile->eng->ogl.push_tex_array(tex, store, "torch_side"_);
	exile->eng->ogl.push_tex_array(tex, store, "torch_bot"_);
	exile->eng->ogl.push_tex_array(tex, store, "torch_top"_);

	block_meta* torch = get_info(block_id::torch);
	*torch = {
		block_id::torch,
		{false, false, false, false, false, false}, false,
		{tex_idx, tex_idx + 1, tex_idx, tex_idx, tex_idx + 2, tex_idx},
		{false, false, false, false, false, false},
		16, true, false, true, FPTR(torch_model)
	};	
}


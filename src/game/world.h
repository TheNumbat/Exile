
#pragma once

struct chunk;

// -x is east, +x is west

enum class block_id : u16 {
	none,
	bedrock,
	stone,
	path,
	stone_slab,
	torch,

	total_blocks
};

struct block_meta {
	block_id type;

	bool opaque[6]; // -x -y -z +x +y +z
	i32 textures[6];
	bool merge[6];

	u8 emit_light;
	bool renders;
	bool does_ao;
	bool custom_model;
	
	func_ptr<void, mesh_chunk*, block_meta*, i32, iv3, iv2, u8, bv4, bv4> model;
};

struct chunk_pos {
	i32 x = 0, y = 0, z = 0;

	chunk_pos(i32 _x = 0, i32 _y = 0, i32 _z = 0);
	static chunk_pos from_abs(v3 pos);

	v3 center_xz();
	chunk_pos operator+(chunk_pos other);
	chunk_pos operator-(chunk_pos other);
};
bool operator==(chunk_pos l, chunk_pos r);
inline u32 hash(chunk_pos key);

struct block_light {
	u8 t = 0; // 0..255 for large world light propagation. gets clamped to 0..15 in renderer
	u8 s0 = 0; // 0..15, 4 bits wasted!!
	u8 s1 = 0; // 0..15, 0..15  [FOR LATER SUNLIGHT PROP]

	u8 first_u8();
};
static_assert(sizeof(block_light) == 3, "sizeof(block_light) != 3");

struct light_gather {
	u16 t = 0;
	u8 s0 = 0, s1 = 0, s2 = 0;
	void operator+=(block_light l) {t += l.t; s0 += l.s0; s1 += (l.s1 & 0xf); s1 += (l.s1 >> 4);}
};
bool operator==(light_gather l, light_gather r) {return l.t==r.t && l.s0==r.s0 && l.s1==r.s1 && l.s2==r.s2;}

struct mesh_face {
	
	static bool can_merge(mesh_face f1, mesh_face f2, i32 dir);

	block_meta* info = null;
	light_gather l[4] = {};
};

enum class chunk_stage : u8 {
	none,
	generating,
	lighting,
	lit,
	meshing,
	meshed
};

enum class light_update : u8 {
	none,
	add,
	remove,
	block,
	gen_sun,
	add_sun,
	remove_sun,
	trigger
};

struct light_work {
	light_update type;
	
	iv3 pos;
	union {
		u8 intensity;
		block_id id;
	};
};

struct chunk;
struct world;

struct block_node {
	iv3 pos;
	chunk* owner = null;

	block_id get_type();
	block_light get_l();
	void set_l(u8 intensity);
	void set_s(u8 intensity);
	bool propogate_light_through_vert(world* w, i32 dir);
};

struct light_rem_node {
	u8 val = 0;
	iv3 pos;
	chunk* owner = null;
};

static iv3 g_directions[] = {{-1, 0, 0}, {0, -1, 0}, {0, 0, -1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
struct chunk {

	// TODO(max): 16 units / voxel + cubic chunks
	static const i32 wid = 31, hei = 511;
	static const i32 units_per_voxel = 8;

	chunk_pos pos;

	// NOTE(max): x z y
	block_id blocks[wid][wid][hei] = {};
	block_light light[wid][wid][hei] = {};

	atomic_enum<chunk_stage> state;
	locking_queue<light_work> lighting_updates;	
	
	platform_mutex swap_mut;
	mesh_chunk mesh;
	u32 mesh_faces = 0;

	world* w = null;
	chunk* neighbors[8] = {}; // x+ x- z+ z- x+z+ x+z- x-z+ x-z-

	allocator* alloc = null;


	void init(world* w, chunk_pos pos, allocator* a);
	static chunk* make_new(world* w, chunk_pos pos, allocator* a);

	void do_gen();
	void do_light();
	void do_mesh();
	void destroy();
	
	void place_light(iv3 pos, u8 intensity);
	void rem_light(iv3 pos);
	void set_block(iv3 pos, block_id id);

	static i32 y_at(i32 x, i32 z);
	
	u8 ao_at_vert(iv3 vert);
	u8 l_at_vert(iv3 vert);
	light_gather gather_l(iv3 vert);
	
	block_light l_at(iv3 block);
	block_id block_at(iv3 block);
	block_node canonical_block(iv3 block);
	
	void light_add(light_work work);
	void light_remove(light_work work);
	void light_add_sun(light_work work);
	void light_rem_sun(light_work work);

	mesh_face build_face(block_id t, iv3 p, i32 dir);
};

struct player {

	render_camera camera;

	f32 speed = 5.0f;
	v3  velocity;
	u64 last = 0;

	bool enable = true;
	bool noclip = true;

	void reset();
};

enum class light_debug : i32 {
	none = 0,
	torch,
	sun
};

struct world_settings {

	f32 gravity = 0.0f;
	i32 view_distance = 4;
	i32 max_light_propogation = 1;
	bool respect_cam = true;
	
	f32 gamma = 2.2f;
	bool wireframe = false;
	bool cull_backface = true;
	bool sample_shading = true;
	bool draw_chunk_corners = false;

	bool dist_fog = false;
	
	bool block_light = true;
	bool smooth_light = true;
	light_debug light_debug_mode = light_debug::none;

	float ambient_factor = 0.1f;

	v4 ao_curve = v4(0.25f, 0.5f, 0.75f, 1.0f);
};

struct world_time {

	bool enable = true;

	// scale is 60 * time_scale
	f64 time_scale = 1.0f;

	u64 last_update = 0;
	u64 absolute = 0;

	f64 absolute_ms = 0.0f, minute_ms = 0.0f;

	u32 minute = 0, hour = 0, day = 0;

	void update(u64 now);
	f32 day_01();
};

struct world_environment {

	texture_id sky_texture = -1, env_texture = -1;
	
	mesh_3d_tex sky;
	mesh_pointcloud stars;

	void init(asset_store* store, allocator* a);
	void destroy();

	void render(player* p, world_time* t);
	void build_sun_moon(f32 day_01);
};

struct world {
	
	// TODO(max): how do we really want to do storage here?
	// 			  we need to support loaded/unloaded chunks
	// 			  for simulation and paging to disk

	// NOTE(max): map to pointers to chunk so the map can transform while chunks are being operated on
	// TODO(max): use a free-list allocator to allocate the chunks
	map<chunk_pos, chunk*> chunks;

	vector<block_meta> block_info;

	world_settings settings;
	player p;

	threadpool thread_pool;
	allocator* alloc = null;
	
	texture_id block_textures = -1;

	world_time time;
	world_environment env;

	void init(asset_store* store, allocator* a);
	void init_blocks(asset_store* store);
	block_meta* get_info(block_id id);

	void destroy();
	void destroy_chunks();
	void regenerate();

	void update(u64 now);
	void update_player(u64 now);

	void render();
	void render_chunks();
	void render_player();
	void render_sky();
	
	void local_populate();
	void local_generate();
	void local_light();
	void local_mesh();

	void place_light(iv3 pos, u8 intensity);
	void rem_light(iv3 pos);
	void set_block(iv3 pos, block_id id);

	void player_break_block();
	void player_place_block();

	block_node world_to_canonical(iv3 pos);
	block_id block_at(iv3 pos);

	v3 raymarch(v3 origin, v3 dir, f32 max);
	v3 raymarch(v3 origin, v3 max);
};

CALLBACK void world_debug_ui(world* w);
CALLBACK void unlock_chunk(chunk* v);
float check_pirority(super_job* j, void* param);

CALLBACK void cancel_gen(chunk* param);
CALLBACK void cancel_light(chunk* param);
CALLBACK void cancel_mesh(chunk* param);

CALLBACK void slab_model(mesh_chunk* m, block_meta* i, i32 dir, iv3 v, iv2 wh, u8 ql, bv4 ao, bv4 l);
CALLBACK void torch_model(mesh_chunk* m, block_meta* i, i32 dir, iv3 v, iv2 wh, u8 ql, bv4 ao, bv4 l);

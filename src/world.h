
enum class block_type : u8 {
	air, 
	bedrock,
	stone,
};
#define NUM_BLOCKS (TYPEINFO(block_type)->_enum.member_count)

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

struct mesh_face {
	
	static bool can_merge(mesh_face f1, mesh_face f2, i32 dir);

	block_type type = block_type::air;
	bv4 ao;
};

enum class work : u8 {
	none,
	in_flight,
	done
};

struct chunk {

	static const i32 xsz = 31, ysz = 511, zsz = 31;

	static const i32 units_per_voxel = 8;

	chunk_pos pos;

	// NOTE(max): x z y
	block_type blocks[xsz][zsz][ysz] = {};
	
	platform_mutex swap_mut;
	atomic_enum<work> job_state;
	
	mesh_chunk mesh, backface_mesh;
	u32 mesh_faces = 0, backface_mesh_faces = 0;

	allocator* alloc = null;

	void init(chunk_pos pos, allocator* a);
	static chunk* make_new(chunk_pos pos, allocator* a);

	void gen();
	void destroy();

	static i32 y_at(i32 x, i32 z);
	static bool valid_q(v3 v_0, v3 v_1, v3 v_2, v3 v_3);
	u8 ao_at(v3 vert);
	block_type block_at(i32 x, i32 y, i32 z);
	mesh_face build_face(block_type t, iv3 p, i32 dir);

	void build_data();
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

struct world_settings {

	f32 gravity = 0.0f;
	i32 view_distance = 4;
	bool respect_cam = true;
	
	bool wireframe = false;
	bool cull_backface = false;
	bool sample_shading = true;
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

struct world {
	
	// TODO(max): how do we really want to do storage here?
	// 			  we need to support loaded/unloaded chunks
	// 			  for simulation and paging to disk

	// NOTE(max): map to pointers to chunk so the map can transform while chunks are being operated on
	// TODO(max): use a free-list allocator to allocate the chunks
	map<chunk_pos, chunk*> chunks;

	world_settings settings;
	player p;

	threadpool thread_pool;
	allocator* alloc = null;

	texture_id block_textures = -1;
	texture_id sky_texture = -1, night_sky_texture = -1;
	mesh_3d_tex sky;

	world_time time;

	void init(asset_store* store, allocator* a);
	void destroy();
	void destroy_chunks();
	void regenerate();

	void update(u64 now);
	void update_player(u64 now);

	void render();
	void render_chunks();
	void render_player();
	void render_sky();
	void populate_local_area();

	v3 raymarch(v3 origin, v3 dir, f32 max);
	v3 raymarch(v3 origin, v3 max);
};

CALLBACK void world_debug_ui(world* w);
CALLBACK void unlock_chunk(chunk* v);
CALLBACK void cancel_build(chunk* param);
float check_pirority(super_job* j, void* param);

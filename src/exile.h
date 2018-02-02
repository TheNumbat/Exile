
enum class block_type : u8 {
	air,
	stone,
};

struct chunk_pos {
	i32 x = 0, y = 0, z = 0;

	chunk_pos(i32 _x = 0, i32 _y = 0, i32 _z = 0);
	static chunk_pos from_abs(v3 pos);
	
	chunk_pos operator+(chunk_pos other);
	chunk_pos operator-(chunk_pos other);
};
bool operator==(chunk_pos l, chunk_pos r);
u32 hash(chunk_pos key);

struct chunk {

	static const i32 xsz = 16, ysz = 256, zsz = 16;

	// NOTE(max): x z y
	block_type blocks[xsz][zsz][ysz] = {};
	mesh_chunk mesh;

	static chunk make(allocator* a);
	void destroy();

	void build_data();
};

struct player {

	render_camera camera;

	f32 speed = 5.0f;
	v3  velocity;
	platform_perfcount last = 0;

	void init();
	void update(platform_perfcount now);
};

struct exile {

	// TODO(max): how do we really want to do storage here?
	// 			  we need to support loaded/unloaded chunks
	// 			  for simulation and paging to disk
	map<chunk_pos, chunk> chunks;
	i32 view_distance = 4;

	texture_id cube_tex;
	evt_handler_id default_evt = 0, camera_evt = 0;

	player p;

	engine* state = null;
	platform_allocator alloc;

	void init(engine* state);
	void destroy();

	void update();
	void render();
	void populate_local_area();
};

CALLBACK bool default_evt_handle(void* param, platform_event evt);
CALLBACK bool camera_evt_handle(void* param, platform_event evt);

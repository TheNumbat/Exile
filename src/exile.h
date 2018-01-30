
enum class block_type : u8 {
	air,
	stone,
};

struct chunk {

	block_type blocks[16][16][256] = {};
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

	chunk the_chunk;

	texture_id cube_tex;
	evt_handler_id default_evt = 0, camera_evt = 0;

	player p;

	engine* state = null;
	platform_allocator alloc;

	void init(engine* state);
	void destroy();

	void update();
	void render();
};

CALLBACK bool default_evt_handle(void* param, platform_event evt);
CALLBACK bool camera_evt_handle(void* param, platform_event evt);

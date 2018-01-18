
enum class block_type : u8 {
	air,
	stone,
};

struct chunk {

	block_type blocks[16][16][256] = {};
	mesh_3d_tex_instance_data cube_data;

	static chunk make(mesh_3d_tex* cube, allocator* a);
	void destroy();

	void build_data();
};

struct player {

	render_camera camera;

	f32 max_speed = 5.0f;
	v3  velocity;
	platform_perfcount last = 0;

	void forward();
	void left();
	void right();
	void back();
	void update(platform_perfcount now);
};

struct exile {

	chunk the_chunk;
	mesh_3d_tex cube;

	texture_id cube_tex;
	evt_handler_id handler = 0;

	player p;

	engine* state = null;
	platform_allocator alloc;

	void init(engine* state);
	void destroy();

	void update();
	void render();
};

CALLBACK bool default_evt_handle(void* param, platform_event evt);

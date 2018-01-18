
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

struct exile {

	chunk the_chunk;
	mesh_3d_tex cube;

	texture_id cube_tex;

	render_camera camera;

	engine* state = null;
	platform_allocator alloc;

	void init(engine* state);
	void destroy();

	void update();
	void render();
};

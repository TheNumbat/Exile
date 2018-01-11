
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

struct world_manager {

	chunk the_chunk;
	mesh_3d_tex cube;

	texture_id cube_tex;

	game_state* state = null;
	allocator*  alloc = null;

	void init(game_state* state, allocator* a);
	void destroy();

	void render();
};

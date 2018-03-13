
enum class block_type : u8 {
	air, 
	stone,
	numbat,
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

enum class work : u8 {
	none,
	in_flight,
	done
};

struct chunk {

	static const i32 xsz = 32, ysz = 255, zsz = 32;

	chunk_pos pos;

	// NOTE(max): x z y
	block_type blocks[xsz][zsz][ysz] = {};
	
	platform_mutex swap_mut;
	atomic_enum<work> job_state;
	mesh_chunk mesh;

	u32 mesh_triangles = 0;

	allocator* alloc = null;

	static chunk make(chunk_pos pos, allocator* a);
	static chunk* make_new(chunk_pos pos, allocator* a);

	void gen();
	void destroy();

	static i32 y_at(i32 x, i32 z);
	u8 ao_at(v3 vert);
	block_type block_at(i32 x, i32 y, i32 z);

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

struct world {
	
	// TODO(max): how do we really want to do storage here?
	// 			  we need to support loaded/unloaded chunks
	// 			  for simulation and paging to disk

	// NOTE(max): map to pointers to chunk so the map can transform while chunks are being operated on
	// TODO(max): use a free-list allocator to allocate the chunks
	map<chunk_pos, chunk*> chunks;
	i32 view_distance = 8;

	texture_id block_textures;

	player p;

	threadpool thread_pool;
	allocator* alloc = null;

	void init(asset_store* store, allocator* a);
	void destroy();

	void update(platform_perfcount now);
	void render();
	void populate_local_area();
};

CALLBACK void unlock_chunk(void* v);
CALLBACK void cancel_build(void* param);
float check_pirority(super_job* j, void* param);

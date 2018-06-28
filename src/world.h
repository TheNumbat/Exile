
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

struct chunk_vertex {
	u8 x = 0, z = 0;
	u16 y_ao = 0;	

	u16 ao_t = 0;
	u8 u = 0, v = 0;

	static chunk_vertex from_vec(v3 v, v3 uv, bv4 ao);
};
static_assert(sizeof(chunk_vertex) == 8, "chunk_vertex size != 8");

namespace render_command_type {
	u16 mesh_chunk = 16;
};

struct mesh_chunk {

	vector<chunk_vertex> 	vertices;
	vector<uv3> 			elements;

	GLuint vao = 0;
	GLuint vbos[2] = {};
	bool dirty = false;

	static mesh_chunk make(u32 verts = 8192, allocator* alloc = null);
	static mesh_chunk make_cpu(u32 verts = 8192, allocator* alloc = null);
	static mesh_chunk make_gpu();
	void destroy();
	void free_cpu();
	void clear();
	void swap_mesh(mesh_chunk other);

	void quad(v3 p1, v3 p2, v3 p3, v3 p4, v3 uv_ext, bv4 ao);
	void cube(v3 pos, f32 len);
};

struct chunk {

	static const i32 xsz = 31, ysz = 255, zsz = 31;

	static const i32 units_per_voxel = 256 / (xsz + 1);

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
	mesh_face build_face(block_type t, iv3 p, i32 dir);

	void build_data();
};

struct player {

	render_camera camera;

	f32 speed = 5.0f;
	v3  velocity;
	u64 last = 0;

	bool enable = true;

	void reset();
};

struct world {
	
	// TODO(max): how do we really want to do storage here?
	// 			  we need to support loaded/unloaded chunks
	// 			  for simulation and paging to disk

	// NOTE(max): map to pointers to chunk so the map can transform while chunks are being operated on
	// TODO(max): use a free-list allocator to allocate the chunks
	map<chunk_pos, chunk*> chunks;
	
	i32 view_distance = 1;
	bool wireframe = false;
	bool respect_cam = false;
	f32 gravity = 10.0f;

	texture_id block_textures;

	player p;

	threadpool thread_pool;
	allocator* alloc = null;

	void init(asset_store* store, allocator* a);
	void destroy();
	void destroy_chunks();

	void update(u64 now);
	void update_player(u64 now);

	void render();
	void render_chunks();
	void render_player();
	void populate_local_area();

	v3 raymarch(v3 origin, v3 dir, f32 max);
	v3 raymarch(v3 origin, v3 max);
};

CALLBACK void unlock_chunk(void* v);
CALLBACK void cancel_build(void* param);
float check_pirority(super_job* j, void* param);

CALLBACK void run_mesh_chunk(render_command* cmd);
CALLBACK void buffers_mesh_chunk(render_command* cmd);
CALLBACK void uniforms_mesh_chunk(shader_program* prog, render_command* cmd, render_command_list* rcl);
CALLBACK bool compat_mesh_chunk(ogl_info* info);

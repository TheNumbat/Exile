
typedef i32 texture_id;

enum class render_command_type : u8 {
	none,
	mesh_2d_col,
	mesh_2d_tex,
	mesh_2d_tex_col,
	mesh_3d_tex,
	mesh_3d_tex_instanced,
	mesh_chunk,
};

u32 hash(render_command_type key);

enum face : u8 {
	none,
	up,
	down,
	east,
	west,
	north,
	south
};

struct chunk_vertex {
	u8 x = 0, z = 0;
	u16 y_norm = 0;	

	u16 t_ao = 0;
	u8 u = 0, v = 0;

	static chunk_vertex from_vec(v3 v, v2 uv);
};
static_assert(sizeof(chunk_vertex) == 8, "chunk_vertex size != 8");

struct mesh_chunk {

	vector<chunk_vertex> 	vertices;
	vector<uv3> 			elements;

	GLuint vao = 0;
	GLuint vbos[2] = {};
	bool dirty = false;

	static mesh_chunk make(u32 verts = 8192, allocator* alloc = null);
	void destroy();
	void free_cpu();
	void clear();

	void push_cube(v3 pos, f32 len);
};

struct mesh_2d_col {
	vector<v2>		vertices;	// x y 
	vector<colorf>	colors;		// r g b a
	vector<uv3> 	elements;

	GLuint vao = 0;
	GLuint vbos[3] = {};
	bool dirty = false;

	static mesh_2d_col make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_tri(v2 p1, v2 p2, v2 p3, color c);
	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
};

struct mesh_2d_tex {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<uv3> 	elements;

	GLuint vao = 0;
	GLuint vbos[3] = {};
	bool dirty = false;

	static mesh_2d_tex make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();
};

struct mesh_2d_tex_col {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<colorf> 	colors;
	vector<uv3> 	elements;

	GLuint vao = 0;
	GLuint vbos[4] = {};
	bool dirty = false;

	static mesh_2d_tex_col make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
	f32 push_text_line(asset* font, string text_utf8, v2 pos, f32 point = 0.0f, color c = V4b(255, 255, 255, 255)); 
};

struct mesh_3d_tex {
	vector<v3>  vertices;	// x y z
	vector<v2>  texCoords; 	// u v
	vector<uv3> elements;

	GLuint vao = 0;
	GLuint vbos[3] = {};
	bool dirty = false;

	static mesh_3d_tex make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_cube(v3 pos, f32 len);
};

struct mesh_3d_tex_instance_data {
	
	mesh_3d_tex* parent = null;
	vector<v3> data;

	u32 instances = 0;
	GLuint vbo = 0;

	bool dirty = false;

	static mesh_3d_tex_instance_data make(mesh_3d_tex* parent, u32 instances = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();
};

struct render_command {
	render_command_type cmd 	= render_command_type::none;
	
	texture_id 			texture = -1;
	
	m4 model;
	u32 sort_key = 0;

	// triangle index, gets * 3 to compute element index
	u32 offset = 0, num_tris = 0, start_tri = 0;

	// zero for entire window
	r2 viewport;
	r2 scissor;

	void* mesh = null;

	render_command() {}

	static render_command make(render_command_type type, void* data = null, u32 key = 0);
};

bool operator<=(render_command& first, render_command& second);

struct render_command_list {
	vector<render_command> commands;
	allocator* alloc = null;
	m4 view;
	m4 proj;

	static render_command_list make(allocator* alloc = null, u32 cmds = 8);
	void destroy();
	void add_command(render_command rc);
	void sort();
};

struct render_camera {

	v3 	  pos, front, up, right;
	float pitch = 0.0f, yaw = 0.0f, fov = 60.0f;

	void update();
	void reset();
	void move(i32 dx, i32 d, f32 sens);
	m4 view();
};

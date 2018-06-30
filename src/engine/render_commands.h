
typedef i32 texture_id;

#ifndef __clang__
namespace render_command_type {
	enum default_command_types : u16 {
		none = 0,
		push_settings,
		pop_settings,
		setting,
		mesh_2d_col,
		mesh_2d_tex,
		mesh_2d_tex_col,
		mesh_3d_tex,
		mesh_3d_tex_instanced,
		mesh_lines
	};
};
#endif

enum class render_setting : u8 {
	none,
	wireframe,
	depth_test,
	aa_lines,
	blend,
	scissor,
	cull,
	msaa
};

struct mesh_lines {
	vector<v3> 	   vertices;
	vector<colorf> colors;
	
	GLuint vao = 0;
	GLuint vbos[2] = {};
	bool dirty = false;

	static mesh_lines make(allocator* alloc = null);
	void destroy();

	void push(v3 p1, v3 p2, colorf c1 = colorf(0,0,0,1), colorf c2 = colorf(0,0,0,1));
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
	f32 push_text_line(asset* font, string text_utf8, v2 pos, f32 point = 0.0f, color c = color(255, 255, 255, 255)); 
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
	u16 cmd = render_command_type::none;

	struct {	
		texture_id texture = -1;
		
		m4 model;
		u32 sort_key = 0;

		// triangle index, gets * 3 to compute element index
		u32 offset = 0, num_tris = 0, start_tri = 0;

		// zero for entire window
		r2 viewport;
		r2 scissor;

		void* mesh = null;

		func_ptr<void, void*> callback;
		void* param = null;
	};

	struct {
		render_setting setting;
		bool enable = false;
	};

	render_command() {}

	static render_command make(u16 type);
	static render_command make(u16 type, void* data, u32 key = 0);
	static render_command make(u16 type, render_setting setting, bool enable);
};

bool operator<=(render_command& first, render_command& second);

struct render_command_list {
	vector<render_command> commands;
	allocator* alloc = null;
	m4 view;
	m4 proj;

	static render_command_list make(allocator* alloc = null, u32 cmds = 8);
	void destroy();
	void clear();
	void add_command(render_command rc);
	void sort();

	void push_settings();
	void pop_settings();
	void set_setting(render_setting setting, bool enable);
};

enum class camera_mode : u8 {
	first,
	third
};

struct render_camera {

	v3 	  pos, front, up, right;
	float pitch = 0.0f, yaw = 0.0f, fov = 60.0f;

	v3 offset3rd = v3(0, 1, 0);
	f32 reach = 5.0f;

	camera_mode mode = camera_mode::third;

	void update();
	void reset();
	void move(i32 dx, i32 d, f32 sens);
	
	m4 view();
	m4 view_no_translate();
};

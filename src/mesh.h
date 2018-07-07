
enum class mesh_cmd_types : u16 {
	mesh_2d_col = 16,
	mesh_2d_tex,
	mesh_2d_tex_col,
	mesh_3d_tex,
	mesh_3d_tex_instanced,
	mesh_lines
};

struct mesh_lines {
	vector<v3> 	   vertices;
	vector<colorf> colors;
	
	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();

	void push(v3 p1, v3 p2, colorf c1 = colorf(0,0,0,1), colorf c2 = colorf(0,0,0,1));
};

struct mesh_2d_col {
	vector<v2>		vertices;	// x y 
	vector<colorf>	colors;		// r g b a
	vector<uv3> 	elements;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
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

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();
};

struct mesh_2d_tex_col {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<colorf> 	colors;
	vector<uv3> 	elements;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
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

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_cube(v3 pos, f32 len);
};

struct mesh_3d_tex_instance_data {
	
	mesh_3d_tex* parent = null;
	vector<v3> data;

	u32 instances = 0;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(mesh_3d_tex* parent, u32 instances = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();
};

void setup_mesh_commands();

CALLBACK void uniforms_mesh_2d_col(shader_program* prog, render_command* cmd, render_command_list* rcl);
CALLBACK void uniforms_mesh_2d_tex(shader_program* prog, render_command* cmd, render_command_list* rcl);
CALLBACK void uniforms_mesh_2d_tex_col(shader_program* prog, render_command* cmd, render_command_list* rcl);
CALLBACK void uniforms_mesh_3d_tex(shader_program* prog, render_command* cmd, render_command_list* rcl);
CALLBACK void uniforms_mesh_3d_tex_instanced(shader_program* prog, render_command* cmd, render_command_list* rcl);
CALLBACK void uniforms_mesh_lines(shader_program* prog, render_command* cmd, render_command_list* rcl);

CALLBACK void setup_mesh_2d_col(gpu_object* obj);
CALLBACK void setup_mesh_2d_tex(gpu_object* obj);
CALLBACK void setup_mesh_2d_tex_col(gpu_object* obj);
CALLBACK void setup_mesh_3d_tex(gpu_object* obj);
CALLBACK void setup_mesh_3d_tex_instanced(gpu_object* obj);
CALLBACK void setup_mesh_lines(gpu_object* obj);

CALLBACK void update_mesh_2d_col(gpu_object* obj, void* data, bool force);
CALLBACK void update_mesh_2d_tex(gpu_object* obj, void* data, bool force);
CALLBACK void update_mesh_2d_tex_col(gpu_object* obj, void* data, bool force);
CALLBACK void update_mesh_3d_tex(gpu_object* obj, void* data, bool force);
CALLBACK void update_mesh_3d_tex_instanced(gpu_object* obj, void* data, bool force);
CALLBACK void update_mesh_lines(gpu_object* obj, void* data, bool force);

CALLBACK void run_mesh_2d_col(render_command* cmd, gpu_object* gpu);
CALLBACK void run_mesh_2d_tex(render_command* cmd, gpu_object* gpu);
CALLBACK void run_mesh_2d_tex_col(render_command* cmd, gpu_object* gpu);
CALLBACK void run_mesh_3d_tex(render_command* cmd, gpu_object* gpu);
CALLBACK void run_mesh_3d_tex_instanced(render_command* cmd, gpu_object* gpu);
CALLBACK void run_mesh_lines(render_command* cmd, gpu_object* gpu);

CALLBACK bool compat_mesh_2d_col(ogl_info* info);
CALLBACK bool compat_mesh_2d_tex(ogl_info* info);
CALLBACK bool compat_mesh_2d_tex_col(ogl_info* info);
CALLBACK bool compat_mesh_3d_tex(ogl_info* info);
CALLBACK bool compat_mesh_3d_tex_instanced(ogl_info* info);
CALLBACK bool compat_mesh_lines(ogl_info* info);

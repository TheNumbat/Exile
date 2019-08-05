
#pragma once

#include <engine/basic.h>
#include <engine/ds/vector.h>
#include <engine/render.h>
#include "world.h"
#include "gfx_mesh.h"

enum class exile_component_view : i32 {
	none = 0,
	albedo,
	pos,
	norm,
	all_light,
	torch,
	sun,
	ao,
	dynamic,
	depth,
	light_volume
};

struct render_settings {
	
	float ambient_factor = 0.005f;
	v4 ao_curve = v4(0.25f, 0.4f, 0.6f, 0.9f);

	bool wireframe = false;
	bool cull_backface = true;
	bool sample_shading = true;
	
	bool dist_fog = false;
	bool block_light = true;
	bool smooth_light = true;
	bool dynamic_light = true;
	bool ambient_occlusion = true;

	exile_component_view view =  exile_component_view::none;

	f32 torch_cutoff = 3.5f;
	f32 spot_cutoff = 0.5f;

	i32 num_samples = 4;
	f32 gamma = 2.1f;
	bool enable_gamma = true;
	bool invert_effect = false;
};

struct world_buffers {
	texture_id col_buf, norm_buf, light_buf, depth_buf;
	render_target col_buf_target, depth_buf_target;
	render_target norm_buf_target, light_buf_target;
	
	framebuffer_id chunk_target = 0;
	framebuffer_id light_target = 0;
};
struct effect_buffers {
	texture_id effect0, effect1;
	render_target effect0_target, effect1_target; 
	framebuffer_id effect0_fb = 0;
	framebuffer_id effect1_fb = 0;
};

struct world_proj_info {
	m4 ivp, vp;
	f32 near;
};

struct world_target_info {

	// NOTE(max): only the buffers + textures use GPU memory,
	// the rest are reference objects

	world_buffers  w;
	effect_buffers e;

	bool msaa = false;
	bool current0 = true;

	void init(iv2 dim, i32 samples);
	void destroy();
	
	void flip_fb();
	framebuffer_id world_fb();
	framebuffer_id get_fb();
	texture_id get_output();
};

struct effect_pass {

	draw_cmd_id cmd_id = -1;

	void init(_FPTR* uniforms, string frag);
	void init(_FPTR* uniforms, string vert, string frag);
	void destroy();

	render_command make_cmd();
	void effect(render_command_list* list);
};
CALLBACK void run_effect(render_command* cmd, gpu_object* gpu);		
CALLBACK void setup_mesh_quad(gpu_object* obj);
CALLBACK void update_mesh_quad(gpu_object* obj, void* data, bool force);

// NOTE(max): sort of a client implementation using the engine OGL renderer 
struct exile_renderer {

	allocator* alloc = null;

	void init(allocator* a);
	void destroy();

	draw_cmd_id cmd_2D_col           = 0, cmd_2D_tex      = 0,
                cmd_2D_tex_col       = 0, cmd_3D_tex      = 0,
                cmd_3D_tex_instanced = 0, cmd_lines       = 0;

	draw_cmd_id cmd_pointcloud = 0, cmd_cubemap = 0,
                cmd_chunk      = 0, cmd_skydome = 0,
                cmd_skyfar     = 0, cmd_light   = 0,
                cmd_light_ms   = 0, cmd_dlight  = 0,
                cmd_dlight_ms  = 0;

	render_settings settings;

	// NOTE(max): these should be static, but hot reloading
	mesh_cubemap the_cubemap;
	mesh_quad 	 the_quad;
	
	sun_light sun;
	mesh_light_list lights;

	effect_pass invert, gamma;
	effect_pass composite, composite_resolve, resolve;
	effect_pass comp_resolve_light, comp_light;

	world_target_info world_target;

	// TODO(max): combine to one list and sort
	render_command_list world_tasks, hud_tasks;
	world_proj_info proj_info;

	void hud_2D(gpu_object_id gpu_id);

	void world_clear();
	void world_lines(gpu_object_id id, m4 view, m4 proj);
	void world_stars(gpu_object_id gpu_id, world_time* time, m4 view, m4 proj);
	void world_skydome(gpu_object_id gpu_id, world_time* time, texture_id sky, m4 view, m4 proj);
	
	void world_begin_chunks(world* w, bool offset);
	// NOTE(max): this assumes the chunk mesh object is long-lived
	void world_chunk(chunk* c, block_textures block_tex, texture_id sky, m4 model, m4 view, m4 proj);
	void world_finish_chunks();

	// void push_point_light(v3 p, v3 d, v3 s, v3 a);
	void resolve_lighting();

	void generate_commands();
	void generate_targets();
	void recreate_targets();
	void check_recreate();

	void calculate_light_quad(m4 m, m4 vp, v3 pos, v3 col);

	iv2 prev_dim;
	i32 prev_samples = 0;

	void end_frame();
};

#define decl_mesh(name) \
	CALLBACK void uniforms_mesh_##name(shader_program* prog, render_command* cmd); 	\
	CALLBACK void setup_mesh_##name(gpu_object* obj); 								\
	CALLBACK void update_mesh_##name(gpu_object* obj, void* data, bool force);		\
	CALLBACK void run_mesh_##name(render_command* cmd, gpu_object* gpu);

decl_mesh(skyfar);
decl_mesh(skydome);
decl_mesh(cubemap);
decl_mesh(chunk);
decl_mesh(2D_col);
decl_mesh(2D_tex);
decl_mesh(2D_tex_col);
decl_mesh(3D_tex);
decl_mesh(3D_tex_instanced);
decl_mesh(lines);
decl_mesh(pointcloud);

CALLBACK void uniforms_gamma(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_invert(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_resolve(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_composite(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_comp_light(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_composite_resolve(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_comp_resolve_light(shader_program* prog, render_command* cmd);

CALLBACK void run_dir(render_command* cmd, gpu_object* gpu);
CALLBACK void run_light(render_command* cmd, gpu_object* gpu);
CALLBACK void uniforms_dir(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_light(shader_program* prog, render_command* cmd);

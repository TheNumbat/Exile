
// TODO(max): remove all OpenGL calls; abstract into renderer system

void exile_renderer::init(allocator* a) {
	
	exile->eng->dbg.store.add_var("render/settings"_, &settings);
	
	generate_commands();
	generate_targets();

	the_cubemap.init();
	the_quad.init();

	alloc = a;
	hud_tasks = render_command_list::make(alloc, 32);
	frame_tasks = render_command_list::make(alloc, 1024);

	hud_tasks.set_setting(render_setting::depth_test, false);
}

void exile_renderer::world_skydome(gpu_object_id gpu_id, world_time* time, texture_id sky, m4 view, m4 proj) {

	render_command cmd = render_command::make_cst(cmd_skydome, gpu_id);

	cmd.info.fb_id = world_target.world_fb();
	cmd.info.user_data0 = time;
	cmd.info.textures[0] = sky;

	cmd.info.view = view;
	cmd.info.proj = proj;

	frame_tasks.add_command(cmd);
}

void exile_renderer::world_stars(gpu_object_id gpu_id, world_time* time, m4 view, m4 proj) {

	frame_tasks.push_settings();
	frame_tasks.set_setting(render_setting::point_size, true);

	render_command cmd = render_command::make_cst(cmd_pointcloud, gpu_id);

	cmd.info.fb_id = world_target.world_fb();
	cmd.info.user_data0 = time;
	cmd.info.view = view;
	cmd.info.proj = proj;

	frame_tasks.add_command(cmd);
	frame_tasks.pop_settings();
}

void exile_renderer::world_begin_chunks() {

	frame_tasks.push_settings();
	if(settings.world_set.wireframe)
		frame_tasks.set_setting(render_setting::wireframe, true);
	if(settings.world_set.cull_backface)
		frame_tasks.set_setting(render_setting::cull, true);
	if(settings.world_set.sample_shading)
		frame_tasks.set_setting(render_setting::aa_shading, true);
}

void exile_renderer::world_finish_chunks() {

	frame_tasks.pop_settings();
}

void exile_renderer::world_chunk(world* w, chunk* c, texture_id blocks, texture_id sky, m4 model, m4 view, m4 proj) {

	render_command cmd = render_command::make_cst(cmd_chunk, c->mesh.gpu);

	cmd.info.fb_id = world_target.world_fb();
	cmd.info.textures[0] = blocks;
	cmd.info.textures[1] = sky;
	cmd.info.num_tris = c->mesh_faces;
	cmd.info.user_data0 = w;
	cmd.info.user_data1 = &settings.world_set;

	cmd.info.model = model;
	cmd.info.view = view;
	cmd.info.proj = proj;

	cmd.callback = FPTR(unlock_chunk);
	cmd.callback_data = c;

	frame_tasks.add_command(cmd);
}

void exile_renderer::world_lines(gpu_object_id gpu_id, m4 view, m4 proj) {
	
	render_command cmd = render_command::make_cst(cmd_lines, gpu_id);

	cmd.info.fb_id = world_target.world_fb();

	cmd.info.view = view;
	cmd.info.proj = proj;
	
	frame_tasks.add_command(cmd);
}

void exile_renderer::hud_2D(gpu_object_id gpu_id) {

	render_command cmd = render_command::make_cst(cmd_2D_col, gpu_id);
	
	f32 w = (f32)exile->eng->window.settings.w, h = (f32)exile->eng->window.settings.h;
	cmd.info.proj = ortho(0, w, h, 0, -1, 1);

	hud_tasks.add_command(cmd);
}

void exile_renderer::world_clear() {

	render_command cmd = render_command::make(ogl_manager::cmd_clear);
	
	cmd.clear.fb_id = world_target.world_fb();
	cmd.clear.col = settings.clear_color;
	cmd.clear.components = (GLbitfield)gl_clear::color_buffer_bit | (GLbitfield)gl_clear::depth_buffer_bit;

	frame_tasks.add_command(cmd);
}


void exile_renderer::end_frame() {

	// run world effects
	frame_tasks.push_settings();
	frame_tasks.set_setting(render_setting::depth_test, false);	

	{	
		world_target.resolve(&frame_tasks);
		if(settings.invert_effect) {

			render_command cmd = invert.make_cmd();
			
			cmd.info.textures[0] = world_target.get_output();
			
			world_target.flip_fb();
			cmd.info.fb_id = world_target.get_fb();

			frame_tasks.add_command(cmd);
		}
	}

	// composite to screen
	{
		if(settings.gamma)
			frame_tasks.set_setting(render_setting::output_srgb, true);

		// render_command cmd = composite.make_cmd();
		// cmd.info.textures[0] = world_target.get_output();
		
		// frame_tasks.add_command(cmd);

		render_command cmd = render_command::make(ogl_manager::cmd_blit_fb);
		cmd.blit.src = world_target.get_fb();
		cmd.blit.mask = (GLbitfield)gl_clear::color_buffer_bit;
		cmd.blit.filter = gl_tex_filter::scaled_resolve_fastest;
		
		frame_tasks.add_command(cmd);
 	}

 	frame_tasks.pop_settings();

	exile->eng->ogl.execute_command_list(&frame_tasks);
	exile->eng->ogl.execute_command_list(&hud_tasks);

	frame_tasks.clear();
	hud_tasks.clear();
	hud_tasks.set_setting(render_setting::depth_test, false);

	check_recreate();
}

void exile_renderer::generate_targets() {

	prev_dim = iv2(exile->eng->window.settings.w, exile->eng->window.settings.h);
	prev_samples = settings.num_samples;

	world_target.init(prev_dim, prev_samples);
}

void effect_pass::init(_FPTR* uniforms, string frag) {

	cmd_id = exile->eng->ogl.add_command(FPTR(run_effect), uniforms, "shaders/effects/effect.v"_, string::makef("shaders/effects/%"_, frag));
}

void effect_pass::destroy() {

	exile->eng->ogl.rem_command(cmd_id);
	cmd_id = -1;	
}

render_command effect_pass::make_cmd() {

	return render_command::make_cst(cmd_id, exile->ren.the_quad.gpu);
}

void world_target_info::init(iv2 dim, i32 samples) {

	msaa = samples != 1;

	if(msaa) {
		col_buf = exile->eng->ogl.add_texture_target(dim, samples, gl_tex_format::rgba16f);
		col_buf_target = exile->eng->ogl.make_target(gl_draw_target::color_0, col_buf);
	
		depth_buf = render_buffer::make(gl_tex_format::depth_component, dim, samples);
		depth_buf_target = exile->eng->ogl.make_target(gl_draw_target::depth, &depth_buf);
		
		render_ms = exile->eng->ogl.add_framebuffer();
		exile->eng->ogl.add_target(render_ms, col_buf_target);
		exile->eng->ogl.add_target(render_ms, depth_buf_target);
		exile->eng->ogl.commit_framebuffer(render_ms);
	}

	effect0 = exile->eng->ogl.add_texture_target(dim, 1, gl_tex_format::rgba16f);
	effect1 = exile->eng->ogl.add_texture_target(dim, 1, gl_tex_format::rgba16f);
	effect0_target = exile->eng->ogl.make_target(gl_draw_target::color_0, effect0);
	effect1_target = exile->eng->ogl.make_target(gl_draw_target::color_0, effect1);

	effect_depth = render_buffer::make(gl_tex_format::depth_component, dim, 1);
	effect_depth_target = exile->eng->ogl.make_target(gl_draw_target::depth, &effect_depth);

	effect0_fb = exile->eng->ogl.add_framebuffer();
	exile->eng->ogl.add_target(effect0_fb, effect0_target);
	exile->eng->ogl.add_target(effect0_fb, effect_depth_target);
	exile->eng->ogl.commit_framebuffer(effect0_fb);

	effect1_fb = exile->eng->ogl.add_framebuffer();
	exile->eng->ogl.add_target(effect1_fb, effect1_target);
	exile->eng->ogl.add_target(effect1_fb, effect_depth_target);
	exile->eng->ogl.commit_framebuffer(effect1_fb);
}

void world_target_info::resolve(render_command_list* list) {
	
	if(msaa) {
		render_command cmd = exile->ren.resolve.make_cmd(); // TODO(max): clean
		cmd.info.fb_id = get_fb();
		cmd.info.textures[0] = col_buf;
		cmd.info.user_data0 = &exile->ren;
		list->add_command(cmd);

		// render_command cmd = render_command::make(ogl_manager::cmd_blit_fb);
		// cmd.blit.src = render_ms;
		// cmd.blit.dst = get_fb();
		// cmd.blit.mask = (GLbitfield)gl_clear::color_buffer_bit;
		// cmd.blit.filter = gl_tex_filter::scaled_resolve_fastest;
		
		// list->add_command(cmd);
	}
}

framebuffer_id world_target_info::world_fb() {
	return msaa ? render_ms : get_fb();
}

texture_id world_target_info::get_output() {
	return current0 ? effect0 : effect1;
}

framebuffer_id world_target_info::get_fb() {
	return current0 ? effect0_fb : effect1_fb;
}

void world_target_info::flip_fb() {
	current0 = !current0;
}

void world_target_info::destroy() {

	exile->eng->ogl.destroy_texture(effect0);
	exile->eng->ogl.destroy_texture(effect1);
	effect_depth.destroy();

	exile->eng->ogl.destroy_framebuffer(effect0_fb);
	exile->eng->ogl.destroy_framebuffer(effect1_fb);
	
	effect0 = effect1 = effect0_fb = effect1_fb = -1;
	effect0_target = effect1_target = effect_depth_target = {};

	if(msaa) {
		depth_buf.destroy();
		exile->eng->ogl.destroy_texture(col_buf);
		exile->eng->ogl.destroy_framebuffer(render_ms);
		
		depth_buf_target = col_buf_target = {};
		col_buf = render_ms = -1;
	}
}

void exile_renderer::recreate_targets() {
	destroy_targets();
	generate_targets();
}

void exile_renderer::check_recreate() {
	if(exile->eng->window.settings.w != prev_dim.x ||
	   exile->eng->window.settings.h != prev_dim.y ||
	   settings.num_samples != prev_samples) {

		recreate_targets();
	}
}

void exile_renderer::generate_commands() { 
	
	#define reg(cmdn, name, path) cmd_##cmdn = exile->eng->ogl.add_command(FPTR(run_##name), FPTR(uniforms_##name), \
											   "shaders/" path #cmdn ".v"_, "shaders/" path #cmdn ".f"_);
 
	reg(chunk, mesh_chunk, "");
	reg(cubemap, mesh_cubemap, "");

	reg(skyfar, mesh_skyfar, "env/");
	reg(skydome, mesh_skydome, "env/");
	reg(pointcloud, mesh_pointcloud, "env/");

	reg(lines, mesh_lines, "mesh/");
	reg(2D_col, mesh_2D_col, "mesh/");
	reg(2D_tex, mesh_2D_tex, "mesh/");
	reg(3D_tex, mesh_3D_tex, "mesh/");
	reg(2D_tex_col, mesh_2D_tex_col, "mesh/");
	reg(3D_tex_instanced, mesh_3D_tex_instanced, "mesh/");

	resolve.init(FPTR(uniforms_resolve), "resolve.f"_);
	composite.init(FPTR(uniforms_composite), "composite.f"_);
	composite_resolve.init(FPTR(uniforms_composite_resolve), "composite_resolve.f"_);
	invert.init(FPTR(uniforms_invert), "invert.f"_);

	#undef reg
}

void exile_renderer::destroy() {

	exile->eng->ogl.rem_command(cmd_2D_col);
	exile->eng->ogl.rem_command(cmd_2D_tex);
	exile->eng->ogl.rem_command(cmd_2D_tex_col);
	exile->eng->ogl.rem_command(cmd_3D_tex);
	exile->eng->ogl.rem_command(cmd_3D_tex_instanced);
	exile->eng->ogl.rem_command(cmd_lines);
	exile->eng->ogl.rem_command(cmd_pointcloud);
	exile->eng->ogl.rem_command(cmd_cubemap);
	exile->eng->ogl.rem_command(cmd_chunk);
	exile->eng->ogl.rem_command(cmd_skydome);
	exile->eng->ogl.rem_command(cmd_skyfar);

	cmd_2D_col           = 0;
	cmd_2D_tex           = 0;
	cmd_2D_tex_col       = 0;
	cmd_3D_tex           = 0;
	cmd_3D_tex_instanced = 0;
	cmd_lines            = 0;
	cmd_pointcloud       = 0;
	cmd_cubemap          = 0;
	cmd_chunk            = 0;
	cmd_skydome          = 0;
	cmd_skyfar           = 0;

	destroy_targets();
	resolve.destroy();
	composite.destroy();
	composite_resolve.destroy();
	invert.destroy();

	hud_tasks.destroy();
	frame_tasks.destroy();

	the_cubemap.destroy();
	the_quad.destroy();
}

void exile_renderer::destroy_targets() {

	world_target.destroy();
}

CALLBACK void uniforms_composite(shader_program* prog, render_command* cmd) {

	i32 textures = 0;
	DO(8) {
		if(cmd->info.textures[__i]) {
			glUniform1i(prog->location(string::makef("textures[%]"_, __i)), __i);
			textures++;
		}
	}	
	glUniform1i(prog->location("num_textures"_), textures);
}

CALLBACK void uniforms_composite_resolve(shader_program* prog, render_command* cmd) {

	uniforms_composite(prog, cmd);

	exile_renderer* set = (exile_renderer*)cmd->info.user_data0;

	glUniform1i(prog->location("num_samples"_), set->prev_samples);
	glUniform2f(prog->location("screen_size"_), (f32)set->prev_dim.x, (f32)set->prev_dim.y);
}

CALLBACK void uniforms_resolve(shader_program* prog, render_command* cmd) {

	exile_renderer* set = (exile_renderer*)cmd->info.user_data0;

	glUniform1i(prog->location("tex"_), 0);
	glUniform1i(prog->location("num_samples"_), set->prev_samples);
	glUniform2f(prog->location("screen_size"_), (f32)set->prev_dim.x, (f32)set->prev_dim.y);
}

CALLBACK void uniforms_invert(shader_program* prog, render_command* cmd) {

	glUniform1i(prog->location("tex"_), 0);
}

CALLBACK void run_effect(render_command* cmd, gpu_object* gpu) {
	glDrawArrays(gl_draw_mode::triangles, 0, 6);
}

CALLBACK void setup_mesh_quad(gpu_object* obj) {

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);

	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (GLvoid*)0);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (GLvoid*)(sizeof(v2)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

CALLBACK void update_mesh_quad(gpu_object* obj, void* data, bool force) {
	
	mesh_quad* m = (mesh_quad*)data;
	if(!force) return;

	glNamedBufferData(obj->vbos[0], sizeof(m->vbo_data), m->vbo_data, gl_buf_usage::static_draw);
}

void mesh_quad::init() {

	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_quad), FPTR(update_mesh_quad), this);
	exile->eng->ogl.object_trigger_update(gpu, this, true);
}

void mesh_quad::destroy() {

	exile->eng->ogl.destroy_object(gpu);
}

CALLBACK void uniforms_mesh_skyfar(shader_program* prog, render_command* cmd) { 
	
	uniforms_mesh_skydome(prog, cmd);
}

CALLBACK void uniforms_mesh_pointcloud(shader_program* prog, render_command* cmd) { 

	uniforms_mesh_skydome(prog, cmd);
}

CALLBACK void uniforms_mesh_skydome(shader_program* prog, render_command* cmd) { 

	world_time* time = (world_time*)cmd->info.user_data0;

	GLint tloc = prog->location("transform"_);
	GLint dloc = prog->location("day_01"_);
	GLint sloc = prog->location("tex"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniform1i(sloc, 0);
	glUniform1f(dloc, time->day_01());
	glUniformMatrix4fv(tloc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_cubemap(shader_program* prog, render_command* cmd) { 

	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_chunk(shader_program* prog, render_command* cmd) { 

	world* w = (world*)cmd->info.user_data0;
	world_time* time = &w->time;
	world_render_settings* set = (world_render_settings*)cmd->info.user_data1;
	
	m4 mvp = cmd->info.proj * cmd->info.view * cmd->info.model;
	m4 mv = w->p.camera.offset() * cmd->info.model;

	glUniform1i(prog->location("blocks_tex"_), 0);
	glUniform1i(prog->location("sky_tex"_), 1);

	glUniform1i(prog->location("do_fog"_), set->dist_fog);
	glUniform1i(prog->location("do_light"_), set->block_light);
	glUniform1i(prog->location("smooth_light"_), set->smooth_light);
	glUniform1i(prog->location("debug_light"_), (i32)set->light_debug_mode);

	glUniform1f(prog->location("day_01"_), time->day_01());
	glUniform1f(prog->location("ambient"_), set->ambient_factor);
	glUniform1f(prog->location("units_per_voxel"_), (f32)chunk::units_per_voxel);
	glUniform1f(prog->location("render_distance"_), (f32)w->settings.view_distance * chunk::wid);

	glUniform4fv(prog->location("ao_curve"_), 1, set->ao_curve.a);

	glUniformMatrix4fv(prog->location("mvp"_), 1, gl_bool::_false, mvp.a);
	glUniformMatrix4fv(prog->location("m"_), 1, gl_bool::_false, mv.a);
}

CALLBACK void uniforms_mesh_2D_col(shader_program* prog, render_command* cmd) { 

	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_2D_tex(shader_program* prog, render_command* cmd) { 

	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_2D_tex_col(shader_program* prog, render_command* cmd) { 

	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_3D_tex(shader_program* prog, render_command* cmd) { 
	
	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_lines(shader_program* prog, render_command* cmd) { 
	
	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void uniforms_mesh_3D_tex_instanced(shader_program* prog, render_command* cmd) { 
	
	GLint loc = prog->location("transform"_);

	m4 transform = cmd->info.proj * cmd->info.view * cmd->info.model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
}

CALLBACK void update_mesh_pointcloud(gpu_object* obj, void* data, bool force) { 

	mesh_pointcloud* m = (mesh_pointcloud*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->vertices.size * sizeof(v4), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_cubemap(gpu_object* obj, void* data, bool force) { 

	mesh_cubemap* m = (mesh_cubemap*)data;
	if(!force) return;

	glNamedBufferData(obj->vbos[0], 36 * sizeof(v3), m->vertices, gl_buf_usage::static_draw);
}

CALLBACK void update_mesh_chunk(gpu_object* obj, void* data, bool force) { 

	mesh_chunk* m = (mesh_chunk*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->quads.size * sizeof(chunk_quad), m->quads.size ? m->quads.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_2D_col(gpu_object* obj, void* data, bool force) { 

	mesh_2d_col* m = (mesh_2d_col*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[1], m->colors.size * sizeof(colorf), m->colors.size ? m->colors.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[2], m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_2D_tex(gpu_object* obj, void* data, bool force) { 

	mesh_2d_tex* m = (mesh_2d_tex*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[1], m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[2], m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_2D_tex_col(gpu_object* obj, void* data, bool force) { 

	mesh_2d_tex_col* m = (mesh_2d_tex_col*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->vertices.size * sizeof(v2), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[1], m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[2], m->colors.size * sizeof(colorf), m->colors.size ? m->colors.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[3], m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_3D_tex(gpu_object* obj, void* data, bool force) { 

	mesh_3d_tex* m = (mesh_3d_tex*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->vertices.size * sizeof(v3), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[1], m->texCoords.size * sizeof(v2), m->texCoords.size ? m->texCoords.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[2], m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_lines(gpu_object* obj, void* data, bool force) { 

	mesh_lines* m = (mesh_lines*)data;
	if(!force && !m->dirty) return;

	glNamedBufferData(obj->vbos[0], m->vertices.size * sizeof(v3), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glNamedBufferData(obj->vbos[1], m->colors.size * sizeof(colorf), m->colors.size ? m->colors.memory : null, gl_buf_usage::dynamic_draw);

	m->dirty = false;
}

CALLBACK void update_mesh_3D_tex_instanced(gpu_object* obj, void* d, bool force) { 

	mesh_3d_tex_instance_data* data = (mesh_3d_tex_instance_data*)d;
	mesh_3d_tex* m = data->parent;

	exile->eng->ogl.object_trigger_update(m->gpu, m, force);

	if(force || data->dirty) {

		glNamedBufferData(obj->vbos[0], data->data.size * sizeof(v3), data->data.size ? data->data.memory : null, gl_buf_usage::dynamic_draw);
	}

	data->dirty = false;
}

CALLBACK void run_mesh_skyfar(render_command* cmd, gpu_object* gpu) { 

	run_mesh_3D_tex(cmd, gpu);
}

CALLBACK void run_mesh_pointcloud(render_command* cmd, gpu_object* gpu) { 

	mesh_pointcloud* m = (mesh_pointcloud*)gpu->data;

	u32 num_pts = cmd->info.num_tris ? cmd->info.num_tris : m->vertices.size;
	glDrawArrays(gl_draw_mode::points, 0, num_pts);
}

CALLBACK void run_mesh_skydome(render_command* cmd, gpu_object* gpu) { 	

	run_mesh_3D_tex(cmd, gpu);
}

CALLBACK void run_mesh_cubemap(render_command* cmd, gpu_object* gpu) { 

	glDrawArrays(gl_draw_mode::triangles, 0, 36);
}

CALLBACK void run_mesh_chunk(render_command* cmd, gpu_object* gpu) { 

	mesh_chunk* m = (mesh_chunk*)gpu->data;

	u32 num_faces = cmd->info.num_tris ? cmd->info.num_tris : m->quads.size;
	glDrawArraysInstanced(gl_draw_mode::triangle_strip, 0, 4, num_faces);
}

CALLBACK void run_mesh_2D_col(render_command* cmd, gpu_object* gpu) { 

	mesh_2d_col* m = (mesh_2d_col*)gpu->data;

	u32 num_tris = ((cmd->info.num_tris ? cmd->info.num_tris : m->elements.size) - cmd->info.start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->info.offset);
}

CALLBACK void run_mesh_2D_tex(render_command* cmd, gpu_object* gpu) { 

	mesh_2d_tex* m = (mesh_2d_tex*)gpu->data;

	u32 num_tris = ((cmd->info.num_tris ? cmd->info.num_tris : m->elements.size) - cmd->info.start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->info.offset);
}

CALLBACK void run_mesh_2D_tex_col(render_command* cmd, gpu_object* gpu) { 

	mesh_2d_tex_col* m = (mesh_2d_tex_col*)gpu->data;

	u32 num_tris = ((cmd->info.num_tris ? cmd->info.num_tris : m->elements.size) - cmd->info.start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->info.offset);
}

CALLBACK void run_mesh_3D_tex(render_command* cmd, gpu_object* gpu) { 

	mesh_3d_tex* m = (mesh_3d_tex*)gpu->data;

	u32 num_tris = ((cmd->info.num_tris ? cmd->info.num_tris : m->elements.size) - cmd->info.start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->info.offset);
}

CALLBACK void run_mesh_lines(render_command* cmd, gpu_object* gpu) { 

	mesh_lines* m = (mesh_lines*)gpu->data;

	glDrawArrays(gl_draw_mode::lines, 0, m->vertices.size);
}

CALLBACK void run_mesh_3D_tex_instanced(render_command* cmd, gpu_object* gpu) { 

	mesh_3d_tex_instance_data* data = (mesh_3d_tex_instance_data*)gpu->data;
	mesh_3d_tex* m = data->parent;

	u32 num_tris = ((cmd->info.num_tris ? cmd->info.num_tris : m->elements.size) - cmd->info.start_tri) * 3;

	glDrawElementsInstancedBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), data->instances, cmd->info.offset);
}

CALLBACK void setup_mesh_pointcloud(gpu_object* obj) { 

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);

	glVertexAttribPointer(0, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glEnableVertexAttribArray(0);
}

CALLBACK void setup_mesh_cubemap(gpu_object* obj) { 

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);

	glVertexAttribPointer(0, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glEnableVertexAttribArray(0);
}

CALLBACK void setup_mesh_chunk(gpu_object* obj) { 

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);

	glVertexAttribIPointer(0, 4, gl_vert_attrib_type::unsigned_int, sizeof(chunk_quad), (void*)(0));
	glVertexAttribIPointer(1, 3, gl_vert_attrib_type::unsigned_int, sizeof(chunk_quad), (void*)(16));
	glVertexAttribDivisor(0, 1);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

CALLBACK void setup_mesh_2D_col(gpu_object* obj) { 

	glBindVertexArray(obj->vao);

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, obj->vbos[1]);
	glVertexAttribPointer(1, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(gl_buf_target::element_array, obj->vbos[2]);

	glBindVertexArray(0);
}

CALLBACK void setup_mesh_2D_tex(gpu_object* obj) { 

	glBindVertexArray(obj->vao);

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, obj->vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(gl_buf_target::element_array, obj->vbos[2]);

	glBindVertexArray(0);
}

CALLBACK void setup_mesh_2D_tex_col(gpu_object* obj) { 

	glBindVertexArray(obj->vao);

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);
	glVertexAttribPointer(0, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, obj->vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(gl_buf_target::array, obj->vbos[2]);
	glVertexAttribPointer(2, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v4), (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(gl_buf_target::element_array, obj->vbos[3]);

	glBindVertexArray(0);
}

CALLBACK void setup_mesh_3D_tex(gpu_object* obj) { 

	glBindVertexArray(obj->vao);

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);
	glVertexAttribPointer(0, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, obj->vbos[1]);
	glVertexAttribPointer(1, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v2), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(gl_buf_target::element_array, obj->vbos[2]);

	glBindVertexArray(0);
}

CALLBACK void setup_mesh_3D_tex_instanced(gpu_object* obj) { 

	glBindVertexArray(obj->vao);

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);
	glVertexAttribPointer(2, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

CALLBACK void setup_mesh_lines(gpu_object* obj) { 

	glBindVertexArray(obj->vao);

	glBindBuffer(gl_buf_target::array, obj->vbos[0]);
	glVertexAttribPointer(0, 3, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(v3), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::array, obj->vbos[1]);
	glVertexAttribPointer(1, 4, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(colorf), (void*)0);
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
}

void mesh_pointcloud::init(allocator* alloc) {

	if(!alloc) alloc = CURRENT_ALLOC();

	vertices = vector<v4>::make(32, alloc);
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_pointcloud), FPTR(update_mesh_pointcloud), this);	
}

void mesh_pointcloud::destroy() {

	vertices.destroy();

	exile->eng->ogl.destroy_object(gpu);
}

void mesh_pointcloud::push(v3 p, f32 s) {

	vertices.push(v4(p, s));
	dirty = true;
}

void mesh_cubemap::init() {

	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_cubemap), FPTR(update_mesh_cubemap), this);
	
	exile->eng->ogl.object_trigger_update(gpu, this, true);
}

void mesh_cubemap::destroy() {

	exile->eng->ogl.destroy_object(gpu);
}

void mesh_lines::clear() {
	vertices.clear();
	colors.clear();
}

void mesh_lines::init(allocator* alloc) { 

	if(!alloc) alloc = CURRENT_ALLOC();

	vertices = vector<v3>::make(32, alloc);
	colors = vector<colorf>::make(32, alloc);
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_lines), FPTR(update_mesh_lines), this);
}

void mesh_lines::push(v3 p1, v3 p2, colorf c) {

	push(p1, p2, c, c);
}

void mesh_lines::push(v3 p1, v3 p2, colorf c1, colorf c2) { 

	vertices.push(p1);
	vertices.push(p2);
	colors.push(c1);
	colors.push(c2);

	dirty = true;
}

void mesh_lines::destroy() { 

	vertices.destroy();
	colors.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_3d_tex_instance_data::init(mesh_3d_tex* par, u32 i, allocator* alloc) { 

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	parent = par;
	data = vector<v3>::make(i, alloc);
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_3D_tex_instanced), FPTR(update_mesh_3D_tex_instanced), this);
}

void mesh_3d_tex_instance_data::destroy() { 

	data.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_3d_tex_instance_data::clear() { 

	data.clear();
	instances = 0;
	dirty = true;
}

bool mesh_3d_tex_instance_data::empty() { 

	return !data.size;
}

void mesh_2d_col::init(allocator* alloc) { 

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	vertices = vector<v2>::make(1024, alloc);
	colors 	  =	vector<colorf>::make(1024, alloc);
	elements  = vector<uv3>::make(1024, alloc); 
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_2D_col), FPTR(update_mesh_2D_col), this);
}

void mesh_2d_col::destroy() { 

	vertices.destroy();
	colors.destroy();
	elements.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_2d_col::clear() { 
	vertices.clear();
	colors.clear();
	elements.clear();
	dirty = true;
}

bool mesh_2d_col::empty() { 

	return !vertices.size;
}

void mesh_2d_col::push_tri(v2 p1, v2 p2, v2 p3, color c) { 

	u32 idx = vertices.size;
	
	vertices.push(p1);
	vertices.push(p2);
	vertices.push(p3);

	DO(3) colors.push(c.to_f());

	elements.push(uv3(idx, idx + 1, idx + 2));

	dirty = true;
}

void mesh_2d_col::push_cutrect(r2 r, f32 round, color c) { 

	u32 idx = vertices.size;

	vertices.push(v2(r.x, r.y + round));
	vertices.push(v2(r.x, r.y + r.h - round));
	vertices.push(v2(r.x + round, r.y + r.h));
	vertices.push(v2(r.x + r.w - round, r.y + r.h));
	vertices.push(v2(r.x + r.w, r.y + r.h - round));
	vertices.push(v2(r.x + r.w, r.y + round));
	vertices.push(v2(r.x + r.w - round, r.y));
	vertices.push(v2(r.x + round, r.y));

	colorf cf = c.to_f();
	DO(8) colors.push(cf);

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx, idx + 2, idx + 7));
	elements.push(uv3(idx + 7, idx + 2, idx + 3));
	elements.push(uv3(idx + 7, idx + 6, idx + 3));
	elements.push(uv3(idx + 3, idx + 4, idx + 5));
	elements.push(uv3(idx + 3, idx + 5, idx + 6));

	dirty = true;
}

void mesh_2d_col::push_rect(r2 r, color c) { 

	u32 idx = vertices.size;

	vertices.push(v2(r.x, r.y + r.h));	// BLC
	vertices.push(r.xy);				// TLC
	vertices.push(r.xy + r.wh);		 	// BRC
	vertices.push(v2(r.x + r.w, r.y));	// TRC

	DO(4) colors.push(c.to_f());

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx + 1, idx + 2, idx + 3));

	dirty = true;
}


void mesh_2d_tex::init(allocator* alloc) { 

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	vertices = vector<v2>::make(1024, alloc);
	texCoords =	vector<v2>::make(1024, alloc);
	elements  = vector<uv3>::make(1024, alloc); 
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_2D_tex), FPTR(update_mesh_2D_tex), this);
}

void mesh_2d_tex::destroy() { 

	vertices.destroy();
	texCoords.destroy();
	elements.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_2d_tex::clear() { 
	vertices.clear();
	texCoords.clear();
	elements.clear();

	dirty = true;
}

bool mesh_2d_tex::empty() { 

	return !vertices.size;
}

void mesh_2d_tex_col::init(allocator* alloc) { 

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	vertices  = vector<v2>::make(1024, alloc);
	texCoords =	vector<v2>::make(1024, alloc);
	colors 	  = vector<colorf>::make(1024, alloc);
	elements  = vector<uv3>::make(1024, alloc); 
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_2D_tex_col), FPTR(update_mesh_2D_tex_col), this);
}

void mesh_2d_tex_col::destroy() { 

	vertices.destroy();
	texCoords.destroy();
	colors.destroy();
	elements.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_2d_tex_col::clear() { 
	vertices.clear();
	texCoords.clear();
	elements.clear();
	colors.clear();
}

bool mesh_2d_tex_col::empty() { 

	return !vertices.size;
}

void mesh_3d_tex::init(allocator* alloc) { 

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	vertices = vector<v3>::make(1024, alloc);
	texCoords = vector<v2>::make(1024, alloc);
	elements  = vector<uv3>::make(1024, alloc);
	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_3D_tex), FPTR(update_mesh_3D_tex), this);
}

void mesh_3d_tex::destroy() { 

	vertices.destroy();
	texCoords.destroy();
	elements.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_3d_tex::clear() { 
	vertices.clear();
	texCoords.clear();
	elements.clear();
}

bool mesh_3d_tex::empty() { 

	return !vertices.size;
}

f32 mesh_2d_tex_col::push_text_line(asset* font, string text_utf8, v2 pos, f32 point, color c) { 

	colorf cf = c.to_f();
	f32 x = pos.x;
	f32 y = pos.y;
	f32 scale = point / font->raster_font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}
	y += scale * font->raster_font.linedist;

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		u32 idx = vertices.size;
		glyph_data glyph = font->raster_font.get_glyph(codepoint);

		f32 w = (f32)font->raster_font.width;
		f32 h = (f32)font->raster_font.height;
		v2 tlc (glyph.x1/w, 1.0f - glyph.y1/h);
		v2 brc (glyph.x2/w, 1.0f - glyph.y2/h);
		v2 trc (glyph.x2/w, 1.0f - glyph.y1/h);
		v2 blc (glyph.x1/w, 1.0f - glyph.y2/h);

		vertices.push(v2(x + scale*glyph.xoff1, y + scale*glyph.yoff2)); 	// BLC
 		vertices.push(v2(x + scale*glyph.xoff1, y + scale*glyph.yoff1));	// TLC
 		vertices.push(v2(x + scale*glyph.xoff2, y + scale*glyph.yoff2));	// BRC
 		vertices.push(v2(x + scale*glyph.xoff2, y + scale*glyph.yoff1));	// TRC

		texCoords.push(blc);
		texCoords.push(tlc);
		texCoords.push(brc);
		texCoords.push(trc);

		DO(4) colors.push(cf);

		elements.push(uv3(idx, idx + 1, idx + 2));
		elements.push(uv3(idx + 1, idx + 2, idx + 3));

		x += scale * glyph.advance;
	}

	dirty = true;
	return scale * font->raster_font.linedist;
}

void mesh_pointcloud::push_points(v3 center, f32 r, i32 points, f32 jitter) { 

	for(i32 i = 0; i < points; i++) {
		
		f32 y = randf() * 2.0f - 1.0f;
		f32 ph = acos(y);
		f32 th = randf() * 2.0f * PI32;
	
		f32 ct = cos(th), st = sin(th), sp = sin(ph);
		v3 point = v3(r * ct * sp, r * y, r * st * sp) + jitter * rand_unit();

		vertices.push(v4(point, 2.0f * abs(randf())));
	}

	dirty = true;
}

// Concept from https://github.com/fogleman/Craft
void mesh_3d_tex::push_dome(v3 center, f32 r, i32 divisions) { 

	i32 p_divisions = divisions / 2 + 1;

	f32 th = 0.0f;
	for(i32 i = 0; i <= divisions; i++) {

		f32 ph = 0.0f;
		for(i32 j = 0; j < p_divisions; j++) {

			f32 ct = cos(th), st = sin(th), sp = sin(ph), cp = cos(ph);
			v3 point = v3(r * ct * sp, r * cp, r * st * sp);

			vertices.push(center + point);
			texCoords.push(v2(th / (2.0f * PI32), (0.5f * cp) + 0.5f));
			// texCoords.push(v2(th / (2.0f * PI32), ph / (1.0f * PI32)));

			ph += (PI32 * 2.0f) / divisions;
		}

		th += (PI32 * 2.0f) / divisions;
		if(i == divisions - 1) th += 0.0001f;
	}

	for (int x = 0; x < divisions; x++) {
		for (int y = 0; y < p_divisions - 1; y++) {
			GLuint idx = x * p_divisions + y;

			i32 idx1 = (idx + 1);
			i32 idxp = (idx + p_divisions);
			i32 idxp1 = (idx + p_divisions + 1);

			elements.push(uv3(idx, idx1, idxp));
			elements.push(uv3(idx1, idxp, idxp1));
		}
	}

	dirty = true;
}

void mesh_3d_tex::push_cube(v3 pos, f32 len) {

	u32 idx = vertices.size;

	f32 len2 = len / 2.0f;
	vertices.push(pos + v3( len2,  len2,  len2));
	vertices.push(pos + v3(-len2,  len2,  len2));
	vertices.push(pos + v3( len2, -len2,  len2));
	vertices.push(pos + v3( len2,  len2, -len2));
	vertices.push(pos + v3(-len2, -len2,  len2));
	vertices.push(pos + v3( len2, -len2, -len2));
	vertices.push(pos + v3(-len2,  len2, -len2));
	vertices.push(pos + v3(-len2, -len2, -len2));

	texCoords.push(v2(0.0f, 0.0f));
	texCoords.push(v2(1.0f, 0.0f));
	texCoords.push(v2(0.0f, 1.0f));
	texCoords.push(v2(0.0f, 0.0f));
	texCoords.push(v2(1.0f, 0.0f));
	texCoords.push(v2(0.0f, 1.0f));
	texCoords.push(v2(1.0f, 0.0f));
	texCoords.push(v2(1.0f, 1.0f));	

	elements.push(uv3(idx + 0, idx + 3, idx + 6));
	elements.push(uv3(idx + 0, idx + 3, idx + 5));
	elements.push(uv3(idx + 0, idx + 1, idx + 6));
	elements.push(uv3(idx + 1, idx + 4, idx + 7));
	elements.push(uv3(idx + 1, idx + 6, idx + 7));
	elements.push(uv3(idx + 4, idx + 2, idx + 5));
	elements.push(uv3(idx + 4, idx + 7, idx + 5));
	elements.push(uv3(idx + 7, idx + 5, idx + 3));
	elements.push(uv3(idx + 7, idx + 6, idx + 3));
	elements.push(uv3(idx + 0, idx + 2, idx + 4));
	elements.push(uv3(idx + 0, idx + 1, idx + 4));

	dirty = true;
}

mesh_chunk mesh_chunk::make_cpu(u32 verts, allocator* alloc) { 

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_chunk ret;

	ret.quads = vector<chunk_quad>::make(verts, alloc);

	return ret;
}

void mesh_chunk::init_gpu() { 

	gpu = exile->eng->ogl.add_object(FPTR(setup_mesh_chunk), FPTR(update_mesh_chunk), this);
}

void mesh_chunk::swap_mesh(mesh_chunk other) { 

	quads.destroy();
	quads = other.quads;

	dirty = true;
}

void mesh_chunk::destroy() { 

	quads.destroy();

	exile->eng->ogl.destroy_object(gpu);
	gpu = -1;
}

void mesh_chunk::free_cpu() { 

	quads.resize(0);
}

void mesh_chunk::clear() { 

	quads.clear();

	dirty = true;
}

void mesh_chunk::quad(iv3 v_0, iv3 v_1, iv3 v_2, iv3 v_3, iv2 uv, i32 t, u8 ql, bv4 ao, bv4 l) {

	chunk_quad q = {};

	LOG_DEBUG_ASSERT(
		0 <= v_0.x && v_0.x < 256 && 
		0 <= v_0.y && v_0.y < 4096 &&
		0 <= v_0.z && v_0.z < 256 && 

		0 <= v_1.x && v_1.x < 256 && 
		0 <= v_1.y && v_1.y < 4096 &&
		0 <= v_1.z && v_1.z < 256 && 

		0 <= v_2.x && v_2.x < 256 && 
		0 <= v_2.y && v_2.y < 4096 &&
		0 <= v_2.z && v_2.z < 256 && 

		0 <= v_3.x && v_3.x < 256 && 
		0 <= v_3.y && v_3.y < 4096 &&
		0 <= v_3.z && v_3.z < 256 && 

		0 <= uv.x && uv.x < 256 && 
		0 <= uv.y && uv.y < 256 && 

		0 <= t && t < 65536
	);

	q.x_0 = (u8)v_0.x; q.z_0 = (u8)v_0.z;
	q.x_1 = (u8)v_1.x; q.z_1 = (u8)v_1.z;
	q.x_2 = (u8)v_2.x; q.z_2 = (u8)v_2.z;
	q.x_3 = (u8)v_3.x; q.z_3 = (u8)v_3.z;

	q.uy01 |= (u32)v_0.y << 20; q.uy01 |= (u32)v_1.y << 8;
	q.vy23 |= (u32)v_2.y << 20; q.vy23 |= (u32)v_3.y << 8;

	q.uy01 |= (u8)uv.x; q.vy23 |= (u8)uv.y;

	q.t = (u16)t;
	q.aol |= (u16)ql << 8;
	q.aol |= (u8)ao.w; q.aol |= (u8)ao.z << 2; q.aol |= (u8)ao.y << 4; q.aol |= (u8)ao.x << 6; 

	q.l3 = l.w; q.l2 = l.z; q.l1 = l.y; q.l0 = l.x; 

	quads.push(q);
	dirty = true;
}
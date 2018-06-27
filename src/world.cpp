
void world::init(asset_store* store, allocator* a) { PROF

	alloc = a;
	p.init();

	LOG_INFO_F("units_per_voxel: %", chunk::units_per_voxel);

	eng->ogl.add_command(render_command_type::mesh_chunk, FPTR(buffers_mesh_chunk), FPTR(run_mesh_chunk), "shaders/mesh_chunk.v"_, "shaders/mesh_chunk.f"_, FPTR(uniforms_mesh_chunk), FPTR(compat_mesh_chunk));
	
	block_textures = eng->ogl.begin_tex_array(iv3(32, 32, (i32)NUM_BLOCKS), texture_wrap::repeat, true, 1);
	eng->ogl.push_tex_array(block_textures, store, "bedrock"_);
	eng->ogl.push_tex_array(block_textures, store, "stone"_);

	chunks = map<chunk_pos, chunk*>::make(512, a);

	LOG_DEBUG_F("% logical cores % physical cores", global_api->get_num_cpus(), global_api->get_phys_cpus());

	thread_pool = threadpool::make(a, eng->platform->get_phys_cpus() - 1);
	thread_pool.start_all();
}

void world::destroy_chunks() { PROF 

	FORMAP(it, chunks) {
		it->value->destroy();
		PUSH_ALLOC(it->value->alloc) {
			free(it->value, sizeof(chunk));
		} POP_ALLOC();
	}
	chunks.destroy();
}

void world::destroy() { PROF

	thread_pool.stop_all();
	thread_pool.destroy();
	destroy_chunks();
}

v3 world::raymarch(v3 pos3, v3 dir3, f32 max) {

	chunk** start = chunks.try_get(chunk_pos::from_abs(pos3));
	if(!start) return pos3;
	chunk* c = *start;

	v4 pos; pos.xyz = pos3;
	v4 dir; dir.xyz = norm(dir3);
	
	f32 progress = 0.0f;

	while(progress < max) {

		v4 current = pos + dir * progress;
		iv3 current_vox = current;

		if(c->block_at(current_vox.x % chunk::xsz, current_vox.y % chunk::ysz, current_vox.z % chunk::zsz) != block_type::air) {
			return current.xyz;
		}

		v4 delta = (v4(1.0f) - fract(current)) / dir;
		progress += min3(delta.x, delta.y, delta.z);
	}

	return pos3 + dir3 * max;
}

void world::update(u64 now) { PROF

	ImGui::Begin("Exile"_, null, ImGuiWindowFlags_AlwaysAutoResize);
	
	ImGui::SliderInt("view", &view_distance, 0, 32);

	ImGui::Checkbox("Wireframe "_, &wireframe);
	ImGui::SameLine();
	ImGui::Checkbox("Respect Cam "_, &respect_cam);
	
	if(ImGui::Button("Regenerate"_)) {
		
		destroy_chunks();
		chunks = map<chunk_pos, chunk*>::make(512, alloc);
	}

	p.update(now);

	ImGui::End();
}

void world::populate_local_area() { PROF

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = camera + chunk_pos(x,0,z);
			current.y = 0;
			
			if(!chunks.try_get(current)) {
				
				chunk** c = chunks.insert(current, chunk::make_new(current, alloc));
				
				(*c)->job_state.set(work::in_flight);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;

					c->gen();
					c->build_data();
					c->job_state.set(work::done);

				}, *c, 1.0f / lensq(current.center_xz() - p.camera.pos));
			}
		}
	}
}

CALLBACK void unlock_chunk(void* v) { PROF

	chunk* c = (chunk*)v;

	eng->platform->release_mutex(&c->swap_mut);
}

float check_pirority(super_job* j, void* param) {

	world* w = (world*)param;
	player* p = &w->p;
	chunk* c = (chunk*)j->data;

	v3 center = c->pos.center_xz();

	if(abs(center.x - p->camera.pos.x) > (f32)(w->view_distance + 1) * chunk::xsz ||
	   abs(center.z - p->camera.pos.z) > (f32)(w->view_distance + 1) * chunk::zsz) {
		return -FLT_MAX;
	}

	return 1.0f / lensq(center - p->camera.pos);
}

CALLBACK void cancel_build(void* param) {

	chunk* c = (chunk*)param;
	c->job_state.set(work::none);
}

void world::render() { PROF

	render_command_list rcl = render_command_list::make();
	thread_pool.renew_priorities(check_pirority, this);

	chunk_pos camera = chunk_pos::from_abs(p.camera.pos);
	for(i32 x = -view_distance; x <= view_distance; x++) {
		for(i32 z = -view_distance; z <= view_distance; z++) {

			chunk_pos current = respect_cam ? camera + chunk_pos(x,0,z) : chunk_pos(x,0,z);
			current.y = 0;
			chunk** ch = chunks.try_get(current);

			auto build_job = [&]() {

				(*ch)->job_state.set(work::in_flight);

				thread_pool.queue_job([](void* p) -> void {
					chunk* c = (chunk*)p;

					c->gen();
					c->build_data();
					c->job_state.set(work::done);

				}, *ch, 1.0f / lensq(current.center_xz() - p.camera.pos), FPTR(cancel_build));
			};
			
			if(!ch) {
				ch = chunks.insert(current, chunk::make_new(current, alloc));
				build_job();
			}
			chunk* c = *ch;

			if(c->job_state.get() == work::none) {
				build_job();
			}

			if(!c->mesh.dirty) {
				c->mesh.free_cpu();
			}

			eng->platform->aquire_mutex(&c->swap_mut);
			render_command cmd = render_command::make(render_command_type::mesh_chunk, &c->mesh);

			cmd.num_tris = c->mesh_triangles;
			cmd.texture = block_textures;

			v3 chunk_pos = v3((f32)current.x * chunk::xsz, (f32)current.y * chunk::ysz, (f32)current.z * chunk::zsz);
			cmd.model = translate(chunk_pos - p.camera.pos);

			cmd.callback.set(FPTR(unlock_chunk));
			cmd.param = c;

			rcl.add_command(cmd);
		}
	}

	rcl.view = p.camera.view_no_translate();
	rcl.proj = proj(p.camera.fov, (f32)eng->window.w / (f32)eng->window.h, 0.01f, 2000.0f);

	if(wireframe)
		glPolygonMode(gl_poly::front_and_back, gl_poly_mode::line);

	eng->ogl.execute_command_list(&rcl);
	glPolygonMode(gl_poly::front_and_back, gl_poly_mode::fill);

	rcl.destroy();

	p.render();
}

void player::render() { PROF

	render_command_list rcl = render_command_list::make();
	mesh_lines lines = mesh_lines::make();

	lines.push(camera.pos, camera.pos + camera.front, colorf(1,0,0,1), colorf(0,0,1,1));

	render_command cmd = render_command::make(render_command_type::mesh_lines, &lines);

	rcl.add_command(cmd);
	rcl.view = camera.view3();
	rcl.proj = proj(camera.fov, (f32)eng->window.w / (f32)eng->window.h, 0.01f, 2000.0f);

	eng->ogl.execute_command_list(&rcl);
	rcl.destroy();
	lines.destroy();
}

void player::init() { PROF

	speed = 30.0f;
	last = 0;
	camera.reset();
}

void player::update(u64 now) { PROF

	u64 pdt = now - last;
	f32 dt = (f32)pdt / (f32)eng->platform->get_perfcount_freq();

	if(enable && eng->platform->window_focused(&eng->window)) {

		velocity = v3(0.0f, 0.0f, 0.0f);
		if(eng->platform->keydown(platform_keycode::w)) {
			velocity += camera.front * speed;
		}
		if(eng->platform->keydown(platform_keycode::a)) {
			velocity += camera.right * -speed;
		}
		if(eng->platform->keydown(platform_keycode::s)) {
			velocity += camera.front * -speed;
		}
		if(eng->platform->keydown(platform_keycode::d)) {
			velocity += camera.right * speed;
		}
		if(eng->platform->keydown(platform_keycode::space)) {
			velocity.y += speed;
		}
		if(eng->platform->keydown(platform_keycode::lshift)) {
			velocity.y += -speed;
		}

		camera.pos += velocity * dt;
		camera.update();
	}

	ImGui::Text(string::makef("pos: %"_, camera.pos));
	ImGui::Text(string::makef("vel: %"_, velocity));
	ImGui::Text(string::makef("look: %"_, camera.front));
	ImGui::Text(string::makef("chunk: %"_, chunk_pos::from_abs(camera.pos)));

	last = now;
}

inline u32 hash(chunk_pos key) { PROF

	return hash(key.x) ^ hash(key.y) ^ hash(key.z);
}

chunk_pos::chunk_pos(i32 _x, i32 _y, i32 _z) {
	x = _x; y = _y; z = _z;
}

bool operator==(chunk_pos l, chunk_pos r) { PROF

	return l.x == r.x && l.y == r.y && l.z == r.z;
}

chunk_pos chunk_pos::from_abs(v3 pos) { PROF

	chunk_pos ret;
	ret.x = (i32)(pos.x / chunk::xsz) - (pos.x < 0 ? 1 : 0);
	ret.y = (i32)(pos.y / chunk::ysz) - (pos.y < 0 ? 1 : 0);
	ret.z = (i32)(pos.z / chunk::zsz) - (pos.z < 0 ? 1 : 0);
	return ret;
}

v3 chunk_pos::center_xz() { PROF

	return v3(x * chunk::xsz + chunk::xsz / 2.0f, 0.0f, z * chunk::zsz + chunk::zsz / 2.0f);
}

chunk_pos chunk_pos::operator+(chunk_pos other) { PROF
	chunk_pos ret;
	ret.x = x + other.x;
	ret.y = y + other.y;
	ret.z = z + other.z;
	return ret;
}

chunk_pos chunk_pos::operator-(chunk_pos other) { PROF
	chunk_pos ret;
	ret.x = x - other.x;
	ret.y = y - other.y;
	ret.z = z - other.z;
	return ret;
}

chunk chunk::make(chunk_pos p, allocator* a) { PROF

	chunk ret;

	ret.pos = p;
	ret.alloc = a;
	ret.mesh = mesh_chunk::make_gpu();
	eng->platform->create_mutex(&ret.swap_mut, false);

	return ret;
}

chunk* chunk::make_new(chunk_pos p, allocator* a) { PROF

	PUSH_ALLOC(a);

	chunk* ret = NEW(chunk);

	ret->pos = p;
	ret->alloc = a;
	ret->mesh = mesh_chunk::make_gpu();
	eng->platform->create_mutex(&ret->swap_mut, false);

	POP_ALLOC();

	return ret;
}

void chunk::destroy() { PROF

	mesh.destroy();
	eng->platform->destroy_mutex(&swap_mut);
}

i32 chunk::y_at(i32 x, i32 z) { PROF

	f32 val = perlin((f32)x / 32.0f, 0, (f32)z / 32.0f, 0, 0, 0);
	i32 height = (u32)(val * ysz / 2.0f + ysz / 2.0f);

	return height;
}

void chunk::gen() { PROF

	for(u32 x = 0; x < xsz; x++) {
		for(u32 z = 0; z < zsz; z++) {

			u32 height = y_at(pos.x * xsz + x, pos.z * zsz + z);

			blocks[x][z][0] = block_type::bedrock;
			_memset(&blocks[x][z][1], height, (u8)block_type::stone);
		}
	}
}

u8 chunk::ao_at(v3 vert) { 

	i32 x = (i32)vert.x, y = (i32)vert.y, z = (i32)vert.z;

	bool top0 = block_at(x-1,y,z) != block_type::air;
	bool top1 = block_at(x,y,z-1) != block_type::air;
	bool top2 = block_at(x,y,z) != block_type::air;
	bool top3 = block_at(x-1,y,z-1) != block_type::air;
	bool bot0 = block_at(x-1,y-1,z) != block_type::air;
	bool bot1 = block_at(x,y-1,z-1) != block_type::air;
	bool bot2 = block_at(x,y-1,z) != block_type::air;
	bool bot3 = block_at(x-1,y-1,z-1) != block_type::air;

	bool side0, side1, corner;

	if(!top0 && bot0) {
		side0 = top2;
		side1 = top3;
		corner = top1;
	} else if(!top1 && bot1) {
		side0 = top2;
		side1 = top3;
		corner = top0;
	} else if(!top2 && bot2) {
		side0 = top0;
		side1 = top1;
		corner = top3;
	} else if(!top3 && bot3) {
		side0 = top0;
		side1 = top1;
		corner = top2;
	} else {
		return 3;
	}

	if(side0 && side1) {
		return 0;
	}
	return 3 - side0 - side1 - corner;
}

block_type chunk::block_at(i32 x, i32 y, i32 z) { PROF

	if(y < 0) return block_type::air;

	if(x < 0 || x >= xsz || y >= ysz || z < 0 || z >= zsz) {

		// TODO(max): if the neighboring chunk exists, get a block from it
		return y_at(pos.x * xsz + x, pos.z * zsz + z) >= y ? block_type::stone : block_type::air; 
	}

	return blocks[x][z][y];
}

mesh_face chunk::build_face(block_type t, iv3 p, i32 dir) { PROF

	mesh_face ret;
	ret.type = t;

	switch(dir) {
	case 0: {
		ret.ao[0] = ao_at(p);
		ret.ao[1] = ao_at(p + v3(0,1,0));
		ret.ao[2] = ao_at(p + v3(0,0,1));
		ret.ao[3] = ao_at(p + v3(0,1,1));
	} break;
	case 1: {
		ret.ao[0] = ao_at(p);
		ret.ao[1] = ao_at(p + v3(1,0,0));
		ret.ao[2] = ao_at(p + v3(0,0,1));
		ret.ao[3] = ao_at(p + v3(1,0,1));
	} break;
	case 2: {
		ret.ao[0] = ao_at(p);
		ret.ao[1] = ao_at(p + v3(1,0,0));
		ret.ao[2] = ao_at(p + v3(0,1,0));
		ret.ao[3] = ao_at(p + v3(1,1,0));
	} break;
	case 3: {
		ret.ao[0] = ao_at(p + v3(1,0,0));
		ret.ao[1] = ao_at(p + v3(1,1,0));
		ret.ao[2] = ao_at(p + v3(1,0,1));
		ret.ao[3] = ao_at(p + v3(1,1,1));
	} break;
	case 4: {
		ret.ao[0] = ao_at(p + v3(0,1,0));
		ret.ao[1] = ao_at(p + v3(1,1,0));
		ret.ao[2] = ao_at(p + v3(0,1,1));
		ret.ao[3] = ao_at(p + v3(1,1,1));
	} break;
	case 5: {
		ret.ao[0] = ao_at(p + v3(0,0,1));
		ret.ao[1] = ao_at(p + v3(1,0,1));
		ret.ao[2] = ao_at(p + v3(0,1,1));
		ret.ao[3] = ao_at(p + v3(1,1,1));
	} break;
	}

	return ret;
}

bool mesh_face::can_merge(mesh_face f1, mesh_face f2, i32 dir) { PROF

	return f1.type == f2.type && f1.ao == f2.ao;
}

void chunk::build_data() { PROF

	PUSH_PROFILE_PROF(false);

	mesh_chunk new_mesh = mesh_chunk::make_cpu(1024, alloc);

	// Array to hold 2D block slice (sized for largest slice)
	block_type slice[xsz * ysz];

	iv3 max = {xsz, ysz, zsz};

	//  0  1  2  3  4  5 
	// -x -y -z +x +y +z
	for(i32 i = 0; i < 6; i++) {

		// Axes of 2D slice to mesh
		i32 ortho_2d = i % 3;
		i32 u_2d = (i + 1) % 3;
		i32 v_2d = (i + 2) % 3;
		i32 backface_offset = i / 3 * 2 - 1;

		// Iterate over orthogonal orthogonal to slice
		iv3 position;
		for(position[ortho_2d] = 0; position[ortho_2d] < max[ortho_2d]; position[ortho_2d]++) {

			// Iterate over 2D slice blocks to filter culled faces before greedy step
			for(position[v_2d] = 0; position[v_2d] < max[v_2d]; position[v_2d]++) {
				for(position[u_2d] = 0; position[u_2d] < max[u_2d]; position[u_2d]++) {

					block_type block = blocks[position[0]][position[2]][position[1]];
					i32 slice_idx = position[u_2d] + position[v_2d] * max[u_2d];

					// Only add the face to the slice if its opposing face is air
					if(block != block_type::air) {

						iv3 backface = position;
						backface[ortho_2d] += backface_offset;

						block_type backface_block = block_at(backface[0],backface[1],backface[2]);

						if(backface_block != block_type::air) {
							slice[slice_idx] = block_type::air;
						} else {
							slice[slice_idx] = block;
						}
					} else {
						slice[slice_idx] = block_type::air;
					}
				}
			}

			// Iterate over slice filled with relevant faces
			for(i32 v = 0; v < max[v_2d]; v++) {
				for(i32 u = 0; u < max[u_2d];) {

					position[u_2d] = u;
					position[v_2d] = v;
					i32 slice_idx = u + v * max[u_2d];

					block_type single_type = slice[slice_idx];
					
					if(single_type != block_type::air) {

						mesh_face face_type = build_face(single_type, position, i);

						i32 width = 1, height = 1;

						// Combine same faces in +u_2d
						for(; u + width < max[u_2d]; width++) {

							iv3 w_pos = position;
							w_pos[u_2d] += width;

							mesh_face merge = build_face(slice[slice_idx + width], w_pos, i);

							if(!mesh_face::can_merge(merge, face_type, i)) break;
						}

						// Combine all-same face row in +v_2d
						bool done = false;
						for(; v + height < max[v_2d]; height++) {
							for(i32 row_idx = 0; row_idx < width; row_idx++) {

								iv3 wh_pos = position;
								wh_pos[u_2d] += row_idx;
								wh_pos[v_2d] +=  height;

								mesh_face merge = build_face(slice[slice_idx + row_idx + height * max[u_2d]], wh_pos, i);

								if(!mesh_face::can_merge(merge, face_type, i)) {
									done = true;
									break;
								}
							}
							if(done) {
								break;
							}
						}

						// Add quad (u,v,width,height) in 2D slice

						v3 width_offset, height_offset;
						width_offset[u_2d] = (f32)width;
						height_offset[v_2d] = (f32)height;

						v3 v_0 = position;
						if(backface_offset > 0) {
							v_0[ortho_2d] += 1.0f;
						}

						v3 v_1 = v_0 + width_offset;
						v3 v_2 = v_0 + height_offset;
						v3 v_3 = v_0 + width_offset + height_offset;
						v3 wht = v3(width, height, (i32)single_type), hwt = v3(height, width, (i32)single_type);
						u8 ao_0 = ao_at(v_0), ao_1 = ao_at(v_1), ao_2 = ao_at(v_2), ao_3 = ao_at(v_3);

						switch (i) {
						case 0: // -X
							new_mesh.quad(v_0, v_2, v_1, v_3, hwt, bv4(ao_0,ao_2,ao_1,ao_3));
							break;
						case 1: // -Y
							new_mesh.quad(v_2, v_3, v_0, v_1, wht, bv4(ao_2,ao_3,ao_0,ao_1));
							break;
						case 2: // -Z
							new_mesh.quad(v_2, v_3, v_0, v_1, wht, bv4(ao_2,ao_3,ao_0,ao_1));
							break;
						case 3: // +X
							new_mesh.quad(v_2, v_0, v_3, v_1, hwt, bv4(ao_2,ao_0,ao_3,ao_1));
							break;
						case 4: // +Y
							new_mesh.quad(v_0, v_1, v_2, v_3, wht, bv4(ao_0,ao_1,ao_2,ao_3));
							break;
						case 5: // +Z
							new_mesh.quad(v_0, v_1, v_2, v_3, wht, bv4(ao_0,ao_1,ao_2,ao_3));
							break;
						}

						// Erase quad area in slice
						for(i32 h = 0; h < height; h++)  {
							_memset(&slice[slice_idx + h * max[u_2d]], sizeof(block_type) * width, 0);
						}

						u += width;
					} else {
						u++;
					}
				}
			}
		}
	}

	POP_PROFILE_PROF();

	eng->platform->aquire_mutex(&swap_mut);
	mesh.swap_mesh(new_mesh);
	mesh_triangles = mesh.elements.size;
	eng->platform->release_mutex(&swap_mut);
}

CALLBACK void run_mesh_chunk(render_command* cmd) { PROF

	mesh_chunk* m = (mesh_chunk*)cmd->mesh;

	glBindVertexArray(m->vao);

	glEnable(gl_capability::depth_test);
	glEnable(gl_capability::cull_face);

	u32 num_tris = ((cmd->num_tris ? cmd->num_tris : m->elements.size) - cmd->start_tri) * 3;
	glDrawElementsBaseVertex(gl_draw_mode::triangles, num_tris, gl_index_type::unsigned_int, (void*)(u64)(0), cmd->offset);

	glDisable(gl_capability::cull_face);
	glDisable(gl_capability::depth_test);

	glBindVertexArray(0);
}

CALLBACK void buffers_mesh_chunk(render_command* cmd) { PROF

	mesh_chunk* m = (mesh_chunk*)cmd->mesh;
	if(!m->dirty) return;

	glBindVertexArray(m->vao);

	glBindBuffer(gl_buf_target::array, m->vbos[0]);
	glBufferData(gl_buf_target::array, m->vertices.size * sizeof(chunk_vertex), m->vertices.size ? m->vertices.memory : null, gl_buf_usage::dynamic_draw);

	glBindBuffer(gl_buf_target::element_array, m->vbos[1]);
	glBufferData(gl_buf_target::element_array, m->elements.size * sizeof(uv3), m->elements.size ? m->elements.memory : null, gl_buf_usage::dynamic_draw);

	glBindVertexArray(0);

	m->dirty = false;
}

CALLBACK void uniforms_mesh_chunk(shader_program* prog, render_command* cmd, render_command_list* rcl) { PROF

	GLint loc = glGetUniformLocation(prog->handle, "transform");
	GLint szloc = glGetUniformLocation(prog->handle, "units_per_voxel");

	m4 transform = rcl->proj * rcl->view * cmd->model;

	glUniformMatrix4fv(loc, 1, gl_bool::_false, transform.a);
	glUniform1f(szloc, (f32)chunk::units_per_voxel);
}

CALLBACK bool compat_mesh_chunk(ogl_info* info) { PROF
	return info->check_version(3, 2);
}

chunk_vertex chunk_vertex::from_vec(v3 v, v3 uv, bv4 ao) { PROF

	LOG_DEBUG_ASSERT(v.x >= 0 && v.x < 256);
	LOG_DEBUG_ASSERT(v.y >= 0 && v.y < 4096);
	LOG_DEBUG_ASSERT(v.z >= 0 && v.z < 256);
	LOG_DEBUG_ASSERT(uv.x >= 0 && uv.x < 256);
	LOG_DEBUG_ASSERT(uv.y >= 0 && uv.y < 256);
	LOG_DEBUG_ASSERT(uv.z >= 0 && uv.z < 1024);

	chunk_vertex ret;
	ret.x = (u8)v.x;
	ret.z = (u8)v.z;
	ret.y_ao |= (u16)v.y << 4;
	ret.y_ao |= ao.x << 2;
	ret.y_ao |= ao.y;

	ret.u = (u8)uv.x;
	ret.v = (u8)uv.y;
	ret.ao_t = (u16)uv.z;
	ret.ao_t |= (u16)ao.z << 14;
	ret.ao_t |= (u16)ao.w << 12;

	return ret;
}

mesh_chunk mesh_chunk::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_chunk ret;

	ret.vertices = vector<chunk_vertex>::make(verts, alloc);
	ret.elements = vector<uv3>::make(verts, alloc);

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(2, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);

	glVertexAttribIPointer(0, 2, gl_vert_attrib_type::unsigned_int, sizeof(chunk_vertex), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::element_array, ret.vbos[1]);

	glBindVertexArray(0);

	return ret;
}

mesh_chunk mesh_chunk::make_gpu() { PROF

	mesh_chunk ret;

	glGenVertexArrays(1, &ret.vao);
	glGenBuffers(2, ret.vbos);

	glBindVertexArray(ret.vao);

	glBindBuffer(gl_buf_target::array, ret.vbos[0]);

	glVertexAttribIPointer(0, 2, gl_vert_attrib_type::unsigned_int, sizeof(chunk_vertex), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(gl_buf_target::element_array, ret.vbos[1]);

	glBindVertexArray(0);

	return ret;
}

mesh_chunk mesh_chunk::make_cpu(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_chunk ret;

	ret.vertices = vector<chunk_vertex>::make(verts, alloc);
	ret.elements = vector<uv3>::make(verts, alloc);

	return ret;
}

void mesh_chunk::swap_mesh(mesh_chunk other) { PROF

	vertices.destroy();
	elements.destroy();

	vertices = other.vertices;
	elements = other.elements;

	dirty = true;
}

void mesh_chunk::destroy() { PROF

	glDeleteBuffers(2, vbos);
	glDeleteVertexArrays(1, &vao);

	vertices.destroy();
	elements.destroy();
}

void mesh_chunk::free_cpu() { PROF

	vertices.resize(0);
	elements.resize(0);
}

void mesh_chunk::clear() { PROF

	vertices.clear();
	elements.clear();

	dirty = true;
}

void mesh_chunk::quad(v3 p1, v3 p2, v3 p3, v3 p4, v3 uv_ext, bv4 ao) { PROF

	u32 idx = vertices.size;

	vertices.push(chunk_vertex::from_vec(p1 * (f32)chunk::units_per_voxel, v3(0.0f, 0.0f, uv_ext.z), ao));
	vertices.push(chunk_vertex::from_vec(p2 * (f32)chunk::units_per_voxel, v3(uv_ext.x, 0.0f, uv_ext.z), ao));
	vertices.push(chunk_vertex::from_vec(p3 * (f32)chunk::units_per_voxel, v3(0.0f, uv_ext.y, uv_ext.z), ao));
	vertices.push(chunk_vertex::from_vec(p4 * (f32)chunk::units_per_voxel, v3(uv_ext.x, uv_ext.y, uv_ext.z), ao));

	elements.push(uv3(idx, idx + 1, idx + 2));
	elements.push(uv3(idx + 3, idx + 2, idx + 1));

	dirty = true;
}

void mesh_chunk::cube(v3 pos, f32 len) { PROF

	u32 idx = vertices.size;

	f32 len2 = len / 2.0f;
	pos += {len2, len2, len2};

	vertices.push(chunk_vertex::from_vec(pos + v3( len2,  len2,  len2), v3(0,0,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2,  len2,  len2), v3(1,0,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3( len2, -len2,  len2), v3(0,1,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3( len2,  len2, -len2), v3(0,0,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2, -len2,  len2), v3(1,0,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3( len2, -len2, -len2), v3(0,1,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2,  len2, -len2), v3(1,0,0), bv4()));
	vertices.push(chunk_vertex::from_vec(pos + v3(-len2, -len2, -len2), v3(1,1,0), bv4()));

	elements.push(uv3(idx + 0, idx + 3, idx + 5));
	elements.push(uv3(idx + 0, idx + 3, idx + 6));
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

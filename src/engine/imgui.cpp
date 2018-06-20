
void* imgui_alloc(u64 size, void* data) { PROF

	allocator* a = (allocator*)data;
	return a->allocate_(size, 0, a, CONTEXT);
}

void imgui_free(void* mem, void* data) { PROF

	if (mem) {
		allocator* a = (allocator*)data;
		a->free_(mem, 0, a, CONTEXT);
	}
}

imgui_manager imgui_manager::make(platform_window* window, allocator* a) { PROF

	imgui_manager ret;

	a->track_sizes = false;
	ret.alloc = a;
	ret.perf_freq = global_api->get_perfcount_freq();

	ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, a);
	
	ret.context = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.MouseDrawCursor = true;

	io.KeyMap[ImGuiKey_Tab]        = (i32)platform_keycode::tab;
	io.KeyMap[ImGuiKey_LeftArrow]  = (i32)platform_keycode::left;
	io.KeyMap[ImGuiKey_RightArrow] = (i32)platform_keycode::right;
	io.KeyMap[ImGuiKey_UpArrow]    = (i32)platform_keycode::up;
	io.KeyMap[ImGuiKey_DownArrow]  = (i32)platform_keycode::down;
	io.KeyMap[ImGuiKey_PageUp]     = (i32)platform_keycode::pgup;
	io.KeyMap[ImGuiKey_PageDown]   = (i32)platform_keycode::pgdown;
	io.KeyMap[ImGuiKey_Home]       = (i32)platform_keycode::home;
	io.KeyMap[ImGuiKey_End]        = (i32)platform_keycode::end;
	io.KeyMap[ImGuiKey_Insert]     = (i32)platform_keycode::insert;
	io.KeyMap[ImGuiKey_Delete]     = (i32)platform_keycode::del;
	io.KeyMap[ImGuiKey_Backspace]  = (i32)platform_keycode::backspace;
	io.KeyMap[ImGuiKey_Space]      = (i32)platform_keycode::space;
	io.KeyMap[ImGuiKey_Enter]      = (i32)platform_keycode::enter;
	io.KeyMap[ImGuiKey_Escape]     = (i32)platform_keycode::escape;
	io.KeyMap[ImGuiKey_A]          = (i32)platform_keycode::a;
	io.KeyMap[ImGuiKey_C]          = (i32)platform_keycode::c;
	io.KeyMap[ImGuiKey_V]          = (i32)platform_keycode::v;
	io.KeyMap[ImGuiKey_X]          = (i32)platform_keycode::x;
	io.KeyMap[ImGuiKey_Y]          = (i32)platform_keycode::y;
	io.KeyMap[ImGuiKey_Z]          = (i32)platform_keycode::z;

	ret.cursor_values[(i32)ImGuiMouseCursor_Arrow] = platform_cursor::pointer;
	ret.cursor_values[(i32)ImGuiMouseCursor_TextInput] = platform_cursor::I;

	// TODO(max): non-windows clipboard handling

	ret.gl_info.program = glCreateProgram();
	ret.gl_info.vertex = glCreateShader(gl_shader_type::vertex);
	ret.gl_info.fragment = glCreateShader(gl_shader_type::fragment);
	glShaderSource(ret.gl_info.vertex, 1, &imgui_vertex_shader, null);
	glShaderSource(ret.gl_info.fragment, 1, &imgui_fragment_shader, null);
	glCompileShader(ret.gl_info.vertex);
	glCompileShader(ret.gl_info.fragment);
	glAttachShader(ret.gl_info.program, ret.gl_info.vertex);
	glAttachShader(ret.gl_info.program, ret.gl_info.fragment);
	glLinkProgram(ret.gl_info.program);

	ret.gl_info.tex_loc = glGetUniformLocation(ret.gl_info.program, "Texture");
	ret.gl_info.mat_loc = glGetUniformLocation(ret.gl_info.program, "ProjMtx");
	ret.gl_info.pos_loc = glGetAttribLocation(ret.gl_info.program, "Position");
	ret.gl_info.uv_loc = glGetAttribLocation(ret.gl_info.program, "UV");
	ret.gl_info.color_loc = glGetAttribLocation(ret.gl_info.program, "Color");

	glGenVertexArrays(1, &ret.gl_info.vao);
	glBindVertexArray(ret.gl_info.vao);

	glGenBuffers(1, &ret.gl_info.vbo);
	glGenBuffers(1, &ret.gl_info.ebo);
	
	glBindBuffer(gl_buf_target::array, ret.gl_info.vbo);
	glBindBuffer(gl_buf_target::element_array, ret.gl_info.ebo);

	glEnableVertexAttribArray(ret.gl_info.pos_loc);
	glEnableVertexAttribArray(ret.gl_info.uv_loc);
	glEnableVertexAttribArray(ret.gl_info.color_loc);

	glVertexAttribPointer(ret.gl_info.pos_loc, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
	glVertexAttribPointer(ret.gl_info.uv_loc, 2, gl_vert_attrib_type::_float, gl_bool::_false, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
	glVertexAttribPointer(ret.gl_info.color_loc, 4, gl_vert_attrib_type::unsigned_byte, gl_bool::_true, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));

	glBindVertexArray(0);

	u8* bitmap;
	i32 w, h;
	io.Fonts->GetTexDataAsRGBA32(&bitmap, &w, &h);

	glGenTextures(1, &ret.gl_info.font_texture);
	glBindTexture(gl_tex_target::_2D, ret.gl_info.font_texture);
	glTexParameteri(gl_tex_target::_2D, gl_tex_param::min_filter, (GLint)gl_tex_filter::nearest);
	glTexParameteri(gl_tex_target::_2D, gl_tex_param::mag_filter, (GLint)gl_tex_filter::nearest);
	glPixelStorei(gl_pix_store::unpack_row_length, 0);
	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba, w, h, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, bitmap);
	glBindTexture(gl_tex_target::_2D, 0);

	io.Fonts->TexID = (void*)(u64)ret.gl_info.font_texture;

	return ret;
}

void imgui_manager::destroy() { PROF

	glDeleteBuffers(1, &gl_info.vbo);
	glDeleteBuffers(1, &gl_info.ebo);

	glDeleteProgram(gl_info.program);
	glDeleteShader(gl_info.vertex);
	glDeleteShader(gl_info.fragment);

	glDeleteTextures(1, &gl_info.font_texture);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->TexID = 0;

	gl_info = {};

	ImGui::DestroyContext(context);
	context = null;
}

void imgui_manager::reload() { PROF

	ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, alloc);
	ImGui::SetCurrentContext(context);
}

void imgui_manager::process_event(platform_event evt) { PROF

	ImGuiIO& io = ImGui::GetIO();

	if(evt.type == platform_event_type::key) {

		io.KeysDown[(i32)evt.key.code] = (evt.key.flags & (u16)platform_keyflag::press) != 0 || 
										 (evt.key.flags & (u16)platform_keyflag::repeat) != 0;
		io.KeyShift = (evt.key.flags & (u16)platform_keyflag::shift) != 0;
		io.KeyCtrl 	= (evt.key.flags & (u16)platform_keyflag::ctrl) != 0;
		io.KeyAlt 	= (evt.key.flags & (u16)platform_keyflag::alt) != 0;
	}

	else if(evt.type == platform_event_type::mouse) {
	
		if(evt.mouse.flags & (u16)platform_mouseflag::press) {

			mouse[0] = (evt.mouse.flags & (u16)platform_mouseflag::lclick) != 0;
			mouse[1] = (evt.mouse.flags & (u16)platform_mouseflag::rclick) != 0;
			mouse[2] = (evt.mouse.flags & (u16)platform_mouseflag::mclick) != 0;
		}

		else if(evt.mouse.flags & (u16)platform_mouseflag::wheel) {

			io.MouseWheel += evt.mouse.w;
		}
	}

	// TODO(max): text input
}

void imgui_manager::begin_frame(platform_window* window) { PROF

	ImGuiIO& io = ImGui::GetIO();

    int w, h;
    int display_w, display_h;
    global_api->get_window_size(window, &w, &h);
    global_api->get_window_drawable(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((f32)w, (f32)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((f32)display_w / w) : 0, h > 0 ? ((f32)display_h / h) : 0);

	u64 perf = global_api->get_perfcount();
	
	io.DeltaTime = (f32)((f64)(perf - last_perf) / perf_freq);
	last_perf = perf;

	if(io.WantSetMousePos)
		CHECKED(set_cursor_pos, window, (i32)io.MousePos.x, (i32)io.MousePos.y);

	io.MouseDown[0] = mouse[0] || global_api->mousedown(platform_mouseflag::lclick);
	io.MouseDown[1] = mouse[1] || global_api->mousedown(platform_mouseflag::rclick);
	io.MouseDown[2] = mouse[2] || global_api->mousedown(platform_mouseflag::mclick);
	mouse[0] = mouse[1] = mouse[2] = false;

	if(global_api->window_focused(window)) {

		i32 mx, my;
		CHECKED(get_cursor_pos, window, &mx, &my);
		io.MousePos = ImVec2((f32)mx, (f32)my);
	}

	global_api->set_cursor(cursor_values[ImGui::GetMouseCursor()]);

	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
}

void imgui_manager::end_frame() { PROF

	ImGui::Render();

	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGuiIO& io = ImGui::GetIO();

    i32 fb_width = (i32)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    i32 fb_height = (i32)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	glEnable(gl_capability::blend);
	glBlendEquation(gl_blend_mode::add);
	glBlendFunc(gl_blend_factor::src_alpha, gl_blend_factor::one_minus_src_alpha);
	glDisable(gl_capability::cull_face);
	glDisable(gl_capability::depth_test);
	glEnable(gl_capability::scissor_test);
	glPolygonMode(gl_poly::front_and_back, gl_poly_mode::fill);

	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	m4 mat = ortho(0, (f32)io.DisplaySize.x, (f32)io.DisplaySize.y, 0, -1, 1);

	glUseProgram(gl_info.program);
	glUniform1i(gl_info.tex_loc, 0);
	glUniformMatrix4fv(gl_info.mat_loc, 1, gl_bool::_false, &mat[0][0]);
	glBindVertexArray(gl_info.vao);

	for(i32 n = 0; n < draw_data->CmdListsCount; n++) {

		ImDrawList* cmd_list = draw_data->CmdLists[n];
		ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(gl_buf_target::array, gl_info.vbo);
		glBufferData(gl_buf_target::array, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, gl_buf_usage::stream_draw);
		glBindBuffer(gl_buf_target::element_array, gl_info.ebo);
		glBufferData(gl_buf_target::element_array, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, gl_buf_usage::stream_draw);

		for(i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {

			ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			if(pcmd->UserCallback) {
				pcmd->UserCallback(cmd_list, pcmd);
			} else {
				glBindTexture(gl_tex_target::_2D, (GLuint)(u64)pcmd->TextureId);
				glScissor((i32)pcmd->ClipRect.x, (i32)(io.DisplaySize.y - pcmd->ClipRect.w), (i32)(pcmd->ClipRect.z - pcmd->ClipRect.x), (i32)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(gl_draw_mode::triangles, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? gl_index_type::unsigned_short : gl_index_type::unsigned_int, idx_buffer_offset);
			}

			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	glUseProgram(0);
	glBindTexture(gl_tex_target::_2D, 0);
	glBindVertexArray(0);
	glDisable(gl_capability::scissor_test);
}


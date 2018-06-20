
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

imgui_manager imgui_manager::make(allocator* a) { PROF

	imgui_manager ret;

	ret.alloc = a;
	a->track_sizes = false;

	ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, a);
	
	ret.context = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 

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

    glGenBuffers(1, &ret.gl_info.vbo);
    glGenBuffers(1, &ret.gl_info.ebo);

    u8* bitmap;
    i32 w, h;
    io.Fonts->GetTexDataAsRGBA32(&bitmap, &w, &h);

    glGenTextures(1, &ret.gl_info.font_texture);
    glBindTexture(gl_tex_target::_2D, ret.gl_info.font_texture);
	glTexParameteri(gl_tex_target::_2D, gl_tex_param::min_filter, (GLint)gl_tex_filter::nearest);
	glTexParameteri(gl_tex_target::_2D, gl_tex_param::mag_filter, (GLint)gl_tex_filter::nearest);
	glTexImage2D(gl_tex_target::_2D, 0, gl_tex_format::rgba, w, h, 0, gl_pixel_data_format::rgba, gl_pixel_data_type::unsigned_byte, bitmap);
	glBindTexture(gl_tex_target::_2D, 0);

	io.Fonts->TexID = (void*)(u64)ret.gl_info.font_texture;

	ImGui::ShowDemoWindow();

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

}

void imgui_manager::begin_frame() { PROF


}

void imgui_manager::end_frame() { PROF

}

void imgui_manager::render(ImDrawData* draw_data) { PROF

}

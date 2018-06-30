
#pragma once

namespace ImGui {

	bool InputText(string label, string buf, ImGuiInputTextFlags flags = 0, ImGuiTextEditCallback callback = null, void* user_data = null);
	void Text(string text);
	template<typename E> void EnumCombo(string label, E* val, ImGuiComboFlags flags = 0);
	template<typename V> void MapCombo(string label, map<string,V> options, V* val, ImGuiComboFlags flags = 0);

	template<typename S> void ViewAny(string label, S val, bool open = false);
	template<typename S> void EditAny(string label, S* val, bool open = false);

	bool TreeNode(string label);

	void EnumCombo_T(string label, void* val, _type_info* info, ImGuiComboFlags flags);
	void View_T(string label, void* val, _type_info* info, bool open = false);
	void Edit_T(string label, void* val, _type_info* info, bool open = false);
}

struct imgui_gl_info {
	
	GLuint font_texture = 0;
	GLuint program = 0;
	GLuint vertex = 0, fragment = 0;
	GLuint tex_loc = 0, mat_loc = 0;
	GLuint pos_loc = 0, uv_loc = 0, color_loc = 0;
	GLuint vao = 0, vbo = 0, ebo = 0;
};

struct imgui_manager {

	ImGuiContext* context = null;
	allocator* alloc = null;

	// NOTE(max): maybe make this use my rendering system like the original GUI system
	imgui_gl_info gl_info;

	u64 last_perf = 0, perf_freq = 0;
	bool mouse[3] = {};

	platform_cursor cursor_values[ImGuiMouseCursor_COUNT] = {platform_cursor::pointer};
	platform_cursor current_cursor = platform_cursor::pointer;

	static imgui_manager make(platform_window* window, allocator* a);
	void destroy(); 

	void reload();
	void process_event(platform_event evt);

	void begin_frame(platform_window* window);
	void end_frame();
};

void* imgui_alloc(u64 size, void* data);
void imgui_free(void* mem, void* data);

const GLchar* imgui_vertex_shader =
	"#version 330\n"
    "uniform mat4 ProjMtx;\n"
    "in vec2 Position;\n"
    "in vec2 UV;\n"
    "in vec4 Color;\n"
    "out vec2 Frag_UV;\n"
    "out vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "	Frag_UV = UV;\n"
    "	Frag_Color = Color;\n"
    "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
    "}\n";

const GLchar* imgui_fragment_shader =
	"#version 330\n"
    "uniform sampler2D Texture;\n"
    "in vec2 Frag_UV;\n"
    "in vec4 Frag_Color;\n"
    "out vec4 Out_Color;\n"
    "void main()\n"
    "{\n"
    "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
    "}\n";

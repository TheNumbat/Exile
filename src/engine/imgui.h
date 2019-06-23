
#pragma once

#include <imgui/imgui.h>
#include "ds/string.h"
#include "ds/map.h"
#include "platform/gl.h"
#include "asset.h"

namespace ImGui {

	bool InputText(string label, string buf, ImGuiInputTextFlags flags = 0, ImGuiTextEditCallback callback = null, void* user_data = null);
	void Text(string text);
	bool TreeNodeNoNull(string label);

	template<typename E> void EnumCombo(string label, E* val, ImGuiComboFlags flags = 0);
	template<typename V> void MapCombo(string label, map<string,V> options, V* val, ImGuiComboFlags flags = 0);

	template<typename S> void ViewAny(string label, S val, bool open = false);
	template<typename S> void EditAny(string label, S* val, bool open = false);

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

	string font_asset_name;
	asset_store* last_store = null;
	f32 font_size = 12.0f;

	static imgui_manager make(platform_window* window, allocator* a);
	void destroy(); 

	void set_font(string name, f32 size, asset_store* store);
	void load_font(asset_store* store = null);

	void gl_load();
	void gl_destroy();

	void reload();
	void process_event(platform_event evt);

	void begin_frame(platform_window* window);
	void end_frame();
};

void* imgui_alloc(u64 size, void* data);
void imgui_free(void* mem, void* data);

extern const GLchar* imgui_vertex_shader;
extern const GLchar* imgui_fragment_shader;

namespace ImGui {
	template<typename V>
	void MapCombo(string label, map<string,V> options, V* val, ImGuiComboFlags flags) { 

		string preview;
		FORMAP(it, options) {
			if(it->value == *val) {
				preview = it->key;
			}
		}

		if(BeginCombo(label, preview, flags)) {
			
			FORMAP(it, options) {
				bool selected = it->value == *val;
				if(Selectable(it->key, selected)) {
					*val = it->value;
				}
				if(selected) {
					SetItemDefaultFocus();
				}
			}
			EndCombo();
		}
	}

	template<typename S>
	void ViewAny(string label, S val, bool open) { 

		View_T(label, &val, TYPEINFO(S), open);
	}

	template<typename S>
	void EditAny(string label, S* val, bool open) { 

		Edit_T(label, val, TYPEINFO(S), open);
	}
	
	template<typename E>
	void EnumCombo(string label, E* val, ImGuiComboFlags flags) { 

		_type_info* info = TYPEINFO(E);
		LOG_DEBUG_ASSERT(info->type_type == Type::_enum);

		EnumCombo_T(label, val, info, flags);
	}
}

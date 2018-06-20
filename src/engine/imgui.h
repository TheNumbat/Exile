
#pragma once

struct imgui_manager {

	ImGuiContext* context = null;
	allocator* alloc = null;

	static imgui_manager make(allocator* a);
	void destroy(); 

	void reload();
};

void* imgui_alloc(u64 size, void* data);
void imgui_free(void* mem, void* data);

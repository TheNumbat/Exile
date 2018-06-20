
void* imgui_alloc(u64 size, void* data) { PROF

	allocator* a = (allocator*)data;
	return a->allocate_(size, 0, a, CONTEXT);
}

void imgui_free(void* mem, void* data) { PROF

	allocator* a = (allocator*)data;
	a->free_(mem, 0, a, CONTEXT);
}

imgui_manager imgui_manager::make(allocator* a) { PROF

	imgui_manager ret;

	ret.alloc = a;
	a->track_sizes = false;

	ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, a);
	
	ret.context = ImGui::CreateContext();
	ImGui::SetCurrentContext(ret.context);

	ImGui::ShowDemoWindow();

	return ret;
}

void imgui_manager::destroy() { PROF

	ImGui::DestroyContext(context);
	context = null;
}

void imgui_manager::reload() { PROF

	ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, alloc);
	ImGui::SetCurrentContext(context);
}

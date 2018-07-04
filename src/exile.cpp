	// 
void exile::init() { PROF

	alloc = MAKE_PLATFORM_ALLOCATOR("world");

	store = asset_store::make(&alloc);
	store.load("assets/game.asset"_);

	w.init(&store, &alloc);

	{
		default_evt = eng->evt.add_handler(FPTR(default_evt_handle), this);

		controls = evt_state_machine::make(&eng->evt, &alloc);

		camera_evt = controls.add_state(FPTR(camera_evt_handle), this);
		ui_evt = controls.add_state(FPTR(ui_evt_handle), this);

		controls.add_transition(camera_evt, ui_evt, FPTR(camera_to_ui), &w.p);
		controls.add_transition(ui_evt, camera_evt, FPTR(ui_to_camera), &w.p);
		
		controls.set_state(camera_evt);
		eng->platform->capture_mouse(&eng->window);
	}
}

void exile::update() { PROF

	u64 now = eng->platform->get_perfcount();

	w.update(now);

#ifndef RELEASE 
	if(store.try_reload()) {
		eng->ogl.reload_texture_assets();
		eng->imgui.load_font(&store);
	}
#endif
}

void exile::render() { PROF

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize;
	if(!eng->dbg.show_ui) {
		ImGui::SetNextWindowPos({0.0f, 0.0f});
		ImGui::Begin("Help"_, null, flags);
		ImGui::Text("GRAVE : show debug UI");
		ImGui::Text("W/A/S/D/Space/Shift : move");
		ImGui::End();
	} else {
		ImGui::SetNextWindowPos({0.0f, 0.0f});
		ImGui::Begin("Help"_, null, flags);
		ImGui::Text("GRAVE : hide debug UI");
		ImGui::Text("P : toggle profiler");
		ImGui::Text("O : toggle debug vars");
		ImGui::End();
	}

	w.render();
}

void exile::destroy() { PROF

	eng->evt.rem_handler(default_evt);

	w.destroy();
	store.destroy();
	
	controls.destroy();
}

CALLBACK bool default_evt_handle(void* param, platform_event evt) { PROF

	if(evt.type == platform_event_type::key) {
		if(evt.key.flags & (u16)platform_keyflag::press) {
			if(evt.key.code == platform_keycode::escape) {

				eng->running = false;
				return true;
			}
		}
	}

	return false;
}

CALLBACK void camera_to_ui(void* param) { PROF

	eng->platform->release_mouse(&eng->window);
	eng->dbg.show_ui = true;

	player* p = (player*)param;
	p->enable = false;
}

CALLBACK void ui_to_camera(void* param) { PROF

	eng->platform->capture_mouse(&eng->window);
	eng->dbg.show_ui = false;

	player* p = (player*)param;
	p->enable = true;
}

CALLBACK bool camera_evt_handle(void* param, platform_event evt) { PROF

	exile* game = (exile*)param;
	player& p = game->w.p;

	if(evt.type == platform_event_type::key) {

		if(evt.key.flags & (u16)platform_keyflag::press) {

			switch(evt.key.code) {

			case platform_keycode::grave: {

				game->controls.transition(game->ui_evt);

			} return true;

			default: return false;
			}
		}
	}

	else if(evt.type == platform_event_type::mouse) {
	
		if(evt.mouse.flags & (u16)platform_mouseflag::move) {
		
			p.camera.move(evt.mouse.x, evt.mouse.y, 0.1f);

			return true;
		}
	}

	else if(evt.type == platform_event_type::window) {

		if(evt.window.op == platform_windowop::unfocused) {

			game->controls.transition(game->ui_evt);
		}
	}

	return false;
}

CALLBACK bool ui_evt_handle(void* param, platform_event evt) { PROF

	exile* game = (exile*)param;

	if(evt.type == platform_event_type::key) {

		if(evt.key.flags & (u16)platform_keyflag::press) {

			switch(evt.key.code) {

			case platform_keycode::grave: {

				game->controls.transition(game->camera_evt);

			} return true;

			case platform_keycode::p: {

				eng->dbg.toggle_profile();

			} return true;

			case platform_keycode::o: {

				eng->dbg.toggle_vars();
				
			} return true;

			default: return false;
			}
		}
	}

	return false;
}

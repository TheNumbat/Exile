
void exile::init() { PROF_FUNC

	alloc = MAKE_PLATFORM_ALLOCATOR("world"_);

	store = asset_store::make(&alloc);
	store.load("assets/game.asset"_);

	setup_mesh_commands();
	setup_console_commands();

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

void exile::gl_reload() { 

	w.regenerate();	
}

void exile::update() { PROF_FUNC

	u64 now = eng->platform->get_perfcount();

	w.update(now);

#ifndef RELEASE 
	if(store.try_reload()) {
		eng->ogl.reload_texture_assets();
		eng->imgui.load_font(&store);
	}
#endif
}

void exile::render() { PROF_FUNC

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
		ImGui::Text("P : toggle iler");
		ImGui::Text("O : toggle debug vars");
		ImGui::End();
	}

	w.render();
}

void exile::destroy() { 

	eng->evt.rem_handler(default_evt);

	w.destroy();
	store.destroy();
	
	controls.destroy();
}

CALLBACK bool default_evt_handle(void* param, platform_event evt) { 

	if(evt.type == platform_event_type::key) {
		if(evt.key.flags & (u16)platform_keyflag::press) {
			
			if(evt.key.code == platform_keycode::escape) {

				eng->running = false;
				return true;
			
			} else if(evt.key.code == platform_keycode::f11) {

				eng->window.settings.mode = eng->window.settings.mode == platform_window_mode::fullscreen ? platform_window_mode::windowed : platform_window_mode::fullscreen;
				eng->platform->apply_window_settings(&eng->window);
				return true;
			}
		}
	}

	return false;
}

CALLBACK void camera_to_ui(void* param) { 

	eng->platform->release_mouse(&eng->window);
	eng->dbg.show_ui = true;

	player* p = (player*)param;
	p->enable = false;
}

CALLBACK void ui_to_camera(void* param) { 

	eng->platform->capture_mouse(&eng->window);
	eng->dbg.show_ui = false;

	player* p = (player*)param;
	p->enable = true;
}

CALLBACK bool camera_evt_handle(void* param, platform_event evt) { 

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

CALLBACK bool ui_evt_handle(void* param, platform_event evt) { 

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

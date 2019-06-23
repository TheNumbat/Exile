
#include "exile.h"
#include "console.h"

exile_state* exile = null;

void* start_up_game(engine* e) {
	exile = new (e->platform->heap_alloc(sizeof(exile_state))) exile_state;
	exile->eng = e;
	exile->init();
	return exile;
}

void gl_reload_game(void* game) {
	exile->gl_reload();
}

void run_game(void* game) {
	exile->update();
	exile->render();
}

void shut_down_game(void* game) {
	exile->destroy();
	exile->eng->platform->heap_free(exile);
}

void unload_game(engine* e, void* game) {
	exile->w.thread_pool.stop_all();
}

void reload_game(engine* e, void* game) {
	exile = (exile_state*)game;
	exile->w.thread_pool.start_all();
}

void exile_state::init() { PROF_FUNC

	a_general = MAKE_PLATFORM_ALLOCATOR("game_general"_);
	a_assets = MAKE_PLATFORM_ALLOCATOR("game_assets"_);
	a_world = MAKE_PLATFORM_ALLOCATOR("game_world"_);
	a_render = MAKE_PLATFORM_ALLOCATOR("game_render"_);

	store = asset_store::make(&a_assets);
	store.load("assets/game.asset"_);

	setup_console_commands();

	ren.init(&a_render);

	w.init(&store, &a_world);

	{
		default_evt = exile->eng->evt.add_handler(FPTR(default_evt_handle), this);

		controls = evt_state_machine::make(&exile->eng->evt, &a_general);

		camera_evt = controls.add_state(FPTR(camera_evt_handle), this);
		ui_evt = controls.add_state(FPTR(ui_evt_handle), this);

		controls.add_transition(camera_evt, ui_evt, FPTR(camera_to_ui), &w.p);
		controls.add_transition(ui_evt, camera_evt, FPTR(ui_to_camera), &w.p);
		
		controls.set_state(camera_evt);
		exile->eng->platform->capture_mouse(&exile->eng->window);
	}
}

void exile_state::gl_reload() { 

	w.regenerate();	
	ren.recreate_targets();
}

void exile_state::update() { PROF_FUNC

	u64 now = exile->eng->platform->get_perfcount();

	w.update(now);

#ifndef RELEASE 
	if(store.try_reload()) {
		exile->eng->ogl.reload_texture_assets();
		exile->eng->imgui.load_font(&store);
	}
#endif
}

void exile_state::render() { PROF_FUNC

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize;
	if(!exile->eng->dbg.show_ui) {
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

	// NOTE(max): engine IMGUI-based debug UI is rendered on top of everything, separately
	w.render();
	ren.end_frame();
}

void exile_state::destroy() { 

	exile->eng->evt.rem_handler(default_evt);

	w.destroy();
	store.destroy();
	ren.destroy();
	controls.destroy();
}

CALLBACK bool default_evt_handle(void* param, platform_event evt) { 

	if(evt.type == platform_event_type::key) {
		if(evt.key.flags & (u16)platform_keyflag::press) {
			
			if(evt.key.code == platform_keycode::escape) {

				exile->eng->running = false;
				return true;
			
			} else if(evt.key.code == platform_keycode::f11) {

				exile->eng->window.settings.mode = exile->eng->window.settings.mode == platform_window_mode::fullscreen ? platform_window_mode::windowed : platform_window_mode::fullscreen;
				exile->eng->platform->apply_window_settings(&exile->eng->window);
				return true;
			}
		}
	}

	return false;
}

CALLBACK void camera_to_ui(void* param) { 

	exile->eng->platform->release_mouse(&exile->eng->window);
	exile->eng->dbg.show_ui = true;

	player* p = (player*)param;
	p->enable = false;
}

CALLBACK void ui_to_camera(void* param) { 

	exile->eng->platform->capture_mouse(&exile->eng->window);
	exile->eng->dbg.show_ui = false;

	player* p = (player*)param;
	p->enable = true;
}

CALLBACK bool camera_evt_handle(void* param, platform_event evt) { 

	player& p = exile->w.p;

	if(evt.type == platform_event_type::key) {

		if(evt.key.flags & (u16)platform_keyflag::press) {

			switch(evt.key.code) {

			case platform_keycode::grave: {

				exile->controls.transition(exile->ui_evt);

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

		else if(evt.mouse.flags & (u16)platform_mouseflag::press) {

			if(evt.mouse.flags & (u16)platform_mouseflag::lclick) {
				exile->w.player_break_block();
			} else if(evt.mouse.flags & (u16)platform_mouseflag::rclick) {
				exile->w.player_place_block();
			}

			return true;
		}
	}

	else if(evt.type == platform_event_type::window) {

		if(evt.window.op == platform_windowop::unfocused) {

			exile->controls.transition(exile->ui_evt);
		}
	}

	return false;
}

CALLBACK bool ui_evt_handle(void* param, platform_event evt) { 

	if(evt.type == platform_event_type::key) {

		if(evt.key.flags & (u16)platform_keyflag::press) {

			switch(evt.key.code) {

			case platform_keycode::grave: {

				exile->controls.transition(exile->camera_evt);

			} return true;

			case platform_keycode::p: {

				exile->eng->dbg.toggle_profile();

			} return true;

			case platform_keycode::o: {

				exile->eng->dbg.toggle_vars();
				
			} return true;

			default: return false;
			}
		}
	}

	return false;
}


void exile::init() { PROF

	alloc = MAKE_PLATFORM_ALLOCATOR("world");

	store = asset_store::make(&alloc);
	store.load("assets/game.asset"_);

	w = world::make(&store, &alloc);

	{
		default_evt = eng->evt.add_handler(FPTR(default_evt_handle), this);

		controls = evt_state_machine::make(&eng->evt, &alloc);

		camera_evt = controls.add_state(FPTR(camera_evt_handle), this);
		ui_evt = controls.add_state(FPTR(ui_evt_handle), this);

		controls.add_transition(camera_evt, ui_evt, FPTR(camera_to_ui), this);
		controls.add_transition(ui_evt, camera_evt, FPTR(ui_to_camera), this);
		
		controls.set_state(camera_evt);
		eng->platform->capture_mouse(&eng->window);
	}
}

void exile::update() { PROF

	platform_perfcount now = eng->platform->get_perfcount();

	w.update(now);

	if(!eng->dbg.show_ui && eng->platform->window_focused(&eng->window)) {
		eng->platform->set_cursor_pos(&eng->window, eng->window.w / 2, eng->window.h / 2);
	}
}

void exile::render() { PROF

	w.render();
}

void exile::destroy() { PROF

	eng->evt.rem_handler(default_evt);

	w.destroy();
	store.destroy();
	
	controls.destroy();
	alloc.destroy();
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

	eng->dbg.show_ui = true;
	eng->platform->release_mouse();
}

CALLBACK void ui_to_camera(void* param) { PROF

	eng->dbg.show_ui = false;
	eng->platform->capture_mouse(&eng->window);
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
		
			i16 dx = evt.mouse.x - (i16)(eng->window.w / 2);
			i16 dy = evt.mouse.y - (i16)(eng->window.h / 2);

			p.camera.move(dx, dy, 0.1f);

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

			default: return false;
			}
		}
	}

	return false;
}

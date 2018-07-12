
EXPORT engine* start_up(platform_api* api) { 

	engine* state = new(api->heap_alloc(sizeof(engine))) engine;
	state->func_state.this_dll = api->your_dll;
	state->platform = api;

	global_api  = api;
	global_log  = &state->log;
	global_dbg  = &state->dbg;
	global_func = &state->func_state;

	setup_fptrs();

	state->basic_a = MAKE_PLATFORM_ALLOCATOR("basic");

	begin_thread("main"_, &state->basic_a);

	state->dbg_a = MAKE_PLATFORM_ALLOCATOR("dbg");
	state->dbg.init(&state->dbg_a);
	state->dbg.profiler.register_thread(180);

	BEGIN_FRAME();

	state->log_a = MAKE_PLATFORM_ALLOCATOR("log");
	state->log = log_manager::make(&state->log_a);
	state->dbg.console.setup_log(&state->log);

	platform_file log_all_file;
	CHECKED(create_file, &log_all_file, "log_all.html"_, platform_file_open_op::cleared);
	state->log.add_file(log_all_file, log_level::alloc, log_out_type::html);
	state->log.add_stdout(log_level::debug);

	LOG_INFO("Beginning startup..."_);
	LOG_PUSH_CONTEXT_L("");

	LOG_INFO("Starting logger..."_);
	state->log.start();

	LOG_INFO("Setting up events..."_);
	state->evt_a = MAKE_PLATFORM_ALLOCATOR("event");
	state->evt = evt_manager::make(&state->evt_a);
	state->evt.start();

	LOG_INFO("Creating window..."_);
	_memcpy("Exile", state->window.settings.c_title, 6);
	CHECKED(create_window, &state->window);
	state->dbg.store.add_var("window/settings"_, &state->window.settings);
	state->dbg.store.add_ele("window/apply"_, FPTR(dbg_reup_window), state);

	LOG_INFO("Setting up OpenGL..."_);
	state->ogl_a = MAKE_PLATFORM_ALLOCATOR("ogl");
	state->ogl = ogl_manager::make(&state->window, &state->ogl_a);

	LOG_INFO("Setting up IMGUI..."_);
	state->imgui_a = MAKE_PLATFORM_ALLOCATOR("imgui");
	state->imgui = imgui_manager::make(&state->window, &state->imgui_a);

	LOG_INFO("Setting up game..."_);
	state->game_state = start_up_game(state);

	LOG_INFO("Done with startup!"_);
	LOG_POP_CONTEXT();

	END_FRAME();

	state->running = true;
	return state;
}

EXPORT bool main_loop(engine* state) {

	BEGIN_FRAME();

	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear((GLbitfield)gl_clear::color_buffer_bit | (GLbitfield)gl_clear::depth_buffer_bit);

	state->evt.run_events(state); 
		
	state->imgui.begin_frame(&state->window);
	
	run_game(state->game_state);

	state->dbg.UI(&state->window);

	state->imgui.end_frame();

	RESET_ARENA(&this_thread_data.scratch_arena);

	CHECKED(swap_buffers, &state->window);

#ifndef RELEASE
	state->ogl.try_reload_programs();
#endif

	if(state->apply_window_settings) {
		if(global_api->apply_window_settings(&state->window)) {

			state->imgui.gl_destroy();
			state->ogl.gl_begin_reload();

			global_api->recreate_window(&state->window);
		
			state->ogl.gl_end_reload();
			state->imgui.gl_load();

			gl_reload_game(state->game_state);
		}
		state->apply_window_settings = false;
	}

	END_FRAME();

	return state->running;
}

EXPORT void shut_down(engine* state) { 

	BEGIN_FRAME();
	LOG_INFO("Beginning shutdown..."_);

	LOG_DEBUG("Destroying game..."_);
	shut_down_game(state->game_state);

	LOG_DEBUG("Destroying IMGUI"_);
	state->imgui.destroy();
	
	LOG_DEBUG("Destroying OpenGL"_);
	state->ogl.destroy();

	LOG_DEBUG("Destroying window"_);
	CHECKED(destroy_window, &state->window);

	LOG_DEBUG("Destroying events"_);
	state->evt.destroy();

	LOG_DEBUG("Destroying debug system"_);
	state->dbg.console.shutdown_log(&state->log);	
	state->dbg.destroy();

	LOG_DEBUG("Done with shutdown!"_);

	state->log.stop();
	state->log.destroy();
	state->dbg.destroy_prof();

	cleanup_fptrs();
	end_thread();

	global_log = null;
	global_dbg = null;
	global_func = null;
	global_api->heap_free(state);
	global_api = null;
}

EXPORT void on_reload(platform_api* api, engine* state) { 

	global_api = api;
	global_log = &state->log;
	global_dbg = &state->dbg;
	global_func = &state->func_state;

	state->func_state.reload_all();

	begin_thread("main"_, &state->basic_a);

	state->ogl.load_global_funcs();
	state->imgui.reload();

	state->evt.start(); // NOTE(max): needed to reset platform function pointer pointing into the game DLL
	state->log.start();

	LOG_INFO("End reloading game code"_);

	reload_game(state, state->game_state);
}

EXPORT void on_unload(engine* state) {
	
	LOG_INFO("Begin reloading game code"_);

	unload_game(state, state->game_state);

	state->log.stop();
	
	end_thread();
}

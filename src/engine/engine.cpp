
EXPORT engine* start_up(platform_api* api) { 

	engine* state = new(api->heap_alloc(sizeof(engine))) engine;
	state->func_state.this_dll = api->your_dll;
	state->platform = api;

	global_api  = api;
	global_log  = &state->log;
	global_dbg  = &state->dbg;
	global_func = &state->func_state;

	setup_fptrs();

	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR("default");
	state->suppressed_platform_allocator = MAKE_PLATFORM_ALLOCATOR("default/suppress");
	state->suppressed_platform_allocator.suppress_messages = true;

	begin_thread("main"_, &state->suppressed_platform_allocator);

	state->dbg_a = MAKE_PLATFORM_ALLOCATOR("dbg");
	state->dbg_a.suppress_messages = true;
	state->dbg = dbg_manager::make(&state->dbg_a);
	state->dbg.register_thread(180);

	BEGIN_FRAME();

	state->log_a = MAKE_PLATFORM_ALLOCATOR("log");
	state->log_a.suppress_messages = true;
	state->log = log_manager::make(&state->log_a);
	state->dbg.setup_log(&state->log);

	platform_file stdout_file, log_all_file;
	CHECKED(get_stdout_as_file, &stdout_file);
	CHECKED(create_file, &log_all_file, "log_all.html"_, platform_file_open_op::create);
	state->log.add_file(log_all_file, log_level::alloc, log_out_type::html);
	state->log.add_file(stdout_file, log_level::debug, log_out_type::plaintext, true);

	LOG_INFO("Beginning startup...");
	LOG_PUSH_CONTEXT_L("");

	LOG_INFO("Starting logger...");
	state->log.start();

	LOG_INFO("Setting up events...");
	state->evt_a = MAKE_PLATFORM_ALLOCATOR("event");
	state->evt = evt_manager::make(&state->evt_a);
	state->evt.start();

	LOG_INFO("Allocating transient store...");
	state->transient_arena = MAKE_ARENA("transient"_, MEGABYTES(8), &state->default_platform_allocator, false);

	LOG_INFO("Setting up default assets...");
	state->default_store = asset_store::make(&state->default_platform_allocator);
	state->default_store.load("assets/engine.asset"_);

	LOG_INFO("Creating window...");
	CHECKED(create_window, &state->window, "Exile"_, 1280, 720);

	LOG_INFO("Setting up OpenGL...");
	state->ogl_a = MAKE_PLATFORM_ALLOCATOR("ogl");
	state->ogl = ogl_manager::make(&state->window, &state->ogl_a);

	LOG_INFO("Setting up GUI...");
	state->gui_a = MAKE_PLATFORM_ALLOCATOR("gui");
	state->gui = gui_manager::make(&state->gui_a, &state->window);
	state->gui.add_font(&state->ogl, "guimono"_, &state->default_store, true);
	state->gui.register_events(&state->evt);

	LOG_INFO("Setting up game...");
	state->game_state = start_up_game(state);

	LOG_INFO("Done with startup!");
	LOG_POP_CONTEXT();

	END_FRAME();

	state->running = true;
	return state;
}

EXPORT bool main_loop(engine* state) {

	BEGIN_FRAME();

	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear((GLbitfield)gl_clear::color_buffer_bit | (GLbitfield)gl_clear::depth_buffer_bit);

	PUSH_ALLOC(&state->transient_arena) {

		state->evt.run_events(state); 
		
		state->gui.begin_frame();
		
		run_game(state->game_state);

		state->dbg.UI();

		state->gui.end_frame(&state->window, &state->ogl);

	} POP_ALLOC();
	RESET_ARENA(&state->transient_arena);

	CHECKED(swap_buffers, &state->window);

#ifndef RELEASE
	state->ogl.try_reload_programs();
	if(state->default_store.try_reload()) {
		state->gui.reload_fonts(&state->ogl);
	}
#endif

	END_FRAME();

	return state->running;
}

EXPORT void shut_down(engine* state) { 

	BEGIN_FRAME();
	LOG_INFO("Beginning shutdown...");

	LOG_DEBUG("Destroying game...");
	shut_down_game(state->game_state);

	LOG_DEBUG("Destroying asset system");
	state->default_store.destroy();

	LOG_DEBUG("Destroying transient store");
	DESTROY_ARENA(&state->transient_arena);

	LOG_DEBUG("Destroying GUI");
	state->gui.unregister_events(&state->evt);
	state->gui.destroy();
	
	LOG_DEBUG("Destroying OpenGL");
	state->ogl.destroy();

	LOG_DEBUG("Destroying window");
	CHECKED(destroy_window, &state->window);

	LOG_DEBUG("Destroying events");
	state->evt.destroy();

	LOG_DEBUG("Destroying debug system");
	state->dbg.shutdown_log(&state->log);	
	END_FRAME();
	state->dbg.destroy();
	
	LOG_DEBUG("Done with shutdown!");

	state->log.stop();
	state->log.destroy();

	cleanup_fptrs();
	end_thread();

	state->dbg_a.destroy();
	state->log_a.destroy();
	state->ogl_a.destroy();
	state->gui_a.destroy();
	state->evt_a.destroy();
	state->suppressed_platform_allocator.destroy();
	state->default_platform_allocator.destroy();

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
	state->gui.style = _gui_style();

	state->func_state.reload_all();

	begin_thread("main"_, &state->suppressed_platform_allocator);

	state->ogl.load_global_funcs();

	state->evt.start(); // NOTE(max): needed to reset platform function pointer pointing into the game DLL
	state->log.start();

	LOG_INFO("End reloading game code");

	reload_game(state, state->game_state);
}

EXPORT void on_unload(engine* state) {
	
	LOG_INFO("Begin reloading game code");

	unload_game(state, state->game_state);

	state->log.stop();
	
	end_thread();
}

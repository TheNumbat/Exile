
#include "everything.h"

extern "C" game_state* start_up(platform_api* api) { 

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));
	global_api = api;
	global_log = &state->log;
	global_dbg = &state->dbg;

	state->default_platform_allocator = np_make_platform_allocator(np_string_literal("default"), CONTEXT);
	state->suppressed_platform_allocator = np_make_platform_allocator(np_string_literal("default/suppress"), CONTEXT);
	state->suppressed_platform_allocator.suppress_messages = true;

	begin_thread(np_string_literal("main"), &state->suppressed_platform_allocator);

	state->log_a = MAKE_PLATFORM_ALLOCATOR("log");
	state->log_a.suppress_messages = true;
	state->log = log_manager::make(&state->log_a);

	platform_file stdout_file, log_all_file;
	api->platform_get_stdout_as_file(&stdout_file);
	api->platform_create_file(&log_all_file, string_literal("log_all.html"), platform_file_open_op::create);
	state->log.add_file(log_all_file, log_level::alloc, log_out_type::html);
	state->log.add_file(stdout_file, log_level::info);

	LOG_INFO("Beginning startup...");
	LOG_PUSH_CONTEXT_L("");

	LOG_INFO("Starting debug system...");
	state->dbg_a = MAKE_PLATFORM_ALLOCATOR("dbg");
	state->dbg_a.suppress_messages = true;
	state->dbg = dbg_manager::make(&state->log, &state->dbg_a);

	LOG_INFO("Starting logger...");
	state->log.start();

	LOG_INFO("Setting up events...");
	state->evt_a = MAKE_PLATFORM_ALLOCATOR("event");
	state->evt = evt_manager::make(&state->evt_a);
	state->evt.start();

	LOG_INFO("Starting thread pool");
	state->thread_pool_a = MAKE_PLATFORM_ALLOCATOR("threadpool");
	state->thread_pool_a.suppress_messages = true;
	state->thread_pool = threadpool::make(&state->thread_pool_a);
	state->thread_pool.start_all();

	LOG_INFO("Allocating transient store...");
	state->transient_arena = MAKE_ARENA("transient", MEGABYTES(8), &state->default_platform_allocator, false);

	job_id assets = state->thread_pool.queue_job([](void* s) -> job_callback {
		game_state* state = (game_state*)s;
		LOG_INFO("Setting up asset system");
		state->default_store_a = MAKE_PLATFORM_ALLOCATOR("asset");
		state->default_store = asset_store::make(&state->default_store_a);
		state->default_store.load(string_literal("assets/assets.asset"));
		return null;
	}, state);

	LOG_INFO("Creating window");
	platform_error err = api->platform_create_window(&state->window, string_literal("CaveGame"), 1280, 720);

	if (!err.good) {
		LOG_FATAL_F("Failed to create window, error: %", err.error);
	}

	LOG_INFO("Setting up OpenGL");
	ogl_load_global_funcs();
	state->ogl_a = MAKE_PLATFORM_ALLOCATOR("ogl");
	state->ogl = ogl_manager::make(&state->ogl_a);

	state->thread_pool.wait_job(assets);

	LOG_INFO("Setting up GUI");
	state->gui_a = MAKE_PLATFORM_ALLOCATOR("gui");
	state->gui = gui_manager::make(&state->ogl, &state->gui_a);
	state->gui.add_font(&state->ogl, string_literal("gui14"), &state->default_store);
	state->gui.add_font(&state->ogl, string_literal("gui24"), &state->default_store);
	state->gui.add_font(&state->ogl, string_literal("gui40"), &state->default_store);
	state->gui.add_font(&state->ogl, string_literal("guimono"), &state->default_store, true);

	LOG_INFO("Done with startup!");
	LOG_POP_CONTEXT();

	// LOG_INFO_F("%", state); // Don't do this anymore, it's 409 thousand characters and will only grow

	state->dbg.really_running = true;
	state->running = true;
	return state;
}

extern "C" bool main_loop(game_state* state) { 

	glUseProgram(0); // why tho?? https://twitter.com/fohx/status/619887799462985729?lang=en
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	PUSH_ALLOC(&state->transient_arena) {

		gui_input_state input = run_events(state); 
		
		state->gui.begin_frame(input);

		state->dbg.render_debug_gui(&state->window);

		state->gui.end_frame(&state->window, &state->ogl);

	} POP_ALLOC();
	RESET_ARENA(&state->transient_arena);

	global_api->platform_swap_buffers(&state->window);

#ifdef _DEBUG
	state->ogl.try_reload_programs();
	if(state->default_store.try_reload()) {
		state->gui.reload_fonts(&state->ogl);
	}
#endif

	state->dbg.collate();

	return state->running;
}

extern "C" void shut_down(game_state* state) { PROF

	LOG_INFO("Beginning shutdown...");

	LOG_DEBUG("Destroying GUI");
	state->gui.destroy();

	LOG_DEBUG("Destroying OpenGL")
	state->ogl.destroy();

	LOG_DEBUG("Destroying asset system");
	state->default_store.destroy();

	LOG_DEBUG("Destroying thread pool");
	state->thread_pool.stop_all();
	state->thread_pool.destroy();

	LOG_DEBUG("Destroying window");
	platform_error err = global_api->platform_destroy_window(&state->window);
	if(!err.good) {
		LOG_ERR_F("Failed to destroy window, error: %", err.error);	
	}

	LOG_DEBUG("Destroying events");
	state->evt.destroy();

	LOG_DEBUG("Destroying transient store");
	DESTROY_ARENA(&state->transient_arena);

	LOG_DEBUG("Done with shutdown!");

	state->log.stop();
	state->dbg.destroy();
	state->log.destroy();
	
	end_thread();

	global_api->platform_heap_free(state);
}

extern "C" void on_reload(platform_api* api, game_state* state) { 

	global_api = api;
	global_log = &state->log;
	global_dbg = &state->dbg;

	begin_thread(np_string_literal("main"), &state->suppressed_platform_allocator);

	ogl_load_global_funcs();

	state->evt.start(); // NOTE(max): needed to reset platform function pointer pointing into the game DLL
	state->log.start();
	state->thread_pool.start_all();

	LOG_INFO("End reloading game code");
}

extern "C" void on_unload(game_state* state) {
	
	LOG_INFO("Begin reloading game code");

	state->thread_pool.stop_all();
	state->log.stop();

	end_thread();
}

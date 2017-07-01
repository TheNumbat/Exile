
#include <iostream>
#include "everything.h"

template<typename T>
void print_type(T& val, _type_info* info = NULL) {
	if(info == NULL) {
		info = TYPEINFO(T);
	}
	switch(info->type_type) {
	case Type::_int:
		std::cout << *(i32*)&val;
	break;
	case Type::_float:
		std::cout << *(float*)&val;
	break;
	case Type::_bool:
		std::cout << *(bool*)&val ? "true" : "false";
	break;
	case Type::_ptr:
	break;
	case Type::_func:
	break;
	case Type::_struct:
		std::cout << "{ ";		
		for(u32 j = 0; j < info->_struct.member_count; j++) {
			std::cout << info->_struct.member_names[j].c_str << " : ";

			_type_info* member = info->_struct.member_types[j];
			u8* place = (u8*)&val + info->_struct.member_offsets[j];

			print_type(*place, member);
			std::cout << " ";
		}
		std::cout << "}";
	break;
	case Type::_enum:
	break;
	case Type::_string:
		std::cout << ((string*)&val)->c_str;
	break;
	}
}
void cool_printf(string fmt) {
	std::cout << fmt.c_str;
}
template<typename T, typename... Targs>
void cool_printf(string fmt, T value, Targs... Fargs) {

	for(u32 i = 0; i < fmt.len - 1; i++) {
		if(fmt.c_str[i] == '%') {
			if(fmt.c_str[i + 1] == '%') {
				std::cout << '%';
			} else {
				print_type(value);
				cool_printf(string_from_c_str(fmt.c_str + i + 1), Fargs...);
				return;
			}
		} else {
			std::cout << fmt.c_str[i];
		}
	}
}

extern "C" game_state* start_up(platform_api* api) { FUNC 

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));
	state->api = api;
	global_state = state;

	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR("default");
	state->suppressed_platform_allocator = MAKE_PLATFORM_ALLOCATOR("default/suppress");
	state->suppressed_platform_allocator.suppress_messages = true;

	begin_thread(string_literal("main"), &state->suppressed_platform_allocator);

	state->api = api;
	state->log_a = MAKE_PLATFORM_ALLOCATOR("log");
	state->log_a.suppress_messages = true;
	state->log = make_logger(&state->log_a);

	make_type_table(&state->default_platform_allocator);

	platform_file stdout_file, log_all_file;
	api->platform_get_stdout_as_file(&stdout_file);
	api->platform_create_file(&log_all_file, string_literal("log_all.txt"), platform_file_open_op::create);
	logger_add_file(&state->log, log_all_file, log_level::alloc);
	logger_add_file(&state->log, stdout_file, log_level::info);

	LOG_DEBUG("Beginning startup...");
	LOG_PUSH_CONTEXT_L("");

	LOG_DEBUG("Allocating transient store...");
	state->transient_arena = MAKE_ARENA("transient", MEGABYTES(16), &state->default_platform_allocator, false);

	LOG_DEBUG("Starting debug system");
	state->dbg_a = MAKE_PLATFORM_ALLOCATOR("dbg");
	state->dbg_a.suppress_messages = true;
	state->dbg = make_dbg_manager(&state->dbg_a);

	LOG_DEBUG("Starting logger");
	logger_start(&state->log);

	LOG_DEBUG("Starting thread pool");
	state->thread_pool_a = MAKE_PLATFORM_ALLOCATOR("threadpool");
	state->thread_pool = make_threadpool(&state->thread_pool_a);
	threadpool_start_all(&state->thread_pool);

	LOG_DEBUG("Setting up events");
	state->evt_a = MAKE_PLATFORM_ALLOCATOR("event");
	state->evt = make_evt_manager(&state->evt_a);
	start_evt_manger(&state->evt);

	LOG_DEBUG("Setting up asset system");
	state->default_store_a = MAKE_PLATFORM_ALLOCATOR("asset");
	state->default_store = make_asset_store(&state->default_store_a);
	load_asset_store(&state->default_store, string_literal("assets/assets.asset"));

	LOG_DEBUG("Creating window");
	platform_error err = api->platform_create_window(&state->window, string_literal("CaveGame"), 1280, 720);
	state->window_w = 1280;
	state->window_h = 720;

	if(!err.good) {
		LOG_FATAL_F("Failed to create window, error: %i", err.error);
		api->platform_heap_free(state);
		return NULL;
	}

	LOG_DEBUG("Setting up OpenGL");
	ogl_load_global_funcs();
	state->ogl_a = MAKE_PLATFORM_ALLOCATOR("ogl");
	state->ogl = make_opengl(&state->ogl_a);

	LOG_DEBUG("Setting up GUI");
	state->gui_a = MAKE_PLATFORM_ALLOCATOR("gui");
	state->gui = make_gui(&state->ogl, &state->gui_a);
	gui_add_font(&state->ogl, &state->gui, string_literal("gui14"), &state->default_store);
	gui_add_font(&state->ogl, &state->gui, string_literal("gui24"), &state->default_store);
	gui_add_font(&state->ogl, &state->gui, string_literal("gui40"), &state->default_store);
	gui_add_font(&state->ogl, &state->gui, string_literal("guimono"), &state->default_store, true);

	LOG_INFO("Done with startup!");
	LOG_POP_CONTEXT();

	// testing
	cool_printf(string_literal("owo what's this? % %"), string_literal("LUL"), gui_style);
	std::cout << std::endl;

	state->running = true;
	return state;
}

extern "C" bool main_loop(game_state* state) { FUNC

	glUseProgram(0); // why tho?? https://twitter.com/fohx/status/619887799462985729?lang=en
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	PUSH_ALLOC(&state->transient_arena) {

		gui_input_state input = run_events(state); 

		gui_begin_frame(&state->gui, input);

		render_debug_gui(state);

		gui_end_frame(&state->ogl);

	} POP_ALLOC();
	RESET_ARENA(&state->transient_arena);

	state->api->platform_swap_buffers(&state->window);

#ifdef _DEBUG
	ogl_try_reload_programs(&state->ogl);
	if(try_reload_asset_store(&state->default_store)) {
		gui_reload_fonts(&state->ogl, &state->gui);
	}
#endif

	return state->running;
}

extern "C" void shut_down(platform_api* api, game_state* state) { FUNC

	LOG_INFO("Beginning shutdown...");

	LOG_DEBUG("Destroying GUI");
	destroy_gui(&state->gui);

	LOG_DEBUG("Destroying OpenGL")
	destroy_opengl(&state->ogl);

	LOG_DEBUG("Destroying asset system");
	destroy_asset_store(&state->default_store);

	LOG_DEBUG("Destroying thread pool");
	threadpool_stop_all(&state->thread_pool);
	destroy_threadpool(&state->thread_pool);

	LOG_DEBUG("Destroying window");
	platform_error err = api->platform_destroy_window(&state->window);
	if(!err.good) {
		LOG_ERR_F("Failed to destroy window, error: %i", err.error);	
	}

	LOG_DEBUG("Destroying events");
	destroy_evt_manager(&state->evt);

	LOG_DEBUG("Destroying transient store");
	DESTROY_ARENA(&state->transient_arena);

	LOG_DEBUG("Done with shutdown!");

	destroy_type_table();
	logger_stop(&state->log);
	destroy_logger(&state->log);
	destroy_dbg_manager(&state->dbg);
	
	end_thread();

	api->platform_heap_free(state);
}

extern "C" void on_reload(game_state* state) { FUNC

	global_state = state;
	ogl_load_global_funcs();

	gui_style = _gui_style();

	begin_thread(string_literal("main"), &state->suppressed_platform_allocator);
	logger_start(&state->log);
	threadpool_start_all(&state->thread_pool);
	make_type_table(&state->default_platform_allocator);

	LOG_INFO("End reloading game code");
}

extern "C" void on_unload(game_state* state) { FUNC
	
	LOG_INFO("Begin reloading game code");

	destroy_type_table();
	threadpool_stop_all(&state->thread_pool);
	logger_stop(&state->log);

	end_thread();
}

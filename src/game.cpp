
#include "everything.h"

extern "C" game_state* start_up(platform_api* api) {

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));
	global_state = state;

	state->api = api;
	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR("default");
	state->suppressed_platform_allocator = state->default_platform_allocator;
	state->suppressed_platform_allocator.suppress_messages = true;

	api->platform_create_mutex(&state->alloc_contexts_mutex, false);
	state->alloc_contexts = make_map<platform_thread_id,stack<allocator*>>(api->platform_get_num_cpus() + 2, &state->suppressed_platform_allocator);

	map_insert(&state->alloc_contexts, api->platform_this_thread_id(), make_stack<allocator*>(0, &state->suppressed_platform_allocator));

	state->log = make_logger(&state->suppressed_platform_allocator);

	platform_file stdout_file, log_all_file;
	api->platform_get_stdout_as_file(&stdout_file);
	api->platform_create_file(&log_all_file, string_literal("log_all.txt"), open_file_create);
	logger_add_file(&state->log, log_all_file, log_alloc);
	logger_add_file(&state->log, stdout_file, log_info);

	LOG_INIT_THREAD(string_literal("main"));

	logger_start(&state->log);

	LOG_DEBUG("Beginning startup...");
	LOG_PUSH_CONTEXT_L("startup");

	LOG_DEBUG("Starting thread pool");
	LOG_PUSH_CONTEXT_L("threadpool");
	state->thread_pool = make_threadpool(&state->default_platform_allocator);
	threadpool_start_all(&state->thread_pool);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up events");
	LOG_PUSH_CONTEXT_L("events");
	state->events = make_event_manager(&state->default_platform_allocator);
	start_event_manger(&state->events);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up asset system");
	LOG_PUSH_CONTEXT_L("assets");
	state->test_store = make_asset_store(&state->default_platform_allocator);
	load_asset_store(&state->test_store, string_literal("assets/cats.asset"));
	LOG_POP_CONTEXT();

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
	LOG_PUSH_CONTEXT_L("ogl");
	ogl_load_global_funcs();
	state->ogl = make_opengl(&state->default_platform_allocator);
	LOG_POP_CONTEXT();

	LOG_INFO("Done with startup!");
	LOG_POP_CONTEXT();
	
	state->texture = ogl_add_texture(&state->ogl, &state->test_store, string_literal("cat"), wrap_clamp_border);
	state->context = ogl_add_draw_context(&state->ogl, &ogl_mesh_2d_attribs);
	state->shader = ogl_add_program(&state->ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);

	return state;
}

extern "C" bool main_loop(game_state* state) {

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_command_list rcl = make_command_list(&state->default_platform_allocator);
	mesh_2d mesh = make_mesh_2d(&state->default_platform_allocator);
	{
		vector_push(&mesh.verticies, V2f(0, 0));
		vector_push(&mesh.verticies, V2f(0, state->window_h));
		vector_push(&mesh.verticies, V2f(state->window_w, 0));
		vector_push(&mesh.verticies, V2f(0, state->window_h));
		vector_push(&mesh.verticies, V2f(state->window_w, 0));
		vector_push(&mesh.verticies, V2f(state->window_w, state->window_h));
	}
	{
		vector_push(&mesh.texCoords, V2(0.0f, 0.0f));
		vector_push(&mesh.texCoords, V2(0.0f, 1.0f));
		vector_push(&mesh.texCoords, V2(1.0f, 0.0f));
		vector_push(&mesh.texCoords, V2(0.0f, 1.0f));
		vector_push(&mesh.texCoords, V2(1.0f, 0.0f));
		vector_push(&mesh.texCoords, V2(1.0f, 1.0f));
	}
	{
		vector_push(&mesh.colors, V4(1.0f, 1.0f, 1.0f, 1.0f));
		vector_push(&mesh.colors, V4(1.0f, 1.0f, 1.0f, 1.0f));
		vector_push(&mesh.colors, V4(1.0f, 1.0f, 1.0f, 1.0f));
		vector_push(&mesh.colors, V4(1.0f, 1.0f, 1.0f, 1.0f));
		vector_push(&mesh.colors, V4(1.0f, 1.0f, 1.0f, 1.0f));
		vector_push(&mesh.colors, V4(1.0f, 1.0f, 1.0f, 1.0f));	
	}
	render_command cmd = make_render_command(render_mesh_2d, &mesh);
	cmd.shader = state->shader;
	cmd.texture = state->texture;
	cmd.context = state->context;

	render_add_command(&rcl, cmd);
	rcl.proj = ortho(0, (f32)state->window_w, 0, (f32)state->window_h, -1, 1);

	ogl_render_command_list(&state->ogl, &rcl);

	destroy_mesh_2d(&mesh);
	destroy_command_list(&rcl);
	state->api->platform_swap_buffers(&state->window);
	
	return run_events(&state->events);
}

extern "C" void shut_down(platform_api* api, game_state* state) {

	LOG_INFO("Beginning shutdown...");
	LOG_PUSH_CONTEXT_L("shutdown");

	LOG_DEBUG("Destroying OpenGL")
	destroy_opengl(&state->ogl);

	LOG_DEBUG("Destroying asset system");
	destroy_asset_store(&state->test_store);

	LOG_DEBUG("Destroying thread pool");
	threadpool_stop_all(&state->thread_pool);
	destroy_threadpool(&state->thread_pool);

	LOG_DEBUG("Destroying window");
	platform_error err = api->platform_destroy_window(&state->window);
	if(!err.good) {
		LOG_ERR_F("Failed to destroy window, error: %i", err.error);	
	}

	LOG_DEBUG("Destroying events");
	destroy_event_manager(&state->events);

	LOG_DEBUG("Done with shutdown!");

	// not actually quite done but we can't log anything after this
	LOG_END_THREAD();
	logger_stop(&state->log);
	destroy_logger(&state->log);

	destroy_stack(map_get(&state->alloc_contexts, state->api->platform_this_thread_id()));
	map_erase(&state->alloc_contexts, state->api->platform_this_thread_id());
	destroy_map(&state->alloc_contexts);
	api->platform_destroy_mutex(&state->alloc_contexts_mutex);

	api->platform_heap_free(state);
}

extern "C" void on_reload(game_state* state) {

	global_state = state;
	ogl_load_global_funcs();

	logger_start(&state->log);
	threadpool_start_all(&state->thread_pool);

	LOG_INFO("End reloading game code");
}

extern "C" void on_unload(game_state* state) {
	
	LOG_INFO("Begin reloading game code");

	threadpool_stop_all(&state->thread_pool);
	logger_stop(&state->log);
}

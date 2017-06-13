
#pragma push_macro("gui_window")
#pragma push_macro("gui_text_line")
#pragma push_macro("gui_text_line_f")
#undef gui_window
#undef gui_text_line
#undef gui_text_line_f

gui_manager make_gui(allocator* alloc, opengl* ogl, asset* font) {

	gui_manager ret;

	ret.alloc = alloc;
	ret.font = font;
	ret.font_point = font->font.point;

	ret.ogl.context = ogl_add_draw_context(ogl, &ogl_mesh_2d_attribs);
	ret.ogl.texture = ogl_add_texture_from_font(ogl, font);
	ret.ogl.shader 	= ogl_add_program(ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);
	ret.windows = make_vector<_gui_window>(16, alloc);

	ret.style.win_margin = V2(10.0f, ret.font_point + 5.0f);;

	return ret;
}

void destroy_gui(gui_manager* gui) {

	FORVEC(gui->windows,
		destroy_vector(&it->widgets);
	)

	destroy_vector(&gui->windows);
	destroy_mesh_2d(&gui->mesh);

	gui->ogl.context = 0;
	gui->ogl.texture = 0;
	gui->ogl.shader  = 0;
}

void gui_begin_frame(gui_manager* gui) {

	gui->mesh = make_mesh_2d();
}

void gui_end_frame_render(opengl* ogl, gui_manager* gui) {

	FORVEC(gui->windows,
		push_windowshape(gui, it);

		v2 vtitle = add(it->rect.xy, V2(15.0f, gui->font_point));
		mesh_push_text_line(&gui->mesh, gui->font, it->title, vtitle, gui->font_point, V4b(255, 255, 255, 255));

		for(u32 i = 0; i < it->widgets.size; i++) {

			gui_widget* w = vector_get(&it->widgets, i);

			switch(w->type) {
			case widget_text: {
				
				mesh_push_text_line(&gui->mesh, gui->font, w->text.text, add(it->rect.xy, w->pos), w->text.point, w->text.c);

			} break;
			}
		}
	)

	render_command_list rcl = make_command_list();
	render_command cmd = make_render_command(render_mesh_2d, &gui->mesh);
	cmd.shader  = gui->ogl.shader;
	cmd.texture = gui->ogl.texture;
	cmd.context = gui->ogl.context;

	render_add_command(&rcl, cmd);
	rcl.proj = ortho(0, (f32)global_state->window_w, (f32)global_state->window_h, 0, -1, 1);

	ogl_send_mesh_2d(ogl, &gui->mesh, gui->ogl.context);
	ogl_render_command_list(ogl, &rcl);
	destroy_command_list(&rcl);

	destroy_mesh_2d(&gui->mesh);
}

void push_windowshape(gui_manager* gui, _gui_window* win) {

	u32 idx = gui->mesh.verticies.size;
	r2 r = win->rect;
	f32 pt = gui->font_point + 5.0f;

	vector_push(&gui->mesh.verticies, V2(r.x, r.y + pt));
	vector_push(&gui->mesh.verticies, V2(r.x, r.y + r.h));
	vector_push(&gui->mesh.verticies, V2(r.x + r.w - 10.0f, r.y + r.h));
	vector_push(&gui->mesh.verticies, V2(r.x + r.w, r.y + r.h - 10.0f));
	vector_push(&gui->mesh.verticies, V2(r.x + r.w, r.y + pt));

	vector_push(&gui->mesh.verticies, V2(r.x + r.w - 10.0f, r.y));
	vector_push(&gui->mesh.verticies, V2(r.x + 10.0f, r.y));
	vector_push(&gui->mesh.verticies, V2(r.x, r.y + pt));
	vector_push(&gui->mesh.verticies, V2(r.x + r.w, r.y + pt));

	vector_push(&gui->mesh.verticies, V2(r.x + r.w - 15, r.y + (pt / 2) - 5));
	vector_push(&gui->mesh.verticies, V2(r.x + r.w - 20, r.y + (pt / 2) + 5));
	vector_push(&gui->mesh.verticies, V2(r.x + r.w - 10, r.y + (pt / 2) + 5));

	FOR(12) vector_push(&gui->mesh.texCoords, V3f(0,0,0));

	colorf cf = color_to_f(V4b(gui->style.win_back, win->opacity * 255.0f));
	colorf topf = color_to_f(V4b(gui->style.win_top, 255));
	colorf win_closef = color_to_f(V4b(gui->style.win_close, 255));
	FOR(5) vector_push(&gui->mesh.colors, cf);
	FOR(4) vector_push(&gui->mesh.colors, topf);
	FOR(3) vector_push(&gui->mesh.colors, win_closef);

	vector_push(&gui->mesh.elements, V3u(idx, idx + 1, idx + 2));
	vector_push(&gui->mesh.elements, V3u(idx, idx + 2, idx + 3));
	vector_push(&gui->mesh.elements, V3u(idx, idx + 3, idx + 4));

	vector_push(&gui->mesh.elements, V3u(idx + 7, idx + 5, idx + 6));
	vector_push(&gui->mesh.elements, V3u(idx + 8, idx + 5, idx + 7));

	vector_push(&gui->mesh.elements, V3u(idx + 9, idx + 10, idx + 11));
}

void gui_window(u32 id, gui_manager* gui, string title, r2 rect, f32 opacity) {

	bool found = false;
	FORVEC(gui->windows,
		if(it->id == id) {
			gui->current = __i;
			found = true;
		}
	)

	_gui_window* current = vector_get(&gui->windows, gui->current);
	
	if(!found) {
		_gui_window win;
		gui->current = gui->windows.size;
		vector_push(&gui->windows, win);
		current = vector_back(&gui->windows);

		current->id = id;
		current->widgets = make_vector<gui_widget>(16, gui->alloc);
		current->margin = gui->style.win_margin; 
	}

	// TODO(max): update these dynamically
	current->rect = rect;
	current->opacity = opacity;
	current->title = title;
	current->last_y = 0;
}

void gui_text_line_f(u32 id, gui_manager* gui, string fmt, f32 point, color c, ...) {

	string final;

	va_list args;
	va_start(args, c);
	PUSH_ALLOC(gui->alloc) {
		final = make_vstringf(fmt, args);
	} POP_ALLOC();
	va_end(args);

	gui_text_line(id, gui, final, point, c);
}

void gui_text_line(u32 id, gui_manager* gui, string str, f32 point, color c) {

	if (point == 0.0f) {
		point = gui->font_point;
	}

	_gui_window* current = vector_get(&gui->windows,gui->current);
	gui_widget* w = NULL;

	bool found = false;
	FORVEC(current->widgets,
		if(it->id == id) {
			w = it;
			found = true;
		}
	)
	if(!found) {
		gui_widget t;
		t.type = widget_text;
		t.id = id;
	
		vector_push(&current->widgets, t);

		w = vector_back(&current->widgets);
	}

	current->last_y += point;
	w->pos = V2(current->margin.x, current->last_y + current->margin.y);
	w->text.text = str;
	w->text.point = point;
	w->text.c = c;
}

#pragma pop_macro("gui_window")
#pragma pop_macro("gui_text_line")
#pragma pop_macro("gui_text_line_f")

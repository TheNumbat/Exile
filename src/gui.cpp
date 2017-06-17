
u32 guiid_map_hash(guiid id) {
	return id;
}

guiid id_hash(string name, guiid seed) {

	u32 hash = hash_string(name);
	return hash ^ seed;
}

gui_manager make_gui(asset* font, opengl* ogl, allocator* alloc) {

	gui_manager ret;

	ret.alloc = alloc;

	ret.ogl.context = ogl_add_draw_context(ogl, &ogl_mesh_2d_attribs);
	ret.ogl.texture = ogl_add_texture_from_font(ogl, font);
	ret.ogl.shader 	= ogl_add_program(ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);

	ret.window_state_data = make_map<guiid, gui_window_state>(32, alloc, &guiid_map_hash);
	ret.state_data = make_map<guiid, gui_state_data>(128, alloc, &guiid_map_hash);
	
	ret.font = font;
	ret.style.font = font->font.point;

	return ret;
}

void destroy_gui(gui_manager* gui) {

	FORMAP(gui->window_state_data,
		destroy_stack(&it->value.id_stack);
		destroy_mesh(&it->value.mesh);
	)

	destroy_map(&gui->window_state_data);
	destroy_map(&gui->state_data);
}

void gui_begin_frame(gui_manager* gui, gui_input_state input) {

	ggui = gui;
	gui->input = input;
}

void gui_end_frame(opengl* ogl) {

	if(!ggui->input.lclick && !ggui->input.rclick && !ggui->input.mclick && !ggui->input.ldbl) {
		ggui->active = 1;
	} else if(ggui->active == 1) {
		ggui->active = 0;
	}

	render_command_list rcl = make_command_list();
	FORMAP(ggui->window_state_data,

		render_command cmd = make_render_command(render_mesh_2d, &it->value.mesh);
		cmd.shader  = ggui->ogl.shader;
		cmd.texture = ggui->ogl.texture;
		cmd.context = ggui->ogl.context;
		render_add_command(&rcl, cmd);
	)

	rcl.proj = ortho(0, (f32)global_state->window_w, (f32)global_state->window_h, 0, -1, 1);
	ogl_render_command_list(ogl, &rcl);
	destroy_command_list(&rcl);

	FORMAP(ggui->window_state_data,
		clear_mesh(&it->value.mesh);
	)
}

bool gui_begin(string name, r2 first_size, f32 first_alpha, gui_window_flags flags) {

	guiid id = hash_string(name);

	gui_window_state* window = map_try_get(&ggui->window_state_data, id);

	if(!window) {

		gui_window_state ns;

		ns.rect = first_size;
		if(first_size.w == 0.0f || first_size.h == 0.0f) {
			ns.rect.wh = ggui->style.default_win_size;
		}

		if(first_alpha == 0.0f) {
			ns.opacity = ggui->style.default_win_a;
		} else {
			ns.opacity = first_alpha;
		}

		ns.mesh = make_mesh_2d(32, ggui->alloc);
		ns.id_stack = make_stack<guiid>(16, ggui->alloc);
		stack_push(&ns.id_stack, id);
		ns.flags = flags;

		window = map_insert(&ggui->window_state_data, id, ns);
	}

	ggui->current = window;
	push_windowhead(window);

	v2 title_pos = add(window->rect.xy, V2(15.0f, 0.0f));
	push_text(window, title_pos, name, ggui->style.font, V4b(ggui->style.win_title, 255));

	r2 top_rect = R2(window->rect.xy, V2(window->rect.w - 20.0f, ggui->style.font + ggui->style.title_padding));	
	if(!(window->flags & win_nohide)) {

		v2 carrot_pos = add(window->rect.xy, V2(window->rect.w - 20.0f, 5.0f));
		gui_carrot_toggle(string_literal("#CLOSE"), window->active, V4b(ggui->style.win_close, 255), carrot_pos, &window->active);

		if(inside(top_rect, ggui->input.mousepos)) {

			if(ggui->active == 1 && ggui->input.ldbl) {
				
				window->active = !window->active;
				ggui->active = id;
				window->resizing = false;
			}
		}
	}
	if(!(window->flags & win_nomove)) {

		if(inside(top_rect, ggui->input.mousepos)) {

			if(ggui->active == 1 && ggui->input.lclick) {

				ggui->active = id;
				window->move_click_offset = sub(ggui->input.mousepos, window->rect.xy);
				window->resizing = false;
			}
		}
	}
	if(!(window->flags & win_noresize)) {

		r2 resize_rect = R2(sub(add(window->rect.xy, window->rect.wh), V2f(15, 15)), V2f(15, 15));
		if(inside(resize_rect, ggui->input.mousepos)) {

			if(ggui->active == 1 && ggui->input.lclick) {

				ggui->active = id;
				window->resizing = true;
			}
		}
	}

	if(window->active) {
		push_windowbody(window);
	}

	if(ggui->active == id) {
		if(window->resizing) {

			v2 wh = sub(ggui->input.mousepos, window->rect.xy);
			if(wh.x < ggui->style.min_win_size.x) {
				wh.x = ggui->style.min_win_size.x;
			}
			if(wh.y < ggui->style.min_win_size.y) {
				wh.y = ggui->style.min_win_size.y;
			}
			window->rect = R2(window->rect.xy, wh);

		} else {

			window->rect = R2(sub(ggui->input.mousepos, window->move_click_offset), window->rect.wh);
		}
	}

	return false;
}

bool gui_carrot_toggle(string name, bool initial, color c, v2 pos, bool* toggleme) {

	guiid id = id_hash(name, *stack_top(&ggui->current->id_stack));

	gui_state_data* data = map_try_get(&ggui->state_data, id);

	if(!data) {

		gui_state_data nd;

		nd.b = initial;

		data = map_insert(&ggui->state_data, id, nd);
	}

	if(toggleme) {
		data->b = *toggleme;
	}

	if(inside(R2(pos, V2f(10, 10)), ggui->input.mousepos)) {

		if(ggui->active == 1 && (ggui->input.lclick || ggui->input.ldbl)) {

			data->b = !data->b;
			ggui->active = id;

			if(toggleme) {
				*toggleme = !*toggleme;
			}
		}
	}

	push_carrot(ggui->current, pos, data->b, c);

	return data->b;
}

void push_carrot(gui_window_state* win, v2 pos, bool active, color c) {

	u32 idx = win->mesh.verticies.size;

	if(active) {
		vector_push(&win->mesh.verticies, V2(pos.x, 	pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + 10, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + 5, pos.y + 10));
	} else {
		vector_push(&win->mesh.verticies, V2(pos.x, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x, 	 pos.y + 10));
		vector_push(&win->mesh.verticies, V2(pos.x + 10, pos.y + 5));
	}

	colorf cf = color_to_f(c);
	FOR(3) vector_push(&win->mesh.colors, cf);

	FOR(3) vector_push(&win->mesh.texCoords, V3(0.0f, 0.0f, 0.0f));

	vector_push(&win->mesh.elements, V3(idx, idx + 1, idx + 2));
}

void push_text(gui_window_state* win, v2 pos, string text, f32 point, color c) {

	mesh_push_text_line(&win->mesh, ggui->font, text, pos, point, c);
}

void push_windowhead(gui_window_state* win) {
	
	u32 idx = win->mesh.verticies.size;
	r2 r = win->rect;
	f32 pt = ggui->style.font + ggui->style.title_padding;

	vector_push(&win->mesh.verticies, V2(r.x + r.w - 10.0f, r.y));
	vector_push(&win->mesh.verticies, V2(r.x + 10.0f, r.y));
	vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
	vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

	FOR(4) vector_push(&win->mesh.texCoords, V3f(0,0,0));

	colorf topf = color_to_f(V4b(ggui->style.win_top, 255));
	FOR(4) vector_push(&win->mesh.colors, topf);

	vector_push(&win->mesh.elements, V3u(idx + 2, idx, idx + 1));
	vector_push(&win->mesh.elements, V3u(idx + 3, idx, idx + 2));
}

void push_windowbody(gui_window_state* win) {

	u32 idx = win->mesh.verticies.size;
	r2 r = win->rect;
	f32 pt = ggui->style.font + ggui->style.title_padding;

	vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
	vector_push(&win->mesh.verticies, V2(r.x, r.y + r.h));
	vector_push(&win->mesh.verticies, V2(r.x + r.w - 10.0f, r.y + r.h));
	vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + r.h - 10.0f));
	vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

	FOR(5) vector_push(&win->mesh.texCoords, V3f(0,0,0));

	colorf cf = color_to_f(V4b(ggui->style.win_back, win->opacity * 255.0f));

	FOR(5) vector_push(&win->mesh.colors, cf);

	vector_push(&win->mesh.elements, V3u(idx, idx + 1, idx + 2));
	vector_push(&win->mesh.elements, V3u(idx, idx + 2, idx + 3));
	vector_push(&win->mesh.elements, V3u(idx, idx + 3, idx + 4));
}
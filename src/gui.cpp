
u32 guiid_hash(guiid id) { FUNC

	u32 hash = hash_string(id.name);
	return hash ^ id.base;
}

bool operator==(guiid l, guiid r) { FUNC
	return l.base == r.base && l.name == r.name;
}

gui_manager make_gui(ogl_manager* ogl, allocator* alloc) { FUNC

	gui_manager ret;

	ret.alloc = alloc;
	ret.scratch = MAKE_ARENA("gui_scratch", 1024, alloc, false);

	ret.ogl.context = ogl_add_draw_context(ogl, &ogl_mesh_2d_attribs);
	ret.ogl.shader 	= ogl_add_program(ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);

	ret.window_state_data = make_map<guiid, gui_window_state>(32, alloc, &guiid_hash);
	ret.state_data = make_map<guiid, gui_state_data>(128, alloc, &guiid_hash);
	ret.fonts = make_vector<gui_font>(4, alloc);

	return ret;
}

void destroy_gui(gui_manager* gui) { FUNC

	FORMAP(gui->window_state_data,
		destroy_stack(&it->value.id_hash_stack);
		destroy_vector(&it->value.offset_stack);
		destroy_mesh(&it->value.mesh);
	)

	destroy_map(&gui->window_state_data);
	destroy_map(&gui->state_data);
	destroy_vector(&gui->fonts);

	DESTROY_ARENA(&gui->scratch);
}

void gui_reload_fonts(ogl_manager* ogl, gui_manager* gui) { FUNC

	FORVEC(gui->fonts,

		ogl_destroy_texture(ogl, it->texture);

		asset* font = get_asset(it->store, it->asset_name);

		it->font = font;
		it->texture = ogl_add_texture_from_font(ogl, font);
	)
}

void gui_add_font(ogl_manager* ogl, gui_manager* gui, string asset_name, asset_store* store, bool mono) { FUNC

	asset* font = get_asset(store, asset_name);

	if(vector_empty(&gui->fonts)) {
		gui_style.font = font->font.point;
	}

	gui_font f;
	
	f.asset_name = asset_name;
	f.store = store;
	f.font = font;
	f.mono = mono;
	f.texture = ogl_add_texture_from_font(ogl, font);

	vector_push(&gui->fonts, f);
}

gui_font* gui_select_best_font_scale(gui_window_state* win) { FUNC

	gui_font* f = NULL;

	f32 defl = gui_style.font * gui_style.gscale;
	f32 min_off = FLT_MAX;
	FORVEC(ggui->fonts,
		f32 off = absf(defl - it->font->font.point);
		if(off < min_off && it->mono == win->mono) {
			min_off = off;
			f = it;
		}
	)

	return f;
}

void gui_begin_frame(gui_manager* gui, gui_input_state input) { FUNC

	ggui = gui;
	gui->input = input;

	FORMAP(gui->window_state_data,
		it->value.font = gui_select_best_font_scale(&it->value);
	)
}

void gui_end_frame(ogl_manager* ogl) { FUNC

	if(!ggui->input.lclick && !ggui->input.rclick && !ggui->input.mclick && !ggui->input.ldbl) {
		if(ggui->active != gui_captured) {
			ggui->active_id = {};
			ggui->active = gui_none;
		}
	} else if(ggui->active == gui_none) {
		ggui->active = gui_invalid;
	}

	render_command_list rcl = make_command_list();
	FORMAP(ggui->window_state_data,

		render_command cmd = make_render_command(render_mesh_2d, &it->value.mesh, it->value.z);
		cmd.shader  = ggui->ogl.shader;
		cmd.texture = it->value.font->texture;
		cmd.context = ggui->ogl.context;
		render_add_command(&rcl, cmd);
	)

	rcl.proj = ortho(0, (f32)global_state->window_w, (f32)global_state->window_h, 0, -1, 1);
	sort_render_commands(&rcl);

	ogl_render_command_list(ogl, &rcl);
	destroy_command_list(&rcl);

	FORMAP(ggui->window_state_data,
		clear_vector(&it->value.offset_stack);
		clear_stack(&it->value.id_hash_stack);
		clear_mesh(&it->value.mesh);
	)

	RESET_ARENA(&ggui->scratch);
}

void gui_push_offset(v2 offset) { FUNC
	switch(ggui->current->offset_mode) {
	case gui_offset_xy:
		vector_push(&ggui->current->offset_stack, V2(gui_style.win_margin.x + offset.x, offset.y));
		break;
	case gui_offset_x:
		vector_push(&ggui->current->offset_stack, V2(gui_style.win_margin.x + offset.x, 0.0f));
		break;
	case gui_offset_y:
		vector_push(&ggui->current->offset_stack, V2(gui_style.win_margin.x, offset.y));
		break;
	}
}

void gui_pop_offset() { FUNC
	vector_pop(&ggui->current->offset_stack);
}

void gui_set_offset_mode(gui_offset_mode mode) { FUNC
	ggui->current->offset_mode = mode;
}

bool gui_occluded() { FUNC
	FORMAP(ggui->window_state_data,
		if(&it->value != ggui->current && it->value.z > ggui->current->z) {
			if(it->value.active && inside(mult(it->value.rect, gui_style.gscale), ggui->input.mousepos)) {
				return true;
			} else {
				r2 title_rect = R2(it->value.rect.xy, V2(it->value.rect.w, gui_style.gscale * gui_style.font + gui_style.title_padding));
				title_rect = mult(title_rect, gui_style.gscale);
				if(inside(title_rect, ggui->input.mousepos)) {
					return true;
				}
			}
		}
	)
	return false;
}

bool gui_begin(string name, r2 first_size, f32 first_alpha, gui_window_flags flags, bool mono) { FUNC

	guiid id;
	id.name = name;

	gui_window_state* window = map_try_get(&ggui->window_state_data, id);

	if(!window) {

		gui_window_state ns;

		ns.rect = first_size;
		if(first_size.w == 0.0f || first_size.h == 0.0f) {
			ns.rect.wh = gui_style.default_win_size;
		}

		if(first_alpha == 0.0f) {
			ns.opacity = gui_style.default_win_a;
		} else {
			ns.opacity = first_alpha;
		}

		ns.mesh = make_mesh_2d(1024, ggui->alloc);
		ns.id_hash_stack = make_stack<u32>(16, ggui->alloc);
		ns.offset_stack = make_vector<v2>(16, ggui->alloc);
		ns.flags = flags;
		ns.mono = mono;
		ns.font = gui_select_best_font_scale(&ns);
		ns.z = ggui->last_z;

		window = map_insert(&ggui->window_state_data, id, ns);
	}

	stack_push(&window->id_hash_stack, guiid_hash(id));

	ggui->current = window;

	f32 gscale = 1.0f;
	if((window->flags & win_ignorescale) != win_ignorescale) {
		gscale = gui_style.gscale;
	}

	r2 real_rect = mult(window->rect, gscale);

	if(ggui->active_id == id) {
		if(window->resizing) {

			v2 wh = sub(ggui->input.mousepos, real_rect.xy);
			if(wh.x < gui_style.min_win_size.x) {
				wh.x = gui_style.min_win_size.x;
			}
			if(wh.x < gui_style.min_win_size.x) {
				wh.x = gui_style.min_win_size.x;
			}
			if(wh.y < gui_style.min_win_size.y) {
				wh.y = gui_style.min_win_size.y;
			}
			window->rect = R2(real_rect.xy, div(wh, gscale));

		} else {

			window->rect = R2(sub(ggui->input.mousepos, window->move_click_offset), window->rect.wh);
		}
	}

	real_rect = mult(window->rect, gscale);

	if((window->flags & win_nohead) != win_nohead) {
		push_windowhead(window);

		v2 title_pos = add(window->rect.xy, V2(15.0f, 0.0f));
		push_text(window, title_pos, name, gui_style.font, V4b(gui_style.win_title, 255));
	}

	if((window->flags & win_noback) != win_noback && window->active) {
		push_windowbody(window);
	}

	f32 carrot_x_diff = gui_style.default_carrot_size.x * gscale + gui_style.carrot_padding.x;
	
	r2 top_rect = R2(real_rect.xy, V2(real_rect.w - carrot_x_diff, gscale * gui_style.font + gui_style.title_padding));	
	r2 bod_rect = R2(real_rect.x, real_rect.y + gui_style.font + gui_style.title_padding, real_rect.w, real_rect.h - gui_style.font + gui_style.title_padding);

	bool occluded = gui_occluded();
	if((window->flags & win_nohide) != win_nohide) {

		v2 carrot_pos = V2(real_rect.w - carrot_x_diff, ((gui_style.font + gui_style.title_padding) * gscale / 2.0f) - (gscale * gui_style.default_carrot_size.y / 2.0f));
		gui_carrot_toggle(string_literal("#CLOSE"), window->active, carrot_pos, &window->active);

		if(!occluded && inside(top_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_none && ggui->input.ldbl) {
				
				window->active = !window->active;
				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active;
				window->resizing = false;
			}
		}
	}
	if((window->flags & win_nomove) != win_nomove) {

		if(!occluded && inside(top_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active;
				window->move_click_offset = sub(ggui->input.mousepos, real_rect.xy);
				window->resizing = false;
			}
		}
	}
	if((window->flags & win_noresize) != win_noresize) {

		v2 resize_tab = clamp(mult(real_rect.wh, gui_style.resize_tab), 5.0f, 25.0f);

		r2 resize_rect = R2(sub(add(real_rect.xy, real_rect.wh), resize_tab), resize_tab);
		if(!occluded && inside(resize_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active;
				window->resizing = true;
			}
		}
	}
	if((window->flags & win_noback) != win_noback && window->active) {
		
		if(!occluded && inside(bod_rect, ggui->input.mousepos)) {
			if(ggui->active == gui_none && ggui->input.lclick) {
				window->z = ggui->last_z++;
			}
		}
	}

	gui_push_offset(V2(0.0f, gscale * gui_style.font + gui_style.title_padding + gui_style.win_margin.z));

	return window->active;
}

void gui_log_wnd(string name, vector<log_message>* cache) { FUNC

	f32 height = gui_style.log_win_lines * gui_style.font + 2 * gui_style.font + gui_style.title_padding + gui_style.win_margin.x + gui_style.win_margin.w;
	gui_begin(name, R2(0.0f, global_state->window_h - height, (f32)global_state->window_w, height), 0.5f, win_nowininput | win_nohead | win_ignorescale, true);

	gui_window_state* current = ggui->current;
	current->rect = R2(0.0f, global_state->window_h - height, (f32)global_state->window_w, height);

	guiid id;
	id.base = *stack_top(&current->id_hash_stack);
	id.name = string_literal("#LOG");

	gui_state_data* data = map_try_get(&ggui->state_data, id);

	if(!data) {

		gui_state_data nd;

		nd.u32_1 = 0; // scroll position

		data = map_insert(&ggui->state_data, id, nd);
	}

	// we don't need real_rect, as this will always ignore global scale
	if(!gui_occluded() && inside(current->rect, ggui->input.mousepos)) {
		if(ggui->input.scroll + (i32)data->u32_1 < 0) {
			data->u32_1 = 0;
		} else {
			data->u32_1 += ggui->input.scroll;
		}
		if(cache->size - data->u32_1 < gui_style.log_win_lines) {
			data->u32_1 = cache->size - gui_style.log_win_lines;
		}
		if(cache->size < gui_style.log_win_lines) {
			data->u32_1 = 0;
		}
	}
	
	r2 scroll_back = R2(add(current->rect.xy, V2(current->rect.w - gui_style.win_scroll_w, gui_style.font + gui_style.title_padding)), V2(gui_style.win_scroll_w, current->rect.h));

	mesh_push_rect(&current->mesh, scroll_back, V4b(gui_style.win_scroll_back, current->opacity * 255.0f));

	f32 scroll_y = lerpf(current->rect.y + current->rect.h, current->rect.y, (f32)data->u32_1 / (f32)(cache->size - gui_style.log_win_lines));
	if(scroll_y < current->rect.y + gui_style.font + gui_style.title_padding) {
		scroll_y = current->rect.y + gui_style.font + gui_style.title_padding;
	}
	if(scroll_y > current->rect.y + current->rect.h - gui_style.win_scroll_bar_h) {
		scroll_y = current->rect.y + current->rect.h - gui_style.win_scroll_bar_h;
	}
	r2 scroll_bar = R2(scroll_back.x + gui_style.win_scroll_margin, scroll_y, gui_style.win_scroll_w - gui_style.win_scroll_margin * 2.0f, gui_style.win_scroll_bar_h);

	mesh_push_rect(&current->mesh, scroll_bar, V4b(gui_style.win_scroll_bar, 255));

	v2 pos = add(ggui->current->rect.xy, V2(gui_style.win_margin.x, height - gui_style.win_margin.w - gui_style.font));

	i32 ignore;
	gui_box_select(&ignore, 3, pos, string_literal("DEBUG"), string_literal("INFO"), string_literal("WARN/ERR"));

	pos.y -= gui_style.font + 5.0f;

	for(i32 i = cache->size - data->u32_1; i > 0; i--) {
		log_message* it = vector_get(cache, i - 1);

		string fmt;
		PUSH_ALLOC(&ggui->scratch) {
			
			string level	 = log_fmt_msg_level(it);

			fmt = make_stringf(string_literal("[context] [file:line] [%-5s] %*s\r\n"), level.c_str, 3 * it->call_stack.capacity + it->msg.len - 1, it->msg.c_str);
			push_text(current, pos, fmt, gui_style.font, WHITE);

			free_string(fmt);

		} POP_ALLOC();
		RESET_ARENA(&ggui->scratch);

		pos.y -= gui_style.font;

		if(pos.y < current->rect.y + gui_style.font + gui_style.title_padding + gui_style.win_margin.z) {
			break;
		}
	}
}

void gui_box_select(i32* selected, i32 num, v2 pos, ...) { FUNC

	gui_window_state* current = ggui->current;

	f32 gscale = 1.0f;
	if((current->flags & win_ignorescale) != win_ignorescale) {
		gscale = gui_style.gscale;
	}

	va_list args;
	va_start(args, pos);

	for(i32 i = 0; i < num; i++) {

		string option = va_arg(args, string);

		v2 wh = add(size_text(current->font->font, option, gui_style.font * gscale), gui_style.box_sel_padding);
		v2 txy = add(pos, div(gui_style.box_sel_padding, 2.0f));
		r2 box = R2(pos, wh);

		mesh_push_rect(&current->mesh, box, V4b(gui_style.wid_back, current->opacity * 255.0f));
		mesh_push_text_line(&current->mesh, current->font->font, option, txy, gui_style.font * gscale);

		pos.x += wh.x + 5.0f;
	}

	va_end(args);
}

bool gui_carrot_toggle(string name, bool initial, v2 pos, bool* toggleme) { FUNC

	guiid id;
	id.base = *stack_top(&ggui->current->id_hash_stack);
	id.name = name;

	gui_state_data* data = map_try_get(&ggui->state_data, id);

	if(!data) {

		gui_state_data nd;

		nd.b = initial;

		data = map_insert(&ggui->state_data, id, nd);
	}

	if(toggleme) {
		data->b = *toggleme;
	}

	pos = add(ggui->current->rect.xy, pos);
	FORVEC(ggui->current->offset_stack,
		pos = add(pos, *it);
	)

	v2 size = gui_style.default_carrot_size;
	size = mult(size, gui_style.gscale);
	if(!gui_occluded() && inside(R2(pos, size), ggui->input.mousepos)) {

		if(ggui->active == gui_none && (ggui->input.lclick || ggui->input.ldbl)) {

			data->b = !data->b;
			ggui->active_id = id;
			ggui->active = gui_active;

			if(toggleme) {
				*toggleme = !*toggleme;
			}
		}
	}

	push_carrot(ggui->current, pos, data->b);

	return data->b;
}

void push_carrot(gui_window_state* win, v2 pos, bool active) { FUNC

	f32 gscale = 1.0f;
	if((win->flags & win_ignorescale) != win_ignorescale) {
		gscale = gui_style.gscale;
	}

	u32 idx = win->mesh.verticies.size;
	f32 size = gui_style.default_carrot_size.x * gscale;

	if(active) {
		vector_push(&win->mesh.verticies, V2(pos.x 		 		, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + size 		, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + size / 2.0f, pos.y + size));
	} else {
		vector_push(&win->mesh.verticies, V2(pos.x 		 , pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x 		 , pos.y + size));
		vector_push(&win->mesh.verticies, V2(pos.x + size, pos.y + size / 2.0f));
	}

	colorf cf = color_to_f(V4b(gui_style.wid_back, 255));
	FOR(3) vector_push(&win->mesh.colors, cf);

	FOR(3) vector_push(&win->mesh.texCoords, V3(0.0f, 0.0f, 0.0f));

	vector_push(&win->mesh.elements, V3(idx, idx + 1, idx + 2));
}

void push_text(gui_window_state* win, v2 pos, string text, f32 point, color c) { FUNC

	f32 gscale = 1.0f;
	if((win->flags & win_ignorescale) != win_ignorescale) {
		gscale = gui_style.gscale;
	}

	mesh_push_text_line(&win->mesh, win->font->font, text, pos, point * gscale, c);
}

void push_windowhead(gui_window_state* win) { FUNC
	
	f32 gscale = 1.0f;
	if((win->flags & win_ignorescale) != win_ignorescale) {
		gscale = gui_style.gscale;
	}

	u32 idx = win->mesh.verticies.size;
	r2 r = mult(win->rect, gscale);
	f32 pt = gui_style.font + gui_style.title_padding;
	pt *= gscale;

	vector_push(&win->mesh.verticies, V2(r.x + r.w - 10.0f, r.y));
	vector_push(&win->mesh.verticies, V2(r.x + 10.0f, r.y));
	vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
	vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

	FOR(4) vector_push(&win->mesh.texCoords, V3f(0,0,0));

	colorf topf = color_to_f(V4b(gui_style.win_top, 255));
	FOR(4) vector_push(&win->mesh.colors, topf);

	vector_push(&win->mesh.elements, V3u(idx + 2, idx, idx + 1));
	vector_push(&win->mesh.elements, V3u(idx + 3, idx, idx + 2));
}

void push_windowbody(gui_window_state* win) { FUNC

	f32 gscale = 1.0f;
	if((win->flags & win_ignorescale) != win_ignorescale) {
		gscale = gui_style.gscale;
	}

	u32 idx = win->mesh.verticies.size;
	r2 r = mult(win->rect, gscale);
	f32 pt = gui_style.font + gui_style.title_padding;
	pt *= gscale;

	if((win->flags & win_noresize) == win_noresize) {

		vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
		vector_push(&win->mesh.verticies, V2(r.x, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

		FOR(4) vector_push(&win->mesh.texCoords, V3f(0,0,0));

		colorf cf = color_to_f(V4b(gui_style.win_back, win->opacity * 255.0f));

		FOR(4) vector_push(&win->mesh.colors, cf);

		vector_push(&win->mesh.elements, V3u(idx, idx + 1, idx + 2));
		vector_push(&win->mesh.elements, V3u(idx, idx + 2, idx + 3));

	} else {

		v2 resize_tab = clamp(mult(r.wh, gui_style.resize_tab), 5.0f, 25.0f);

		vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
		vector_push(&win->mesh.verticies, V2(r.x, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w - resize_tab.x, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + r.h - resize_tab.y));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

		FOR(5) vector_push(&win->mesh.texCoords, V3f(0,0,0));

		colorf cf = color_to_f(V4b(gui_style.win_back, win->opacity * 255.0f));

		FOR(5) vector_push(&win->mesh.colors, cf);

		vector_push(&win->mesh.elements, V3u(idx, idx + 1, idx + 2));
		vector_push(&win->mesh.elements, V3u(idx, idx + 2, idx + 3));
		vector_push(&win->mesh.elements, V3u(idx, idx + 3, idx + 4));
	}
}
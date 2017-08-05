
u32 guiid_hash(guiid id) { PROF

	u32 hash = hash_string(id.name);
	return hash ^ id.base;
}

bool operator==(guiid l, guiid r) { PROF
	return l.base == r.base && l.name == r.name;
}

gui_manager make_gui(ogl_manager* ogl, allocator* alloc) { PROF

	gui_manager ret;

	ret.alloc = alloc;
	ret.scratch = MAKE_ARENA("gui_scratch", KILOBYTES(512), alloc, false);

	ret.ogl.context = ogl_add_draw_context(ogl, &ogl_mesh_2d_attribs);
	ret.ogl.shader 	= ogl_add_program(ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);

	ret.window_state_data = map<guiid, gui_window_state>::make(32, alloc, &guiid_hash);
	ret.state_data = map<guiid, gui_state_data>::make(128, alloc, &guiid_hash);
	ret.fonts = make_vector<gui_font>(4, alloc);

	return ret;
}

void destroy_gui(gui_manager* gui) { PROF

	FORMAP(gui->window_state_data,
		it->value.id_hash_stack.destroy();
		destroy_vector(&it->value.offset_stack);
		destroy_mesh(&it->value.mesh);
	)

	gui->window_state_data.destroy();
	gui->state_data.destroy();
	destroy_vector(&gui->fonts);

	DESTROY_ARENA(&gui->scratch);
}

void gui_reload_fonts(ogl_manager* ogl, gui_manager* gui) { PROF

	FORVEC(gui->fonts,

		ogl_destroy_texture(ogl, it->texture);

		asset* font = get_asset(it->store, it->asset_name);

		it->font = font;
		it->texture = ogl_add_texture_from_font(ogl, font);
	)
}

void gui_add_font(ogl_manager* ogl, gui_manager* gui, string asset_name, asset_store* store, bool mono) { PROF

	asset* font = get_asset(store, asset_name);

	gui_font f;
	
	f.asset_name = asset_name;
	f.store = store;
	f.font = font;
	f.mono = mono;
	f.texture = ogl_add_texture_from_font(ogl, font);

	vector_push(&gui->fonts, f);
}

gui_font* gui_select_best_font_scale(gui_window_state* win) { PROF

	gui_font* f = null;

	f32 defl = ggui->style.font * ggui->style.gscale;
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

void gui_begin_frame(gui_manager* gui, gui_input_state input) { PROF

	ggui = gui;
	gui->input = input;
	ggui->style = gui->style;
	ggui->style.font = vector_front(&gui->fonts)->font->font.point;

	FORMAP(gui->window_state_data,
		it->value.font = gui_select_best_font_scale(&it->value);
	)
}

void gui_end_frame(platform_window* win, ogl_manager* ogl) { PROF

	if(!ggui->input.lclick && !ggui->input.rclick && !ggui->input.mclick && !ggui->input.ldbl) {
		if(ggui->active != gui_active_state::captured) {
			ggui->active_id = {};
			ggui->active = gui_active_state::none;
		}
	} else if(ggui->active == gui_active_state::none) {
		ggui->active = gui_active_state::invalid;
	}

	render_command_list rcl = make_command_list();
	FORMAP(ggui->window_state_data,

		render_command cmd = make_render_command(render_command_type::mesh_2d, &it->value.mesh, it->value.z);
		cmd.shader  = ggui->ogl.shader;
		cmd.texture = it->value.font->texture;
		cmd.context = ggui->ogl.context;
		render_add_command(&rcl, cmd);
	)

	rcl.proj = ortho(0, (f32)win->w, (f32)win->h, 0, -1, 1);
	sort_render_commands(&rcl);

	ogl_render_command_list(win, ogl, &rcl);
	destroy_command_list(&rcl);

	FORMAP(ggui->window_state_data,
		vector_clear(&it->value.offset_stack);
		it->value.id_hash_stack.clear();
		clear_mesh(&it->value.mesh);
	)

	RESET_ARENA(&ggui->scratch);
}

void gui_push_offset(v2 offset) { PROF
	switch(ggui->current->offset_mode) {
	case gui_offset_mode::xy:
		vector_push(&ggui->current->offset_stack, V2(ggui->style.win_margin.x + offset.x, offset.y));
		break;
	case gui_offset_mode::x:
		vector_push(&ggui->current->offset_stack, V2(ggui->style.win_margin.x + offset.x, 0.0f));
		break;
	case gui_offset_mode::y:
		vector_push(&ggui->current->offset_stack, V2(ggui->style.win_margin.x, offset.y));
		break;
	}
}

void gui_pop_offset() { PROF
	vector_pop(&ggui->current->offset_stack);
}

void gui_set_offset_mode(gui_offset_mode mode) { PROF
	ggui->current->offset_mode = mode;
}

bool gui_occluded() { PROF
	FORMAP(ggui->window_state_data,
		if(&it->value != ggui->current && it->value.z > ggui->current->z) {
			if(it->value.active && inside(mult(it->value.rect, ggui->style.gscale), ggui->input.mousepos)) {
				return true;
			} else {
				r2 title_rect = R2(it->value.rect.xy, V2(it->value.rect.w, ggui->style.gscale * ggui->style.font + ggui->style.title_padding));
				title_rect = mult(title_rect, ggui->style.gscale);
				if(inside(title_rect, ggui->input.mousepos)) {
					return true;
				}
			}
		}
	)
	return false;
}

bool gui_begin(string name, r2 first_size, f32 first_alpha, gui_window_flags flags, bool mono) { PROF

	guiid id;
	id.name = name;

	gui_window_state* window = ggui->window_state_data.try_get(id);

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

		ns.mesh = make_mesh_2d(1024, ggui->alloc);
		ns.id_hash_stack = stack<u32>::make(16, ggui->alloc);
		ns.offset_stack = make_vector<v2>(16, ggui->alloc);
		ns.flags = flags;
		ns.mono = mono;
		ns.font = gui_select_best_font_scale(&ns);
		ns.z = ggui->last_z;

		window = ggui->window_state_data.insert(id, ns);
	}

	window->id_hash_stack.push(guiid_hash(id));

	ggui->current = window;

	f32 gscale = 1.0f;
	if((window->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	r2 real_rect = mult(window->rect, gscale);

	if(ggui->active_id == id) {
		if(window->resizing) {

			v2 wh = sub(ggui->input.mousepos, real_rect.xy);
			if(wh.x < ggui->style.min_win_size.x * gscale) {
				wh.x = ggui->style.min_win_size.x * gscale;
			}
			if(wh.x < ggui->style.min_win_size.x * gscale) {
				wh.x = ggui->style.min_win_size.x * gscale;
			}
			if(wh.y < ggui->style.min_win_size.y * gscale) {
				wh.y = ggui->style.min_win_size.y * gscale;
			}
			window->rect = R2(real_rect.xy, div(wh, gscale));

		} else {

			window->rect = R2(sub(ggui->input.mousepos, window->move_click_offset), window->rect.wh);
		}
	}

	real_rect = mult(window->rect, gscale);

	if((window->flags & (u16)window_flags::nohead) != (u16)window_flags::nohead) {
		push_windowhead(window);

		v2 title_pos = add(window->rect.xy, V2(15.0f, 0.0f));
		push_text(window, title_pos, name, ggui->style.font, V4b(ggui->style.win_title, 255));
	}

	if((window->flags & (u16)window_flags::noback) != (u16)window_flags::noback && window->active) {
		push_windowbody(window);
	}

	f32 carrot_x_diff = ggui->style.default_carrot_size.x * gscale + ggui->style.carrot_padding.x;
	
	r2 top_rect = R2(real_rect.xy, V2(real_rect.w - carrot_x_diff, gscale * ggui->style.font + ggui->style.title_padding));	
	r2 bod_rect = R2(real_rect.x, real_rect.y + gscale * ggui->style.font + ggui->style.title_padding, real_rect.w, real_rect.h - gscale * ggui->style.font + ggui->style.title_padding);

	bool occluded = gui_occluded();
	if((window->flags & (u16)window_flags::nohide) != (u16)window_flags::nohide) {

		v2 carrot_pos = V2(real_rect.w - carrot_x_diff, ((ggui->style.font + ggui->style.title_padding) * gscale / 2.0f) - (gscale * ggui->style.default_carrot_size.y / 2.0f));
		gui_carrot_toggle(string_literal("#CLOSE"), window->active, carrot_pos, &window->active);

		if(!occluded && inside(top_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.ldbl) {
				
				window->active = !window->active;
				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->resizing = false;
			}
		}
	}
	if((window->flags & (u16)window_flags::nomove) != (u16)window_flags::nomove) {

		if(!occluded && inside(top_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->move_click_offset = sub(ggui->input.mousepos, real_rect.xy);
				window->resizing = false;
			}
		}
	}
	if((window->flags & (u16)window_flags::noresize) != (u16)window_flags::noresize) {

		v2 resize_tab = clamp(mult(real_rect.wh, ggui->style.resize_tab), 5.0f, 25.0f);

		r2 resize_rect = R2(sub(add(real_rect.xy, real_rect.wh), resize_tab), resize_tab);
		if(!occluded && inside(resize_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->resizing = true;
			}
		}
	}
	if((window->flags & (u16)window_flags::noback) != (u16)window_flags::noback && window->active) {
		
		if(!occluded && inside(bod_rect, ggui->input.mousepos)) {
			if(ggui->active == gui_active_state::none && ggui->input.lclick) {
				window->z = ggui->last_z++;
			}
		}
	}

	gui_push_offset(V2(0.0f, gscale * ggui->style.font + ggui->style.title_padding + ggui->style.win_margin.z));

	return window->active;
}

void gui_log_wnd(platform_window* win, string name, vector<log_message>* cache) { PROF

	f32 height = ggui->style.log_win_lines * ggui->style.font + 2 * ggui->style.font + ggui->style.title_padding + ggui->style.win_margin.x + ggui->style.win_margin.w + 7.0f;
	gui_begin(name, R2(0.0f, win->h - height, (f32)win->w, height), 0.5f, (u16)window_flags::nowininput | (u16)window_flags::nohead | (u16)window_flags::ignorescale, true);
	gui_pop_offset();

	gui_window_state* current = ggui->current;
	current->rect = R2(0.0f, win->h - height, (f32)win->w, height);

	guiid id;
	id.base = *current->id_hash_stack.top();
	id.name = string_literal("#LOG");

	gui_state_data* data = ggui->state_data.try_get(id);

	if(!data) {

		gui_state_data nd;

		nd.u32_1 = 0; // scroll position

		data = ggui->state_data.insert(id, nd);
	}

	// we don't need real_rect, as this will always ignore global scale
	if(!gui_occluded() && inside(current->rect, ggui->input.mousepos)) {
		if(ggui->input.scroll + (i32)data->u32_1 < 0) {
			data->u32_1 = 0;
		} else {
			data->u32_1 += ggui->input.scroll;
		}
		if(cache->size - data->u32_1 < ggui->style.log_win_lines) {
			data->u32_1 = cache->size - ggui->style.log_win_lines;
		}
		if(cache->size < ggui->style.log_win_lines) {
			data->u32_1 = 0;
		}
	}
	
	r2 scroll_back = R2(add(current->rect.xy, V2(current->rect.w - ggui->style.win_scroll_w, ggui->style.font + ggui->style.title_padding)), V2(ggui->style.win_scroll_w, current->rect.h));

	mesh_push_rect(&current->mesh, scroll_back, V4b(ggui->style.win_scroll_back, current->opacity * 255.0f));

	f32 scroll_y = lerpf(current->rect.y + current->rect.h, current->rect.y, (f32)data->u32_1 / (f32)(cache->size - ggui->style.log_win_lines));
	if(scroll_y < current->rect.y + ggui->style.font + ggui->style.title_padding) {
		scroll_y = current->rect.y + ggui->style.font + ggui->style.title_padding;
	}
	if(scroll_y > current->rect.y + current->rect.h - ggui->style.win_scroll_bar_h) {
		scroll_y = current->rect.y + current->rect.h - ggui->style.win_scroll_bar_h;
	}
	r2 scroll_bar = R2(scroll_back.x + ggui->style.win_scroll_margin, scroll_y, ggui->style.win_scroll_w - ggui->style.win_scroll_margin * 2.0f, ggui->style.win_scroll_bar_h);

	mesh_push_rect(&current->mesh, scroll_bar, V4b(ggui->style.win_scroll_bar, 255));

	v2 pos = V2(ggui->style.win_margin.x, height - ggui->style.win_margin.w - ggui->style.font);

	i32 ignore;
	gui_box_select(&ignore, 3, pos, string_literal("DEBUG"), string_literal("INFO"), string_literal("WARN/ERR"));

	pos = add(current->rect.xy, pos);
	pos.y -= ggui->style.font + 7.0f;

	for(i32 i = cache->size - data->u32_1; i > 0; i--) {
		log_message* it = vector_get(cache, i - 1);

		string fmt;
		PUSH_ALLOC(&ggui->scratch) {
			
			string level = log_fmt_msg_level(it);

			fmt = make_stringf(string_literal("[context] [file:line] [%-5] %+*\r\n"), level, 3 * it->call_stack.capacity + it->msg.len - 1, it->msg);
			push_text(current, pos, fmt, ggui->style.font, WHITE);

			free_string(fmt);

		} POP_ALLOC();
		RESET_ARENA(&ggui->scratch);

		pos.y -= ggui->style.font;

		if(pos.y < current->rect.y + ggui->style.font + ggui->style.title_padding + ggui->style.win_margin.z) {
			break;
		}
	}
}

void gui_box_select(i32* selected, i32 num, v2 pos, ...) { PROF

	gui_window_state* current = ggui->current;

	f32 gscale = 1.0f;
	if((current->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	va_list args;
	va_start(args, pos);

	pos = add(current->rect.xy, pos);
	FORVEC(ggui->current->offset_stack,
		pos = add(pos, *it);
	)

	for(i32 i = 0; i < num; i++) {

		string option = va_arg(args, string);

		v2 wh = add(size_text(current->font->font, option, ggui->style.font * gscale), ggui->style.box_sel_padding);
		v2 txy = add(pos, V2(ggui->style.box_sel_padding.x / 2.0f, -ggui->style.box_sel_padding.y / 2.0f + 4.0f));
		r2 box = R2(pos, wh);

		mesh_push_rect(&current->mesh, box, V4b(ggui->style.wid_back, current->opacity * 255.0f));
		mesh_push_text_line(&current->mesh, current->font->font, option, txy, ggui->style.font * gscale);

		pos.x += wh.x + 5.0f;
	}

	va_end(args);
}

bool gui_carrot_toggle(string name, bool initial, v2 pos, bool* toggleme) { PROF

	guiid id;
	id.base = *ggui->current->id_hash_stack.top();
	id.name = name;

	gui_state_data* data = ggui->state_data.try_get(id);

	if(!data) {

		gui_state_data nd;

		nd.b = initial;

		data = ggui->state_data.insert(id, nd);
	}

	if(toggleme) {
		data->b = *toggleme;
	}

	pos = add(ggui->current->rect.xy, pos);
	FORVEC(ggui->current->offset_stack,
		pos = add(pos, *it);
	)

	v2 size = ggui->style.default_carrot_size;
	size = mult(size, ggui->style.gscale);
	if(!gui_occluded() && inside(R2(pos, size), ggui->input.mousepos)) {

		if(ggui->active == gui_active_state::none && (ggui->input.lclick || ggui->input.ldbl)) {

			data->b = !data->b;
			ggui->active_id = id;
			ggui->active = gui_active_state::active;

			if(toggleme) {
				*toggleme = !*toggleme;
			}
		}
	}

	push_carrot(ggui->current, pos, data->b);

	return data->b;
}

void push_carrot(gui_window_state* win, v2 pos, bool active) { PROF

	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	u32 idx = win->mesh.verticies.size;
	f32 size = ggui->style.default_carrot_size.x * gscale;

	if(active) {
		vector_push(&win->mesh.verticies, V2(pos.x 		 		, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + size 		, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + size / 2.0f, pos.y + size));
	} else {
		vector_push(&win->mesh.verticies, V2(pos.x 		 , pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x 		 , pos.y + size));
		vector_push(&win->mesh.verticies, V2(pos.x + size, pos.y + size / 2.0f));
	}

	colorf cf = color_to_f(V4b(ggui->style.wid_back, 255));
	FOR(3) vector_push(&win->mesh.colors, cf);

	FOR(3) vector_push(&win->mesh.texCoords, V3(0.0f, 0.0f, 0.0f));

	vector_push(&win->mesh.elements, V3(idx, idx + 1, idx + 2));
}

void push_text(gui_window_state* win, v2 pos, string text, f32 point, color c) { PROF

	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	mesh_push_text_line(&win->mesh, win->font->font, text, pos, point * gscale, c);
}

void push_windowhead(gui_window_state* win) { PROF
	
	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	u32 idx = win->mesh.verticies.size;
	r2 r = mult(win->rect, gscale);
	f32 pt = ggui->style.font + ggui->style.title_padding;
	pt *= gscale;

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

void push_windowbody(gui_window_state* win) { PROF

	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	u32 idx = win->mesh.verticies.size;
	r2 r = mult(win->rect, gscale);
	f32 pt = ggui->style.font + ggui->style.title_padding;
	pt *= gscale;

	if((win->flags & (u16)window_flags::noresize) == (u16)window_flags::noresize) {

		vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
		vector_push(&win->mesh.verticies, V2(r.x, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

		FOR(4) vector_push(&win->mesh.texCoords, V3f(0,0,0));

		colorf cf = color_to_f(V4b(ggui->style.win_back, win->opacity * 255.0f));

		FOR(4) vector_push(&win->mesh.colors, cf);

		vector_push(&win->mesh.elements, V3u(idx, idx + 1, idx + 2));
		vector_push(&win->mesh.elements, V3u(idx, idx + 2, idx + 3));

	} else {

		v2 resize_tab = clamp(mult(r.wh, ggui->style.resize_tab), 5.0f, 25.0f);

		vector_push(&win->mesh.verticies, V2(r.x, r.y + pt));
		vector_push(&win->mesh.verticies, V2(r.x, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w - resize_tab.x, r.y + r.h));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + r.h - resize_tab.y));
		vector_push(&win->mesh.verticies, V2(r.x + r.w, r.y + pt));

		FOR(5) vector_push(&win->mesh.texCoords, V3f(0,0,0));

		colorf cf = color_to_f(V4b(ggui->style.win_back, win->opacity * 255.0f));

		FOR(5) vector_push(&win->mesh.colors, cf);

		vector_push(&win->mesh.elements, V3u(idx, idx + 1, idx + 2));
		vector_push(&win->mesh.elements, V3u(idx, idx + 2, idx + 3));
		vector_push(&win->mesh.elements, V3u(idx, idx + 3, idx + 4));
	}
}
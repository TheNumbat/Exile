
u32 guiid_hash(guiid id) {

	u32 hash = hash_string(id.name);
	return hash ^ id.base;
}

bool operator==(guiid l, guiid r) {
	return l.base == r.base && l.name == r.name;
}

gui_manager make_gui(ogl_manager* ogl, allocator* alloc) {

	gui_manager ret;

	ret.alloc = alloc;

	ret.ogl.context = ogl_add_draw_context(ogl, &ogl_mesh_2d_attribs);
	ret.ogl.shader 	= ogl_add_program(ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);

	ret.window_state_data = make_map<guiid, gui_window_state>(32, alloc, &guiid_hash);
	ret.state_data = make_map<guiid, gui_state_data>(128, alloc, &guiid_hash);
	ret.fonts = make_vector<gui_font>(4, alloc);

	return ret;
}

void destroy_gui(gui_manager* gui) {

	FORMAP(gui->window_state_data,
		destroy_stack(&it->value.id_hash_stack);
		destroy_vector(&it->value.offset_stack);
		destroy_mesh(&it->value.mesh);
	)

	destroy_map(&gui->window_state_data);
	destroy_map(&gui->state_data);
	destroy_vector(&gui->fonts);
}

void gui_reload_fonts(ogl_manager* ogl, gui_manager* gui) {

	FORVEC(gui->fonts,

		ogl_destroy_texture(ogl, it->texture);

		asset* font = get_asset(it->store, it->asset_name);

		it->font = font;
		it->texture = ogl_add_texture_from_font(ogl, font);
	)
}

void gui_add_font(ogl_manager* ogl, gui_manager* gui, string asset_name, asset_store* store) {

	asset* font = get_asset(store, asset_name);

	if(vector_empty(&gui->fonts)) {
		gui->style.font = font->font.point;
	}

	gui_font f;
	
	f.asset_name = asset_name;
	f.store = store;
	f.font = font;
	f.texture = ogl_add_texture_from_font(ogl, font);

	vector_push(&gui->fonts, f);
}

void gui_select_best_font_scale() {

	gui_font* f = NULL;

	f32 defl = ggui->style.font * ggui->style.gscale;
	f32 min_off = FLT_MAX;
	FORVEC(ggui->fonts,
		f32 off = absf(defl - it->font->font.point);
		if(off < min_off) {
			min_off = off;
			f = it;
		}
	)

	ggui->current_font = f;
	ggui->ogl.current_font = f->texture;
}

void gui_begin_frame(gui_manager* gui, gui_input_state input) {

	ggui = gui;
	gui->input = input;

	gui_select_best_font_scale();
}

void gui_end_frame(ogl_manager* ogl) {

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
		cmd.texture = ggui->ogl.current_font;
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
}

bool gui_occluded() {
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

bool gui_begin(string name, r2 first_size, f32 first_alpha, gui_window_flags flags) {

	guiid id;
	id.name = name;

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
		ns.id_hash_stack = make_stack<u32>(16, ggui->alloc);
		ns.offset_stack = make_vector<v2>(16, ggui->alloc);
		ns.title_size = size_text(ggui->current_font->font, name, ggui->style.font);
		ns.flags = flags;
		ns.z = ggui->last_z++;

		window = map_insert(&ggui->window_state_data, id, ns);
	}

	stack_push(&window->id_hash_stack, guiid_hash(id));

	ggui->current = window;
	if((window->flags & win_nohead) != win_nohead) {
		push_windowhead(window);

		v2 title_pos = add(window->rect.xy, V2(15.0f, 0.0f));
		push_text(window, title_pos, name, ggui->style.font, V4b(ggui->style.win_title, 255));
	}

	if((window->flags & win_noback) != win_noback && window->active) {
		push_windowbody(window);
	}

	r2 real_rect = mult(window->rect, ggui->style.gscale);

	f32 carrot_x_diff = ggui->style.default_carrot_size.x * ggui->style.gscale + ggui->style.carrot_padding.x;
	r2 top_rect = R2(real_rect.xy, V2(real_rect.w - carrot_x_diff, ggui->style.gscale * ggui->style.font + ggui->style.title_padding));	
	bool occluded = gui_occluded();
	if((window->flags & win_nohide) != win_nohide) {

		v2 carrot_pos = add(real_rect.xy, V2(real_rect.w - carrot_x_diff, ((ggui->style.font + ggui->style.title_padding) * ggui->style.gscale / 2.0f) - (ggui->style.gscale * ggui->style.default_carrot_size.y / 2.0f)));
		gui_carrot_toggle(string_literal("#CLOSE"), window->active, V4b(ggui->style.win_close, 255), carrot_pos, &window->active);

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

		r2 resize_rect = R2(sub(add(real_rect.xy, real_rect.wh), V2f(15, 15)), V2f(15, 15));
		if(!occluded && inside(resize_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_none && ggui->input.lclick) {

				ggui->active_id = id;
				ggui->active = gui_active;
				window->resizing = true;
			}
		}
	}

	vector_push(&window->offset_stack, V2(5.0f, ggui->style.gscale * ggui->style.font + ggui->style.title_padding + 5.0f));

	if(ggui->active_id == id) {
		if(window->resizing) {

			v2 wh = sub(ggui->input.mousepos, real_rect.xy);
			if(wh.x < ggui->style.min_win_size.x) {
				wh.x = ggui->style.min_win_size.x;
			}
			if(wh.x < (window->title_size.x + ggui->style.default_carrot_size.x) * ggui->style.gscale + ggui->style.carrot_padding.x + 20.0f) {
				wh.x = (window->title_size.x + ggui->style.default_carrot_size.x) * ggui->style.gscale + ggui->style.carrot_padding.x + 20.0f;
			}
			if(wh.y < ggui->style.min_win_size.y) {
				wh.y = ggui->style.min_win_size.y;
			}
			window->rect = R2(real_rect.xy, div(wh, ggui->style.gscale));

		} else {

			window->rect = R2(sub(ggui->input.mousepos, window->move_click_offset), window->rect.wh);
		}
	}

	return false;
}

void gui_log_dsp(string name, vector<log_message>* cache) {

	guiid id;
	id.base = *stack_top(&ggui->current->id_hash_stack);
	id.name = name;

	gui_state_data* data = map_try_get(&ggui->state_data, id);

	if(!data) {

		gui_state_data nd;

		nd.u16_1 = log_info;		// level
		nd.u16_2 = 0;				// place

		data = map_insert(&ggui->state_data, id, nd);
	}

	// input
}

bool gui_carrot_toggle(string name, bool initial, color c, v2 pos, bool* toggleme) {

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

	FORVEC(ggui->current->offset_stack,
		pos = add(pos, *it);
	)

	v2 size = ggui->style.default_carrot_size;
	size = mult(size, ggui->style.gscale);
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

	push_carrot(ggui->current, pos, data->b, c);

	return data->b;
}

void push_carrot(gui_window_state* win, v2 pos, bool active, color c) {

	u32 idx = win->mesh.verticies.size;
	f32 size = 10.0f * ggui->style.gscale;

	if(active) {
		vector_push(&win->mesh.verticies, V2(pos.x, 	pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + size, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x + size / 2.0f, pos.y + size));
	} else {
		vector_push(&win->mesh.verticies, V2(pos.x, pos.y));
		vector_push(&win->mesh.verticies, V2(pos.x, 	 pos.y + size));
		vector_push(&win->mesh.verticies, V2(pos.x + size, pos.y + size / 2.0f));
	}

	colorf cf = color_to_f(c);
	FOR(3) vector_push(&win->mesh.colors, cf);

	FOR(3) vector_push(&win->mesh.texCoords, V3(0.0f, 0.0f, 0.0f));

	vector_push(&win->mesh.elements, V3(idx, idx + 1, idx + 2));
}

void push_text(gui_window_state* win, v2 pos, string text, f32 point, color c) {

	mesh_push_text_line(&win->mesh, ggui->current_font->font, text, pos, point * ggui->style.gscale, c);
}

void push_windowhead(gui_window_state* win) {
	
	u32 idx = win->mesh.verticies.size;
	r2 r = mult(win->rect, ggui->style.gscale);
	f32 pt = ggui->style.font + ggui->style.title_padding;
	pt *= ggui->style.gscale;

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
	r2 r = mult(win->rect, ggui->style.gscale);
	f32 pt = ggui->style.font + ggui->style.title_padding;
	pt *= ggui->style.gscale;

	if((win->flags & win_noresize) == win_noresize) {

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
}
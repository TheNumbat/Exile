
CALLBACK u32 guiid_hash(guiid id) { PROF

	u32 hash = hash_string(id.name);
	return hash ^ id.base;
}

bool operator==(guiid l, guiid r) { PROF
	return l.base == r.base && l.name == r.name;
}

gui_manager gui_manager::make(ogl_manager* ogl, allocator* alloc, platform_window* win) { PROF

	gui_manager ret;

	ret.alloc = alloc;
	ret.window = win;

	ret.window_state_data = map<guiid, gui_window_state>::make(32, alloc, FPTR(guiid_hash));
	ret.state_data = map<guiid, gui_state_data>::make(128, alloc, FPTR(guiid_hash));
	ret.fonts = vector<gui_font>::make(4, alloc);

	return ret;
}

void gui_manager::destroy() { PROF

	FORMAP(win, window_state_data) {
		win->value.id_hash_stack.destroy();
		win->value.offset_stack.destroy();
		win->value.shape_mesh.destroy();
		win->value.text_mesh.destroy();
		win->key.name.destroy(alloc);
	}
	FORMAP(st, state_data) {
		st->key.name.destroy(alloc);
	}

	window_state_data.destroy();
	state_data.destroy();
	fonts.destroy();
}

void gui_manager::reload_fonts(ogl_manager* ogl) { PROF

	FORVEC(it, fonts) {

		ogl->destroy_texture(it->texture);

		asset* font = it->store->get(it->asset_name);

		it->font = font;
		it->texture = ogl->add_texture_from_font(font);
	}
}

void gui_manager::add_font(ogl_manager* ogl, string asset_name, asset_store* store, bool mono) { PROF

	asset* font = store->get(asset_name);

	gui_font f;
	
	f.asset_name = asset_name;
	f.store = store;
	f.font = font;
	f.mono = mono;
	f.texture = ogl->add_texture_from_font(font);

	fonts.push(f);
}

gui_font* gui_select_best_font_scale(gui_window_state* win) { PROF

	gui_font* f = null;

	f32 defl = ggui->style.font * ggui->style.gscale;
	f32 min_off = FLT_MAX;
	FORVEC(it, ggui->fonts) {
		f32 off = absf(defl - it->font->font.point);
		if(off < min_off) {
			min_off = off;
			f = it;
		}
	}

	return f;
}

gui_state_data* gui_manager::add_state_data(guiid id, gui_state_data data) { PROF

	guiid cp = id;
	cp.name = string::make_copy(cp.name, alloc);
	return state_data.insert(cp, data);	
}

gui_window_state* gui_manager::add_window_state_data(guiid id, gui_window_state data) { PROF

	guiid cp = id;
	cp.name = string::make_copy(cp.name, alloc);
	return window_state_data.insert(cp, data);	
}

void gui_manager::begin_frame(gui_input_state new_input) { PROF

	ggui = this;
	input = new_input;
	style.font = fonts.front()->font->font.point;

	FORMAP(it, window_state_data) {
		it->value.font = gui_select_best_font_scale(&it->value);
	}
}

void gui_manager::end_frame(platform_window* win, ogl_manager* ogl) { PROF

	if(!input.lclick && !input.rclick && !input.mclick && !input.ldbl) {
		if(active != gui_active_state::captured) {
			active_id = {};
			active = gui_active_state::none;
		}
	} else if(active == gui_active_state::none) {
		active = gui_active_state::invalid;
	}

	render_command_list rcl = render_command_list::make();
	FORMAP(it, window_state_data) {

		render_command cmd = render_command::make(render_command_type::mesh_2d_col, &it->value.shape_mesh, it->value.z);
		cmd.texture = -1;
		cmd.scissor = it->value.get_real();
		rcl.add_command(cmd);

		cmd = render_command::make(render_command_type::mesh_2d_tex_col, &it->value.text_mesh, it->value.z);
		cmd.texture = it->value.font->texture;
		cmd.scissor = it->value.get_real_content();
		rcl.add_command(cmd);
	}
 
	rcl.proj = ortho(0, (f32)window->w, (f32)window->h, 0, -1, 1);
	rcl.sort();

	ogl->execute_command_list(win, &rcl);
	rcl.destroy();

	FORMAP(it, window_state_data) {
		it->value.offset_stack.clear();
		it->value.id_hash_stack.clear();
		it->value.shape_mesh.clear();
		it->value.text_mesh.clear();
	}
}

v2 gui_window_state::current_offset() {
	v2 pos = rect.xy;
	FORVEC(it, offset_stack) {
		pos = pos + *it * ggui->style.gscale;
	}
	return pos;
}

r2 gui_window_state::get_real_content() {

	r2 r = get_real_body();
	return R2(r.xy, r.wh - ggui->style.win_margin.xy - ggui->style.win_margin.zw);
}

r2 gui_window_state::get_real_top() {

	f32 gscale = 1.0f;
	if((flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	f32 carrot_x_diff = ggui->style.default_carrot_size.x + ggui->style.carrot_padding.x;
	r2 real_rect = get_real();

	return R2(real_rect.xy, V2(real_rect.w - carrot_x_diff, gscale * ggui->style.font + ggui->style.title_padding));
}

r2 gui_window_state::get_real_body() {

	f32 gscale = 1.0f;
	if((flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	r2 real_rect = get_real();
	return R2(real_rect.x, real_rect.y + gscale * ggui->style.font + ggui->style.title_padding, real_rect.w, real_rect.h - gscale * ggui->style.font + ggui->style.title_padding);
}

void gui_window_state::update_rect() {
	
	f32 gscale = 1.0f;
	if((flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	r2 real_rect = get_real();
	if(resizing) {
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
		rect = R2(real_rect.xy, div(wh, gscale));

	} else {

		rect = R2(sub(ggui->input.mousepos, move_click_offset), rect.wh);
	}
}

bool gui_window_state::seen(r2 r) {
	if(override_seen) return true;
	return intersect(get_real_body(), r);
}

r2 gui_window_state::get_real() {
	f32 gscale = 1.0f;
	if((flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}
	return mult(rect, gscale);
}

v2 gui_window_dim() {
	return V2f(ggui->window->w, ggui->window->h);
}

void gui_set_offset(v2 offset) {
	ggui->current->offset_stack.clear();
	ggui->current->offset_stack.push(offset);
}

void gui_push_offset(v2 offset, gui_offset_mode mode) { PROF
	switch(mode) {
	case gui_offset_mode::xy:
		ggui->current->offset_stack.push(V2(offset.x, offset.y));
		break;
	case gui_offset_mode::x:
		ggui->current->offset_stack.push(V2(offset.x, 0.0f));
		break;
	case gui_offset_mode::y:
		ggui->current->offset_stack.push(V2(0.0f, offset.y));
		break;
	}
}

void gui_pop_offset() { PROF
	ggui->current->offset_stack.pop();
}

bool gui_occluded() { PROF
	FORMAP(it, ggui->window_state_data) {
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
	}
	return false;
}

bool gui_begin(string name, r2 first_size, gui_window_flags flags, f32 first_alpha) { PROF

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

		ns.shape_mesh = mesh_2d_col::make(128, ggui->alloc);
		ns.text_mesh = mesh_2d_tex_col::make(1024, ggui->alloc);
		ns.id_hash_stack = stack<u32>::make(16, ggui->alloc);
		ns.offset_stack = vector<v2>::make(16, ggui->alloc);
		ns.flags = flags;
		ns.font = gui_select_best_font_scale(&ns);
		ns.z = ggui->last_z;

		window = ggui->add_window_state_data(id, ns);
	}

	window->id_hash_stack.push(guiid_hash(id));
	window->flags = flags;

	ggui->current = window;

	if(ggui->active_id == id) {
		window->update_rect();
	}

	r2 real_rect = window->get_real();

	v2 header_offset;
	if((window->flags & (u16)window_flags::nohead) != (u16)window_flags::nohead) {
		render_windowhead(window);

		v2 title_pos = V2(3.0f, 0.0f);
		gui_set_offset(title_pos);
		window->override_active = true;
		window->override_seen = true;
		gui_text(name, V4b(ggui->style.win_title, 255));
		window->override_active = false;
		window->override_seen = false;

		header_offset = V2(0.0f, window->default_point + ggui->style.title_padding);
	}

	if((window->flags & (u16)window_flags::noback) != (u16)window_flags::noback && window->active) {
		render_windowbody(window);
	}

	f32 carrot_x_diff = ggui->style.default_carrot_size.x + ggui->style.carrot_padding.x;
	
	r2 real_top = window->get_real_top(); 
	r2 real_body = window->get_real_body();

	bool occluded = gui_occluded();
	if((window->flags & (u16)window_flags::nohide) != (u16)window_flags::nohide) {

		v2 carrot_pos = V2(window->rect.w - carrot_x_diff, ggui->style.default_carrot_size.y / 2.0f);
		
		gui_set_offset(carrot_pos);
		window->override_active = true;
		window->override_seen = true;
		gui_carrot_toggle(string::literal("#CLOSE"), window->active, &window->active);
		window->override_active = false;
		window->override_seen = false;

		if(!occluded && inside(real_top, ggui->input.mousepos)) {

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

		if(!occluded && inside(real_top, ggui->input.mousepos)) {

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
		
		if(!occluded && inside(real_body, ggui->input.mousepos)) {
			if(ggui->active == gui_active_state::none && ggui->input.lclick) {
				window->z = ggui->last_z++;
			}
		}
	}
	if((window->flags & (u16)window_flags::noscroll) != (u16)window_flags::noscroll && window->active) {
		
		if(!occluded && inside(real_body, ggui->input.mousepos)) {
			window->scroll_pos.y += ggui->input.scroll * ggui->style.win_scroll_speed;

			if(window->scroll_pos.y > 0.0f) window->scroll_pos.y = 0.0f;
		}
	}

	gui_set_offset(header_offset + ggui->style.win_margin.xy + window->scroll_pos);

	return window->active;
}

void gui_end() {
	ggui->current = null;
}

bool gui_carrot_toggle(string name, bool initial, bool* toggleme) { PROF

	gui_window_state* win = ggui->current;
	if(!win->active && !win->override_active) return initial;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = name;

	gui_state_data* data = ggui->state_data.try_get(id);

	if(!data) {

		gui_state_data nd;

		nd.b = initial;

		data = ggui->add_state_data(id, nd);
	}

	if(toggleme) {
		data->b = *toggleme;
	}

	v2 pos = win->current_offset();
	v2 size = ggui->style.default_carrot_size * ggui->style.gscale;
	gui_push_offset(size);

	if(!win->seen(R2(pos, size))) {
		return data->b;
	}

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

	render_carrot(ggui->current, pos, data->b);

	return data->b;
}

void render_carrot(gui_window_state* win, v2 pos, bool active) { PROF

	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	u32 idx = win->shape_mesh.vertices.size;
	f32 size = ggui->style.default_carrot_size.x * gscale;

	if(active) {
		win->shape_mesh.vertices.push(V2(pos.x 		 		, pos.y));
		win->shape_mesh.vertices.push(V2(pos.x + size 		, pos.y));
		win->shape_mesh.vertices.push(V2(pos.x + size / 2.0f, pos.y + size));
	} else {
		win->shape_mesh.vertices.push(V2(pos.x 		 , pos.y));
		win->shape_mesh.vertices.push(V2(pos.x 		 , pos.y + size));
		win->shape_mesh.vertices.push(V2(pos.x + size, pos.y + size / 2.0f));
	}

	colorf cf = color_to_f(V4b(ggui->style.wid_back, 255));
	DO(3) win->shape_mesh.colors.push(cf);

	win->shape_mesh.elements.push(V3(idx, idx + 1, idx + 2));
}

void gui_text(string text, color c, f32 point) { PROF

	gui_window_state* win = ggui->current;
	if(!win->active && !win->override_active) return;

	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	if(!point) point = win->default_point;

	v2 pos = win->current_offset();
	v2 size = size_text(win->font->font, text, point * gscale);
	gui_push_offset(V2(0.0f, point));

	if(!win->seen(R2(pos, size))) {
		return;
	}

	win->text_mesh.push_text_line(win->font->font, text, pos, point * gscale, c);
}

void render_windowhead(gui_window_state* win) { PROF
	
	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	u32 idx = win->shape_mesh.vertices.size;
	r2 r = mult(win->rect, gscale);
	f32 pt = ggui->style.font + ggui->style.title_padding;
	pt *= gscale;

	win->shape_mesh.vertices.push(V2(r.x + r.w - 10.0f, r.y));
	win->shape_mesh.vertices.push(V2(r.x, r.y));
	win->shape_mesh.vertices.push(V2(r.x, r.y + pt));
	win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + pt));

	colorf topf = color_to_f(V4b(ggui->style.win_top, 255));
	DO(4) win->shape_mesh.colors.push(topf);

	win->shape_mesh.elements.push(V3u(idx + 2, idx, idx + 1));
	win->shape_mesh.elements.push(V3u(idx + 3, idx, idx + 2));
}

void render_windowbody(gui_window_state* win) { PROF

	f32 gscale = 1.0f;
	if((win->flags & (u16)window_flags::ignorescale) != (u16)window_flags::ignorescale) {
		gscale = ggui->style.gscale;
	}

	u32 idx = win->shape_mesh.vertices.size;
	r2 r = mult(win->rect, gscale);
	f32 pt = ggui->style.font + ggui->style.title_padding;
	pt *= gscale;

	if((win->flags & (u16)window_flags::noresize) == (u16)window_flags::noresize) {

		win->shape_mesh.vertices.push(V2(r.x, r.y + pt));
		win->shape_mesh.vertices.push(V2(r.x, r.y + r.h));
		win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + r.h));
		win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + pt));

		colorf cf = color_to_f(V4b(ggui->style.win_back, win->opacity * 255.0f));

		DO(4) win->shape_mesh.colors.push(cf);

		win->shape_mesh.elements.push(V3u(idx, idx + 1, idx + 2));
		win->shape_mesh.elements.push(V3u(idx, idx + 2, idx + 3));

	} else {

		v2 resize_tab = ggui->style.resize_tab;

		win->shape_mesh.vertices.push(V2(r.x, r.y + pt));
		win->shape_mesh.vertices.push(V2(r.x, r.y + r.h));
		win->shape_mesh.vertices.push(V2(r.x + r.w - resize_tab.x, r.y + r.h));
		win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + r.h - resize_tab.y));
		win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + pt));
		
		win->shape_mesh.vertices.push(V2(r.x + r.w - resize_tab.x, r.y + r.h));
		win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + r.h - resize_tab.y));
		win->shape_mesh.vertices.push(V2(r.x + r.w, r.y + r.h));

		colorf cf = color_to_f(V4b(ggui->style.win_back, win->opacity * 255.0f));
		colorf tf = color_to_f(V4b(ggui->style.tab_color, win->opacity * 255.0f));

		DO(5) win->shape_mesh.colors.push(cf);
		DO(3) win->shape_mesh.colors.push(tf);

		win->shape_mesh.elements.push(V3u(idx, idx + 1, idx + 2));
		win->shape_mesh.elements.push(V3u(idx, idx + 2, idx + 3));
		win->shape_mesh.elements.push(V3u(idx, idx + 3, idx + 4));
		win->shape_mesh.elements.push(V3u(idx + 5, idx + 6, idx + 7));
	}
}

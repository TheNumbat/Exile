
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

	FORMAP(it, window_state_data) {
		it->value.id_hash_stack.destroy();
		it->value.offset_stack.destroy();
		it->value.shape_mesh.destroy();
		it->value.text_mesh.destroy();
		it->key.name.destroy(alloc);
	}
	FORMAP(it, state_data) {
		it->key.name.destroy(alloc);
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

	f32 defl = ggui->style.font;
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
		cmd.num_tris = it->value.title_tris;
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
		it->value.previous_content_size = it->value.current_offset() - it->value.scroll_pos - it->value.rect.xy;
		if(it->value.previous_content_size.y > it->value.rect.h) it->value.can_scroll = true;
		else it->value.can_scroll = false;
		it->value.offset_stack.clear();
		it->value.id_hash_stack.clear();
		it->value.shape_mesh.clear();
		it->value.text_mesh.clear();
	}
}

v2 gui_window_state::current_offset() { PROF
	v2 pos = rect.xy;
	FORVEC(it, offset_stack) {
		pos = pos + *it;
	}
	return pos;
}

r2 gui_window_state::get_real_content() { PROF

	r2 r = get_real_body();
	return R2(r.xy + ggui->style.win_margin.xy, r.wh - ggui->style.win_margin.xy - ggui->style.win_margin.zw);
}

r2 gui_window_state::get_real_top() { PROF

	f32 carrot_x_diff = ggui->style.default_carrot_size.x + ggui->style.carrot_padding.x;
	r2 real_rect = get_real();

	return R2(real_rect.xy, V2(real_rect.w - carrot_x_diff, ggui->style.font + ggui->style.title_padding));
}

r2 gui_window_state::get_real_body() { PROF

	r2 real_rect = get_real();
	r2 body = R2(real_rect.x, real_rect.y + ggui->style.font + ggui->style.title_padding, real_rect.w, real_rect.h - ggui->style.font + ggui->style.title_padding);
	
	if(can_scroll) {
		body.w -= ggui->style.win_scroll_w;
	}
	return body;
}

void gui_window_state::update_rect() { PROF
	
	r2 real_rect = get_real();
	if(resizing) {
		v2 wh = ggui->input.mousepos - real_rect.xy + move_click_offset;
		if(wh.x < ggui->style.min_win_size.x) {
			wh.x = ggui->style.min_win_size.x;
		}
		if(wh.x < ggui->style.min_win_size.x) {
			wh.x = ggui->style.min_win_size.x;
		}
		if(wh.y < ggui->style.min_win_size.y) {
			wh.y = ggui->style.min_win_size.y;
		}
		rect = R2(real_rect.xy, wh);

	} else {

		rect = R2(ggui->input.mousepos - move_click_offset, rect.wh);
	}
}

bool gui_window_state::seen(r2 r) { PROF
	if(override_seen) return true;
	return intersect(get_real_content(), r);
}

r2 gui_window_state::get_real() { PROF
	return rect;
}

v2 gui_window_dim() { PROF
	return V2f(ggui->window->w, ggui->window->h);
}

void gui_set_offset(v2 offset) { PROF
	ggui->current->offset_stack.clear();
	ggui->current->offset_stack.push(offset);
}

void gui_push_offset(v2 offset, gui_offset_mode mode) { PROF
	switch(mode) {
	case gui_offset_mode::xy:
		ggui->current->offset_stack.push(offset);
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
			if(it->value.active && inside(it->value.rect, ggui->input.mousepos)) {
				return true;
			} else {
				r2 title_rect = R2(it->value.rect.xy, V2(it->value.rect.w, ggui->style.font + ggui->style.title_padding));
				title_rect = title_rect;
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
	{//rendering
		if((window->flags & (u16)window_flags::nohead) != (u16)window_flags::nohead) {
			render_windowhead(window);

			v2 title_pos = V2(3.0f, -1.0f);
			gui_set_offset(title_pos);
			window->override_active = true;
			window->override_seen = true;
			gui_text(name, V4b(ggui->style.win_title, 255));
			window->title_tris = window->text_mesh.elements.size;
			window->override_active = false;
			window->override_seen = false;

			header_offset = V2(0.0f, window->default_point + ggui->style.title_padding);
		}

		if((window->flags & (u16)window_flags::noback) != (u16)window_flags::noback && window->active) {
			render_windowbody(window);
		}
	}

	// input
	f32 carrot_x_diff = ggui->style.default_carrot_size.x + ggui->style.carrot_padding.x;
	
	r2 real_top = window->get_real_top(); 
	r2 real_body = window->get_real_body();
	r2 real_content = window->get_real_content();

	bool occluded = gui_occluded();
	if((window->flags & (u16)window_flags::nohide) != (u16)window_flags::nohide) {

		v2 carrot_pos = V2(window->rect.w - carrot_x_diff, ggui->style.default_carrot_size.y / 2.0f);
		
		gui_set_offset(carrot_pos);
		window->override_active = true;
		window->override_seen = true;
		gui_carrot_toggle("#CLOSE"_, window->active, &window->active);
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
				window->move_click_offset = ggui->input.mousepos - real_rect.xy;
				window->resizing = false;
			}
		}
	}
	if((window->flags & (u16)window_flags::noresize) != (u16)window_flags::noresize) {

		v2 resize_tab = ggui->style.resize_tab;
		r2 resize_rect = R2(real_rect.xy + real_rect.wh - resize_tab, resize_tab);
		if(!occluded && inside(resize_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->move_click_offset = real_rect.xy + real_rect.wh - ggui->input.mousepos;
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
	if((window->flags & (u16)window_flags::noscroll) != (u16)window_flags::noscroll && window->active && window->can_scroll) {
		
		r2 scroll_bar = R2(real_body.x + real_body.w, real_body.y, ggui->style.win_scroll_w, real_body.h);

		if(!occluded && inside(real_body, ggui->input.mousepos) || inside(scroll_bar, ggui->input.mousepos)) {
			window->scroll_pos.y += ggui->input.scroll * ggui->style.win_scroll_speed;

			f32 content_offset = -ggui->style.win_margin.y - ggui->style.win_margin.w + real_rect.h;
			if(window->scroll_pos.y < -window->previous_content_size.y + content_offset) window->scroll_pos.y = -window->previous_content_size.y + content_offset;
			if(window->scroll_pos.y > 0.0f) window->scroll_pos.y = 0.0f;
		}

		if(!occluded && inside(scroll_bar, ggui->input.mousepos)) {
			if(ggui->active == gui_active_state::none && ggui->input.lclick) {
				
				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
			} 
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
	v2 size = ggui->style.default_carrot_size;
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

	f32 size = ggui->style.default_carrot_size.x;

	if(active) {
		win->shape_mesh.push_tri(V2(pos.x, pos.y), V2(pos.x + size, pos.y), V2(pos.x + size / 2.0f, pos.y + size), V4b(ggui->style.wid_back, 255));
	} else {
		win->shape_mesh.push_tri(V2(pos.x, pos.y), V2(pos.x, pos.y + size), V2(pos.x + size, pos.y + size / 2.0f), V4b(ggui->style.wid_back, 255));
	}
}

void gui_text(string text, color c, f32 point) { PROF

	gui_window_state* win = ggui->current;
	if(!win->active && !win->override_active) return;

	if(!point) point = win->default_point;

	v2 pos = win->current_offset();
	gui_push_offset(V2(0.0f, point));

	// TODO(max): preliminary check bounds without doing size_text

	// r2 content = win->get_real_content();
	// if(!(pos.y + point >= content.y && pos.y <= content.y + content.h)) {
	// 	return;
	// }

	v2 size = size_text(win->font->font, text, point);
	if(!win->seen(R2(pos, size))) {
		return;
	}

	win->text_mesh.push_text_line(win->font->font, text, pos, point, c);
}

void render_windowhead(gui_window_state* win) { PROF
	
	r2 r = win->get_real();
	f32 pt = ggui->style.font + ggui->style.title_padding;

	r2 render = R2(r.x, r.y, r.w, pt);
	win->shape_mesh.push_rect(render, V4b(ggui->style.win_top, 255));
}

void render_windowbody(gui_window_state* win) { PROF

	r2 c = win->get_real_content();
	r2 r = win->get_real();
	f32 pt = ggui->style.font + ggui->style.title_padding;
	v2 resize_tab = ggui->style.resize_tab;
	
	color c_back   		= V4b(ggui->style.win_back, win->opacity * 255.0f);
	color c_scroll 		= V4b(ggui->style.win_scroll_back, win->opacity * 255.0f);
	color c_scroll_bar 	= V4b(ggui->style.win_scroll_bar, win->opacity * 255.0f);

	bool resizeable = (win->flags & (u16)window_flags::noresize) != (u16)window_flags::noresize;
	if(!resizeable) {

		r2 render = R2(r.x, r.y + pt, r.w, r.h - pt);
		win->shape_mesh.push_rect(render, c_back);

	} else {

		v2 p1 = V2(r.x, r.y + pt);
		v2 p2 = V2(r.x, r.y + r.h);
		v2 p3 = V2(r.x + r.w - resize_tab.x, r.y + r.h);
		v2 p4 = V2(r.x + r.w, r.y + r.h - resize_tab.y);
		v2 p5 = V2(r.x + r.w, r.y + pt);

		win->shape_mesh.push_tri(p1, p2, p3, c_back);
		win->shape_mesh.push_tri(p1, p3, p4, c_back);
		win->shape_mesh.push_tri(p1, p4, p5, c_back);
	}

	if(win->can_scroll) {

		v2 p1 = V2(r.x + r.w - ggui->style.win_scroll_w, r.y + pt);
		v2 p2 = V2(r.x + r.w, r.y + pt);
		v2 p3 = V2(p1.x, p1.y + r.h);
		v2 p4 = V2(p2.x, p1.y + r.h);

		win->shape_mesh.push_tri(p1, p2, p3, c_scroll);
		win->shape_mesh.push_tri(p2, p3, p4, c_scroll);

		f32 scroll_ratio = clamp(-win->scroll_pos.y / (win->previous_content_size.y - r.h), 0.0f, 1.0f);
		f32 scroll_size  = max(c.h * c.h / win->previous_content_size.y, 5.0f);
		f32 scroll_pos   = lerpf(0.0f, r.h - pt - scroll_size, scroll_ratio);

		v2 p5 = V2(p1.x + 2, p1.y + scroll_pos);
		v2 p6 = V2(p2.x - 2, p1.y + scroll_pos);
		v2 p7 = V2(p5.x, p1.y + scroll_pos + scroll_size);
		v2 p8 = V2(p6.x, p1.y + scroll_pos + scroll_size);

		win->shape_mesh.push_tri(p5, p6, p7, c_scroll_bar);
		win->shape_mesh.push_tri(p6, p7, p8, c_scroll_bar);
	}

	if(resizeable) {

		v2 r_1 = V2(r.x + r.w - resize_tab.x, r.y + r.h);
		v2 r_2 = V2(r.x + r.w, r.y + r.h - resize_tab.y);
		v2 r_3 = V2(r.x + r.w, r.y + r.h);
		win->shape_mesh.push_tri(r_1, r_2, r_3, V4b(ggui->style.tab_color, win->opacity * 255.0f));
	}
}

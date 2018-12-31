
inline u32 hash(guiid id) { 

	u32 h = hash(id.name);
	return h ^ id.base;
}

bool operator==(guiid l, guiid r) { 
	return l.base == r.base && l.name == r.name;
}

CALLBACK bool gui_evt_handle(void* param, platform_event evt) { 
	
	gui_input_state* input = (gui_input_state*)param;

	if(evt.type == platform_event_type::mouse) {
	
		if(evt.mouse.flags & (u16)platform_mouseflag::move) {
		
			input->mousepos.x = evt.mouse.x;
			input->mousepos.y = evt.mouse.y;
		}

		if(evt.mouse.flags & (u16)platform_mouseflag::wheel) {
			
			input->scroll = evt.mouse.w;
		}
		
		if(evt.mouse.flags & (u16)platform_mouseflag::lclick) {
			input->lclick = true;
			if(evt.mouse.flags & (u16)platform_mouseflag::release) {
				input->lclick = false;
				input->ldbl = false;
			}
			if(evt.mouse.flags & (u16)platform_mouseflag::dbl) {
				input->lclick = false;
				input->ldbl = true;
			}
		}

		if(evt.mouse.flags & (u16)platform_mouseflag::rclick) {
			input->rclick = true;
			if(evt.mouse.flags & (u16)platform_mouseflag::release) {
				input->rclick = false;
			}
		}

		if(evt.mouse.flags & (u16)platform_mouseflag::mclick) {
			input->mclick = true;
			if(evt.mouse.flags & (u16)platform_mouseflag::release) {
				input->mclick = false;
			}
		}
	}
	return false;
}

gui_window gui_window::make(r2 first_size, f32 first_alpha, u16 flags, allocator* alloc) { 

	gui_window ret;

	ret.alloc = alloc;
	ret.rect = first_size;
	if(first_size.w == 0.0f || first_size.h == 0.0f) {
		ret.rect.wh = ggui->style.default_win_size;
	}

	if(first_alpha == 0.0f) {
		ret.opacity = ggui->style.default_win_a;
	} else {
		ret.opacity = first_alpha;
	}

	ret.background_mesh = mesh_2d_col::make(32, ret.alloc);
	ret.shape_mesh = mesh_2d_col::make(128, ret.alloc);
	ret.text_mesh = mesh_2d_tex_col::make(1024, ret.alloc);
	ret.id_hash_stack = stack<u32>::make(16, ret.alloc);
	ret.flags = flags;
	ret.font = gui_select_best_font_scale();
	ret.z = ggui->last_z++;
	ret.state_data = map<guiid, gui_state_data>::make(32, ret.alloc);

	return ret;
}

void gui_window::reset() { 

	r2 content = get_real_content();

	if(active_this_frame) {
		active_this_frame = false;
		active_last_frame = true;
	} else {
		active_last_frame = false;
	}

	previous_content_size = cursor - scroll_pos - content.xy;
	if(previous_content_size.y > content.h - ggui->style.scroll_slop) can_scroll = true;
	else can_scroll = false;

	cursor = rect.xy;
	clamp_scroll();

	id_hash_stack.clear();
	background_mesh.clear();
	shape_mesh.clear();
	text_mesh.clear();
}

void gui_window::destroy() { 

	FORMAP(st, state_data) {
		st->key.name.destroy(alloc);
	}

	background_mesh.destroy();
	id_hash_stack.destroy();
	shape_mesh.destroy();
	text_mesh.destroy();
	state_data.destroy();
}

void gui_manager::register_events(evt_manager* evt) { 

	handler_id = evt->add_handler(FPTR(gui_evt_handle), &input);
}

void gui_manager::unregister_events(evt_manager* evt) { 

	evt->rem_handler(handler_id);
	handler_id = 0;
}

gui_state_data* gui_window::add_state(guiid id, gui_state_data state) { 

	guiid cp = id;
	cp.name = string::make_copy(cp.name, alloc);
	return state_data.insert(cp, state);
}

gui_manager gui_manager::make(allocator* alloc, platform_window* win) { 

	gui_manager ret;

	ret.alloc = alloc;
	ret.window = win;

	ret.windows = map<guiid, gui_window>::make(32, alloc);
	ret.fonts = vector<gui_font>::make(4, alloc);

	return ret;
}

void gui_manager::destroy() { 

	FORMAP(it, windows) {
		it->key.name.destroy(alloc);
		it->value.destroy();
	}

	windows.destroy();
	fonts.destroy();
}

void gui_manager::reload_fonts(ogl_manager* ogl) { 

	FORVEC(it, fonts) {

		ogl->destroy_texture(it->texture);

		asset* font = it->store->get(it->asset_name);

		it->font = font;
		it->texture = ogl->add_texture_from_font(font);
	}
}

void gui_manager::add_font(ogl_manager* ogl, string asset_name, asset_store* store, bool mono) { 

	asset* font = store->get(asset_name);

	gui_font f;
	
	f.asset_name = asset_name;
	f.store = store;
	f.font = font;
	f.mono = mono;
	f.texture = ogl->add_texture_from_font(font);

	fonts.push(f);
}

gui_font* gui_select_best_font_scale() { 

	gui_font* f = null;

	f32 defl = ggui->style.font_size;
	f32 min_off = FLT_MAX;
	FORVEC(it, ggui->fonts) {
		f32 off = abs(defl - it->font->font.point);
		if(off < min_off) {
			min_off = off;
			f = it;
		}
	}

	return f;
}

gui_window* gui_manager::add_window(guiid id, gui_window data) { 

	guiid cp = id;
	cp.name = string::make_copy(cp.name, alloc);
	return windows.insert(cp, data);	
}

void gui_manager::begin_frame() { 

	ggui = this;
	style.font_size = fonts.front()->font->font.point; // evil

	FORMAP(it, windows) {
		it->value.font = gui_select_best_font_scale();
	}
}

void gui_manager::end_frame(platform_window* win, ogl_manager* ogl) { 

	if(!input.lclick && !input.rclick && !input.mclick && !input.ldbl) {
		if(active != gui_active_state::captured) {
			active_id = {};
			active = gui_active_state::none;
		}
	} else if(active == gui_active_state::none) {
		active = gui_active_state::invalid;
	}
	input.scroll = 0;

	render_command_list rcl = render_command_list::make();
	FORMAP(it, windows) {
		
		if(!it->value.background_mesh.empty()) {
			render_command cmd = render_command::make(render_command_type::mesh_2d_col, &it->value.background_mesh, it->value.z);
			rcl.add_command(cmd);
		}
		if(!it->value.shape_mesh.empty()) {
			render_command cmd = render_command::make(render_command_type::mesh_2d_col, &it->value.shape_mesh, it->value.z);
			cmd.scissor = it->value.get_real_content();
			rcl.add_command(cmd);
		}
		if(it->value.title_verts && !it->value.text_mesh.empty()) {
			render_command cmd = render_command::make(render_command_type::mesh_2d_tex_col, &it->value.text_mesh, it->value.z);
			cmd.num_tris = it->value.title_elements;
			cmd.texture = it->value.font->texture;
			rcl.add_command(cmd);
		}
		if(!it->value.text_mesh.empty()) {
			render_command cmd = render_command::make(render_command_type::mesh_2d_tex_col, &it->value.text_mesh, it->value.z);
			cmd.offset = it->value.title_verts;
			cmd.start_tri = it->value.title_elements;
			cmd.texture = it->value.font->texture;
			cmd.scissor = it->value.get_real_content();
			rcl.add_command(cmd);
		}
	}
 
	rcl.proj = ortho(0, (f32)window->w, (f32)window->h, 0, -1, 1);
	rcl.sort();

	ogl->execute_command_list(&rcl);
	rcl.destroy();

	FORMAP(it, windows) {
		it->value.reset();
	}
}

bool gui_manager::any_active() { 

	return active == gui_active_state::active || active == gui_active_state::captured;
}

void gui_window::clamp_scroll() { 

	r2 content = get_real_content();

	scroll_pos.y = round(scroll_pos.y);

	if(scroll_pos.y < -previous_content_size.y + content.h - ggui->style.scroll_slop) scroll_pos.y = -previous_content_size.y + content.h - ggui->style.scroll_slop;
	if(scroll_pos.y > 0.0f) scroll_pos.y = 0.0f;
}

r2 gui_window::get_real_content() { 

	r2 r = get_real_body();
	return r2(r.xy + ggui->style.win_margin.xy, r.wh - ggui->style.win_margin.xy - ggui->style.win_margin.zw);
}

r2 gui_window::get_real_top() { 

	f32 carrot_x_diff = ggui->style.carrot_size.x + ggui->style.carrot_padding.x;
	r2 real_rect = get_real();

	return r2(real_rect.xy, v2(real_rect.w - carrot_x_diff, ggui->style.font_size + ggui->style.title_padding));
}

r2 gui_window::get_real_body() { 

	r2 real_rect = get_real();
	r2 body = r2(real_rect.x, real_rect.y + ggui->style.font_size + ggui->style.title_padding, real_rect.w, real_rect.h - ggui->style.font_size + ggui->style.title_padding);
	
	if(can_scroll) {
		body.w -= ggui->style.win_scroll_w;
	}
	return body;
}

void gui_window::update_input() { 
	
	r2 real_rect = get_real();
	if(input == win_input_state::resizing) {
		v2 wh = ggui->input.mousepos - real_rect.xy + click_offset;
		if(wh.x < ggui->style.min_win_size.x) {
			wh.x = ggui->style.min_win_size.x;
		}
		if(wh.x < ggui->style.min_win_size.x) {
			wh.x = ggui->style.min_win_size.x;
		}
		if(wh.y < ggui->style.min_win_size.y) {
			wh.y = ggui->style.min_win_size.y;
		}
		rect = r2(real_rect.xy, wh);

	}
	else if(input == win_input_state::moving) {

		rect = r2(ggui->input.mousepos - click_offset, rect.wh);
	}
	else if(input == win_input_state::scrolling) {

		r2 top = get_real_top();
		r2 body = get_real_body();
		r2 content = get_real_content();

		f32 scroll_size = max(content.h * content.h / previous_content_size.y, 5.0f);

		f32 rel_pos = ggui->input.mousepos.y - rect.y - top.h;
		f32 bar_pos = rel_pos - scroll_size / 2.0f;
		f32 ratio = bar_pos / (body.h - scroll_size);
		
		scroll_pos.y = lerp(0.0f, -(previous_content_size.y - rect.h), ratio);
		clamp_scroll();
	}
}

bool gui_window::visible(r2 r) { 
	return intersect(get_real_content(), r);
}

r2 gui_window::get_real() { 
	return rect;
}

v2 gui_window_dim() { 
	return v2((f32)ggui->window->w, (f32)ggui->window->h);
}

r2 gui_window::get_title() {
	return r2(rect.xy, v2(rect.w, ggui->style.font_size + ggui->style.title_padding));
}

void gui_set_offset(v2 offset) { 
	ggui->current->cursor = ggui->current->rect.xy + offset;
}

void gui_add_offset(v2 offset, gui_cursor_mode override_mode) { 
	
	gui_cursor_mode mode = override_mode == gui_cursor_mode::none ? ggui->current->cursor_mode : override_mode;
	v2 old = ggui->current->cursor;

	switch(mode) {
	case gui_cursor_mode::xy:
		ggui->current->cursor = ggui->current->cursor + offset;
		break;
	case gui_cursor_mode::x:
		ggui->current->cursor = ggui->current->cursor + v2(offset.x, 0.0f);
		break;
	case gui_cursor_mode::y:
		ggui->current->cursor = ggui->current->cursor + v2(0.0f, offset.y);
		break;
	}

	ggui->current->last_offset = offset;
}

void gui_push_id(u32 id) { 

	u32 base = *ggui->current->id_hash_stack.top();
	ggui->current->id_hash_stack.push(base ^ hash(id));
}

void gui_pop_id() { 

	ggui->current->id_hash_stack.pop();
}

bool gui_in_win() { 
	FORMAP(it, ggui->windows) {
		if(it->value.active_last_frame && &it->value != ggui->current && it->value.z > ggui->current->z) {
			if(inside(it->value.full_size ? it->value.rect : it->value.get_title(), ggui->input.mousepos)) {
				return false;
			}
		}
	}

	r2 rect = ggui->current->full_size ? ggui->current->rect : ggui->current->get_title();
	return inside(rect, ggui->input.mousepos);
}

bool gui_begin(string name, r2 size, gui_window_flags flags, f32 first_alpha) { 

	guiid id;
	id.name = name;

	gui_window* window = ggui->windows.try_get(id);

	if(!window) {
		window = ggui->add_window(id, gui_window::make(size, first_alpha, flags, ggui->alloc));
	}

	window->flags = flags;
	if((window->flags & (u16)window_flags::noresize) == (u16)window_flags::noresize) {
		window->rect = size;
	}

	window->id_hash_stack.push(hash(id));
	window->active_this_frame = true;
	ggui->current = window;

	if(ggui->active_id == id) {
		window->update_input();
	}

	r2 real_rect = window->get_real();
	
	window->header_size = v2(0.0f, 0.0f);
	{//rendering
		if((window->flags & (u16)window_flags::nohead) != (u16)window_flags::nohead) {
			render_windowhead(window);

			render_title(window, v2(3.0f, -1.0f), name);

			window->header_size = v2(0.0f, ggui->style.font_size + ggui->style.title_padding);
		}

		if((window->flags & (u16)window_flags::noback) != (u16)window_flags::noback && window->full_size) {
			render_windowbody(window);
		}
	}

	
	r2 real_top = window->get_real_top(); 
	r2 real_body = window->get_real_body();
	r2 real_content = window->get_real_content();

	bool in_win = gui_in_win();
	if((window->flags & (u16)window_flags::nohide) != (u16)window_flags::nohide) {

		f32 carrot_x_diff = ggui->style.carrot_size.x + ggui->style.carrot_padding.x;
		v2 carrot_pos = v2(window->rect.w - carrot_x_diff, ggui->style.carrot_size.y / 2.0f);
		
		gui_set_offset(carrot_pos);
		_carrot_toggle_background(&window->full_size);

		if(in_win && inside(real_top, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.ldbl) {
				
				window->full_size = !window->full_size;
				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
			}
		}
	}
	if((window->flags & (u16)window_flags::nomove) != (u16)window_flags::nomove) {

		if(in_win && inside(real_top, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->click_offset = ggui->input.mousepos - real_rect.xy;
				window->input = win_input_state::moving;
			}
		}
	}
	if((window->flags & (u16)window_flags::noresize) != (u16)window_flags::noresize) {

		v2 resize_tab = ggui->style.resize_tab;
		r2 resize_rect = r2(real_rect.xy + real_rect.wh - resize_tab, resize_tab);
		if(in_win && inside(resize_rect, ggui->input.mousepos)) {

			if(ggui->active == gui_active_state::none && ggui->input.lclick) {

				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->click_offset = real_rect.xy + real_rect.wh - ggui->input.mousepos;
				window->input = win_input_state::resizing;
			}
		}
	}
	if((window->flags & (u16)window_flags::noback) != (u16)window_flags::noback && window->full_size) {
		
		if(in_win && inside(real_body, ggui->input.mousepos)) {
			if(ggui->active == gui_active_state::none && ggui->input.lclick) {
				window->z = ggui->last_z++;
			}
		}
	}
	if((window->flags & (u16)window_flags::noscroll) != (u16)window_flags::noscroll && window->full_size && window->can_scroll) {
		
		r2 scroll_bar = r2(real_body.x + real_body.w, real_body.y, ggui->style.win_scroll_w, real_body.h);

		if(in_win && ggui->input.scroll && (inside(real_body, ggui->input.mousepos) || inside(scroll_bar, ggui->input.mousepos))) {
			
			window->scroll_pos.y += ggui->input.scroll * ggui->style.win_scroll_speed;
			window->clamp_scroll();
		}

		if(in_win && inside(scroll_bar, ggui->input.mousepos)) {
			if(ggui->active == gui_active_state::none && ggui->input.lclick) {
				
				window->z = ggui->last_z++;
				ggui->active_id = id;
				ggui->active = gui_active_state::active;
				window->input = win_input_state::scrolling;
			}
		}
	}

	gui_set_offset(window->header_size + ggui->style.win_margin.xy + window->scroll_pos);

	return window->active_this_frame;
}

void gui_push_id(string id) { 

	gui_push_id(hash(id));
}

void gui_end() { 
	ggui->current = null;
}

void gui_indent() { 

	gui_add_offset(v2(ggui->style.indent_size, 0.0f), gui_cursor_mode::x);
	ggui->current->indent_level++;
}

void gui_unindent() { 

	gui_add_offset(v2(-ggui->style.indent_size, 0.0f), gui_cursor_mode::x);
	ggui->current->indent_level--;
}

u32 gui_indent_level() { 
	return ggui->current->indent_level;
}

void gui_same_line() { 

	ggui->current->cursor.y -= ggui->current->last_offset.y;
	ggui->current->cursor.x += ggui->current->last_offset.x;
	ggui->current->last_offset = v2(0.0f, 0.0f);
}

void gui_left_cursor() { 

	gui_window* win = ggui->current;

	v2 old = win->cursor;
	gui_set_offset(win->header_size + ggui->style.win_margin.xy + win->scroll_pos);
	win->cursor.y = old.y;
	win->cursor.x += ggui->current->indent_level * ggui->style.indent_size;
}

bool gui_checkbox(string name, bool* data) { 

	gui_window* win = ggui->current;
	if(!win->full_size) return *data;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = name;

	gui_text(name);
	gui_same_line();

	v2 pos = win->cursor + v2(0.0f, 2.0f);
	v2 size = v2(ggui->style.font_size, ggui->style.font_size);
	gui_add_offset(size);

	if(!win->visible(r2(pos, size))) {
		gui_left_cursor();
		return *data;
	}

	if(gui_in_win() && inside(r2(pos, size), ggui->input.mousepos)) {

		if(ggui->active == gui_active_state::none && (ggui->input.lclick || ggui->input.ldbl)) {

			*data = !*data;
			ggui->active_id = id;
			ggui->active = gui_active_state::active;
		}
	}

	render_checkbox(ggui->current, r2(pos, size), *data);
	gui_left_cursor();
	return *data;
}

i32 gui_int_slider(string text, i32* data, i32 low, i32 high) { 

	gui_window* win = ggui->current;
	if(!win->full_size) return false;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = text;

	string l = string::makef("%"_, low);
	string h = string::makef("%"_, high);

	v2 og_pos = win->cursor;

	gui_text(text);
	gui_same_line();
	gui_text(l);
	gui_same_line();

	f32 point = ggui->style.font_size;
	v2 pos = win->cursor + v2(2.0f, 2.0f);
	v2 size = v2(ggui->style.slider_w, point);
	r2 rect = r2(pos, size);
	
	gui_add_offset(size);

	if(!win->visible(rect)) {
		win->last_offset = win->cursor - og_pos;
		gui_left_cursor();
		return *data;
	}

	if(gui_in_win() && inside(rect, ggui->input.mousepos)) {
		
		if((ggui->active == gui_active_state::none && (ggui->input.lclick || ggui->input.ldbl)) ||
		   (ggui->active == gui_active_state::active && ggui->active_id == id)) {

			f32 bar_w = max(1.0f / (high - low), ggui->style.min_slider_w);
			f32 rel_pos = ggui->input.mousepos.x - rect.x;
			
			f32 bar_pos = clamp(rel_pos - bar_w / 2.0f, 0.0f, rect.w - bar_w);
			f32 ratio = bar_pos / (rect.w - bar_w);
			
			*data = (i32)lerp((f32)low, (f32)high, ratio);
		
			ggui->active_id = id;
			ggui->active = gui_active_state::active;
		}
	}

	gui_same_line();
	gui_add_offset(v2(3.0f, 0.0f));

	render_slider(ggui->current, rect, *data - low, high - low);

	gui_same_line();
	gui_text(h);

	win->last_offset = win->cursor - og_pos;
	gui_left_cursor();

	l.destroy();
	h.destroy();
	return *data;
}

template<typename V>
void gui_combo(string name, map<string,V> options, V* data) { 

	if(gui_node(name, null)) {

		gui_indent();
		FORMAP(it, options) {
			if(gui_button(it->key)) {
				
				guiid id;
				id.base = *ggui->current->id_hash_stack.top();
				id.name = name;
				ggui->current->state_data.get(id)->b = false;

				*data = it->value;
			}
		}
		gui_unindent();
	}
}

bool gui_carrot_toggle(string name, bool initial, bool* toggleme) { 

	gui_window* win = ggui->current;
	if(!win->full_size) return initial;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = name;

	gui_state_data* data = win->state_data.try_get(id);

	if(!data) {

		gui_state_data nd;

		nd.b = initial;

		data = win->add_state(id, nd);
	}

	if(toggleme) {
		data->b = *toggleme;
	}

	v2 pos = win->cursor;
	v2 size = ggui->style.carrot_size;
	gui_add_offset(size);

	if(!win->visible(r2(pos, size))) {
		return data->b;
	}

	if(gui_in_win() && inside(r2(pos, size), ggui->input.mousepos)) {

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

void _carrot_toggle_background(bool* data) { 

	gui_window* win = ggui->current;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = string::literal("#CLOSE");

	v2 pos = win->cursor;
	v2 size = ggui->style.carrot_size;

	if(gui_in_win() && inside(r2(pos, size), ggui->input.mousepos)) {

		if(ggui->active == gui_active_state::none && (ggui->input.lclick || ggui->input.ldbl)) {

			*data = !*data;
			ggui->active_id = id;
			ggui->active = gui_active_state::active;
		}
	}

	if(*data) {
		win->background_mesh.push_tri(v2(pos.x, pos.y), v2(pos.x + size.x, pos.y), v2(pos.x + size.x / 2.0f, pos.y + size.x), color(ggui->style.wid_back, 255));
	} else {
		win->background_mesh.push_tri(v2(pos.x, pos.y), v2(pos.x, pos.y + size.x), v2(pos.x + size.x, pos.y + size.x / 2.0f), color(ggui->style.wid_back, 255));
	}
}

void render_carrot(gui_window* win, v2 pos, bool active) { 

	f32 size = ggui->style.carrot_size.x;

	if(active) {
		win->shape_mesh.push_tri(v2(pos.x, pos.y), v2(pos.x + size, pos.y), v2(pos.x + size / 2.0f, pos.y + size), color(ggui->style.wid_back, 255));
	} else {
		win->shape_mesh.push_tri(v2(pos.x, pos.y), v2(pos.x, pos.y + size), v2(pos.x + size, pos.y + size / 2.0f), color(ggui->style.wid_back, 255));
	}
}

void render_slider(gui_window* win, r2 rect, i32 rel, i32 max) { 


	color out = color(ggui->style.win_scroll_back, 255);
	color in  = color(ggui->style.win_scroll_bar, 255);

	win->shape_mesh.push_rect(rect, out);

	if(!max) return;

	f32 bar_w = max(1.0f / max, ggui->style.min_slider_w);
	f32 bar_x = lerp(rect.x, rect.x + rect.w - bar_w, (f32)rel / max);

	r2 bar = r2(bar_x, rect.y + 2.0f, bar_w, rect.h - 4.0f);

	win->shape_mesh.push_rect(bar, in);
}

void render_checkbox(gui_window* win, r2 pos, bool active) { 

	color out = color(ggui->style.win_scroll_back, 255);
	color in  = color(ggui->style.win_scroll_bar, 255);

	win->shape_mesh.push_rect(pos, out);

	if(active) {

		r2 select = r2(pos.xy + v2(2.0f, 2.0f), pos.wh - v2(4.0f, 4.0f));
		
		win->shape_mesh.push_rect(select, in);
	}
}

template<typename E>
void gui_enum_buttons(string name, E* val) { 

	gui_window* win = ggui->current;
	if(!win->full_size) return;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = name;

	v2 pos = win->cursor;
	
	gui_text(name);
	gui_same_line();

	_type_info* info = TYPEINFO(E);
	for(u32 i = 0; i < info->_enum.member_count; i++) {
		
		gui_same_line();
		if(gui_button(info->_enum.member_names[i])) {
			*val = (E)info->_enum.member_values[i];
		}

		if(i != info->_enum.member_count - 1) {
			gui_same_line();
			gui_text(" | "_);
		}
	}

	win->last_offset = win->cursor - pos;
	gui_left_cursor();
}

bool gui_button(string text) { 

	gui_window* win = ggui->current;
	if(!win->full_size) return false;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = text;

	f32 point = ggui->style.font_size;
	v2 pos = win->cursor;
	v2 size = size_text(win->font->font, text, point);
	bool ret = false;
	gui_add_offset(size);

	if(!win->visible(r2(pos, size))) {
		return ret;
	}

	if(gui_in_win() && inside(r2(pos, size), ggui->input.mousepos)) {
		
		if(ggui->active == gui_active_state::none && (ggui->input.lclick || ggui->input.ldbl)) {

			ret = true;
			ggui->active_id = id;
			ggui->active = gui_active_state::active;
		}
	}

	win->text_mesh.push_text_line(win->font->font, text, pos, point, WHITE);
	return ret;
}

bool gui_node(string text, bool* store) {  

	gui_window* win = ggui->current;
	if(!win->full_size) return false;

	guiid id;
	id.base = *win->id_hash_stack.top();
	id.name = text;

	bool* data = null;
	if(store)  {
		
		data = store;

	} else {

		gui_state_data* state = win->state_data.try_get(id);

		if(!state) {

			gui_state_data nd;

			nd.b = false;

			state = win->add_state(id, nd);
		}

		data = &state->b;
	}

	f32 point = ggui->style.font_size;
	v2 pos = win->cursor;
	v2 size = size_text(win->font->font, text, point);
	gui_add_offset(size);

	if(!win->visible(r2(pos, size))) {
		return *data;
	}

	if(gui_in_win() && inside(r2(pos, size), ggui->input.mousepos)) {
		
		if(ggui->active == gui_active_state::none && (ggui->input.lclick || ggui->input.ldbl)) {

			*data = !*data;
			ggui->active_id = id;
			ggui->active = gui_active_state::active;
		}
	}

	win->text_mesh.push_text_line(win->font->font, text, pos, point, WHITE);
	return *data;
}

void gui_text(string text) { 

	gui_window* win = ggui->current;
	if(!win->full_size) return;

	f32 point = ggui->style.font_size;
	color c = WHITE;

	v2 pos = win->cursor, size = v2(0.0f, point);
	
	if(!win->visible(r2(pos, size))) {
		gui_add_offset(size);
		return;
	}

	size = size_text(win->font->font, text, point);

	if(!win->visible(r2(pos, size))) {
		gui_add_offset(size);
		return;
	}

	gui_add_offset(size);
	win->text_mesh.push_text_line(win->font->font, text, pos, point, c);
}

void render_windowhead(gui_window* win) { 
	
	r2 r = win->get_real();
	f32 pt = ggui->style.font_size + ggui->style.title_padding;

	r2 render = r2(r.x, r.y, r.w, pt);
	win->background_mesh.push_rect(render, color(ggui->style.win_top, 255));
}

void render_title(gui_window* win, v2 pos, string title) { 

	u32 vidx = win->text_mesh.vertices.size;
	u32 eidx = win->text_mesh.elements.size;

	win->text_mesh.push_text_line(win->font->font, title, win->rect.xy + pos, ggui->style.font_size, WHITE);
	
	win->title_verts = win->text_mesh.vertices.size - vidx;
	win->title_elements = win->text_mesh.elements.size - eidx;
}

void render_windowbody(gui_window* win) { 

	r2 c = win->get_real_content();
	r2 r = win->get_real();
	r2 b = win->get_real_body();
	f32 pt = ggui->style.font_size + ggui->style.title_padding;
	v2 resize_tab = ggui->style.resize_tab;
	
	u8 alpha = (u8)round(win->opacity * 255.0f);
	color c_back   		= color(ggui->style.win_back, 		 alpha);
	color c_scroll 		= color(ggui->style.win_scroll_back, alpha);
	color c_scroll_bar 	= color(ggui->style.win_scroll_bar,  alpha);

	bool resizeable = (win->flags & (u16)window_flags::noresize) != (u16)window_flags::noresize;
	if(!resizeable) {

		r2 render = r2(r.x, r.y + pt, r.w, r.h - pt);
		win->background_mesh.push_rect(render, c_back);

	} else {

		v2 p1 = v2(r.x, r.y + pt);
		v2 p2 = v2(r.x, r.y + r.h);
		v2 p3 = v2(r.x + r.w - resize_tab.x, r.y + r.h);
		v2 p4 = v2(r.x + r.w, r.y + r.h - resize_tab.y);
		v2 p5 = v2(r.x + r.w, r.y + pt);

		win->background_mesh.push_tri(p1, p2, p3, c_back);
		win->background_mesh.push_tri(p1, p3, p4, c_back);
		win->background_mesh.push_tri(p1, p4, p5, c_back);
	}

	if(win->can_scroll) {

		v2 p1 = v2(b.x + b.w, r.y + pt);
		v2 p2 = v2(b.x + b.w + ggui->style.win_scroll_w, r.y + pt);
		v2 p3 = v2(p1.x, p1.y + r.h - pt);
		v2 p4 = v2(p2.x, p1.y + r.h - pt);

		win->background_mesh.push_tri(p1, p2, p3, c_scroll);
		win->background_mesh.push_tri(p2, p3, p4, c_scroll);

		f32 scroll_ratio = clamp(-win->scroll_pos.y / (win->previous_content_size.y - r.h), 0.0f, 1.0f);
		f32 scroll_size  = max(c.h * c.h / win->previous_content_size.y, 5.0f);
		f32 scroll_pos   = lerp(0.0f, r.h - pt - scroll_size, scroll_ratio);

		v2 p5 = v2(p1.x + 2, p1.y + scroll_pos);
		v2 p6 = v2(p2.x - 2, p1.y + scroll_pos);
		v2 p7 = v2(p5.x, p1.y + scroll_pos + scroll_size);
		v2 p8 = v2(p6.x, p1.y + scroll_pos + scroll_size);

		win->background_mesh.push_tri(p5, p6, p7, c_scroll_bar);
		win->background_mesh.push_tri(p6, p7, p8, c_scroll_bar);
	}

	if(resizeable) {

		v2 r_1 = v2(r.x + r.w - resize_tab.x, r.y + r.h);
		v2 r_2 = v2(r.x + r.w, r.y + r.h - resize_tab.y);
		v2 r_3 = v2(r.x + r.w, r.y + r.h);
		win->background_mesh.push_tri(r_1, r_2, r_3, color(ggui->style.tab_color, (u8)round(win->opacity * 255.0f)));
	}
}

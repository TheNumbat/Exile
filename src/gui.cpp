
gui_manager make_gui(allocator* alloc, opengl* ogl, asset* font) {

	gui_manager ret;

	ret.alloc = alloc;
	ret.font = font;
	ret.font_point = font->font.point;

	ret.ogl.context = ogl_add_draw_context(ogl, &ogl_mesh_2d_attribs);
	ret.ogl.texture = ogl_add_texture_from_font(ogl, font);
	ret.ogl.shader 	= ogl_add_program(ogl, string_literal("shaders/gui.v"), string_literal("shaders/gui.f"), &ogl_uniforms_gui);

	return ret;
}

void destroy_gui(gui_manager* gui) {

	destroy_mesh_2d(&gui->mesh);

	gui->ogl.context = 0;
	gui->ogl.texture = 0;
	gui->ogl.shader  = 0;
}

void gui_begin_frame(gui_manager* gui) {

	gui->mesh = make_mesh_2d(gui->alloc);
}

void gui_end_frame(gui_manager* gui) {

	destroy_mesh_2d(&gui->mesh);
}

void gui_begin_window(gui_manager* gui, string title, r2 rect, f32 opacity) {

	gui->current.opacity = opacity;
	gui->current.title = title;
	gui->current.rect = rect;
	gui->current.widgets = make_vector<gui_widget>(16, gui->alloc);
	gui->current.margin = V2(10.0f, gui->font_point);
	gui->current.last_y = 0;
}

void gui_end_window(gui_manager* gui) {

	mesh_push_windowshape(&gui->mesh, gui->current.rect, gui->font_point, V4b(34, 43, 47, (i32)roundf(gui->current.opacity * 255)), V4b(74, 79, 137, 255));
	mesh_push_text_line(&gui->mesh, gui->font, gui->current.title, add(gui->current.rect.xy, V2(15.0f, gui->font_point - 5.0f)), gui->font_point, V4b(255, 255, 255, 255));

	for(u32 i = 0; i < gui->current.widgets.size; i++) {

		gui_widget* w = vector_get(&gui->current.widgets, i);

		switch(w->type) {
		case widget_text: {

			mesh_push_text_line(&gui->mesh, gui->font, w->text.text, add(gui->current.rect.xy, w->pos), w->text.point, w->text.c);

		} break;
		}
	}

	destroy_vector(&gui->current.widgets);
}

void gui_text_line(gui_manager* gui, string str, f32 point, color c) {

	if (point == 0.0f) {
		point = gui->font_point;
	}

	gui_widget t;
	t.type = widget_text;
	t.text.text = str;
	t.text.point = point;
	t.text.c = c;
	gui->current.last_y += point;
	t.pos = V2(gui->current.margin.x, gui->current.last_y + gui->current.margin.y);


	vector_push(&gui->current.widgets, t);
}

void gui_render(gui_manager* gui, opengl* ogl) {

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
}


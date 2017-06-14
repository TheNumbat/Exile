

void render_debug_gui(game_state* state) {

	if(gui_window(&state->dgui.info, &state->gui, string_literal("Debug"), R2(20.0f, 20.0f, 300.0f, 500.0f), 0.75f)) {

		if(gui_carrot_text(&state->dgui.window, &state->gui, V4b(255, 255, 255, 255), &state->dgui.bwindow, string_literal("window"))) {

			gui_push_offset(&state->gui, V2(10.0f, 0.0f));
			gui_text_line_f(&state->gui, string_literal("window_w: %i"), state->window_w);
			gui_text_line_f(&state->gui, string_literal("window_h: %i"), state->window_h);
			gui_pop_offset(&state->gui);
		}
	}
}

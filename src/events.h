
#pragma once

void setup_events(game_state* state);
void end_events(game_state* state);

void filter_dupe_window_events(queue<platform_event>* queue);
bool run_events(game_state* state);

void event_enqueue(void* data, platform_event evt);


#pragma once

struct exile;
void setup_console_commands(exile* e);

CALLBACK void console_exit(string, void* e);
CALLBACK void console_place_light(string, void* w);
CALLBACK void console_rem_light(string, void* w);
CALLBACK void console_set_block(string, void* w);

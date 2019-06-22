
#pragma once

#include <engine/ds/string.h>

struct exile;
void setup_console_commands();

CALLBACK void console_exit(string, void* e);
CALLBACK void console_place_light(string, void* w);
CALLBACK void console_rem_light(string, void* w);
CALLBACK void console_set_block(string, void* w);

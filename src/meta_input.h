
#include "lib/basic.h"
#include "lib/alloc.h"
#include "lib/log.h"
#include "lib/string.h"
#include "lib/vec.h"
#include "lib/reflect.h"
#include "engine.h"
#include "platform.h"
#include "gui.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>
#include <sdl/SDL.h>
#include <glad/glad.h>
#ifdef RUNNING_META
#include "lib/alloc.cpp"
#include "lib/string.cpp"
#include "engine.cpp"
#include "platform.cpp"
#include "gui.cpp"
#include "main.cpp"
#endif


#include "basic.h"

#include "math.h"

#include "ds/string.h"
#include "util/context.h"

#include "platform/platform_api.h"
#include "util/fptr.h"

#include "ds/alloc.h"
#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"
#include "ds/buffer.h"
#include "ds/heap.h"

#include "util/reflect.h"

#include "log_html.h"
#include "log.h"

#include "threads.h"
#include "asset.h"
#include "render.h"
#include "events.h"

#include <imgui/imgui.h>
#include "imgui.h"

#include "dbg.h"
#include "util/threadstate.h"

#include "engine.h"

// IMPLEMENTATIONS

#include "util/fptr.cpp"
#include "util/context.cpp"
#include "util/threadstate.cpp"
#include "util/reflect.cpp"

#include "log.cpp"
#include "events.cpp"
#include "render.cpp"
#include "dbg.cpp"
#include "asset.cpp"
#include "threads.cpp"
#include "engine.cpp"
#include "imgui.cpp"

#include "ds/alloc.cpp"
#include "ds/string.cpp"
#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/buffer.cpp"
#include "ds/heap.cpp"

#define MATH_IMPLEMENTATIONS
#include "math.h"
// /IMPLEMENTATIONS

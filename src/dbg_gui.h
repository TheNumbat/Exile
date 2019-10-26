
#pragma once

#include "lib/lib.h"
#include <LegitProfiler/ImGuiProfilerRenderer.h>

struct DbgGui {

    static void Profiler();

    static inline LegitProfiler::ProfilersWindow profiler;
};

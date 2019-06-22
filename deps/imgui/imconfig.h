
#pragma once

#include <engine/math.h>

#ifdef _WIN32
#include "../src/engine/platform/windows/windows.h"
#endif

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_MATH_FUNCTIONS                      // Don't implement ImFabs/ImSqrt/ImPow/ImFmod/ImCos/ImSin/ImAcos/ImAtan2 wrapper so you can implement them yourself. Declare your prototypes in imconfig.h.
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS                  // Don't implement default allocators calling malloc()/free(). You will need to call ImGui::SetAllocatorFunctions().

#define ImFabs  absv
#define ImSqrt  sqrt
#define ImPow   pow
#define ImFmod  mod
#define ImCos   cos
#define ImSin   sin
#define ImAcos  acos
#define ImAtan2 atan2

#define ImFloorStd  floor
#define ImCeil 		ceil

#define ImAtof atof

#define IMGUI_STB_TRUETYPE_FILENAME  <stb/stb_truetype.h>
#define IMGUI_STB_RECT_PACK_FILENAME <stb/stb_rect_pack.h>
#define IMGUI_STB_TEXTEDIT_FILENAME <stb/stb_textedit.h>

//---- Define constructor and implicit cast operators to convert back<>forth from your math types and ImVec2/ImVec4.
// This will be inlined as part of ImVec2 and ImVec4 class declarations.

#define IM_VEC2_CLASS_EXTRA                                                 \
        ImVec2(const v2& f) { x = f.x; y = f.y; }                       \
        operator v2() const { return v2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
        ImVec4(const v4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }     \
        operator v4() const { return v4(x,y,z,w); }


#define EXTERN extern "C"

#ifdef _MSC_VER
#define DLL_EXPORT EXTERN __declspec(dllexport)
#define DLL_IMPORT EXTERN __declspec(dllimport)
#elif defined(__GNUC__)
#define DLL_EXPORT EXTERN __attribute__((dllexport))
#define DLL_IMPORT EXTERN __attribute__((dllimport))
#else
#error unsupported compiler?
#endif

#include "../src/engine/util/basic_types.h"

#define MATH_NO_IMPLEMENTATIONS
#include "../src/engine/math.h"

#ifdef _WIN32
#include "../src/engine/platform/windows/windows.h"
#endif

#include <imgui/imgui.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_demo.cpp>

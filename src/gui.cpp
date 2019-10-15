
#ifndef RUNNING_META
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#endif

#include "gui.h"
#include "imgui_ext.h"

#include "lib/lib.h"

using alloc = Mallocator<Type_Info<Gui>::name>;

static void* imgui_alloc(usize sz, void*) {
    return alloc::alloc<u8>(sz);
}

static void imgui_free(void* mem, void*) {
    alloc::dealloc(mem);
}

void Gui::init(const Platform& plt) {

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(plt.window, plt.gl_context);
    ImGui_ImplOpenGL3_Init();

    ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, null);
    ImGui::StyleColorsDark();
}

void Gui::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Gui::end_frame(const Platform& plt) {

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(plt.window);
}

i32 test(i32 v) {return 0;}


void Gui::begin_frame(const Platform& plt) {

    glClearColor(0.6f, 0.65f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(plt.window);
    ImGui::NewFrame();

    ImGui::Begin("0");
    static u8 u8_; ImGui::Edit("u8",u8_);
    static i8 i8_; ImGui::Edit("i8", i8_);
    static u16 u16_; ImGui::Edit("u16",u16_);
    static i16 i16_; ImGui::Edit("i16", i16_);
    static u32 u32_; ImGui::Edit("u32",u32_);
    static i32 i32_; ImGui::Edit("i32", i32_);
    static u64 u64_; ImGui::Edit("u64",u64_);
    static i64 i64_; ImGui::Edit("i64", i64_);
    static f32 f32_; ImGui::Edit("f32", f32_);
    static f64 f64_; ImGui::Edit("f64", f64_);
    static bool bool_; ImGui::Edit("bool", bool_);
    static char char_; ImGui::Edit("char", char_);
    ImGui::View("const char*");
    static i32 iarr_[10]; ImGui::Edit("i32[]", iarr_);
    static char carr_[10]; ImGui::Edit("char[]", carr_);
    i32* p0 = null;
    i32* p1 = &iarr_[0];
    i32 (*f0)(i32) = test;
    ImGui::Edit("p0", p0);
    ImGui::Edit("p1", p1);
    ImGui::Edit("f0", f0);
    static Type_Type e0 = Type_Type(100);
    ImGui::Edit("e0", e0);
    ImGui::View(e0);
    ImGui::View(carr_);
    ImGui::End();
}

bool Gui::get_event(SDL_Event& e) {
    
    if(SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        return true;
    }
    return false;
}

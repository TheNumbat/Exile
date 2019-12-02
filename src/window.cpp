
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_vulkan.h>
#include <SDL2/SDL.h>

#include "lib/lib.h"
#include "window.h"

#ifdef _MSC_VER
extern "C" {
	__declspec(dllexport) bool NvOptimusEnablement = true;
	__declspec(dllexport) bool AmdPowerXpressRequestHighPerformance = true;
}
#endif

static void* imgui_alloc(usize sz, void*) {
    return Window::gui_alloc::alloc<u8>(sz);
}

static void imgui_free(void* mem, void*) {
    Window::gui_alloc::dealloc(mem);
}

void Window::begin_frame() {

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

void Window::end_frame() {

    assert(!"unimplemented");
    VkCommandBuffer buf = 0;
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buf);
}

bool Window::get_event(SDL_Event& e) {
    
    if(SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        return true;
    }
    return false;
}

void Window::init() {

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("Exile", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if(!window) {
        die("Failed to create SDL window: %", SDL_GetError());
    }
    info("Created SDL window.");
    
    ImGui::SetAllocatorFunctions(imgui_alloc, imgui_free, null);
    ImGui::CreateContext();

    vk.init(window);

    assert(!"unimplemented");
    ImGui_ImplVulkan_InitInfo init = {};
    VkRenderPass pass = 0;

    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_Init(&init, pass);

    ImGui::StyleColorsDark();
}

void Window::destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    vk.destroy();
    SDL_DestroyWindow(window);
    window = null;
    SDL_Quit();	
}

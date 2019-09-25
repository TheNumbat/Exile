
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "gui.h"

void Gui::init(const Platform& plt) {

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(plt.window, plt.gl_context);
	ImGui_ImplOpenGL3_Init();

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

void Gui::begin_frame(const Platform& plt) {

	glClearColor(0.6f, 0.65f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(plt.window);
	ImGui::NewFrame();
}

bool Gui::get_event(SDL_Event& e) {
	
	if(SDL_PollEvent(&e)) {
		ImGui_ImplSDL2_ProcessEvent(&e);
		return true;
	}
	return false;
}

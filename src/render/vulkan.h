
#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include "../lib/lib.h"

struct Vulkan {

    void init(SDL_Window* window);
    void destroy();

    static constexpr char vk_name[] = "Vulkan";
    using vk_alloc_t = Mallocator<vk_name>;

private:
	vec<const char*> instExt, devExt, layers;

	VkInstance instance = {};
	VkSurfaceKHR surface = {};

    void create_instance(SDL_Window* window);
	void enumerate_physical_devices();
	void select_physical_device();
	void create_logical_device_and_queues();
	void create_semaphores();
	void create_commandPool();
	void create_commandBuffer();
	// void vulkanAllocator.Init();
	// void stagingManager.Init();
	void create_swap_chain();
	void create_render_targets();
	void create_render_pass();
	void create_pipeline_cache();
	void create_frame_buffers();
	// void renderProgManager.Init();
};



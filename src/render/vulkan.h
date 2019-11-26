
#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include "../lib/lib.h"

struct Vulkan {

    void init(SDL_Window* window);
    void destroy();

    static constexpr char vk_name[] = "Vulkan";
    using alloc = Mallocator<vk_name>;

private:

	struct GPU {
		VkPhysicalDevice device = {};
		VkSurfaceCapabilitiesKHR surf_caps = {};
		VkPhysicalDeviceProperties dev_prop = {};
		VkPhysicalDeviceMemoryProperties mem_prop = {};
		vec<VkQueueFamilyProperties, alloc> queue_families;
		vec<VkExtensionProperties, alloc> exts;
		vec<VkSurfaceFormatKHR, alloc> fmts;
		vec<VkPresentModeKHR, alloc> modes;

		void destroy() {
			queue_families.destroy();
			modes.destroy();
			exts.destroy();
			fmts.destroy();
		}
	};

	vec<const char*, alloc> inst_ext, dev_ext, layers;
	vec<GPU, alloc> gpus;
	vec<VkExtensionProperties, alloc> extensions;

	VkInstance instance = {};
	VkSurfaceKHR surface = {};
	GPU* gpu = null;

	VkDebugReportCallbackEXT debug_callback_info = {};

    void create_instance(SDL_Window* window);
	void enumerate_gpus();
	void select_gpu();

	void init_debug_callback();
	void destroy_debug_callback();

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



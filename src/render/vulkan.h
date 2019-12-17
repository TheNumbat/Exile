
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
		VkPhysicalDeviceProperties dev_prop = {};
		VkPhysicalDeviceMemoryProperties mem_prop = {};
		VkPhysicalDeviceFeatures features = {};
		VkSurfaceCapabilitiesKHR surf_caps = {};

		vec<VkSurfaceFormatKHR, alloc> fmts;
		vec<VkPresentModeKHR, alloc> modes;
		vec<VkQueueFamilyProperties, alloc> queue_families;
		vec<VkExtensionProperties, alloc> exts;
		
		i32 graphics_idx = 0, present_idx = 0;

		bool supports(const vec<const char*, alloc>& extensions);
		void destroy() {
			queue_families.destroy();
			modes.destroy();
			exts.destroy();
			fmts.destroy();
		}
	};

	// NOTE(max): double buffering
	static const i32 BUF_FRAMES = 2;

	vec<const char*, alloc> inst_ext, dev_ext, layers;
	vec<GPU, alloc> gpus;
	vec<VkExtensionProperties, alloc> extensions;

	GPU* gpu = null;
	VkDevice device = {};
	VkInstance instance = {};
	VkSurfaceKHR surface = {};
	VkQueue graphics_queue = {}, present_queue = {};
	
	VkCommandPool command_pool = {};
	VkCommandBuffer command_buffer[BUF_FRAMES] = {};
	VkSwapchainKHR swapchain = {};
	VkSurfaceFormatKHR swapchain_format = {};
	VkPresentModeKHR present_mode = {};
	VkExtent2D extent;
	VkFormat depth_format;
	VkRenderPass render_pass;

	VkFramebuffer framebuffers[BUF_FRAMES] = {};
	VkImageView depth_view = {};
	VkImageView image_views[BUF_FRAMES] = {};
	VkFence buf_fence[BUF_FRAMES] = {};
	VkSemaphore aquire_sem[BUF_FRAMES] = {}, complete_sem[BUF_FRAMES] = {};

	VkDebugReportCallbackEXT debug_callback_info = {};

    void create_instance(SDL_Window* window);
	void enumerate_gpus();
	void select_gpu();

	void init_debug_callback();
	void destroy_debug_callback();

	void create_logical_device_and_queues();
	void create_semaphores();
	void create_command_pool();
	void create_command_buffers();
	void create_swap_chain(SDL_Window* window);
	void create_shaders();
	void create_render_targets();
	void create_render_pass();
	void create_frame_buffers(SDL_Window* window);

	VkFormat choose_supported_format(VkFormat* formats, i32 n_formats, VkImageTiling tiling, VkFormatFeatureFlags features);
};



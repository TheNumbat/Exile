
#include "vulkan.h"
#include <vulkan/vulkan.h>

static void* vk_alloc(usize sz, void*) {
    return Vulkan::vk_alloc::alloc<u8>(sz);
}

static void vk_free(void* mem, void*) {
    Vulkan::vk_alloc::dealloc(mem);
}

void Vulkan::init(SDL_Window* window) {
    
    assert(window);

    create_instance();
	create_surface(window);
	enumerate_physical_devices();
	select_physical_device();
	create_logical_device_and_queues();
	create_semaphores();
	create_commandPool();
	create_commandBuffer();
	// vulkanAllocator.Init();
	// stagingManager.Init();
	create_swap_chain();
	create_render_targets();
	create_render_pass();
	create_pipeline_cache();
	create_frame_buffers();
	// renderProgManager.Init();
}

void Vulkan::destroy() {

}

void Vulkan::create_instance() {

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Exile";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "Exile 0.2";
    appInfo.engineVersion = 2;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 1, VK_HEADER_VERSION);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
}

void Vulkan::create_surface(SDL_Window* window) {

}

void Vulkan::enumerate_physical_devices() {

}

void Vulkan::select_physical_device() {

}

void Vulkan::create_logical_device_and_queues() {

}

void Vulkan::create_semaphores() {

}

void Vulkan::create_commandPool() {

}

void Vulkan::create_commandBuffer() {

}

void Vulkan::create_swap_chain() {

}

void Vulkan::create_render_targets() {

}

void Vulkan::create_render_pass() {

}

void Vulkan::create_pipeline_cache() {

}

void Vulkan::create_frame_buffers() {

}

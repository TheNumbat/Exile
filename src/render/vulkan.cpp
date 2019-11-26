
#include "vulkan.h"
#include <SDL2/SDL_vulkan.h>

#define VK_CHECK(f) do {                \
	VkResult res = (f);                 \
	if (res != VK_SUCCESS) {	        \
		die("VK_CHECK: %", vk_err_str(res)); \
	}} while(0);                        \

static literal vk_err_str(VkResult errorCode) {
	switch (errorCode) {
#define STR(r) case VK_##r: return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}

static void* vk_alloc(void*, usize sz, usize, VkSystemAllocationScope) {
    return Vulkan::alloc::alloc<u8>(sz);
}

static void* vk_realloc(void*, void* mem, usize sz, usize, VkSystemAllocationScope) {
	Vulkan::alloc::dealloc(mem);
	return Vulkan::alloc::alloc<u8>(sz);
}

static void vk_free(void*, void* mem) {
    Vulkan::alloc::dealloc(mem);
}

void Vulkan::init(SDL_Window* window) {
    
    assert(window);

    create_instance(window);
	init_debug_callback();

	enumerate_gpus();
	select_gpu();

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
	destroy_debug_callback();
	inst_ext.destroy();
	dev_ext.destroy();
	layers.destroy();
	gpus.destroy();
	extensions.destroy();
}

static VkBool32 debug_callback( 
	VkDebugReportFlagsEXT flags, 
	VkDebugReportObjectTypeEXT obj_type, 
	u64 obj, usize loc, i32 code,
	const char* layerPrefix, const char* msg, void* userData) {

	switch(flags) {
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		info("VK (type=% obj=% loc=% code=%): %", obj_type, obj, loc, code, msg);
		break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		warn("VK (type=% obj=% loc=% code=%): %", obj_type, obj, loc, code, msg);
		break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		die("VK (type=% obj=% loc=% code=%): %", obj_type, obj, loc, code, msg);
		break;
	}
	return VK_FALSE;
}

void Vulkan::init_debug_callback() {

	VkDebugReportCallbackCreateInfoEXT callback_info = {};
	callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callback_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | 
						  VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | 
						  VK_DEBUG_REPORT_ERROR_BIT_EXT;
	callback_info.pfnCallback = (PFN_vkDebugReportCallbackEXT) &debug_callback;

	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if(!func) die("Could not find vkCreateDebugReportCallbackEXT");
	VK_CHECK(func( instance, &callback_info, null, &debug_callback_info));
}

void Vulkan::destroy_debug_callback() {

	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	if(!func) die("Could not find vkDestroyDebugReportCallbackEXT");
	func(instance, debug_callback_info, null);
}

void Vulkan::create_instance(SDL_Window* window) {

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Exile";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 2);
    app_info.pEngineName = "Exile 0.2";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 2);
    app_info.apiVersion = VK_MAKE_VERSION(1, 1, VK_HEADER_VERSION);

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
	
	inst_ext.clear(); dev_ext.clear(); layers.clear();
	
	inst_ext.push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	dev_ext.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	layers.push("VK_LAYER_LUNARG_standard_validation");

	u32 sdl_count;
	if(!SDL_Vulkan_GetInstanceExtensions(window, &sdl_count, null)) {
		die("Failed to get required SDL vk instance extensions: %", SDL_GetError());
	}
	inst_ext.extend(inst_ext.size + sdl_count);
	if(!SDL_Vulkan_GetInstanceExtensions(window, &sdl_count, inst_ext.data + inst_ext.size - sdl_count)) {
		die("Failed to get required SDL vk instance extensions: %", SDL_GetError());
	}

	create_info.enabledExtensionCount = inst_ext.size;
	create_info.ppEnabledExtensionNames = inst_ext.data;
	create_info.enabledLayerCount = layers.size;
	create_info.ppEnabledLayerNames = layers.data;

	VkAllocationCallbacks allocator = {};
	allocator.pfnAllocation = vk_alloc;
	allocator.pfnFree = vk_free;
	allocator.pfnReallocation = vk_realloc;

	VkResult res = vkCreateInstance(&create_info, &allocator, &instance);
	if(res != VK_SUCCESS) {
		die("Failed to create VkInstance: %", vk_err_str(res));
	}

	if(!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
		die("Failed to create SDL VkSurface: %", SDL_GetError());
	}

	info("Created Vulkan instance and surface.");

	u32 total_extensions = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(null, &total_extensions, null));
	extensions.clear();
	extensions.extend(total_extensions);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(null, &total_extensions, extensions.data));

	info("Available extensions:");
	for(auto& ext : extensions) {
		info("\t%", literal(ext.extensionName));
	}
}

void Vulkan::enumerate_gpus() {

	u32 devices = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &devices, null));
	if(devices <= 0) {
		die("Found no GPUs.");
	}

	vec<VkPhysicalDevice, alloc> phys_list;
	defer(phys_list.destroy());
	phys_list.extend(devices);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &devices, phys_list.data));

	gpus.clear();
	gpus.extend(devices);
	for(u32 i = 0; i < devices; i++) {
		GPU& gpu = gpus[i];
		gpu.device = phys_list[i];
		
		{
			u32 num_queues = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &num_queues, null);
			if(num_queues <= 0) warn("Found no device queues.");
			gpu.queue_families.extend(num_queues);
			vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &num_queues, gpu.queue_families.data);
		}
		{
			u32 num_exts = 0;
			VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.device, null, &num_exts, null));
			if(num_exts <= 0) warn("Found no device extensions.");
			gpu.exts.extend(num_exts);
			VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.device, null, &num_exts, gpu.exts.data));
		}
		{
			VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.device, surface, &gpu.surf_caps));
		}
		{
			u32 num_fmts = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, surface, &num_fmts, null));
			if(num_fmts <= 0) warn("Found no device surface formats.");
			gpu.fmts.extend(num_fmts);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, surface, &num_fmts, gpu.fmts.data));
		}
		{
			u32 num_modes = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, surface, &num_modes, null));
			if(num_modes <= 0) warn("Found no device present modes.");
			gpu.modes.extend(num_modes);
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, surface, &num_modes, gpu.modes.data));
		}
		{
			vkGetPhysicalDeviceMemoryProperties(gpu.device, &gpu.mem_prop);
		}
		{
			vkGetPhysicalDeviceProperties(gpu.device, &gpu.dev_prop);
		}
	}
}

void Vulkan::select_gpu() {

	for(GPU& gpu : gpus) {
		
		i32 graphics_idx = -1, present_idx = -1;
	
	}
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

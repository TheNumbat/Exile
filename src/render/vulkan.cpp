
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
	create_command_pool();
	create_command_buffers();
	create_swap_chain();
	create_render_targets();
	create_render_pass();
	create_pipeline_cache();
	create_frame_buffers();
}

void Vulkan::destroy() {
	
	for(i32 i = 0; i < BUF_FRAMES; i++) {
		vkDestroySemaphore(device, aquire_sem[i], &allocator);
		vkDestroySemaphore(device, complete_sem[i], &allocator);
		vkDestroyFence(device, buf_fence[i], &allocator);
	}
	vkDestroyDevice(device, &allocator);
	vkDestroySurfaceKHR(instance, surface, &allocator);
	vkDestroyInstance(instance, &allocator);

	destroy_debug_callback();
	inst_ext.destroy();
	dev_ext.destroy();
	layers.destroy();
	gpus.destroy();
	extensions.destroy();

	gpu = null;
	device = {};
	instance = {};
	surface = {};
	graphics_queue = {}, present_queue = {};
	command_pool = {};
	for(i32 i = 0; i < BUF_FRAMES; i++) {
		command_buffer[i] = {};
		buf_fence[i] = {};
		aquire_sem[i] = {};
		complete_sem[i] = {};
	}
	allocator = {};
	debug_callback_info = {};
}

static VkBool32 debug_callback( 
	VkDebugReportFlagsEXT flags, 
	VkDebugReportObjectTypeEXT obj_type, 
	u64 obj, usize loc, i32 code,
	const char* layerPrefix, const char* msg, void* userData) {

	switch(flags) {
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		info("VK (type=% obj=% loc=% code=%): %", (i64)obj_type, obj, loc, code, msg);
		break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		warn("VK (type=% obj=% loc=% code=%): %", (i64)obj_type, obj, loc, code, msg);
		break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		die("VK (type=% obj=% loc=% code=%): %", (i64)obj_type, obj, loc, code, msg);
		break;
	}
	return VK_FALSE;
}

void Vulkan::init_debug_callback() {

	VkDebugReportCallbackCreateInfoEXT callback_info = {};
	callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
						  VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callback_info.pfnCallback = (PFN_vkDebugReportCallbackEXT) &debug_callback;

	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if(!func) die("Could not find vkCreateDebugReportCallbackEXT");
	VK_CHECK(func(instance, &callback_info, null, &debug_callback_info));
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

	u32 total_extensions = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(null, &total_extensions, null));
	extensions.clear();
	extensions.extend(total_extensions);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(null, &total_extensions, extensions.data));

	info("Created Vulkan instance and surface. Extensions:");
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

	info("Enumerated Vulkan devices:");
	for(GPU& gpu : gpus) {
		info("\t%", literal(gpu.dev_prop.deviceName));
	}
}

bool Vulkan::GPU::supports(const vec<const char*, Vulkan::alloc>& extensions) {

	i32 matched = 0;

	for(auto e : extensions) {
		for(auto& p : exts) {
			if(strcmp(e, p.extensionName) == 0) {
				matched++;
				break;
			}
		}
	}

	return matched == extensions.size;
}

void Vulkan::select_gpu() {

	for(GPU& gpu : gpus) {
		
		literal name = gpu.dev_prop.deviceName;
		gpu.graphics_idx = -1;
		gpu.present_idx = -1;

		if(!gpu.supports(dev_ext)) {
			info("Device % does not support device extensions.", name);
			continue;
		}

		if(!gpu.fmts.size) {
			warn("Device % has no available surface formats.", name);
			continue;
		}
		if(!gpu.modes.size) {
			warn("Device % has no available present modes.", name);
			continue;
		}

		for(i32 i = 0; i < gpu.queue_families.size; i++) {
			auto& family = gpu.queue_families[i];
			if(!family.queueCount) continue;
			if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				gpu.graphics_idx = i;
				break;
			}
		}

		for(i32 i = 0; i < gpu.queue_families.size; i++) {
			auto& family = gpu.queue_families[i];
			if(!family.queueCount) continue;

			VkBool32 supports_present = VK_FALSE;
			VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu.device, i, surface, &supports_present));
			if(supports_present) {
				gpu.present_idx = i;
				break;
			}
		}

		if(gpu.graphics_idx >= 0 && gpu.present_idx >= 0) {
			info("Selecting GPU: %", name);
			this->gpu = &gpu;
			return;
		}

		info("Device % does not have a suitable graphics and present queue.", name);
	}

	die("Failed to find compatible Vulkan device.");
}

void Vulkan::create_logical_device_and_queues() {

	static const float priority = 1.0f;
	vec<VkDeviceQueueCreateInfo, alloc> q_info;
	defer(q_info.destroy());

	{
		VkDeviceQueueCreateInfo qinfo = {};
		qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qinfo.queueFamilyIndex = gpu->graphics_idx;
		qinfo.queueCount = 1;
		qinfo.pQueuePriorities = &priority;
		q_info.push(qinfo);
	}
	if(gpu->present_idx != gpu->graphics_idx) {
		VkDeviceQueueCreateInfo qinfo = {};
		qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qinfo.queueFamilyIndex = gpu->present_idx;
		qinfo.queueCount = 1;
		qinfo.pQueuePriorities = &priority;
		q_info.push(qinfo);
	}

	// TODO(max): figure out what device features we need to enable
	VkPhysicalDeviceFeatures features = {};
	
	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.queueCreateInfoCount = q_info.size;
	info.pQueueCreateInfos = q_info.data;
	info.pEnabledFeatures = &features;
	info.enabledExtensionCount = dev_ext.size;
	info.ppEnabledExtensionNames = dev_ext.data;
	info.enabledLayerCount = layers.size;
	info.ppEnabledLayerNames = layers.data;

	VK_CHECK(vkCreateDevice(gpu->device, &info, &allocator, &device));

	vkGetDeviceQueue(device, gpu->graphics_idx, 0, &graphics_queue);
	vkGetDeviceQueue(device, gpu->present_idx, 0, &present_queue);

	info("Created device and queues.");
}

void Vulkan::create_semaphores() {

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for(i32 i = 0; i < BUF_FRAMES; i++) {
		VK_CHECK(vkCreateSemaphore(device, &info, &allocator, &aquire_sem[i]));
		VK_CHECK(vkCreateSemaphore(device, &info, &allocator, &complete_sem[i]));
	}

	info("Created semaphores.");
}

void Vulkan::create_command_pool() {

	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	info.queueFamilyIndex = gpu->graphics_idx;

	VK_CHECK(vkCreateCommandPool(device, &info, &allocator, &command_pool));

	info("Created command pool.");
}

void Vulkan::create_command_buffers() {

	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandPool = command_pool;
	info.commandBufferCount = BUF_FRAMES;

	VK_CHECK(vkAllocateCommandBuffers(device, &info, command_buffer));

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	for (i32 i = 0; i < BUF_FRAMES; i++) {
		VK_CHECK(vkCreateFence(device, &fence_info, &allocator, &buf_fence[i]));
	}

	info("Created command buffers and fences.");
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

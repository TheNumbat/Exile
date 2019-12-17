
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
	create_swap_chain(window);

	create_shaders();

	create_render_targets();
	create_render_pass();
	create_frame_buffers(window);
}

void Vulkan::destroy() {
	
	vkDestroyImageView(device, depth_view, null);
	for(i32 i = 0; i < BUF_FRAMES; i++) {
		vkDestroyImageView(device, image_views[i], null);
		vkDestroyFramebuffer(device, framebuffers[i], null);
		vkDestroySemaphore(device, aquire_sem[i], null);
		vkDestroySemaphore(device, complete_sem[i], null);
		vkDestroyFence(device, buf_fence[i], null);
	}

	vkDestroySwapchainKHR(device, swapchain, null);
	vkDestroyCommandPool(device, command_pool, null);
	vkDestroyDevice(device, null);
	vkDestroySurfaceKHR(instance, surface, null);
	destroy_debug_callback();

	vkDestroyInstance(instance, null);

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
	swapchain = {};
	swapchain_format = {};
	present_mode = {};
	extent = {};
	depth_format = {};
	render_pass = {};
	depth_view = {};
	for(i32 i = 0; i < BUF_FRAMES; i++) {
		framebuffers[i] = {};
		image_views[i] = {};
		command_buffer[i] = {};
		buf_fence[i] = {};
		aquire_sem[i] = {};
		complete_sem[i] = {};
	}
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
    app_info.pEngineName = "Exile";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 2);
    app_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
	
	inst_ext.clear(); dev_ext.clear(); layers.clear();
	
	inst_ext.push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	dev_ext.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	layers.push("VK_LAYER_LUNARG_standard_validation");

	u32 sdl_count = 0;
	if(!SDL_Vulkan_GetInstanceExtensions(window, &sdl_count, null)) {
		die("Failed to get required SDL vk instance extensions: %", SDL_GetError());
	}
	inst_ext.extend(sdl_count);
	if(!SDL_Vulkan_GetInstanceExtensions(window, &sdl_count, inst_ext.data + inst_ext.size - sdl_count)) {
		die("Failed to get required SDL vk instance extensions: %", SDL_GetError());
	}

	create_info.enabledExtensionCount = inst_ext.size;
	create_info.ppEnabledExtensionNames = inst_ext.data;
	create_info.enabledLayerCount = layers.size;
	create_info.ppEnabledLayerNames = layers.data;

	VkResult res = vkCreateInstance(&create_info, null, &instance);
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

	info("Created Vulkan instance and surface.");
}

void Vulkan::enumerate_gpus() {

	u32 devices = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &devices, null));
	if(!devices) {
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
			if(!num_exts) warn("Found no device extensions.");
			gpu.exts.extend(num_exts);
			VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.device, null, &num_exts, gpu.exts.data));
		}
		{
			VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.device, surface, &gpu.surf_caps));
		}
		{
			u32 num_fmts = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, surface, &num_fmts, null));
			if(!num_fmts) warn("Found no device surface formats.");
			gpu.fmts.extend(num_fmts);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, surface, &num_fmts, gpu.fmts.data));
		}
		{
			u32 num_modes = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, surface, &num_modes, null));
			if(!num_modes) warn("Found no device present modes.");
			gpu.modes.extend(num_modes);
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, surface, &num_modes, gpu.modes.data));
		}
		{
			vkGetPhysicalDeviceMemoryProperties(gpu.device, &gpu.mem_prop);
			vkGetPhysicalDeviceFeatures(gpu.device, &gpu.features);
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
			// TODO(max): choose best GPU out of all compatible GPUs (e.g. discrete GPU)
			info("Selecting GPU: %", name);
			this->gpu = &gpu;
			return;
		}

		info("Device % does not have a suitable graphics and present queue.", name);
	}

	die("Failed to find compatible Vulkan device.");
}

void Vulkan::create_logical_device_and_queues() {

	const float priority = 1.0f;
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

	VK_CHECK(vkCreateDevice(gpu->device, &info, null, &device));

	vkGetDeviceQueue(device, gpu->graphics_idx, 0, &graphics_queue);
	vkGetDeviceQueue(device, gpu->present_idx, 0, &present_queue);

	info("Created device and queues.");
}

void Vulkan::create_semaphores() {

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for(i32 i = 0; i < BUF_FRAMES; i++) {
		VK_CHECK(vkCreateSemaphore(device, &info, null, &aquire_sem[i]));
		VK_CHECK(vkCreateSemaphore(device, &info, null, &complete_sem[i]));
	}

	info("Created semaphores.");
}

void Vulkan::create_command_pool() {

	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	info.queueFamilyIndex = gpu->graphics_idx;

	VK_CHECK(vkCreateCommandPool(device, &info, null, &command_pool));

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

	for(i32 i = 0; i < BUF_FRAMES; i++) {
		VK_CHECK(vkCreateFence(device, &fence_info, null, &buf_fence[i]));
	}

	info("Created command buffers and fences.");
}

static VkSurfaceFormatKHR choose_format(vec<VkSurfaceFormatKHR, Vulkan::alloc> formats) {
	
	VkSurfaceFormatKHR result;
	
	if(formats.size == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		result.format = VK_FORMAT_B8G8R8A8_UNORM;
		result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		warn("Undefined surface format give, using default.");
		return result;
	}
	for (u32 i = 0; i < formats.size; i++) {
		VkSurfaceFormatKHR& fmt = formats[i];
		if(fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			info("Found suitable surface format.");
			return fmt;
		}
	}

	warn("Suitable surface format not found!");
	return formats[0];
}

static VkPresentModeKHR choose_present_mode(vec<VkPresentModeKHR, Vulkan::alloc> modes) {

	const VkPresentModeKHR desiredMode = VK_PRESENT_MODE_MAILBOX_KHR;

	for (u32 i = 0; i < modes.size; i++) {
		if (modes[i] == desiredMode) {
			info("Found mailbox present mode.");
			return desiredMode;
		}
	}
	warn("Mailbox present not found, using FIFO.");
	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_surface_extent(SDL_Window* window, VkSurfaceCapabilitiesKHR caps) {
	
	VkExtent2D extent;
	if(caps.currentExtent.width == -1) {
		i32 w, h;
		SDL_GetWindowSize(window, &w, &h);
		extent.width = w;
		extent.height = h;
	} else {
		extent = caps.currentExtent;
	}
	return extent;
}

void Vulkan::create_swap_chain(SDL_Window* window) {

	swapchain_format = choose_format(gpu->fmts);
	present_mode = choose_present_mode(gpu->modes);
	extent = choose_surface_extent(window, gpu->surf_caps);

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;

	info.minImageCount = BUF_FRAMES;
	info.imageFormat = swapchain_format.format;
	info.imageColorSpace = swapchain_format.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if(gpu->graphics_idx != gpu->present_idx) {
		
		u32 indices[] = {(u32)gpu->graphics_idx, (u32)gpu->present_idx};

		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = indices;

	} else {

		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = present_mode;
	info.clipped = VK_TRUE;

	VK_CHECK(vkCreateSwapchainKHR(device, &info, null, &swapchain));

	u32 images = 0;
	vec<VkImage, alloc> swapchain_images;
	defer(swapchain_images.destroy());
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &images, null));
	if(!images) {
		die("Failed to get any images from vk swapchain!");
	}

	swapchain_images.extend(images);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &images, swapchain_images.data));
	if(!images) {
		die("Failed to get any images from vk swapchain!");
	}

	for(u32 i = 0; i < BUF_FRAMES; i++) {

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = swapchain_images[i];
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = swapchain_format.format;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;
		info.flags = 0;
		
		VkImageView view = {};
		VK_CHECK(vkCreateImageView(device, &info, null, &view));
		image_views[i] = view;
	}

	info("Created swapchain and images.");
}

void Vulkan::create_shaders() {

	
}

VkFormat Vulkan::choose_supported_format(VkFormat* formats, i32 n_formats, VkImageTiling tiling, VkFormatFeatureFlags features) {
	
	for(i32 i = 0; i < n_formats; i++) {
		
		VkFormat format = formats[i];
		VkFormatProperties props = {};
		vkGetPhysicalDeviceFormatProperties(gpu->device, format, &props);

		if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	die("Failed to find suitable VkFormat.");
	return VK_FORMAT_UNDEFINED;
}

void Vulkan::create_render_targets() {

	assert(!"unimplemented");

	VkFormat formats[] = {
		VK_FORMAT_D32_SFLOAT_S8_UINT, 
		VK_FORMAT_D24_UNORM_S8_UINT 
	};

	depth_format = choose_supported_format(formats, 2, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depth_view = VK_NULL_HANDLE;

	// VkImageViewCreateInfo info = {};
	// info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	// info.image = m_image;
	// info.viewType = ( m_opts.textureType == TT_CUBIC ) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	// info.format = m_internalFormat;
	// info.components = VK_GetComponentMappingFromTextureFormat( m_opts.format, m_opts.colorFormat );
	// info.subresourceRange.aspectMask = ( m_opts.format == FMT_DEPTH ) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	// info.subresourceRange.levelCount = m_opts.numLevels;
	// info.subresourceRange.layerCount = ( m_opts.textureType == TT_CUBIC ) ? 6 : 1;
	// info.subresourceRange.baseMipLevel = 0;
	
	// VK_CHECK(vkCreateImageView(device, &info, null, &depth_view));	
}

void Vulkan::create_render_pass() {

	assert(!"unimplemented");

	vec<VkAttachmentDescription, alloc> attachments;
	defer(attachments.destroy());

	VkAttachmentDescription color = {};
	color.format = swapchain_format.format;
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments.push(color);

	VkAttachmentDescription depth = {};
	depth.format = depth_format;
	depth.samples = VK_SAMPLE_COUNT_1_BIT;
	depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments.push(depth);

	VkAttachmentReference color_ref = {};
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_ref = {};
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;
	subpass.pDepthStencilAttachment = &depth_ref;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = attachments.size;
	info.pAttachments = attachments.data;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 0;

	VK_CHECK(vkCreateRenderPass(device, &info, null, &render_pass));
	
	info("Created render pass.");
}

void Vulkan::create_frame_buffers(SDL_Window* window) {

	assert(!"unimplemented");

	VkImageView attachments[2] = {};

	attachments[1] = depth_view; // TODO(max): ?????????

	i32 w, h;
	SDL_GetWindowSize(window, &w, &h);

	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = render_pass;
	info.attachmentCount = 2;
	info.pAttachments = attachments;
	info.width = w;
	info.height = h;
	info.layers = 1;

	for(i32 i = 0; i < BUF_FRAMES; i++) {
		attachments[0] = image_views[i];
		VK_CHECK(vkCreateFramebuffer(device, &info, null, &framebuffers[i]));
	}

	info("Created framebuffers.");
}

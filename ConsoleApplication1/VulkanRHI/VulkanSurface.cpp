#include "VulkanSurface.h"
#include "VulkanRHI/VulkanRHI.h"
#include "Core/Display/Window/Window.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"


void global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	SCVulkanSurface* pointer = (SCVulkanSurface*)glfwGetWindowUserPointer(window);
	if(pointer->key_callback)
	{
		pointer->key_callback(key, scancode, action, mods);
	}
}

void global_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCVulkanSurface* pointer = (SCVulkanSurface*)glfwGetWindowUserPointer(window);
	pointer->onresize(width, height);
}


void global_mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	SCVulkanSurface* pointer = (SCVulkanSurface*)glfwGetWindowUserPointer(window);
	if(pointer->mouse_callback)
	{
		pointer->mouse_callback(xposIn, yposIn);
	}
}

void global_window_close(GLFWwindow* window)
{
	SCVulkanSurface* pointer = (SCVulkanSurface*)glfwGetWindowUserPointer(window);
	pointer->uninit();
}

void SCVulkanSurface::onresize(uint32_t width, uint32_t height)
{
	while (m_init_flag.test_and_set())
	{
	}
	if (pre_resize_callback)
	{
		pre_resize_callback(cur_width, cur_height);
	}
	cur_height = height;
	cur_width = width;

	if (width && height)
	{


		if(swapchain != VK_NULL_HANDLE)
		{
			
		}


		VkSwapchainKHR new_swapchain = VK_NULL_HANDLE;
		

		
		VkSurfaceCapabilitiesKHR surface_capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_rhi->get_physical_device(), surface, &surface_capabilities);

		VkExtent2D new_extent;
		new_extent.width = width;
		new_extent.height = height;

		VkSwapchainCreateInfoKHR swapchain_create_info{};

		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.flags = 0;
		swapchain_create_info.pNext = nullptr;
		swapchain_create_info.surface = surface;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.minImageCount = 2;
		swapchain_create_info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_create_info.imageExtent = new_extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queue_families[] = { vulkan_rhi->get_graphics_queue_family_index(), vulkan_rhi->get_presentation_queue_family_index() };
		if (vulkan_rhi->get_presentation_queue() != vulkan_rhi->get_graphics_queue())
		{
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = 2;
			swapchain_create_info.pQueueFamilyIndices = queue_families;
		}
		else
		{
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount = 0;
			swapchain_create_info.pQueueFamilyIndices = nullptr;
		}
		swapchain_create_info.preTransform = surface_capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchain_create_info.oldSwapchain = swapchain;

		vkCreateSwapchainKHR(vulkan_rhi->get_device(), &swapchain_create_info, vulkan_rhi->get_allocator(), &new_swapchain);


		if(swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(vulkan_rhi->get_device(), swapchain, vulkan_rhi->get_allocator());

		}
		swapchain = new_swapchain;
	}

	if(resize_callback)
	{
		resize_callback(cur_width, cur_height);
	}
	m_init_flag.clear();
}

void SCVulkanSurface::internal_uninit()
{
	while (m_init_flag.test_and_set())
	{
	}
	if(close_callback)
	{
		close_callback();
	}
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(vulkan_rhi->get_device(), swapchain, vulkan_rhi->get_allocator());
		swapchain = VK_NULL_HANDLE;
		vkDestroySurfaceKHR(vulkan_rhi->get_instance(), surface, vulkan_rhi->get_allocator());
		surface = VK_NULL_HANDLE;
		glfwDestroyWindow(window);
		window = nullptr;
	}
	vulkan_rhi = nullptr;
	m_init_flag.clear();
}

SCVulkanSurface::~SCVulkanSurface()
{
	internal_uninit();
}

void SCVulkanSurface::init(SSPtr<SCRHIInterface> rhi, uint32_t width, uint32_t height)
{
	vulkan_rhi = rhi.as<SCVulkanRHI>();
	constexpr uint32_t default_width = 512;
	constexpr uint32_t default_height = 512;
	
	resize(width <= 0 ? default_width : width, height <= 0 ? default_height : height);
	//VK_FORMAT_R8G8B8A8_SRGB;
}

void SCVulkanSurface::resize(uint32_t width, uint32_t height)
{
	if(window == nullptr)
	{
		window = glfwCreateWindow(width, height, "test window", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwCreateWindowSurface(vulkan_rhi->get_instance(), window, vulkan_rhi->get_allocator(), &surface);
		onresize(width, height);
		glfwSetFramebufferSizeCallback(window, global_framebuffer_size_callback);
		glfwSetCursorPosCallback(window, global_mouse_callback);
		glfwSetKeyCallback(window, global_key_callback);
		glfwSetWindowCloseCallback(window, global_window_close);
		glfwShowWindow(window);
	}
	else
	{
		glfwSetWindowSize(window, width, height);
	}

	
}

void SCVulkanSurface::uninit()
{
	internal_uninit();
}

bool SCVulkanSurface::is_valid()
{
	return swapchain != VK_NULL_HANDLE;
}

void SCVulkanSurface::set_title(const std::string& in_title)
{
	if(is_valid())
	{
		glfwSetWindowTitle(window, in_title.c_str());
	}
}

#include "VulkanRHI.h"
#include "vector"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"


void SCVulkanRHI::internal_uninit()
{
    if (m_status == SERHIStatus::Inited || m_status == SERHIStatus::InitFaild)
    {
        while (m_init_flag.test_and_set())
        {
        }
        if ((m_status == SERHIStatus::Inited || m_status == SERHIStatus::InitFaild))
        {
            m_status = SERHIStatus::Uniniting;

            if (m_device != VK_NULL_HANDLE)
            {
                vkDestroyDevice(m_device, m_memory);
                m_device = VK_NULL_HANDLE;
            }
            if (m_instance != VK_NULL_HANDLE)
            {
                vkDestroyInstance(m_instance, m_memory);
                m_instance = VK_NULL_HANDLE;
            }
            m_status = SERHIStatus::NotInit;
        }
        m_init_flag.clear();
    }
}

SCVulkanRHI::~SCVulkanRHI()
{
    internal_uninit();
}

void SCVulkanRHI::init()
{
    if(m_status == SERHIStatus::NotInit)
    {
	    while (m_init_flag.test_and_set())
	    {}
        if (m_status == SERHIStatus::NotInit)
        {
            m_status = SERHIStatus::Initing;

            glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
            glfwInit();

            VkApplicationInfo vk_application_info;
            vk_application_info.pNext = nullptr;
            vk_application_info.apiVersion = VK_API_VERSION_1_3;
            vk_application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
            vk_application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
            vk_application_info.pApplicationName = "Test App";
            vk_application_info.pEngineName = "Test Engine";
            vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

            uint32_t instance_extension_count = 0;
            const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);

            VkInstanceCreateInfo vk_instance_create_info;
            vk_instance_create_info.pNext = nullptr;
            vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            vk_instance_create_info.enabledExtensionCount = instance_extension_count;
            vk_instance_create_info.ppEnabledExtensionNames = instance_extensions;
#if SM_DEBUG
            std::vector<const char*> validit_layers = {
                "VK_LAYER_KHRONOS_validation"
            };
            vk_instance_create_info.enabledLayerCount = validit_layers.size();
            vk_instance_create_info.ppEnabledLayerNames = validit_layers.data();
#else
            vk_instance_create_info.enabledLayerCount = 0;
            vk_instance_create_info.ppEnabledLayerNames = nullptr;
#endif
            vk_instance_create_info.flags = 0;
            vk_instance_create_info.pApplicationInfo = &vk_application_info;


            VkResult instance_res = vkCreateInstance(&vk_instance_create_info, m_memory, &m_instance);


            uint32_t physical_device_count = 0;
            vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);
            std::vector<VkPhysicalDevice> physical_devices;
            physical_devices.resize(physical_device_count);
            vkEnumeratePhysicalDevices(m_instance, &physical_device_count, physical_devices.data());

            physical_device_to_use = physical_devices[0];

            VkPhysicalDeviceProperties physical_device_properties;
            vkGetPhysicalDeviceProperties(physical_device_to_use, &physical_device_properties);

            uint32_t physical_device_queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device_to_use, &physical_device_queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_family_propertieses;
            queue_family_propertieses.resize(physical_device_queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device_to_use, &physical_device_queue_family_count, queue_family_propertieses.data());

            VkPhysicalDeviceFeatures vk_physical_device_features;
            vkGetPhysicalDeviceFeatures(physical_device_to_use, &vk_physical_device_features);

            std::vector<uint32_t> family_indices_graphics;
            for (uint32_t i = 0; i < queue_family_propertieses.size(); ++i)
            {
                auto& queue_family_properties = queue_family_propertieses[i];
                if (queue_family_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
                {
                    family_indices_graphics.push_back(i);
                }
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
            GLFWwindow* window = glfwCreateWindow(800, 600, "temp window", nullptr, nullptr);
            VkSurfaceKHR surface_khr;
            glfwCreateWindowSurface(m_instance, window, m_memory, &surface_khr);

            /*uint32_t queue_family_index_graphics_to_use;
            uint32_t queue_family_index_presentation_to_use = 0xffffffff;*/

            bool b_graphics_family_queue_can_presentation = false;
            //most likely
            for(auto&& family_index_graphics : family_indices_graphics)
            {
                VkBool32 b_supported = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_to_use, family_index_graphics, surface_khr, &b_supported);
                if(b_supported)
                {
                    queue_family_index_graphics_to_use = family_index_graphics;
                    queue_family_index_presentation_to_use = family_index_graphics;
                    b_graphics_family_queue_can_presentation = true;
                    break;
                }
            }

            if(!b_graphics_family_queue_can_presentation)
            {
                queue_family_index_graphics_to_use = family_indices_graphics[0];
                for(uint32_t family_index = 0; family_index < queue_family_propertieses.size(); ++family_index)
                {
                    VkBool32 b_supported = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_to_use, family_index, surface_khr, &b_supported);
                    if(b_supported)
                    {
                        queue_family_index_presentation_to_use = family_index;
                        break;
                    }
                }
            }
            

            VkResult device_create_res;
            if(b_graphics_family_queue_can_presentation || queue_family_index_presentation_to_use == 0xffffffff) //graphics queue family can present or there is no presentation queue family
            {
                std::vector<const char*> device_extension_names;
                uint32_t device_extension_count = 0;
                if(queue_family_index_presentation_to_use != 0xffffffff)
                {
                    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                    device_extension_count = device_extension_names.size();
                }

                VkDeviceCreateInfo vk_device_create_info;
                vk_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                vk_device_create_info.flags = 0;
                vk_device_create_info.pNext = nullptr;
                vk_device_create_info.enabledExtensionCount = device_extension_count;
                vk_device_create_info.ppEnabledExtensionNames = device_extension_names.data();
                vk_device_create_info.enabledLayerCount = 0;
                vk_device_create_info.ppEnabledLayerNames = nullptr;
                vk_device_create_info.pEnabledFeatures = &vk_physical_device_features;

                float priority = 1.0f;
                VkDeviceQueueCreateInfo vk_device_queue_create_info;
                vk_device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                vk_device_queue_create_info.flags = 0;
                vk_device_queue_create_info.pNext = nullptr;
                vk_device_queue_create_info.queueFamilyIndex = queue_family_index_graphics_to_use;
                vk_device_queue_create_info.queueCount = 1;
                vk_device_queue_create_info.pQueuePriorities = &priority;

                vk_device_create_info.queueCreateInfoCount = 1;
                vk_device_create_info.pQueueCreateInfos = &vk_device_queue_create_info;

                device_create_res = vkCreateDevice(physical_device_to_use, &vk_device_create_info, m_memory, &m_device);
            }
            else
            {
                VkDeviceCreateInfo vk_device_create_info;
                vk_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                vk_device_create_info.flags = 0;
                vk_device_create_info.pNext = nullptr;
                vk_device_create_info.enabledExtensionCount = 0;
                vk_device_create_info.ppEnabledExtensionNames = nullptr;
                vk_device_create_info.enabledLayerCount = 0;
                vk_device_create_info.ppEnabledLayerNames = nullptr;
                vk_device_create_info.pEnabledFeatures = &vk_physical_device_features;

                float priority = 1.0f;
                VkDeviceQueueCreateInfo vk_device_queue_create_info[2];
                vk_device_queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                vk_device_queue_create_info[0].flags = 0;
                vk_device_queue_create_info[0].pNext = nullptr;
                vk_device_queue_create_info[0].queueFamilyIndex = queue_family_index_graphics_to_use;
                vk_device_queue_create_info[0].queueCount = 1;
                vk_device_queue_create_info[0].pQueuePriorities = &priority;

                vk_device_queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                vk_device_queue_create_info[1].flags = 0;
                vk_device_queue_create_info[1].pNext = nullptr;
                vk_device_queue_create_info[1].queueFamilyIndex = queue_family_index_presentation_to_use;
                vk_device_queue_create_info[1].queueCount = 1;
                vk_device_queue_create_info[1].pQueuePriorities = &priority;

                vk_device_create_info.queueCreateInfoCount = 2;
                vk_device_create_info.pQueueCreateInfos = vk_device_queue_create_info;

                device_create_res = vkCreateDevice(physical_device_to_use, &vk_device_create_info, m_memory, &m_device);
            }

            vkGetDeviceQueue(m_device, queue_family_index_graphics_to_use, 0, &m_graphics_queue);
            if(queue_family_index_presentation_to_use != 0xffffffff)
            {
                vkGetDeviceQueue(m_device, queue_family_index_presentation_to_use, 0, &m_presentation_queue);
            }

            VkSurfaceCapabilitiesKHR surface_capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_to_use, surface_khr, &surface_capabilities);
            uint32_t surface_formats_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_to_use, surface_khr, &surface_formats_count, nullptr);
            std::vector<VkSurfaceFormatKHR> surface_formats;
            surface_formats.resize(surface_formats_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_to_use, surface_khr, &surface_formats_count, surface_formats.data());

            uint32_t presentation_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_to_use, surface_khr, &presentation_mode_count, nullptr);
            std::vector<VkPresentModeKHR> present_modes;
            present_modes.resize(presentation_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_to_use, surface_khr, &presentation_mode_count, present_modes.data());

            vkDestroySurfaceKHR(m_instance, surface_khr, m_memory);
            glfwDestroyWindow(window);
            surface_khr = VK_NULL_HANDLE;
            window = nullptr;
            if(instance_res == VK_SUCCESS && device_create_res == VK_SUCCESS)
            {
                m_status = SERHIStatus::Inited;
            }
            else
            {
                m_status = SERHIStatus::InitFaild;
            }
        }
        m_init_flag.clear();
    }
}

void SCVulkanRHI::uninit()
{
    internal_uninit();
}

SERHIStatus SCVulkanRHI::status() const
{
    return m_status;
}

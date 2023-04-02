#include "VulkanRHI.h"
#include "vector"
#define GLFW_INCLUDE_VULKAN
#include <iostream>

#include "GLFW/glfw3.h"


SCRHIVulkanDeviceFence::SCRHIVulkanDeviceFence(SSPtr<SCRHIInterface> InRhi)
{
    rhi = InRhi.as<SCVulkanRHI>();
    VkFenceCreateInfo fence_create{};
    fence_create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create.pNext = nullptr;
    fence_create.flags = 0;
    vkCreateFence(rhi->get_device(), &fence_create, rhi->get_allocator(), &m_fence);
}

SCRHIVulkanDeviceFence::~SCRHIVulkanDeviceFence()
{
    vkDestroyFence(rhi->get_device(), m_fence, rhi->get_allocator());
}

bool SCRHIVulkanDeviceFence::IsReady() const
{
     return vkGetFenceStatus(rhi->get_device(), m_fence) == VK_SUCCESS;
}

void SCRHIVulkanDeviceFence::Reset()
{
    vkResetFences(rhi->get_device(), 1, &m_fence);
}

SCRHIVulkanDeviceSemaphore::SCRHIVulkanDeviceSemaphore(SSPtr<SCRHIInterface> InRhi)
{
    rhi = InRhi.as<SCVulkanRHI>();
    VkSemaphoreCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.flags = 0;
    info.pNext = nullptr;
    vkCreateSemaphore(rhi->get_device(), &info, rhi->get_allocator(), &m_semaphore);
}

SCRHIVulkanDeviceSemaphore::~SCRHIVulkanDeviceSemaphore()
{
    vkDestroySemaphore(rhi->get_device(), m_semaphore, rhi->get_allocator());
}

void SCVulkanCommandBuffer::Init(const SSPtr<SCRHIInterface>& InRhi, SECommandBufferLifeType InLifeType)
{
    m_fence = SSPtr<SCRHIVulkanDeviceFence>::construct<SCRHIVulkanDeviceFence>(InRhi);
    Fence = m_fence;
    command_buffer_life = InLifeType;
    rhi = InRhi.as<SCVulkanRHI>();
    auto&& loc_rhi = rhi.lock();
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandBufferCount = 1;
    info.commandPool = loc_rhi->get_graphics_pool();
    info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if(vkAllocateCommandBuffers(loc_rhi->get_device(), &info, &m_command_buffer) == VK_SUCCESS)
    {
        command_buffer_status = SECommandBufferStatus::Initial;
    }
}

void SCVulkanCommandBuffer::on_begin_record()
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.pInheritanceInfo = VK_NULL_HANDLE;
    begin_info.flags = (command_buffer_life == SECommandBufferLifeType::ExecuteOnce ? VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);

    vkBeginCommandBuffer(m_command_buffer, &begin_info);
}

void SCVulkanCommandBuffer::on_end_record()
{
    vkEndCommandBuffer(m_command_buffer);
}

void SCVulkanCommandBuffer::on_submit(const std::vector<SSRHICommandBufferWaitInfo>& InWaitInfo, const std::vector<SSRHICommandBufferTriggerInfo>& InTriggerInfo)
{
    auto&& loc_rhi = rhi.lock();
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer;
    submit_info.pNext = nullptr;

    std::vector<VkPipelineStageFlags> stage_masks;
    stage_masks.reserve(InWaitInfo.size());
    std::vector<VkSemaphore> stage_semaphores;
    stage_semaphores.reserve(InWaitInfo.size());

    for(auto&& waits : InWaitInfo)
    {
        stage_masks.push_back(waits.m_flags);
        stage_semaphores.push_back(waits.m_semaphore.as<SCRHIVulkanDeviceSemaphore>()->GetInnerSemaphore());
    }
    submit_info.waitSemaphoreCount = stage_masks.size();
    if(submit_info.waitSemaphoreCount > 0)
    {
        submit_info.pWaitDstStageMask = stage_masks.data();
        submit_info.pWaitSemaphores = stage_semaphores.data();
    }
    else
    {
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.pWaitSemaphores = nullptr;
    }

    std::vector<VkSemaphore> trigger_semaphores;
    trigger_semaphores.reserve(InTriggerInfo.size());
    for(auto&& trigger : InTriggerInfo)
    {
        trigger_semaphores.push_back(trigger.m_semaphore.as<SCRHIVulkanDeviceSemaphore>()->GetInnerSemaphore());
    }

    submit_info.signalSemaphoreCount = trigger_semaphores.size();
    if(submit_info.signalSemaphoreCount > 0)
    {
        submit_info.pSignalSemaphores = trigger_semaphores.data();
    }
    else
    {
        submit_info.pSignalSemaphores = nullptr;
    }
    vkQueueSubmit(loc_rhi->get_graphics_queue(), 1, &submit_info, m_fence->GetInnerFence());
}

void SCVulkanCommandBuffer::wait_until_finish(uint64_t InOutTime)
{
    if(get_status() == SECommandBufferStatus::Pending)
    {
        auto&& loc_rhi = rhi.lock();
        vkWaitForFences(loc_rhi->get_device(), 1, &m_fence->GetInnerFence(), VK_TRUE, InOutTime);
        CheckStatus();
    }
}

void SCVulkanCommandBuffer::uninit()
{
    unint_internal();
}

void SCVulkanCommandBuffer::unint_internal()
{
    if(op)
    {
        auto&& loc_rhi = rhi.lock();
        if (get_status() == SECommandBufferStatus::Pending)
        {
            vkWaitForFences(loc_rhi->get_device(), 1, &m_fence->GetInnerFence(), VK_TRUE, 0xffffffffffffffffull);
        }
        vkResetCommandBuffer(m_command_buffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        m_command_buffer = VK_NULL_HANDLE;
        auto* top = op;
        m_fence = nullptr;
        Fence = nullptr;
    	op = nullptr;
        top->erase(it);

    }
    
}

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

            const bool b_should_destroy_present_pool = m_graphics_command_pool != m_present_command_pool;

            auto temp = m_command_buffers;
            for(auto&& cb : temp)
            {
                cb->wait_until_finish();
                cb->uninit();
            }

            if(m_graphics_command_pool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(m_device, m_graphics_command_pool, m_memory);
                m_graphics_command_pool = VK_NULL_HANDLE;
            }

            if(b_should_destroy_present_pool && m_present_command_pool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(m_device, m_present_command_pool, m_memory);
                m_present_command_pool = VK_NULL_HANDLE;
            }

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


            VkCommandPoolCreateInfo graphics_command_pool_create_info{};
            graphics_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            graphics_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            graphics_command_pool_create_info.pNext = nullptr;
            graphics_command_pool_create_info.queueFamilyIndex = queue_family_index_graphics_to_use;

            vkCreateCommandPool(m_device, &graphics_command_pool_create_info, m_memory, &m_graphics_command_pool);

            if(!b_graphics_family_queue_can_presentation)
            {
                VkCommandPoolCreateInfo present_command_pool_create_info{};
                present_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                present_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                present_command_pool_create_info.pNext = nullptr;
                present_command_pool_create_info.queueFamilyIndex = queue_family_index_presentation_to_use;

                vkCreateCommandPool(m_device, &present_command_pool_create_info, m_memory, &m_present_command_pool);
            }
            else
            {
                m_present_command_pool = m_graphics_command_pool;
            }
            

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

SSPtr<SCRHICommandBuffer> SCVulkanRHI::allocate_command_buffer(SECommandBufferLifeType InLifeType)
{
    SSPtr<SCVulkanCommandBuffer> r = SSPtr<SCVulkanCommandBuffer>::construct<SCVulkanCommandBuffer>();
    r->it = m_command_buffers.emplace(m_command_buffers.end(), r);
    r->op = &m_command_buffers;
    r->Init(SSPtr<SCRHIInterface>(this), InLifeType);
	SSPtr<SCRHICommandBuffer> ret = r;
    return ret;
}

void SCVulkanRHI::reset_command_buffer(SSPtr<SCRHICommandBuffer>& InBuffer)
{
    InBuffer.as<SCVulkanCommandBuffer>()->uninit();
}

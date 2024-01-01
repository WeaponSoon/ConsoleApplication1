#include "VulkanBuffer.h"

void SCVulkanBuffer::init(const SSPtr<SCRHIInterface>& inInterface, SSRHIBufferCreateInfo& createInfo)
{
    vulkan_rhi = inInterface.as<SCVulkanRHI>();
    size = createInfo.size;
    Usage = createInfo.Usage;
    hostVisible = createInfo.hostVisible;

    auto&& loc_rhi = vulkan_rhi.lock();

	VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = createInfo.size;
    bufferInfo.usage = createInfo.Usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
     
    vkCreateBuffer(loc_rhi->get_device(), &bufferInfo, nullptr, &m_buffer);

    VkMemoryRequirements mem{};
    vkGetBufferMemoryRequirements(loc_rhi->get_device(), m_buffer, &mem);

    VkMemoryPropertyFlags needed_memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if(createInfo.hostVisible)
    {
        needed_memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    VkPhysicalDeviceMemoryProperties physical_device_memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(loc_rhi->get_physical_device(), &physical_device_memory_properties);
    uint32_t memory_type_index = 0;
    for (; memory_type_index < physical_device_memory_properties.memoryTypeCount; ++memory_type_index)
    {
        if ((mem.memoryTypeBits & (1 << memory_type_index))
            && (physical_device_memory_properties.memoryTypes[memory_type_index].propertyFlags & needed_memory_property) == needed_memory_property)
        {
            //memory_allocate_info.memoryTypeIndex = memory_type_index;
            break;
        }
    }

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = mem.size;
    allocate_info.memoryTypeIndex = memory_type_index;
    vkAllocateMemory(loc_rhi->get_device(), &allocate_info, loc_rhi->get_allocator(), &m_memory);

    vkBindBufferMemory(loc_rhi->get_device(), m_buffer, m_memory, 0);

}

void SCVulkanBuffer::set_data(const std::vector<uint8_t>& inData)
{
    auto&& loc_rhi = vulkan_rhi.lock();
    if(hostVisible)
    {
        void* data = nullptr;
        vkMapMemory(loc_rhi->get_device(), m_memory, 0, size, 0, &data);
        memcpy(data, inData.data(), size);
        vkUnmapMemory(loc_rhi->get_device(), m_memory);
    }
    //buffer_memory.
}

void SCVulkanBuffer::get_data(std::vector<uint8_t>& outData)
{
    auto&& loc_rhi = vulkan_rhi.lock();
    if (hostVisible)
    {
        outData.resize(size);
        void* data = nullptr;
        vkMapMemory(loc_rhi->get_device(), m_memory, 0, size, 0, &data);
        memcpy(outData.data(), data , size);
        vkUnmapMemory(loc_rhi->get_device(), m_memory);
    }
}

void SCVulkanBuffer::unint_internal()
{
    auto&& loc_rhi = vulkan_rhi.lock();
    vkDestroyBuffer(loc_rhi->get_device(), m_buffer, loc_rhi->get_allocator());
    m_buffer = VK_NULL_HANDLE;
    vkFreeMemory(loc_rhi->get_device(), m_memory, loc_rhi->get_allocator());
    m_memory = VK_NULL_HANDLE;
}

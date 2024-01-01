#include "VulkanTexture.h"

#include "VulkanBuffer.h"

class SCT2DLayoutTransientCmd : public SCRHICommand
{
public:
	VkImageLayout m_src = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout m_dst = VK_IMAGE_LAYOUT_UNDEFINED;

	SSRHITextureAspectFlags m_copy_aspects = static_cast<uint32_t>(SERHITextureAspect::TA_COLOR);
	uint32_t m_mipmap_level = 0;
    uint32_t m_mip_count = 1;
	uint32_t m_array_layer = 0;
	uint32_t m_array_count = 1;

    SERHIPipelineStageFlags m_src_stage = 0;
    SERHIPipelineStageFlags m_dst_stage = 0;

	SSPtr<SCVulkanTexture2D> m_target;

	void on_command_record(SCRHICommandBuffer& cmd_buffer) const override
	{
		SCVulkanCommandBuffer& vk_buffer = static_cast<SCVulkanCommandBuffer&>(cmd_buffer);
		auto&& inernal_buffer = vk_buffer.get_internal_buffer();

		VkImageMemoryBarrier image_memory_barrier{};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.pNext = nullptr;
		image_memory_barrier.oldLayout = m_src;
		image_memory_barrier.newLayout = m_dst;
        image_memory_barrier.srcAccessMask = 0;

        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.image = m_target->get_inner();
        image_memory_barrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(m_copy_aspects);
        image_memory_barrier.subresourceRange.baseArrayLayer = m_array_layer;
        image_memory_barrier.subresourceRange.baseMipLevel = m_mipmap_level;
        image_memory_barrier.subresourceRange.layerCount = m_array_count;
        image_memory_barrier.subresourceRange.levelCount = m_mip_count;

        switch (m_dst) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_GENERAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;

        default:
            image_memory_barrier.dstAccessMask = 0;
            break;
        }
        vkCmdPipelineBarrier(inernal_buffer, m_src_stage, m_dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}
};

class SCVulkanCopyBufferToImage2DCmd : public SCRHICommand
{
public:
    SSRHITextureAspectFlags m_copy_aspects = static_cast<uint32_t>(SERHITextureAspect::TA_COLOR);
    uint64_t m_buffer_offset = 0;
    int32_t m_texture_offset[3] = { 0,0,0 };
    uint32_t m_texture_extent[3] = { 0,0,0 };

    uint32_t m_mipmap_level = 0;
    uint32_t m_array_layer = 0;
    uint32_t m_array_count = 1;

    SSPtr<SCRHIBuffer> m_src_buffer;
    SSPtr<SCRHITexture> m_dst_tex;

	void on_command_record(SCRHICommandBuffer& cmd_buffer) const override
	{
        
        SCVulkanCommandBuffer& vk_buffer = static_cast<SCVulkanCommandBuffer&>(cmd_buffer);
        auto&& inernal_buffer = vk_buffer.get_internal_buffer();

        VkBufferImageCopy copy_region = {
		.bufferOffset = m_buffer_offset,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {m_copy_aspects, m_mipmap_level, m_array_layer, m_array_count},
		.imageOffset = {m_texture_offset[0], m_texture_offset[1], m_texture_offset[2]},
		.imageExtent = {m_texture_extent[0],m_texture_extent[1],m_texture_extent[2]},
        };
        std::vector<VkBufferImageCopy> regions;

        if (m_copy_aspects & VK_IMAGE_ASPECT_COLOR_BIT)
        {
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            regions.push_back(copy_region);
        }

        if (m_copy_aspects & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            regions.push_back(copy_region);
        }

        if (m_copy_aspects & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            regions.push_back(copy_region);
        }


        SSPtr<SCVulkanTexture2D> t2d = m_dst_tex.as<SCVulkanTexture2D>();
        SSPtr<SCVulkanBuffer> vb = m_src_buffer.as<SCVulkanBuffer>();
        if(regions.size() > 0)
        {
            vkCmdCopyBufferToImage(inernal_buffer, vb->get_inner_buffer(), t2d->get_inner(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
        }
	}
};

class SCVulkanCopyImage2DToBufferCmd : public SCRHICommand
{
public:
    SSRHITextureAspectFlags m_copy_aspects = static_cast<uint32_t>(SERHITextureAspect::TA_COLOR);
    uint64_t m_buffer_offset = 0;
    int32_t m_texture_offset[3] = { 0,0,0 };
    uint32_t m_texture_extent[3] = { 0,0,0 };

    uint32_t m_mipmap_level = 0;
    uint32_t m_array_layer = 0;
    uint32_t m_array_count = 1;

    SSPtr<SCRHIBuffer> m_dst_buffer;
    SSPtr<SCRHITexture> m_src_tex;

    void on_command_record(SCRHICommandBuffer& cmd_buffer) const override
    {

        SCVulkanCommandBuffer& vk_buffer = static_cast<SCVulkanCommandBuffer&>(cmd_buffer);
        auto&& inernal_buffer = vk_buffer.get_internal_buffer();

        //vkCmdCopyImageToBuffer()

        VkBufferImageCopy copy_region = {
        .bufferOffset = m_buffer_offset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {m_copy_aspects, m_mipmap_level, m_array_layer, m_array_count},
        .imageOffset = {m_texture_offset[0], m_texture_offset[1], m_texture_offset[2]},
        .imageExtent = {m_texture_extent[0],m_texture_extent[1],m_texture_extent[2]},
        };


        std::vector<VkBufferImageCopy> regions;

        if (m_copy_aspects & VK_IMAGE_ASPECT_COLOR_BIT)
        {
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            regions.push_back(copy_region);
        }

        if (m_copy_aspects & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            regions.push_back(copy_region);
        }

        if (m_copy_aspects & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            regions.push_back(copy_region);
        }


        SSPtr<SCVulkanTexture2D> t2d = m_src_tex.as<SCVulkanTexture2D>();
        SSPtr<SCVulkanBuffer> vb = m_dst_buffer.as<SCVulkanBuffer>();

        if (regions.size() > 0)
        {
            vkCmdCopyImageToBuffer(inernal_buffer,  t2d->get_inner(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vb->get_inner_buffer(), regions.size(), regions.data());
        }
    }
};




//void SCVulkanTexture2D::transfer_layout(VkImageLayout targetLayout, int inMipLevel)
//{
//    SSPtr<SCT2DLayoutTransientCmd> transition_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
//    transition_cmd->m_src = m_curLayout;
//    transition_cmd->m_dst = targetLayout;
//    transition_cmd->m_target = this;
//    transition_cmd->m_mipmap_level = inMipLevel;
//    transition_cmd->m_mip_count = 1;
//    transition_cmd->m_array_layer = 0;
//    transition_cmd->m_array_count = 1;
//    transition_cmd->m_src_stage = RHI_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//    transition_cmd->m_dst_stage = RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//
//    
//
//    transition_cmd->m_copy_aspects = static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR);
//
//
//
//}

bool SCVulkanTexture2D::internal_init(SSPtr<SCRHIInterface> rhi, const SSRHITexture2DCreateInfo& inInfo)

{
	if (m_vkimage != VK_NULL_HANDLE)
	{
		return false;
	}

	SSScopeSpinLock guard(texture_lock);

    vulkan_rhi = rhi.as<SCVulkanRHI>();

	auto vkUsage = SCVulkanTextureHelper::ConvertToVkImageUsage(inInfo.inUsage);
	//bool force_linear = false;

    auto pixelFormat = SCVulkanTextureHelper::ConvertToVkFormat(inInfo.inPixelFormat);

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(vulkan_rhi->get_physical_device(), pixelFormat, &formatProperties);


    switch(inInfo.inUsage)
    {
    case SERHITextureUsage::TU_ShaderResource:
        if(!(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
            || !(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
        {
            return false;
        }
        break;
    case SERHITextureUsage::TU_RenderTargetColor:
        if (!(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
            || !(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        {
            return false;
        }
        break;
    case SERHITextureUsage::TU_RenderTargetDepthResource:
        if (!(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            || !(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        {
            return false;
        }
        break;
    case SERHITextureUsage::TU_RenderTargetDepthStencil:
        if (!(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            return false;
        }
        break;
    case SERHITextureUsage::TU_UnorderedAccess:
        if (!(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
        {
            return false;
        }
        break;
    }


    auto accessType = inInfo.inAccessType;

    if (!(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) ||
        !(formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
    {
        accessType = SERHITextureAccessType::TAT_DEVICE;
    }

	if (accessType == SERHITextureAccessType::TAT_HOST_AND_DEVICE)
	{
		vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	auto&& FixedSize = FixTextureSize(inInfo.inWidth, inInfo.inHeight, inInfo.inPixelFormat);
	auto texture_extent = VkExtent3D(std::get<0>(FixedSize), std::get<1>(FixedSize), 1);

    VkSampleCountFlagBits sampleFlags[] = {
    VK_SAMPLE_COUNT_1_BIT,
    VK_SAMPLE_COUNT_2_BIT,
    VK_SAMPLE_COUNT_4_BIT,
    VK_SAMPLE_COUNT_8_BIT,
    VK_SAMPLE_COUNT_16_BIT,
    VK_SAMPLE_COUNT_32_BIT,
    VK_SAMPLE_COUNT_64_BIT,
    };

    int sampleCount = (inInfo.inUsage == SERHITextureUsage::TU_RenderTargetDepthResource
        || inInfo.inUsage == SERHITextureUsage::TU_RenderTargetDepthStencil
        || inInfo.inUsage == SERHITextureUsage::TU_RenderTargetColor) ? NearestPowerOfTwo(inInfo.inSampleCount) : 1;

    bool supportsMipmapping =
        (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
        (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) &&
        (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

    int mips = std::min(GetMaxMipLevels(texture_extent.width, texture_extent.height), (uint32_t)inInfo.inMipMapLevels);
    if(!supportsMipmapping)
    {
        mips = 1;
    }
	
	VkImageCreateInfo vk_image_create_info{};
	vk_image_create_info.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vk_image_create_info.flags = 0;
	vk_image_create_info.pNext = nullptr;
	vk_image_create_info.arrayLayers = 1;
	vk_image_create_info.extent = texture_extent;
	vk_image_create_info.format = SCVulkanTextureHelper::ConvertToVkFormat(inInfo.inPixelFormat);
	vk_image_create_info.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	vk_image_create_info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
    vk_image_create_info.mipLevels = mips;// inInfo.inMipMapLevels;
	vk_image_create_info.samples = sampleFlags[(int)std::round(std::log2(sampleCount))];
	vk_image_create_info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	vk_image_create_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	vk_image_create_info.usage = vkUsage;/// SCVulkanTextureHelper::ConvertToVkImageUsage(inInfo.inUsage);

	if (vkCreateImage(vulkan_rhi->get_device(), &vk_image_create_info, vulkan_rhi->get_allocator(), &m_vkimage) != VkResult::VK_SUCCESS)
	{
		m_vkimage = VK_NULL_HANDLE;
		vulkan_rhi = nullptr;
		return false;
	}

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(vulkan_rhi->get_device(), m_vkimage, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.pNext = nullptr;
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkMemoryPropertyFlags needed_memory_property = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	/*if(inInfo.inAccessType == SERHITextureAccessType::TAT_HOST_AND_DEVICE)
	{
		needed_memory_property = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}*/

	VkPhysicalDeviceMemoryProperties physical_device_memory_properties{};
	vkGetPhysicalDeviceMemoryProperties(vulkan_rhi->get_physical_device(), &physical_device_memory_properties);
	uint32_t memory_type_index = 0;
	for (; memory_type_index < physical_device_memory_properties.memoryTypeCount; ++memory_type_index)
	{
		if ((memory_requirements.memoryTypeBits & (1 << memory_type_index))
			&& (physical_device_memory_properties.memoryTypes[memory_type_index].propertyFlags & needed_memory_property) == needed_memory_property)
		{
			//memory_allocate_info.memoryTypeIndex = memory_type_index;
			break;
		}
	}

	if (memory_type_index == physical_device_memory_properties.memoryTypeCount)
	{
		vkDestroyImage(vulkan_rhi->get_device(), m_vkimage, vulkan_rhi->get_allocator());
		m_vkimage = VK_NULL_HANDLE;
		vulkan_rhi = nullptr;
		return false;
	}
	memory_allocate_info.memoryTypeIndex = memory_type_index;

	if (vkAllocateMemory(vulkan_rhi->get_device(), &memory_allocate_info, vulkan_rhi->get_allocator(), &m_device_memory) != VK_SUCCESS)
	{
		vkDestroyImage(vulkan_rhi->get_device(), m_vkimage, vulkan_rhi->get_allocator());
		m_vkimage = VK_NULL_HANDLE;
		m_device_memory = VK_NULL_HANDLE;
		vulkan_rhi = nullptr;
		return false;
	}

	if (vkBindImageMemory(vulkan_rhi->get_device(), m_vkimage, m_device_memory, 0) != VK_SUCCESS)
	{
		vkDestroyImage(vulkan_rhi->get_device(), m_vkimage, vulkan_rhi->get_allocator());
		m_vkimage = VK_NULL_HANDLE;
		vkFreeMemory(vulkan_rhi->get_device(), m_device_memory, vulkan_rhi->get_allocator());
		m_device_memory = VK_NULL_HANDLE;
		vulkan_rhi = nullptr;
		return false;
	}

	m_pixel_format = inInfo.inPixelFormat;
    m_texture_usage = inInfo.inUsage;
    m_access_type = accessType;
    m_height = texture_extent.height;
    m_width = texture_extent.width;
    m_sample_count = sampleCount;
    m_mipmap = mips;

    VkImageLayout targetLayout = m_curLayout;
    VkPipelineStageFlagBits dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	switch (m_texture_usage)
	{
	case SERHITextureUsage::TU_ShaderResource:
        targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
	case SERHITextureUsage::TU_RenderTargetDepthResource:
        targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
	case SERHITextureUsage::TU_RenderTargetDepthStencil:
        targetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        break;
	case SERHITextureUsage::TU_RenderTargetColor:
        targetLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        break;
	case SERHITextureUsage::TU_UnorderedAccess:
        targetLayout = VK_IMAGE_LAYOUT_GENERAL;
        break;
	}

    SSPtr<SCT2DLayoutTransientCmd> transition_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    transition_cmd->m_src = VK_IMAGE_LAYOUT_UNDEFINED;
    transition_cmd->m_dst = targetLayout;
    transition_cmd->m_target = this;
    transition_cmd->m_mipmap_level = 0;
    transition_cmd->m_mip_count = m_mipmap;
    transition_cmd->m_array_layer = 0;
    transition_cmd->m_array_count = 1;
    transition_cmd->m_src_stage = RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    transition_cmd->m_dst_stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    transition_cmd->m_copy_aspects = 0;
    if(ContainsDepth(m_pixel_format))
    {
        transition_cmd->m_copy_aspects |= static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_DEPTH);
    }
    if(ContainsStencil(m_pixel_format))
    {
        transition_cmd->m_copy_aspects |= static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_STENCIL);
    }
    if(!transition_cmd->m_copy_aspects)
    {
        transition_cmd->m_copy_aspects = static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR);
    }

    auto&& cmd_buffer = vulkan_rhi->allocate_command_buffer(SECommandBufferLifeType::ExecuteOnce);
    cmd_buffer->begin_record();

    cmd_buffer->record_command(transition_cmd);

    cmd_buffer->end_record();

    cmd_buffer->submit({}, {});
    cmd_buffer->wait_until_finish();

    vulkan_rhi->reset_command_buffer(cmd_buffer);

    m_curLayout = targetLayout;

	return true;
}

bool SCVulkanTexture2D::set_raw_data(const std::vector<std::uint8_t>& inData, uint32_t inMipmapLevel)
{
    //transition_layout_for_copy(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, inMipmapLevel, 1, 0, 1, static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR));

    //VkImageViewCreateInfo view_create_info;
    //view_create_info.viewType;

    if(m_texture_usage != SERHITextureUsage::TU_ShaderResource && m_access_type != SERHITextureAccessType::TAT_HOST_AND_DEVICE)
    {
        return false;
    }

    auto&& sizeAtMip = GetSizeAtMipLevel(m_width, m_height, inMipmapLevel);
    auto&& sizeInByte = CalcTextureSizeInByte(m_width, m_height, m_pixel_format, inMipmapLevel);

    bool hasDepth = ContainsDepth(m_pixel_format);
    bool hasStencil = ContainsStencil(m_pixel_format);

    if(hasStencil && !hasDepth)
    {
        return false;
    }


    if(sizeInByte > inData.size())
    {
        return false;
    }

    SSRHITextureAspectFlags transitionAspect = 0;
    if (hasDepth)
    {
        transitionAspect |= static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_DEPTH);
    }
    if(hasStencil)
    {
        transitionAspect |= static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_STENCIL);
    }

    if(!hasStencil && !hasDepth)
    {
        transitionAspect = static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR);
    }



    SSPtr<SCVulkanCopyBufferToImage2DCmd> cmd = SSPtr<SCVulkanCopyBufferToImage2DCmd>::construct<SCVulkanCopyBufferToImage2DCmd>();

    cmd->m_dst_tex = this;
    cmd->m_mipmap_level = inMipmapLevel;
    cmd->m_array_layer = 0;
    cmd->m_array_count = 1;
    cmd->m_buffer_offset = 0;

	cmd->m_texture_offset[0] = 0;
    cmd->m_texture_offset[1] = 0;
    cmd->m_texture_offset[2] = 0;
    cmd->m_copy_aspects = transitionAspect;// static_cast<SSRHITextureAspectFlags>(!hasStencil && !hasDepth ? SERHITextureAspect::TA_COLOR : SERHITextureAspect::TA_DEPTH);

	cmd->m_texture_extent[0] = std::get<0>(sizeAtMip);
    cmd->m_texture_extent[1] = std::get<1>(sizeAtMip);
    cmd->m_texture_extent[2] = 1;


    SSPtr<SCRHIBuffer> buff = SSPtr<SCRHIBuffer>::construct<SCVulkanBuffer>();
    SSRHIBufferCreateInfo inf;
    
    inf.size = sizeInByte;// // *std::get<0>(ie) / std::get<1>(ie);
    inf.Usage = BU_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inf.hostVisible = true;
    buff->init(vulkan_rhi.as<SCRHIInterface>(), inf);
    buff->set_data(inData);

    cmd->m_src_buffer = buff;


    SSPtr<SCT2DLayoutTransientCmd> transition_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    transition_cmd->m_src = m_curLayout;
    transition_cmd->m_dst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transition_cmd->m_target = this;
    transition_cmd->m_mipmap_level = inMipmapLevel;
    transition_cmd->m_mip_count = 1;
    transition_cmd->m_array_layer = 0;
    transition_cmd->m_array_count = 1;
    transition_cmd->m_src_stage = RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    transition_cmd->m_dst_stage = SERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_TRANSFER_BIT;
    transition_cmd->m_copy_aspects = transitionAspect;

    SSPtr<SCT2DLayoutTransientCmd> transition_back_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    transition_back_cmd->m_src = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transition_back_cmd->m_dst = m_curLayout;
    transition_back_cmd->m_target = this;
    transition_back_cmd->m_mipmap_level = inMipmapLevel;
    transition_back_cmd->m_mip_count = 1;
    transition_back_cmd->m_array_layer = 0;
    transition_back_cmd->m_array_count = 1;
    transition_back_cmd->m_src_stage = RHI_PIPELINE_STAGE_TRANSFER_BIT;
    transition_back_cmd->m_dst_stage = RHI_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    transition_back_cmd->m_copy_aspects = transitionAspect;


    auto&& cmd_buffer = vulkan_rhi->allocate_command_buffer(SECommandBufferLifeType::ExecuteOnce);

	cmd_buffer->begin_record();

    cmd_buffer->record_command(transition_cmd);
    cmd_buffer->record_command(cmd);
    cmd_buffer->record_command(transition_back_cmd);

    cmd_buffer->end_record();

    cmd_buffer->submit({}, {});
    cmd_buffer->wait_until_finish();

    vulkan_rhi->reset_command_buffer(cmd_buffer);
     
    return true;
}

bool SCVulkanTexture2D::get_raw_data(std::vector<std::uint8_t>& outData, uint32_t inMipmapLevel)
{
    if(m_access_type != SERHITextureAccessType::TAT_HOST_AND_DEVICE)
    {
        return false;
    }

    outData.clear();

    

    auto&& sizeAtMip = GetSizeAtMipLevel(m_width, m_height, inMipmapLevel);
    auto&& sizeInByte = CalcTextureSizeInByte(m_width, m_height, m_pixel_format, inMipmapLevel);

    bool hasDepth = ContainsDepth(m_pixel_format);
    bool hasStencil = ContainsStencil(m_pixel_format);

    if (hasStencil && !hasDepth)
    {
        return false;
    }

    SSRHITextureAspectFlags transitionAspect = 0;
    if (hasDepth)
    {
        transitionAspect |= static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_DEPTH);
    }
    if (hasStencil)
    {
        transitionAspect |= static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_STENCIL);
    }

    if (!hasStencil && !hasDepth)
    {
        transitionAspect = static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR);
    }

    SSPtr<SCRHIBuffer> buff = SSPtr<SCRHIBuffer>::construct<SCVulkanBuffer>();
    SSRHIBufferCreateInfo inf;

    inf.size = sizeInByte;// // *std::get<0>(ie) / std::get<1>(ie);
    inf.Usage = BU_BUFFER_USAGE_TRANSFER_DST_BIT;
    inf.hostVisible = true;
    buff->init(vulkan_rhi.as<SCRHIInterface>(), inf);

    SSPtr<SCVulkanCopyImage2DToBufferCmd> cmd = SSPtr<SCVulkanCopyImage2DToBufferCmd>::construct<SCVulkanCopyImage2DToBufferCmd>();
    cmd->m_src_tex = this;
    cmd->m_mipmap_level = inMipmapLevel;
    cmd->m_array_layer = 0;
    cmd->m_array_count = 1;
    cmd->m_buffer_offset = 0;

    cmd->m_texture_offset[0] = 0;
    cmd->m_texture_offset[1] = 0;
    cmd->m_texture_offset[2] = 0;
    cmd->m_copy_aspects = transitionAspect;// static_cast<SSRHITextureAspectFlags>(!hasStencil && !hasDepth ? SERHITextureAspect::TA_COLOR : SERHITextureAspect::TA_DEPTH);

    cmd->m_texture_extent[0] = std::get<0>(sizeAtMip);
    cmd->m_texture_extent[1] = std::get<1>(sizeAtMip);
    cmd->m_texture_extent[2] = 1;
    cmd->m_dst_buffer = buff;



    SSPtr<SCT2DLayoutTransientCmd> transition_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    transition_cmd->m_src = m_curLayout;
    transition_cmd->m_dst = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    transition_cmd->m_target = this;
    transition_cmd->m_mipmap_level = inMipmapLevel;
    transition_cmd->m_mip_count = 1;
    transition_cmd->m_array_layer = 0;
    transition_cmd->m_array_count = 1;
    transition_cmd->m_src_stage = SERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    transition_cmd->m_dst_stage = SERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_TRANSFER_BIT;
    transition_cmd->m_copy_aspects = transitionAspect;

    SSPtr<SCT2DLayoutTransientCmd> transition_back_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    transition_back_cmd->m_src = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    transition_back_cmd->m_dst = m_curLayout;
    transition_back_cmd->m_target = this;
    transition_back_cmd->m_mipmap_level = inMipmapLevel;
    transition_back_cmd->m_mip_count = 1;
    transition_back_cmd->m_array_layer = 0;
    transition_back_cmd->m_array_count = 1;
    transition_back_cmd->m_src_stage = RHI_PIPELINE_STAGE_TRANSFER_BIT;
    transition_back_cmd->m_dst_stage = RHI_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    transition_back_cmd->m_copy_aspects = transitionAspect;

    auto&& cmd_buffer = vulkan_rhi->allocate_command_buffer(SECommandBufferLifeType::ExecuteOnce);

    cmd_buffer->begin_record();

    cmd_buffer->record_command(transition_cmd);
    cmd_buffer->record_command(cmd);
    cmd_buffer->record_command(transition_back_cmd);

    cmd_buffer->end_record();

    cmd_buffer->submit({}, {});
    cmd_buffer->wait_until_finish();

    buff->get_data(outData);

    vulkan_rhi->reset_command_buffer(cmd_buffer);

    return true;

}

//void SCVulkanTexture2D::transition_layout_for_copy(VkImageLayout inFrom, VkImageLayout inTarget, uint32_t inChangeMip, uint32_t inChangeMipCount, uint32_t inChangeBaseLayer, uint32_t inChangeLayerCount, SSRHITextureAspectFlags inChangeAspect)
//{
//    SSPtr<SCT2DLayoutTransientCmd> cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
//    cmd->m_src = inFrom;
//    cmd->m_dst = inTarget;
//    cmd->m_target = this;
//    cmd->m_mipmap_level = inChangeMip;
//    cmd->m_mip_count = inChangeMipCount;
//    cmd->m_array_layer = inChangeBaseLayer;
//    cmd->m_array_count = inChangeLayerCount;
//    cmd->m_src_stage = RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//    cmd->m_dst_stage = SERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_TRANSFER_BIT;
//    cmd->m_copy_aspects = inChangeAspect;
//
//    
//    auto&& cmd_buffer = vulkan_rhi->allocate_command_buffer(SECommandBufferLifeType::ExecuteOnce);
//    cmd_buffer->begin_record();
//
//    cmd_buffer->record_command(cmd);
//
//    cmd_buffer->end_record();
//
//    cmd_buffer->submit({}, {});
//    cmd_buffer->wait_until_finish();
//
//    vulkan_rhi->reset_command_buffer(cmd_buffer);
//
//}

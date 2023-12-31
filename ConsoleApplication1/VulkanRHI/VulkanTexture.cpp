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
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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

class SCVulkanCopyBufferToImage2DCmd : public SCCopyBufferToImgCmd
{
	void on_command_record(SCRHICommandBuffer& cmd_buffer) const override
	{
        
        SCVulkanCommandBuffer& vk_buffer = static_cast<SCVulkanCommandBuffer&>(cmd_buffer);
        auto&& inernal_buffer = vk_buffer.get_internal_buffer();

        VkBufferImageCopy copy_region = {
		.bufferOffset = m_buffer_offset,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, m_mipmap_level, m_array_layer, m_array_count},
		.imageOffset = {m_texture_offset[0], m_texture_offset[1], m_texture_offset[2]},
		.imageExtent = {m_texture_extent[0],m_texture_extent[1],m_texture_extent[2]},
        };

        SSPtr<SCVulkanTexture2D> t2d = m_dst_tex.as<SCVulkanTexture2D>();
        SSPtr<SCVulkanBuffer> vb = m_src_buffer.as<SCVulkanBuffer>();

        vkCmdCopyBufferToImage(inernal_buffer, vb->get_inner_buffer(), t2d->get_inner(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
	}
};

bool SCVulkanTexture2D::set_raw_data(const std::vector<std::uint8_t>& inData, uint32_t inMipmapLevel)
{
    //transition_layout_for_copy(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, inMipmapLevel, 1, 0, 1, static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR));

    //VkImageViewCreateInfo view_create_info;
    //view_create_info.viewType;

    if(m_texture_usage != SERHITextureUsage::TU_ShaderResource)
    {
        return false;
    }

    SSPtr<SCVulkanCopyBufferToImage2DCmd> cmd = SSPtr<SCVulkanCopyBufferToImage2DCmd>::construct<SCVulkanCopyBufferToImage2DCmd>();

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(vulkan_rhi->get_device(), m_vkimage, &memory_requirements);

    cmd->m_dst_tex = this;
    cmd->m_mipmap_level = inMipmapLevel;
    cmd->m_array_layer = 0;
    cmd->m_array_count = 1;
    cmd->m_buffer_offset = 0;

	cmd->m_texture_offset[0] = 0;
    cmd->m_texture_offset[1] = 0;
    cmd->m_texture_offset[2] = 0;

    auto&& sizeAtMip = GetSizeAtMipLevel(texture_extent.width, texture_extent.height, inMipmapLevel);

	cmd->m_texture_extent[0] = std::get<0>(sizeAtMip);
    cmd->m_texture_extent[1] = std::get<1>(sizeAtMip);
    cmd->m_texture_extent[2] = 1;
    
    SSPtr<SCRHIBuffer> buff = SSPtr<SCRHIBuffer>::construct<SCVulkanBuffer>();
    SSRHIBufferCreateInfo inf;
    auto&& ie = GetPixelFormatBlockSizeAndPerBlockNumPixels(m_pixel_format);
    inf.size = CalcTextureSizeInByte(texture_extent.height, texture_extent.width, m_pixel_format, inMipmapLevel);// *std::get<0>(ie) / std::get<1>(ie);
    inf.Usage = BU_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inf.hostVisible = true;
    buff->init(vulkan_rhi.as<SCRHIInterface>(), inf);
    buff->set_data(inData);

    cmd->m_src_buffer = buff;

    SSPtr<SCT2DLayoutTransientCmd> transition_cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    transition_cmd->m_src = VK_IMAGE_LAYOUT_UNDEFINED;
    transition_cmd->m_dst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transition_cmd->m_target = this;
    transition_cmd->m_mipmap_level = inMipmapLevel;
    transition_cmd->m_mip_count = 1;
    transition_cmd->m_array_layer = 0;
    transition_cmd->m_array_count = 1;
    transition_cmd->m_src_stage = RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    transition_cmd->m_dst_stage = SERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_TRANSFER_BIT;
    transition_cmd->m_copy_aspects = static_cast<SSRHITextureAspectFlags>(SERHITextureAspect::TA_COLOR);


    auto&& cmd_buffer = vulkan_rhi->allocate_command_buffer(SECommandBufferLifeType::ExecuteOnce);
    cmd_buffer->begin_record();

    cmd_buffer->record_command(transition_cmd);
    cmd_buffer->record_command(cmd);

    cmd_buffer->end_record();

    cmd_buffer->submit({}, {});
    cmd_buffer->wait_until_finish();

    vulkan_rhi->reset_command_buffer(cmd_buffer);
     
    return true;
}

void SCVulkanTexture2D::transition_layout_for_copy(VkImageLayout inFrom, VkImageLayout inTarget, uint32_t inChangeMip, uint32_t inChangeMipCount, uint32_t inChangeBaseLayer, uint32_t inChangeLayerCount, SSRHITextureAspectFlags inChangeAspect)
{
    SSPtr<SCT2DLayoutTransientCmd> cmd = SSPtr<SCT2DLayoutTransientCmd>::construct<SCT2DLayoutTransientCmd>();
    cmd->m_src = inFrom;
    cmd->m_dst = inTarget;
    cmd->m_target = this;
    cmd->m_mipmap_level = inChangeMip;
    cmd->m_mip_count = inChangeMipCount;
    cmd->m_array_layer = inChangeBaseLayer;
    cmd->m_array_count = inChangeLayerCount;
    cmd->m_src_stage = RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    cmd->m_dst_stage = SERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_TRANSFER_BIT;
    cmd->m_copy_aspects = inChangeAspect;

    
    auto&& cmd_buffer = vulkan_rhi->allocate_command_buffer(SECommandBufferLifeType::ExecuteOnce);
    cmd_buffer->begin_record();

    cmd_buffer->record_command(cmd);

    cmd_buffer->end_record();

    cmd_buffer->submit({}, {});
    cmd_buffer->wait_until_finish();

    vulkan_rhi->reset_command_buffer(cmd_buffer);

}

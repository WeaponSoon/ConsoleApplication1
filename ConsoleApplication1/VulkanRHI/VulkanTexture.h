#pragma once
#include "Core/RHI/RHITexture.h"
#include "VulkanRHI/VulkanRHI.h"


class SCCopyBufferToImgCmd : public SCRHICommand
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
};


class SCVulkanTextureHelper
{
public:

	static VkImageUsageFlags ConvertToVkImageUsage(SERHITextureUsage inUsageFlags)
	{
		VkImageUsageFlags ret = 0;
		switch (inUsageFlags)
		{
		case SERHITextureUsage::TU_RenderTargetColor:
			ret = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			break;
		case SERHITextureUsage::TU_RenderTargetDepthStencil:
			ret = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
		case SERHITextureUsage::TU_RenderTargetDepthResource:
			ret = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			break;
		case SERHITextureUsage::TU_ShaderResource:
			ret = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			break;
		case SERHITextureUsage::TU_UnorderedAccess:
			ret = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
			break;
		}
		return ret;
	}

	static VkFormat ConvertToVkFormat(SERHIPixelFormat inPixelFormat)
	{
		return static_cast<VkFormat>(inPixelFormat);
	}
};

class SCVulkanTexture2D : public SCRHITexture2D
{

private:
	SSSpinLock texture_lock;
	VkExtent3D texture_extent;
	SSPtr<SCVulkanRHI> vulkan_rhi;

	VkImage m_vkimage = VK_NULL_HANDLE;
	VkDeviceMemory m_device_memory = VK_NULL_HANDLE;
	VkImageView m_vkimage_view = VK_NULL_HANDLE;

	void internal_release()
	{
		SSScopeSpinLock guard(texture_lock);
		if(vulkan_rhi)
		{
			vkDestroyImage(vulkan_rhi->get_device(), m_vkimage, vulkan_rhi->get_allocator());
			m_vkimage = VK_NULL_HANDLE;
			vkFreeMemory(vulkan_rhi->get_device(), m_device_memory, vulkan_rhi->get_allocator());
			m_device_memory = VK_NULL_HANDLE;
			vulkan_rhi = nullptr;
		}
	}

	bool internal_init(SSPtr<SCRHIInterface> rhi, const SSRHITexture2DCreateInfo& inInfo)
	{
		if(m_vkimage != VK_NULL_HANDLE)
		{
			return false;
		}

		SSScopeSpinLock guard(texture_lock);

		auto vkUsage = SCVulkanTextureHelper::ConvertToVkImageUsage(inInfo.inUsage);
		//bool force_linear = false;
		if(inInfo.inAccessType == SERHITextureAccessType::TAT_HOST_AND_DEVICE )
		{
			vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		auto&& FixedSize = FixTextureSize(inInfo.inWidth, inInfo.inHeight, inInfo.inPixelFormat);
		texture_extent = VkExtent3D(std::get<0>(FixedSize), std::get<1>(FixedSize), 1);
		
		vulkan_rhi = rhi.as<SCVulkanRHI>();
		VkImageCreateInfo vk_image_create_info{};
		vk_image_create_info.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vk_image_create_info.flags = 0;
		vk_image_create_info.pNext = nullptr;
		vk_image_create_info.arrayLayers = 1;
		vk_image_create_info.extent = texture_extent;
		vk_image_create_info.format = SCVulkanTextureHelper::ConvertToVkFormat(inInfo.inPixelFormat);
		vk_image_create_info.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		vk_image_create_info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		vk_image_create_info.mipLevels = inInfo.inMipMapLevels;
		vk_image_create_info.samples =  static_cast<VkSampleCountFlagBits>(1 << (inInfo.inSampleCount - 1));
		vk_image_create_info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		vk_image_create_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		vk_image_create_info.usage = vkUsage;/// SCVulkanTextureHelper::ConvertToVkImageUsage(inInfo.inUsage);

		if(vkCreateImage(vulkan_rhi->get_device(), &vk_image_create_info, vulkan_rhi->get_allocator(),&m_vkimage) != VkResult::VK_SUCCESS)
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
		for(; memory_type_index < physical_device_memory_properties.memoryTypeCount; ++memory_type_index)
		{
			if((memory_requirements.memoryTypeBits & (1<< memory_type_index)) 
				&& (physical_device_memory_properties.memoryTypes[memory_type_index].propertyFlags & needed_memory_property)== needed_memory_property)
			{
				//memory_allocate_info.memoryTypeIndex = memory_type_index;
				break;
			}
		}

		if(memory_type_index == physical_device_memory_properties.memoryTypeCount)
		{
			vkDestroyImage(vulkan_rhi->get_device(),m_vkimage,vulkan_rhi->get_allocator());
			m_vkimage = VK_NULL_HANDLE;
			vulkan_rhi = nullptr;
			return false;
		}
		memory_allocate_info.memoryTypeIndex = memory_type_index;

		if(vkAllocateMemory(vulkan_rhi->get_device(), &memory_allocate_info, vulkan_rhi->get_allocator(), &m_device_memory) != VK_SUCCESS)
		{
			vkDestroyImage(vulkan_rhi->get_device(), m_vkimage, vulkan_rhi->get_allocator());
			m_vkimage = VK_NULL_HANDLE;
			m_device_memory = VK_NULL_HANDLE;
			vulkan_rhi = nullptr;
			return false;
		}

		if(vkBindImageMemory(vulkan_rhi->get_device(), m_vkimage, m_device_memory, 0) != VK_SUCCESS)
		{
			vkDestroyImage(vulkan_rhi->get_device(), m_vkimage, vulkan_rhi->get_allocator());
			m_vkimage = VK_NULL_HANDLE;
			vkFreeMemory(vulkan_rhi->get_device(), m_device_memory, vulkan_rhi->get_allocator());
			m_device_memory = VK_NULL_HANDLE;
			vulkan_rhi = nullptr;
			return false;
		}
		m_pixel_format = inInfo.inPixelFormat;
		return true;
	}

public:
	std::vector<std::uint8_t> get_raw_data() const override
	{
		return {};
	}

	bool set_raw_data(const std::vector<std::uint8_t>& inData, uint32_t inMipmapLevel) override;

	bool init(SSPtr<SCRHIInterface> rhi, const SSRHITexture2DCreateInfo& inInfo) override
	{
		return internal_init(rhi, inInfo);
	}

	void release() override
	{
		internal_release();
	}

	~SCVulkanTexture2D() override
	{
		internal_release();
	}

	VkImage get_inner() const { return m_vkimage; }
	void transition_layout_for_copy(VkImageLayout inFrom, VkImageLayout inTarget, uint32_t inChangeMip, uint32_t inChangeMipCount, uint32_t inChangeBaseLayer, uint32_t inChangeLayerCount, SSRHITextureAspectFlags inChangeAspect);
};
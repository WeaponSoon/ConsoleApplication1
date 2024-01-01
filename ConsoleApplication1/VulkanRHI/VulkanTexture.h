#pragma once
#include "Core/RHI/RHITexture.h"
#include "VulkanRHI/VulkanRHI.h"


class SCT2DLayoutTransientCmd;


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
	SSPtr<SCVulkanRHI> vulkan_rhi;

	VkImage m_vkimage = VK_NULL_HANDLE;
	VkDeviceMemory m_device_memory = VK_NULL_HANDLE;
	VkImageView m_vkimage_view = VK_NULL_HANDLE;

	VkImageLayout m_curLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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

	bool internal_init(SSPtr<SCRHIInterface> rhi, const SSRHITexture2DCreateInfo& inInfo);

	

public:
	std::vector<std::uint8_t> get_raw_data() const override
	{
		return {};
	}

	bool set_raw_data(const std::vector<std::uint8_t>& inData, uint32_t inMipmapLevel) override;

	virtual bool get_raw_data(std::vector<std::uint8_t>& outData, uint32_t inMipmapLevel) override;

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
};
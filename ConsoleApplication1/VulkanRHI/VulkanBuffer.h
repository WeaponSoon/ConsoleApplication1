#pragma once
#include "Core/RHI/RHIBuffer.h"
#include "VulkanRHI/VulkanRHI.h"

class SCVulkanBuffer:public SCRHIBuffer
{
protected:
	VkBuffer m_buffer;
	SSWeakPtr<SCVulkanRHI> vulkan_rhi;
	VkDeviceMemory m_memory;
public:
	void init(const SSPtr<SCRHIInterface>& inInterface, SSRHIBufferCreateInfo& createInfo) override;
	virtual void set_data(const std::vector<uint8_t>& inData) override;
	void release() override { unint_internal(); }
	void unint_internal();

	~SCVulkanBuffer() override { unint_internal(); }

	VkBuffer get_inner_buffer() const { return m_buffer; }
	VkDeviceMemory get_inner_mem() const { return m_memory; }
};
#pragma once
#include <list>

#include "Core/RHI/RHIInterface.h"

#include "vulkan/vulkan.h"
#include "atomic"

class SCVulkanRHI;

class SCRHIVulkanDeviceFence : public SCRHIDeviceFence
{
	SSPtr<SCVulkanRHI> rhi;
	VkFence m_fence = VK_NULL_HANDLE;
public:
	SCRHIVulkanDeviceFence(SSPtr<SCRHIInterface> InRhi);
	~SCRHIVulkanDeviceFence() override;
	bool IsReady() const override;
	void Reset() override;
	VkFence& GetInnerFence() { return m_fence; }

};

class SCRHIVulkanDeviceSemaphore : public SCRHIDeviceSemaphore
{
	SSPtr<SCVulkanRHI> rhi;
	VkSemaphore m_semaphore = VK_NULL_HANDLE;
public:
	SCRHIVulkanDeviceSemaphore(SSPtr<SCRHIInterface> InRhi);
	~SCRHIVulkanDeviceSemaphore() override;

	VkSemaphore GetInnerSemaphore() const { return m_semaphore; }
};

class SCVulkanCommandBuffer : public SCRHICommandBuffer
{
private:
	SSPtr<SCRHIVulkanDeviceFence> m_fence;
	SSWeakPtr<SCVulkanRHI> rhi;
	VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
public:
	std::list<SSPtr<SCVulkanCommandBuffer>>::iterator it;
	std::list<SSPtr<SCVulkanCommandBuffer>>* op = nullptr;

	void Init(const SSPtr<SCRHIInterface>& InRhi, SECommandBufferLifeType InLifeType);
	void on_begin_record() override;
	void on_end_record() override;
	void on_submit(const std::vector<SSRHICommandBufferWaitInfo>& InWaitInfo, const std::vector<SSRHICommandBufferTriggerInfo>& InTriggerInfo) override;
	void wait_until_finish(uint64_t InOutTime = 0xffffffffffffffffull) override;

	void uninit();

	void unint_internal();

	SSPtr<SCVulkanRHI> get_vulkan_rhi() const { return rhi.lock(); }

	VkCommandBuffer get_internal_buffer() const { return m_command_buffer; }

	~SCVulkanCommandBuffer() override
	{
		unint_internal();
	}

};

class SCVulkanRHI : public SCRHIInterface
{
private:
	std::list<SSPtr<SCVulkanCommandBuffer>> m_command_buffers;

	std::atomic_flag m_init_flag = ATOMIC_FLAG_INIT;
	VkInstance m_instance = VK_NULL_HANDLE;
	SERHIStatus m_status = SERHIStatus::NotInit;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_graphics_queue = VK_NULL_HANDLE;
	VkQueue m_presentation_queue = VK_NULL_HANDLE;

	VkCommandPool m_graphics_command_pool = VK_NULL_HANDLE;
	VkCommandPool m_present_command_pool = VK_NULL_HANDLE;

	uint32_t queue_family_index_graphics_to_use = 0xffffffff;
	uint32_t queue_family_index_presentation_to_use = 0xffffffff;

	VkPhysicalDevice physical_device_to_use = VK_NULL_HANDLE;

	VkAllocationCallbacks* m_memory = nullptr;

	void internal_uninit();

public:
	~SCVulkanRHI();

	void init() override;
	void uninit() override;
	SERHIStatus status() const override;

	virtual SSPtr<SCRHICommandBuffer> allocate_command_buffer(SECommandBufferLifeType InLifeType) override;
	void reset_command_buffer(SSPtr<SCRHICommandBuffer>& InBuffer) override;

	VkInstance get_instance() { return m_instance; }
	VkDevice get_device() { return m_device; }
	VkQueue get_graphics_queue() { return m_graphics_queue; }
	VkQueue get_presentation_queue() { return m_presentation_queue == VK_NULL_HANDLE ? m_graphics_queue : m_presentation_queue; }
	uint32_t get_graphics_queue_family_index() { return queue_family_index_graphics_to_use; }
	uint32_t get_presentation_queue_family_index() { return m_presentation_queue == VK_NULL_HANDLE ? queue_family_index_graphics_to_use : queue_family_index_presentation_to_use; }
	VkPhysicalDevice get_physical_device() { return physical_device_to_use; }

	VkCommandPool get_graphics_pool() const { return m_graphics_command_pool; }

	VkAllocationCallbacks* get_allocator() { return m_memory; }
};
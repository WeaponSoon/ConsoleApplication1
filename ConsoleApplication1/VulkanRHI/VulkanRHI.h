#pragma once
#include "Core/RHI/RHIInterface.h"

#include "vulkan/vulkan.h"
#include "atomic"

class SCVulkanRHI : public SCRHIInterface
{
private:
	std::atomic_flag m_init_flag = ATOMIC_FLAG_INIT;
	VkInstance m_instance = VK_NULL_HANDLE;
	SERHIStatus m_status = SERHIStatus::NotInit;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_graphics_queue = VK_NULL_HANDLE;
	VkQueue m_presentation_queue = VK_NULL_HANDLE;

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

	VkInstance get_instance() { return m_instance; }
	VkDevice get_device() { return m_device; }
	VkQueue get_graphics_queue() { return m_graphics_queue; }
	VkQueue get_presentation_queue() { return m_presentation_queue == VK_NULL_HANDLE ? m_graphics_queue : m_presentation_queue; }
	uint32_t get_graphics_queue_family_index() { return queue_family_index_graphics_to_use; }
	uint32_t get_presentation_queue_family_index() { return m_presentation_queue == VK_NULL_HANDLE ? queue_family_index_graphics_to_use : queue_family_index_presentation_to_use; }
	VkPhysicalDevice get_physical_device() { return physical_device_to_use; }

	VkAllocationCallbacks* get_allocator() { return m_memory; }
};
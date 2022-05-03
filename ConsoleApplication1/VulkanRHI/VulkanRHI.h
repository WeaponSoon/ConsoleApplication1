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

	VkAllocationCallbacks* m_memory = nullptr;
public:
	void init() override;
	void uninit() override;
	SERHIStatus status() const override;
};
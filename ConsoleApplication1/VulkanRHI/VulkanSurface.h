#pragma once
#include "Core/RHI/Surface.h"
#include "vulkan/vulkan.h"


class SCVulkanSurface : public SCSurface
{
private:

	friend void global_key_callback(struct GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void global_framebuffer_size_callback(GLFWwindow* window, int width, int height);
	friend void global_mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

	std::atomic_flag m_init_flag = ATOMIC_FLAG_INIT;

	SSPtr<class SCVulkanRHI> vulkan_rhi;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	struct GLFWwindow* window = nullptr;

	uint32_t cur_width = 0;
	uint32_t cur_height = 0;

	std::string title;

	void onresize(uint32_t width, uint32_t height);
	void internal_uninit();

	std::vector<VkImage> swapchain_images;

public:
	~SCVulkanSurface();
	 
	virtual void init(SSPtr<SCRHIInterface> rhi, uint32_t width, uint32_t height) override;
	void resize(uint32_t width, uint32_t height) override;
	void uninit() override;
	bool is_valid() override;
	virtual void set_title(const std::string& in_title) override;

	virtual std::tuple<uint32_t, uint32_t> get_size() const override { return std::make_tuple(cur_width, cur_height); }
	virtual std::string get_title() const override { return title; };

};
// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include "Core/CoreObject/SCObject.h"
#include "VulkanRHI/VulkanRHI.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"


class SCTest1 : public SCObject
{
	
};

class SCTest2 : public SCObject
{
	
};

class SCTest3 : public SCTest1
{
	
};

void PtrTest()
{
    SSPtr<SCObject> pObject = SSPtr<SCObject>::construct<SCTest3>();
    SSPtr<SCTest1> pTest1_c = pObject.as<SCTest1>();
    SSPtr<SCTest1> pTest1_a = nullptr;
    pTest1_a = pObject.as<SCTest1>();
    SSPtr<SCTest1> pTest1_a_2 = nullptr;
    pTest1_a_2 = pObject.as<SCTest3>();
    SSPtr<SCTest2> pTest2_c = pObject.as<SCTest2>();
    SSPtr<SCTest2> pTest2_a = nullptr;
    pTest2_a = pObject.as<SCTest2>();

    SSPtr<SCObject> pObject_a = pTest1_a.as<SCObject>();
    SSPtr<SCObject> pObject_c = nullptr;
    pObject_c = pTest1_a;
}

int main()
{
    SSPtr<SCRHIInterface> RHIInterface = SSPtr<SCRHIInterface>::construct<SCVulkanRHI>();
    RHIInterface->init();
    RHIInterface->uninit();


 //   glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
	//glfwInit();
 //   VkApplicationInfo vk_application_info;
 //   vk_application_info.pNext = nullptr;
 //   vk_application_info.apiVersion = VK_API_VERSION_1_3;
 //   vk_application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
 //   vk_application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
 //   vk_application_info.pApplicationName = "Test App";
 //   vk_application_info.pEngineName = "Test Engine";
 //   vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

 //   uint32_t InstanceExtensionCount = 0;
 //   const char** InstanceExtensions = glfwGetRequiredInstanceExtensions(&InstanceExtensionCount);

 //   VkInstanceCreateInfo vk_instance_create_info;
 //   vk_instance_create_info.pNext = nullptr;
 //   vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
 //   std::vector<const char*> Layers = {
 //       "VK_LAYER_KHRONOS_validation"
 //   };
 //   vk_instance_create_info.enabledExtensionCount = InstanceExtensionCount;
 //   vk_instance_create_info.enabledLayerCount = Layers.size();
 //   vk_instance_create_info.flags = 0;
 //   vk_instance_create_info.pApplicationInfo = &vk_application_info;
 //   vk_instance_create_info.ppEnabledExtensionNames = InstanceExtensions;
 //   vk_instance_create_info.ppEnabledLayerNames = Layers.data();

 //   VkInstance Instance;
 //   VkResult Res = vkCreateInstance(&vk_instance_create_info, nullptr, &Instance);

 //   uint32_t PhysicalDeviceCount = 0;
 //   vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);
 //   std::vector<VkPhysicalDevice> physical_devices;
 //   physical_devices.resize(PhysicalDeviceCount);
 //   vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, physical_devices.data());
 //   VkPhysicalDeviceProperties physical_device_properties;
 //   vkGetPhysicalDeviceProperties(physical_devices[0], &physical_device_properties);
 //   uint32_t PhysicalDeviceQueueFamilyCount = 0;
 //   vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], &PhysicalDeviceQueueFamilyCount, nullptr);
 //   std::vector<VkQueueFamilyProperties> queue_family_propertieses;
 //   queue_family_propertieses.resize(PhysicalDeviceQueueFamilyCount);
 //   vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], &PhysicalDeviceQueueFamilyCount, queue_family_propertieses.data());

 //   VkPhysicalDeviceFeatures vk_physical_device_features;
 //   vkGetPhysicalDeviceFeatures(physical_devices[0], &vk_physical_device_features);

 //   uint32_t suitable_queue_family_Index = 0;
 //   for(uint32_t i = 0; i < queue_family_propertieses.size(); ++i)
 //   {
 //       auto& queue_family_properties = queue_family_propertieses[i];
	//    if(queue_family_properties.queueFlags | VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
	//    {
 //           suitable_queue_family_Index = i;
 //           break;
	//    }
 //   }
 //   float queue_priority = 1.0f;
 //   VkDeviceQueueCreateInfo vk_device_queue_create_info;
 //   vk_device_queue_create_info.flags = 0;
 //   vk_device_queue_create_info.pNext = nullptr;
 //   vk_device_queue_create_info.queueCount = 1;
 //   vk_device_queue_create_info.queueFamilyIndex = suitable_queue_family_Index;
 //   vk_device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
 //   vk_device_queue_create_info.pQueuePriorities = &queue_priority;

 //   VkDeviceCreateInfo vk_device_create_info;
 //   vk_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
 //   vk_device_create_info.enabledExtensionCount = 0;
	//vk_device_create_info.ppEnabledExtensionNames = nullptr;
 //   vk_device_create_info.enabledLayerCount = 0;
 //   vk_device_create_info.ppEnabledLayerNames = nullptr;
 //   vk_device_create_info.flags = 0;
 //   vk_device_create_info.pEnabledFeatures = &vk_physical_device_features;
 //   vk_device_create_info.pNext = nullptr;
 //   vk_device_create_info.pQueueCreateInfos = &vk_device_queue_create_info;
 //   vk_device_create_info.queueCreateInfoCount = 1;

 //   VkDevice Device;
 //   vkCreateDevice(physical_devices[0], &vk_device_create_info, nullptr, &Device);
 //   VkQueue Queue;
 //   vkGetDeviceQueue(Device, suitable_queue_family_Index, 0, &Queue);
 //   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
 //   GLFWwindow* Window = glfwCreateWindow(800, 600, "test window", nullptr, nullptr);

 //   VkSurfaceKHR surface_khr;
 //   VkResult KHRResult = glfwCreateWindowSurface(Instance, Window, nullptr, &surface_khr);



 //   while (!glfwWindowShouldClose(Window))
 //   {
 //       glfwPollEvents();
 //   }

 //   vkDestroyDevice(Device, nullptr);
 //   Device = VK_NULL_HANDLE;
 //   vkDestroyInstance(Instance, nullptr);
 //   Instance = VK_NULL_HANDLE;

    std::cout << "Hello World!\n";
}

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

    std::cout << "Hello World!\n";
}

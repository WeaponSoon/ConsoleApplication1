// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include "Core/CoreObject/SCObject.h"
#include "VulkanRHI/VulkanRHI.h"
#include "VulkanRHI/VulkanSurface.h"
#define GLFW_INCLUDE_VULKAN
#include "Core/Display/Window/Window.h"
#include "Core/RHI/RHITexture.h"
#include "GLFW/glfw3.h"
#include "VulkanRHI/VulkanTexture.h"


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

SSPtr<SCObject>& dsda()
{
	static SSPtr<SCObject> s;
    return s;
}



int main()
{
    {
        SSPtr<SCObject> la = SSPtr<SCObject>::construct<SCTest1>();
        la = nullptr;
    }
    //dsda() = SSPtr<SCObject>::construct<SCObject>();
    //注册全局状态
    SSPtr<SCRHIInterface> RHIInterface = SSPtr<SCRHIInterface>::construct<SCVulkanRHI>();
    RHIInterface->init();
    RHIInterface->make_current();
    SCRHISurface::get_surface_creator().bind([]()->SSPtr<SCRHISurface> {return SSPtr<SCRHISurface>::construct<SCVulkanSurface>(); });

    auto TestCommandBuffer = RHIInterface->allocate_command_buffer(SECommandBufferLifeType::ExecuteMulti);

    TestCommandBuffer->begin_record();
    TestCommandBuffer->end_record();
    TestCommandBuffer->submit(std::vector<SSRHICommandBufferWaitInfo>(), std::vector<SSRHICommandBufferTriggerInfo>());
    TestCommandBuffer->wait_until_finish();

    TestCommandBuffer->submit(std::vector<SSRHICommandBufferWaitInfo>(), std::vector<SSRHICommandBufferTriggerInfo>());
    TestCommandBuffer->wait_until_finish();

    SSPtr<SCRHITexture2D> tt = SSPtr<SCRHITexture2D>::construct<SCVulkanTexture2D>();
    SSRHITexture2DCreateInfo ttc;
    ttc.inUsage = SERHITextureUsage::TU_RenderTargetDepthResource;
    ttc.inPixelFormat = SERHIPixelFormat::PF_D24_UNORM_S8_UINT;
    ttc.inWidth = 2;
    ttc.inHeight = 2;
    ttc.inSampleCount = 1;
    ttc.inMipMapLevels = 2;
    ttc.inAccessType = SERHITextureAccessType::TAT_HOST_AND_DEVICE;
    tt->init(RHIInterface, ttc);



    std::vector<uint8_t> texture_data
    {
        0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15,
        7,6,5,4,3,2,1,0, 0,1,2,3,4,5,6,7,
    };

    std::vector<uint8_t> texture_data1
    {
        0,0,0,0,0,0,0,0,
    };


    tt->set_raw_data(texture_data,0);
    tt->set_raw_data(texture_data, 1);

    std::vector<uint8_t> out_texture_data;
    tt->get_raw_data(out_texture_data, 0);

    SSPtr<SCWindow> Win = SSPtr<SCWindow>::construct<SCWindow>();
    Win->init("hahaha", 500, 500);

    while (Win->is_valid())
    {
        glfwPollEvents();
    }
    tt->release();
    //销毁全局状态
    RHIInterface->uninit();
    RHIInterface->make_no_current();
    RHIInterface = nullptr;
    std::cout << "Hello World!\n";
}

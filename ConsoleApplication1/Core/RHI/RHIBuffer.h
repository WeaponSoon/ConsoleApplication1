#pragma once
#include <vector>

#include "Core/RHI/RHIResource.h"
#include "Core/RHI/RHIBufferDefs.h"

struct SSRHIBufferCreateInfo
{
	SCRHIBufferUsageFlags Usage = 0;
	uint64_t size = 0;
	bool hostVisible = false;
};

class SCRHIBuffer : public SCRHIResource
{
protected:
	SCRHIBufferUsageFlags Usage = 0;
	uint64_t size = 0;
	bool hostVisible = false;
public:
	virtual void init(const SSPtr<SCRHIInterface>& inInterface, SSRHIBufferCreateInfo& createInfo) = 0;
	virtual void set_data(const std::vector<uint8_t>& inData) = 0;
	virtual void get_data(std::vector<uint8_t>& outData) = 0;
	virtual void release() = 0;
};
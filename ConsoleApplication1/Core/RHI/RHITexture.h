#pragma once
#include <vector>

#include "Core/RHI/RHIResource.h"
#include "Core/RHI/RHITextureDefs.h"
#include "Core/RHI/RHIBuffer.h"

class SCRHITexture : public SCRHIResource
{
protected:
	SERHIPixelFormat m_pixel_format = SERHIPixelFormat::PF_UNDEFINED;
	SERHITextureUsage m_texture_usage = SERHITextureUsage::TU_ShaderResource;

public:
	SERHITextureUsage get_texture_flags() const { return m_texture_usage; }
	SERHIPixelFormat get_pixel_format() const { return m_pixel_format; }
	virtual std::vector<std::uint8_t> get_raw_data() const = 0;
	virtual bool set_raw_data(const std::vector<std::uint8_t>& inData, uint32_t inMipmapLevel) = 0;
};

struct SSRHITexture2DCreateInfo
{
	std::uint16_t inWidth = 1;
	std::uint16_t inHeight = 1;
	std::uint16_t inSampleCount = 1;
	std::uint16_t inMipMapLevels = 1;

	SERHITextureAccessType inAccessType = SERHITextureAccessType::TAT_DEVICE;
	SERHITextureUsage inUsage = SERHITextureUsage::TU_ShaderResource;

	SERHIPixelFormat inPixelFormat = SERHIPixelFormat::PF_UNDEFINED;
};

struct SSRHITexture3DCreateInfo : public SSRHITexture2DCreateInfo
{
	std::uint16_t inDepth = 1;
};

class SCRHITexture2D : public SCRHITexture
{
public:
	virtual bool init(SSPtr<SCRHIInterface> rhi, const SSRHITexture2DCreateInfo& inInfo) = 0;
	virtual void release() = 0;
};

class SCRHITexture3D : public SCRHITexture
{
	
};

class SCRHITextureCube : public SCRHITexture
{
	
};

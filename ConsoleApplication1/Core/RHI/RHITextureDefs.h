#pragma once
#include <map>

#include "Core/CoreObject/SCObject.h"

enum class SERHITextureUsage
{
    TU_RenderTargetColor,
    TU_RenderTargetDepthStencil,
    TU_RenderTargetDepthResource,
    TU_ShaderResource,
    TU_UnorderedAccess,
};

enum class SERHITextureAccessType
{
	TAT_DEVICE,
    TAT_HOST_AND_DEVICE
};

enum class SERHIPixelFormat
{
    PF_UNDEFINED = 0,
    PF_R4G4_UNORM_PACK8 = 1,
    PF_R4G4B4A4_UNORM_PACK16 = 2,
    PF_B4G4R4A4_UNORM_PACK16 = 3,
    PF_R5G6B5_UNORM_PACK16 = 4,
    PF_B5G6R5_UNORM_PACK16 = 5,
    PF_R5G5B5A1_UNORM_PACK16 = 6,
    PF_B5G5R5A1_UNORM_PACK16 = 7,
    PF_A1R5G5B5_UNORM_PACK16 = 8,
    PF_R8_UNORM = 9,
    PF_R8_SNORM = 10,
    PF_R8_USCALED = 11,
    PF_R8_SSCALED = 12,
    PF_R8_UINT = 13,
    PF_R8_SINT = 14,
    PF_R8_SRGB = 15,
    PF_R8G8_UNORM = 16,
    PF_R8G8_SNORM = 17,
    PF_R8G8_USCALED = 18,
    PF_R8G8_SSCALED = 19,
    PF_R8G8_UINT = 20,
    PF_R8G8_SINT = 21,
    PF_R8G8_SRGB = 22,
    PF_R8G8B8_UNORM = 23,
    PF_R8G8B8_SNORM = 24,
    PF_R8G8B8_USCALED = 25,
    PF_R8G8B8_SSCALED = 26,
    PF_R8G8B8_UINT = 27,
    PF_R8G8B8_SINT = 28,
    PF_R8G8B8_SRGB = 29,
    PF_B8G8R8_UNORM = 30,
    PF_B8G8R8_SNORM = 31,
    PF_B8G8R8_USCALED = 32,
    PF_B8G8R8_SSCALED = 33,
    PF_B8G8R8_UINT = 34,
    PF_B8G8R8_SINT = 35,
    PF_B8G8R8_SRGB = 36,
    PF_R8G8B8A8_UNORM = 37,
    PF_R8G8B8A8_SNORM = 38,
    PF_R8G8B8A8_USCALED = 39,
    PF_R8G8B8A8_SSCALED = 40,
    PF_R8G8B8A8_UINT = 41,
    PF_R8G8B8A8_SINT = 42,
    PF_R8G8B8A8_SRGB = 43,
    PF_B8G8R8A8_UNORM = 44,
    PF_B8G8R8A8_SNORM = 45,
    PF_B8G8R8A8_USCALED = 46,
    PF_B8G8R8A8_SSCALED = 47,
    PF_B8G8R8A8_UINT = 48,
    PF_B8G8R8A8_SINT = 49,
    PF_B8G8R8A8_SRGB = 50,
    PF_A8B8G8R8_UNORM_PACK32 = 51,
    PF_A8B8G8R8_SNORM_PACK32 = 52,
    PF_A8B8G8R8_USCALED_PACK32 = 53,
    PF_A8B8G8R8_SSCALED_PACK32 = 54,
    PF_A8B8G8R8_UINT_PACK32 = 55,
    PF_A8B8G8R8_SINT_PACK32 = 56,
    PF_A8B8G8R8_SRGB_PACK32 = 57,
    PF_A2R10G10B10_UNORM_PACK32 = 58,
    PF_A2R10G10B10_SNORM_PACK32 = 59,
    PF_A2R10G10B10_USCALED_PACK32 = 60,
    PF_A2R10G10B10_SSCALED_PACK32 = 61,
    PF_A2R10G10B10_UINT_PACK32 = 62,
    PF_A2R10G10B10_SINT_PACK32 = 63,
    PF_A2B10G10R10_UNORM_PACK32 = 64,
    PF_A2B10G10R10_SNORM_PACK32 = 65,
    PF_A2B10G10R10_USCALED_PACK32 = 66,
    PF_A2B10G10R10_SSCALED_PACK32 = 67,
    PF_A2B10G10R10_UINT_PACK32 = 68,
    PF_A2B10G10R10_SINT_PACK32 = 69,
    PF_R16_UNORM = 70,
    PF_R16_SNORM = 71,
    PF_R16_USCALED = 72,
    PF_R16_SSCALED = 73,
    PF_R16_UINT = 74,
    PF_R16_SINT = 75,
    PF_R16_SFLOAT = 76,
    PF_R16G16_UNORM = 77,
    PF_R16G16_SNORM = 78,
    PF_R16G16_USCALED = 79,
    PF_R16G16_SSCALED = 80,
    PF_R16G16_UINT = 81,
    PF_R16G16_SINT = 82,
    PF_R16G16_SFLOAT = 83,
    PF_R16G16B16_UNORM = 84,
    PF_R16G16B16_SNORM = 85,
    PF_R16G16B16_USCALED = 86,
    PF_R16G16B16_SSCALED = 87,
    PF_R16G16B16_UINT = 88,
    PF_R16G16B16_SINT = 89,
    PF_R16G16B16_SFLOAT = 90,
    PF_R16G16B16A16_UNORM = 91,
    PF_R16G16B16A16_SNORM = 92,
    PF_R16G16B16A16_USCALED = 93,
    PF_R16G16B16A16_SSCALED = 94,
    PF_R16G16B16A16_UINT = 95,
    PF_R16G16B16A16_SINT = 96,
    PF_R16G16B16A16_SFLOAT = 97,
    PF_R32_UINT = 98,
    PF_R32_SINT = 99,
    PF_R32_SFLOAT = 100,
    PF_R32G32_UINT = 101,
    PF_R32G32_SINT = 102,
    PF_R32G32_SFLOAT = 103,
    PF_R32G32B32_UINT = 104,
    PF_R32G32B32_SINT = 105,
    PF_R32G32B32_SFLOAT = 106,
    PF_R32G32B32A32_UINT = 107,
    PF_R32G32B32A32_SINT = 108,
    PF_R32G32B32A32_SFLOAT = 109,
    PF_R64_UINT = 110,
    PF_R64_SINT = 111,
    PF_R64_SFLOAT = 112,
    PF_R64G64_UINT = 113,
    PF_R64G64_SINT = 114,
    PF_R64G64_SFLOAT = 115,
    PF_R64G64B64_UINT = 116,
    PF_R64G64B64_SINT = 117,
    PF_R64G64B64_SFLOAT = 118,
    PF_R64G64B64A64_UINT = 119,
    PF_R64G64B64A64_SINT = 120,
    PF_R64G64B64A64_SFLOAT = 121,
    PF_B10G11R11_UFLOAT_PACK32 = 122,
    PF_E5B9G9R9_UFLOAT_PACK32 = 123,
    PF_D16_UNORM = 124,
    PF_X8_D24_UNORM_PACK32 = 125,
    PF_D32_SFLOAT = 126,
    PF_S8_UINT = 127,
    PF_D16_UNORM_S8_UINT = 128,
    PF_D24_UNORM_S8_UINT = 129,
    PF_D32_SFLOAT_S8_UINT = 130,
    PF_BC1_RGB_UNORM_BLOCK = 131,
    PF_BC1_RGB_SRGB_BLOCK = 132,
    PF_BC1_RGBA_UNORM_BLOCK = 133,
    PF_BC1_RGBA_SRGB_BLOCK = 134,
    PF_BC2_UNORM_BLOCK = 135,
    PF_BC2_SRGB_BLOCK = 136,
    PF_BC3_UNORM_BLOCK = 137,
    PF_BC3_SRGB_BLOCK = 138,
    PF_BC4_UNORM_BLOCK = 139,
    PF_BC4_SNORM_BLOCK = 140,
    PF_BC5_UNORM_BLOCK = 141,
    PF_BC5_SNORM_BLOCK = 142,
    PF_BC6H_UFLOAT_BLOCK = 143,
    PF_BC6H_SFLOAT_BLOCK = 144,
    PF_BC7_UNORM_BLOCK = 145,
    PF_BC7_SRGB_BLOCK = 146,
    PF_ETC2_R8G8B8_UNORM_BLOCK = 147,
    PF_ETC2_R8G8B8_SRGB_BLOCK = 148,
    PF_ETC2_R8G8B8A1_UNORM_BLOCK = 149,
    PF_ETC2_R8G8B8A1_SRGB_BLOCK = 150,
    PF_ETC2_R8G8B8A8_UNORM_BLOCK = 151,
    PF_ETC2_R8G8B8A8_SRGB_BLOCK = 152,
    PF_EAC_R11_UNORM_BLOCK = 153,
    PF_EAC_R11_SNORM_BLOCK = 154,
    PF_EAC_R11G11_UNORM_BLOCK = 155,
    PF_EAC_R11G11_SNORM_BLOCK = 156,
    PF_ASTC_4x4_UNORM_BLOCK = 157,
    PF_ASTC_4x4_SRGB_BLOCK = 158,
    PF_ASTC_5x4_UNORM_BLOCK = 159,
    PF_ASTC_5x4_SRGB_BLOCK = 160,
    PF_ASTC_5x5_UNORM_BLOCK = 161,
    PF_ASTC_5x5_SRGB_BLOCK = 162,
    PF_ASTC_6x5_UNORM_BLOCK = 163,
    PF_ASTC_6x5_SRGB_BLOCK = 164,
    PF_ASTC_6x6_UNORM_BLOCK = 165,
    PF_ASTC_6x6_SRGB_BLOCK = 166,
    PF_ASTC_8x5_UNORM_BLOCK = 167,
    PF_ASTC_8x5_SRGB_BLOCK = 168,
    PF_ASTC_8x6_UNORM_BLOCK = 169,
    PF_ASTC_8x6_SRGB_BLOCK = 170,
    PF_ASTC_8x8_UNORM_BLOCK = 171,
    PF_ASTC_8x8_SRGB_BLOCK = 172,
    PF_ASTC_10x5_UNORM_BLOCK = 173,
    PF_ASTC_10x5_SRGB_BLOCK = 174,
    PF_ASTC_10x6_UNORM_BLOCK = 175,
    PF_ASTC_10x6_SRGB_BLOCK = 176,
    PF_ASTC_10x8_UNORM_BLOCK = 177,
    PF_ASTC_10x8_SRGB_BLOCK = 178,
    PF_ASTC_10x10_UNORM_BLOCK = 179,
    PF_ASTC_10x10_SRGB_BLOCK = 180,
    PF_ASTC_12x10_UNORM_BLOCK = 181,
    PF_ASTC_12x10_SRGB_BLOCK = 182,
    PF_ASTC_12x12_UNORM_BLOCK = 183,
    PF_ASTC_12x12_SRGB_BLOCK = 184,
};

struct SSTextureBlockDesc
{
    int sizePerBlock = 0;
    int numCollumePerBlock = 0;
    int numRowPerBlock = 0;
    SSTextureBlockDesc() = default;
    SSTextureBlockDesc(const SSTextureBlockDesc&) = default;
    SSTextureBlockDesc(const std::pair<int, int>& inPair) : sizePerBlock(inPair.first), numCollumePerBlock(inPair.second), numRowPerBlock(inPair.second) {}
    SSTextureBlockDesc(const std::tuple<int, int, int>& inTuple) : sizePerBlock(std::get<0>(inTuple)), numCollumePerBlock(std::get<1>(inTuple)), numRowPerBlock(std::get<2>(inTuple)) {}

};

extern std::map<SERHIPixelFormat, SSTextureBlockDesc> SCIformatSizeMap;

inline SSTextureBlockDesc GetPixelFormatBlockSizeAndPerBlockNumPixels(SERHIPixelFormat InFormat)
{


    {
        auto&& res = SCIformatSizeMap.find(InFormat);
        if(res != SCIformatSizeMap.end())
        {
            return res->second;
        }
        return {};
    }
}

inline bool ContainsDepth(SERHIPixelFormat Format)
{
	switch (Format)
	{
	case SERHIPixelFormat::PF_D16_UNORM:
	case SERHIPixelFormat::PF_X8_D24_UNORM_PACK32:
	case SERHIPixelFormat::PF_D32_SFLOAT:
    case SERHIPixelFormat::PF_D16_UNORM_S8_UINT:
	case SERHIPixelFormat::PF_D24_UNORM_S8_UINT:
	case SERHIPixelFormat::PF_D32_SFLOAT_S8_UINT:
        return true;
	default:
        return false;
	}
}

inline bool ContainsStencil(SERHIPixelFormat Format)
{
    switch (Format)
    {
    case SERHIPixelFormat::PF_S8_UINT:
    case SERHIPixelFormat::PF_D16_UNORM_S8_UINT:
    case SERHIPixelFormat::PF_D24_UNORM_S8_UINT:
    case SERHIPixelFormat::PF_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}
inline unsigned int NearestPowerOfTwo(unsigned int num) {
    int exponent = static_cast<int>(std::floor(std::log2(num)));
    return static_cast<unsigned int>(std::pow(2, exponent));
}
inline uint32_t GetMaxMipLevels(uint32_t width, uint32_t height) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max({ width, height })))) + 1;
}
inline std::tuple<int,int> FixTextureSize(int X, int Y, SERHIPixelFormat Format)
{
	if(Format <= SERHIPixelFormat::PF_D32_SFLOAT_S8_UINT)//non-compress format
	{
        return std::make_tuple(X, Y);
	}
    if(Format <= SERHIPixelFormat::PF_EAC_R11G11_SNORM_BLOCK)//BC1 to BC7 and ETC2
    {
        return std::make_tuple((X + 3) / 4 * 4, (Y + 3) / 4 * 4);
    }
    return std::make_tuple(X, Y); //ASTC
}

inline std::tuple<int, int> GetSizeAtMipLevel(int baseWidth, int baseHeight, int mipLevel)
{
    int width = std::max(1, baseWidth >> mipLevel);
    int height = std::max(1, baseHeight >> mipLevel);

    return std::make_tuple(width, height);
}

inline int CalcTextureSizeInByte(int X, int Y, SERHIPixelFormat Format, int Mip)
{
    auto&& s = GetSizeAtMipLevel(X, Y, Mip);
    auto&& r = GetPixelFormatBlockSizeAndPerBlockNumPixels(Format);

    /*switch (Format)
    {
    case SERHIPixelFormat::PF_X8_D24_UNORM_PACK32:
    case SERHIPixelFormat::PF_D16_UNORM_S8_UINT:
    case SERHIPixelFormat::PF_D24_UNORM_S8_UINT:
    case SERHIPixelFormat::PF_D32_SFLOAT_S8_UINT:
    case SERHIPixelFormat::PF_S8_UINT:
        r.sizePerBlock -= 1;
        break;
    default:
        ;
    }*/

    return (std::get<0>(s) + r.numCollumePerBlock - 1) / r.numCollumePerBlock * (std::get<1>(s) + r.numRowPerBlock - 1) / r.numRowPerBlock * r.sizePerBlock;
}


enum class SERHITextureAspect
{
    TA_COLOR = 0x1,
    TA_DEPTH = 0x2,
    TA_STENCIL = 0x4
};
typedef SSEnumFlag SSRHITextureAspectFlags;

class SCRHITextureHelper
{
public:

};
#pragma once
#include "Core/CoreObject/SCObject.h"

typedef enum SERHIBufferUsageFlagBits {
    BU_BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001,
    BU_BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002,
    BU_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000004,
    BU_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT = 0x00000008,
    BU_BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
    BU_BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
    BU_BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
    BU_BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
    BU_BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100,
    BU_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT = 0x00020000,

    BU_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT = 0x00000800,
    BU_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT = 0x00001000,
    BU_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT = 0x00000200,
    BU_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR = 0x00080000,
    BU_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR = 0x00100000,
    BU_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR = 0x00000400,

    BU_BUFFER_USAGE_RAY_TRACING_BIT_NV = BU_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
    BU_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT = BU_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    BU_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR = BU_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    BU_BUFFER_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} SERHIBufferUsageFlagBits;

typedef SSEnumFlag SCRHIBufferUsageFlags;
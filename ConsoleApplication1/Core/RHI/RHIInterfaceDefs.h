#pragma once
#include "Core/CoreObject/SCObject.h"

enum SERHIPipelineStageFlagBits {
    RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT = 0x00000001,
    RHI_PIPELINE_STAGE_DRAW_INDIRECT_BIT = 0x00000002,
    RHI_PIPELINE_STAGE_VERTEX_INPUT_BIT = 0x00000004,
    RHI_PIPELINE_STAGE_VERTEX_SHADER_BIT = 0x00000008,
    RHI_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
    RHI_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
    RHI_PIPELINE_STAGE_GEOMETRY_SHADER_BIT = 0x00000040,
    RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT = 0x00000080,
    RHI_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
    RHI_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT = 0x00000200,
    RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
    RHI_PIPELINE_STAGE_COMPUTE_SHADER_BIT = 0x00000800,
    RHI_PIPELINE_STAGE_TRANSFER_BIT = 0x00001000,
    RHI_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT = 0x00002000,
    RHI_PIPELINE_STAGE_HOST_BIT = 0x00004000,
    RHI_PIPELINE_STAGE_ALL_GRAPHICS_BIT = 0x00008000,
    RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT = 0x00010000,
    RHI_PIPELINE_STAGE_NONE = 0,
    RHI_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT = 0x01000000,
    RHI_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT = 0x00040000,
    RHI_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR = 0x02000000,
    RHI_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR = 0x00200000,
    RHI_PIPELINE_STAGE_TASK_SHADER_BIT_NV = 0x00080000,
    RHI_PIPELINE_STAGE_MESH_SHADER_BIT_NV = 0x00100000,
    RHI_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT = 0x00800000,
    RHI_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x00400000,
    RHI_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV = 0x00020000,
    RHI_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV = RHI_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
    RHI_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV = RHI_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
    RHI_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV = RHI_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
    RHI_PIPELINE_STAGE_NONE_KHR = RHI_PIPELINE_STAGE_NONE,
    RHI_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
};
typedef SSEnumFlag SERHIPipelineStageFlags;
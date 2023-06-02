
/*******************************************************************************

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#include "gvk-structures/defaults.hpp"

#include <array>

namespace gvk {

template <>
const VkApplicationInfo& get_default<VkApplicationInfo>()
{
    static const VkApplicationInfo DefaultApplicationInfo {
        /* .sType              = */ get_stype<VkApplicationInfo>(),
        /* .pNext              = */ nullptr,
        /* .pApplicationName   = */ "Intel(R) GPA Utilities for Vulkan*",
        /* .applicationVersion = */ 0,
        /* .pEngineName        = */ "Intel(R) GPA Utilities for Vulkan*",
        /* .engineVersion      = */ 0,
        /* .apiVersion         = */ VK_API_VERSION_1_3,
    };
    return DefaultApplicationInfo;
}

template <>
const VkInstanceCreateInfo& get_default<VkInstanceCreateInfo>()
{
    static const VkInstanceCreateInfo DefaultInstanceCreateInfo {
        /* sType                   = */ get_stype<VkInstanceCreateInfo>(),
        /* pNext                   = */ nullptr,
        /* flags                   = */ 0,
        /* pApplicationInfo        = */ &get_default<VkApplicationInfo>(),
        /* enabledLayerCount       = */ 0,
        /* ppEnabledLayerNames     = */ nullptr,
        /* enabledExtensionCount   = */ 0,
        /* ppEnabledExtensionNames = */ nullptr,
    };
    return DefaultInstanceCreateInfo;
}

template <>
const VkAttachmentDescription& get_default<VkAttachmentDescription>()
{
    static const VkAttachmentDescription DefaultAttachmentDescription {
        /* .flags          = */ 0,
        /* .format         = */ VK_FORMAT_UNDEFINED,
        /* .samples        = */ VK_SAMPLE_COUNT_1_BIT,
        /* .loadOp         = */ VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        /* .storeOp        = */ VK_ATTACHMENT_STORE_OP_DONT_CARE,
        /* .stencilLoadOp  = */ VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        /* .stencilStoreOp = */ VK_ATTACHMENT_STORE_OP_DONT_CARE,
        /* .initialLayout  = */ VK_IMAGE_LAYOUT_UNDEFINED,
        /* .finalLayout    = */ VK_IMAGE_LAYOUT_UNDEFINED,
    };
    return DefaultAttachmentDescription;
}

template <>
const VkAttachmentDescription2& get_default<VkAttachmentDescription2>()
{
    static const VkAttachmentDescription2 DefaultAttachmentDescription{
        /* .sType          = */ get_stype<VkAttachmentDescription2>(),
        /* .pNext          = */ nullptr,
        /* .flags          = */ 0,
        /* .format         = */ VK_FORMAT_UNDEFINED,
        /* .samples        = */ VK_SAMPLE_COUNT_1_BIT,
        /* .loadOp         = */ VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        /* .storeOp        = */ VK_ATTACHMENT_STORE_OP_DONT_CARE,
        /* .stencilLoadOp  = */ VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        /* .stencilStoreOp = */ VK_ATTACHMENT_STORE_OP_DONT_CARE,
        /* .initialLayout  = */ VK_IMAGE_LAYOUT_UNDEFINED,
        /* .finalLayout    = */ VK_IMAGE_LAYOUT_UNDEFINED,
    };
    return DefaultAttachmentDescription;
}

template <>
const VkBufferImageCopy& get_default<VkBufferImageCopy>()
{
    static const VkBufferImageCopy DefaultBuffyImageCopy {
        /* .bufferOffset      = */ 0,
        /* .bufferRowLength   = */ 0,
        /* .bufferImageHeight = */ 0,
        /* .imageSubresource  = */ get_default<VkImageSubresourceLayers>(),
        /* .imageOffset       = */ { 0, 0, 0 },
        /* .imageExtent       = */ { 0, 0, 0 },
    };
    return DefaultBuffyImageCopy;
}

template <>
const VkDescriptorBufferInfo& get_default<VkDescriptorBufferInfo>()
{
    static const VkDescriptorBufferInfo DefaultDescriptorBufferInfo {
        /* .buffer = */ VK_NULL_HANDLE,
        /* .offset = */ { },
        /* .range  = */ VK_WHOLE_SIZE,
    };
    return DefaultDescriptorBufferInfo;
}

template <>
const VkDescriptorPoolCreateInfo& get_default<VkDescriptorPoolCreateInfo>()
{
    static const VkDescriptorPoolCreateInfo DefaultDescriptorPoolInfo {
        /* sType         = */ get_stype< VkDescriptorPoolCreateInfo>(),
        /* pNext         = */ nullptr,
        /* flags         = */ VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        /* maxSets       = */ 0,
        /* poolSizeCount = */ 0,
        /* pPoolSizes    = */ nullptr
    };
    return DefaultDescriptorPoolInfo;
}

template <>
const VkDebugUtilsMessengerCreateInfoEXT& get_default<VkDebugUtilsMessengerCreateInfoEXT>()
{
    static const VkDebugUtilsMessengerCreateInfoEXT DefaultDebugUtilsMessengerCreateInfo{
        /* .sType           = */ get_stype<VkDebugUtilsMessengerCreateInfoEXT>(),
        /* .pNext           = */ nullptr,
        /* .flags           = */ 0,
        /* .messageSeverity = */
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        /* .messageType     = */
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        /* .pfnUserCallback = */ nullptr,
        /* .pUserData       = */ nullptr,
    };
    return DefaultDebugUtilsMessengerCreateInfo;
}

template <>
const VkDeviceQueueCreateInfo& get_default<VkDeviceQueueCreateInfo>()
{
    static const float DefaultDeviceQueuePriority { 0 };
    static const VkDeviceQueueCreateInfo DefaultDeviceQueueCreateInfo {
        /* .sType            = */ get_stype<VkDeviceQueueCreateInfo>(),
        /* .pNext            = */ nullptr,
        /* .flags            = */ 0,
        /* .uint32_t         = */ 0,
        /* .uint32_t         = */ 1,
        /* .pQueuePriorities = */ &DefaultDeviceQueuePriority,
    };
    return DefaultDeviceQueueCreateInfo;
}

template <>
const VkFramebufferCreateInfo& get_default<VkFramebufferCreateInfo>()
{
    static const VkFramebufferCreateInfo DefaultFramebufferCreateInfo {
        /* .sType           = */ get_stype<VkFramebufferCreateInfo>(),
        /* .pNext           = */ nullptr,
        /* .flags           = */ 0,
        /* .renderPass      = */ VK_NULL_HANDLE,
        /* .attachmentCount = */ 0,
        /* .pAttachments    = */ nullptr,
        /* .width           = */ 1,
        /* .height          = */ 1,
        /* .layers          = */ 1,
    };
    return DefaultFramebufferCreateInfo;
}

template <>
const VkImageCreateInfo& get_default<VkImageCreateInfo>()
{
    static const VkImageCreateInfo DefaultImageCreateInfo {
        /* .sType                 = */ get_stype<VkImageCreateInfo>(),
        /* .pNext                 = */ nullptr,
        /* .flags                 = */ 0,
        /* .imageType             = */ VK_IMAGE_TYPE_1D,
        /* .format                = */ VK_FORMAT_UNDEFINED,
        /* .extent                = */ { 1, 1, 1 },
        /* .mipLevels             = */ 1,
        /* .arrayLayers           = */ 1,
        /* .samples               = */ VK_SAMPLE_COUNT_1_BIT,
        /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
        /* .usage                 = */ 0,
        /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
        /* .queueFamilyIndexCount = */ 0,
        /* .pQueueFamilyIndices   = */ nullptr,
        /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
    };
    return DefaultImageCreateInfo;
}

template <>
const VkBufferMemoryBarrier& get_default<VkBufferMemoryBarrier>()
{
    static const VkBufferMemoryBarrier DefaultBufferMemoryBarrier {
        /* .sType               */ get_stype<VkBufferMemoryBarrier>(),
        /* .pNext               */ nullptr,
        /* .srcAccessMask       */ 0,
        /* .dstAccessMask       */ 0,
        /* .srcQueueFamilyIndex */ VK_QUEUE_FAMILY_IGNORED,
        /* .dstQueueFamilyIndex */ VK_QUEUE_FAMILY_IGNORED,
        /* .buffer              */ VK_NULL_HANDLE,
        /* .offset              */ 0,
        /* .size                */ VK_WHOLE_SIZE,
    };
    return DefaultBufferMemoryBarrier;
}

template <>
const VkBufferMemoryBarrier2& get_default<VkBufferMemoryBarrier2>()
{
    static const VkBufferMemoryBarrier2 DefaultBufferMemoryBarrier {
        /* .sType               */ get_stype<VkBufferMemoryBarrier2>(),
        /* .pNext               */ nullptr,
        /* .srcStageMask        */ 0,
        /* .srcAccessMask       */ 0,
        /* .dstStageMask        */ 0,
        /* .dstAccessMask       */ 0,
        /* .srcQueueFamilyIndex */ VK_QUEUE_FAMILY_IGNORED,
        /* .dstQueueFamilyIndex */ VK_QUEUE_FAMILY_IGNORED,
        /* .buffer              */ VK_NULL_HANDLE,
        /* .offset              */ 0,
        /* .size                */ VK_WHOLE_SIZE,
    };
    return DefaultBufferMemoryBarrier;
}

template <>
const VkImageMemoryBarrier& get_default<VkImageMemoryBarrier>()
{
    static const VkImageMemoryBarrier DefaultImageMemoryBarrier {
        /* .sType               = */ get_stype<VkImageMemoryBarrier>(),
        /* .pNext               = */ nullptr,
        /* .srcAccessMask       = */ 0,
        /* .dstAccessMask       = */ 0,
        /* .oldLayout           = */ VK_IMAGE_LAYOUT_UNDEFINED,
        /* .newLayout           = */ VK_IMAGE_LAYOUT_UNDEFINED,
        /* .srcQueueFamilyIndex = */ VK_QUEUE_FAMILY_IGNORED,
        /* .dstQueueFamilyIndex = */ VK_QUEUE_FAMILY_IGNORED,
        /* .image               = */ VK_NULL_HANDLE,
        /* .subresourceRange    = */ get_default<VkImageSubresourceRange>(),
    };
    return DefaultImageMemoryBarrier;
}

template <>
const VkImageMemoryBarrier2& get_default<VkImageMemoryBarrier2>()
{
    static const VkImageMemoryBarrier2 DefaultImageMemoryBarrier {
        /* .sType               = */ get_stype<VkImageMemoryBarrier2>(),
        /* .pNext               = */ nullptr,
        /* .srcStageMask        = */ 0,
        /* .srcAccessMask       = */ 0,
        /* .dstStageMask        = */ 0,
        /* .dstAccessMask       = */ 0,
        /* .oldLayout           = */ VK_IMAGE_LAYOUT_UNDEFINED,
        /* .newLayout           = */ VK_IMAGE_LAYOUT_UNDEFINED,
        /* .srcQueueFamilyIndex = */ VK_QUEUE_FAMILY_IGNORED,
        /* .dstQueueFamilyIndex = */ VK_QUEUE_FAMILY_IGNORED,
        /* .image               = */ VK_NULL_HANDLE,
        /* .subresourceRange    = */ get_default<VkImageSubresourceRange>(),
    };
    return DefaultImageMemoryBarrier;
}

template <>
const VkImageSubresourceLayers& get_default<VkImageSubresourceLayers>()
{
    static const VkImageSubresourceLayers DefaultImageSubresourceLayers {
        /* .aspectMask     = */ 0,
        /* .mipLevel       = */ 0,
        /* .baseArrayLayer = */ 0,
        /* .layerCount     = */ 1, //!< HUH? : VK_REMAINING_ARRAY_LAYERS,
    };
    return DefaultImageSubresourceLayers;
}

template <>
const VkImageSubresourceRange& get_default<VkImageSubresourceRange>()
{
    static const VkImageSubresourceRange DefaultImageMemorySubresourceRange {
        /* .aspectMask     = */ VK_IMAGE_ASPECT_COLOR_BIT,
        /* .baseMipLevel   = */ 0,
        /* .levelCount     = */ VK_REMAINING_MIP_LEVELS,
        /* .baseArrayLayer = */ 0,
        /* .layerCount     = */ VK_REMAINING_ARRAY_LAYERS,
    };
    return DefaultImageMemorySubresourceRange;
}

template <>
const VkImageViewCreateInfo& get_default<VkImageViewCreateInfo>()
{
    static const VkImageViewCreateInfo DefaultImageViewCreateInfo {
        /* .sType            = */ get_stype<VkImageViewCreateInfo>(),
        /* .pNext            = */ nullptr,
        /* .flags            = */ 0,
        /* .image            = */ VK_NULL_HANDLE,
        /* .viewType         = */ VK_IMAGE_VIEW_TYPE_1D,
        /* .format           = */ VK_FORMAT_UNDEFINED,
        /* .components       = */ get_default<VkComponentMapping>(),
        /* .subresourceRange = */ get_default<VkImageSubresourceRange>(),
    };
    return DefaultImageViewCreateInfo;
}

template <>
const VkSamplerCreateInfo& get_default<VkSamplerCreateInfo>()
{
    static const VkSamplerCreateInfo DefaultSamplerCreateInfo {
        /* .sType                   = */ get_stype<VkSamplerCreateInfo>(),
        /* .pNext                   = */ nullptr,
        /* .flags                   = */ 0,
        /* .magFilter               = */ VK_FILTER_LINEAR,
        /* .minFilter               = */ VK_FILTER_LINEAR,
        /* .mipmapMode              = */ VK_SAMPLER_MIPMAP_MODE_LINEAR,
        /* .addressModeU            = */ VK_SAMPLER_ADDRESS_MODE_REPEAT,
        /* .addressModeV            = */ VK_SAMPLER_ADDRESS_MODE_REPEAT,
        /* .addressModeW            = */ VK_SAMPLER_ADDRESS_MODE_REPEAT,
        /* .mipLodBias              = */ 0.0f,
        /* .anisotropyEnable        = */ VK_TRUE,
        /* .maxAnisotropy           = */ 16.0f,
        /* .compareEnable           = */ VK_FALSE,
        /* .compareOp               = */ VK_COMPARE_OP_ALWAYS,
        /* .minLod                  = */ -1000.0f,
        /* .maxLod                  = */ 1000.0f,
        /* .borderColor             = */ VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        /* .unnormalizedCoordinates = */ VK_FALSE,
    };
    return DefaultSamplerCreateInfo;
}

template <>
const VkSwapchainCreateInfoKHR& get_default<VkSwapchainCreateInfoKHR>()
{
    static const VkSwapchainCreateInfoKHR DefaultSwapchainCreateInfo {
        /* .sType                 = */ get_stype<VkSwapchainCreateInfoKHR>(),
        /* .pNext                 = */ nullptr,
        /* .flags                 = */ 0,
        /* .surface               = */ VK_NULL_HANDLE,
        /* .minImageCount         = */ 0,
        /* .imageFormat           = */ VK_FORMAT_UNDEFINED,
        /* .imageColorSpace       = */ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        /* .imageExtent           = */ { 0, 0 },
        /* .imageArrayLayers      = */ 1,
        /* .imageUsage            = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        /* .imageSharingMode      = */ VK_SHARING_MODE_EXCLUSIVE,
        /* .queueFamilyIndexCount = */ 0,
        /* .pQueueFamilyIndices   = */ nullptr,
        /* .preTransform          = */ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        /* .compositeAlpha        = */ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        /* .presentMode           = */ VK_PRESENT_MODE_FIFO_KHR,
        /* .clipped               = */ VK_TRUE,
        /* .oldSwapchain          = */ VK_NULL_HANDLE,
    };
    return DefaultSwapchainCreateInfo;
}

////////////////////////////////////////////////////////////////////////////////
// VkGraphicsPipelineCreateInfo defaults
template <>
const VkPipelineShaderStageCreateInfo& get_default<VkPipelineShaderStageCreateInfo>()
{
    static const VkPipelineShaderStageCreateInfo DefaultPipelineShaderStageCreateInfo {
        /* .sType               = */ get_stype<VkPipelineShaderStageCreateInfo>(),
        /* .pNext               = */ nullptr,
        /* .flags               = */ 0,
        /* .stage               = */ VK_SHADER_STAGE_ALL,
        /* .module              = */ VK_NULL_HANDLE,
        /* .pName               = */ "main",
        /* .pSpecializationInfo = */ nullptr,
    };
    return DefaultPipelineShaderStageCreateInfo;
}

template <>
const VkPipelineInputAssemblyStateCreateInfo& get_default<VkPipelineInputAssemblyStateCreateInfo>()
{
    static const VkPipelineInputAssemblyStateCreateInfo DefaultPipelineInputAssemblyStateCreateInfo {
        /* .sType                  = */ get_stype<VkPipelineInputAssemblyStateCreateInfo>(),
        /* .pNext                  = */ nullptr,
        /* .flags                  = */ 0,
        /* .topology               = */ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        /* .primitiveRestartEnable = */ VK_FALSE,
    };
    return DefaultPipelineInputAssemblyStateCreateInfo;
}

template <>
const VkViewport& get_default<VkViewport>()
{
    static const VkViewport DefaultViewport {
        /* .x        = */ 0.0f,
        /* .y        = */ 0.0f,
        /* .width    = */ 0.0f,
        /* .height   = */ 0.0f,
        /* .minDepth = */ 0.0f,
        /* .maxDepth = */ 1.0f,
    };
    return DefaultViewport;
}

template <>
const VkPipelineViewportStateCreateInfo& get_default<VkPipelineViewportStateCreateInfo>()
{
    static const VkPipelineViewportStateCreateInfo DefaultPipelineViewportStateCreateInfo {
        /* .sType         = */ get_stype<VkPipelineViewportStateCreateInfo>(),
        /* .pNext         = */ nullptr,
        /* .flags         = */ 0,
        /* .viewportCount = */ 1,
        /* .pViewports    = */ &get_default<VkViewport>(),
        /* .scissorCount  = */ 1,
        /* .pScissors     = */ &get_default<VkRect2D>(),
    };
    return DefaultPipelineViewportStateCreateInfo;
}

template <>
const VkPipelineRasterizationStateCreateInfo& get_default<VkPipelineRasterizationStateCreateInfo>()
{
    static const VkPipelineRasterizationStateCreateInfo DefaultPipelineRasterizationStateCreateInfo {
        /* .sType                   = */ get_stype<VkPipelineRasterizationStateCreateInfo>(),
        /* .pNext                   = */ nullptr,
        /* .flags                   = */ 0,
        /* .depthClampEnable        = */ VK_FALSE,
        /* .rasterizerDiscardEnable = */ VK_FALSE,
        /* .polygonMode             = */ VK_POLYGON_MODE_FILL,
        /* .cullMode                = */ VK_CULL_MODE_BACK_BIT,
        /* .frontFace               = */ VK_FRONT_FACE_CLOCKWISE,
        /* .depthBiasEnable         = */ VK_FALSE,
        /* .depthBiasConstantFactor = */ 0.0f,
        /* .depthBiasClamp          = */ 0.0f,
        /* .depthBiasSlopeFactor    = */ 0.0f,
        /* .lineWidth               = */ 1.0f,
    };
    return DefaultPipelineRasterizationStateCreateInfo;
}

template <>
const VkPipelineMultisampleStateCreateInfo& get_default<VkPipelineMultisampleStateCreateInfo>()
{
    static const VkPipelineMultisampleStateCreateInfo DefaultPipelineRasterizationStateCreateInfo {
        /* .sType                 = */ get_stype<VkPipelineMultisampleStateCreateInfo>(),
        /* .pNext                 = */ nullptr,
        /* .flags                 = */ 0,
        /* .rasterizationSamples  = */ VK_SAMPLE_COUNT_1_BIT,
        /* .sampleShadingEnable   = */ VK_FALSE,
        /* .minSampleShading      = */ 1.0f,
        /* .pSampleMask           = */ nullptr,
        /* .alphaToCoverageEnable = */ VK_FALSE,
        /* .alphaToOneEnable      = */ VK_FALSE,
    };
    return DefaultPipelineRasterizationStateCreateInfo;
}

template <>
const VkPipelineColorBlendAttachmentState& get_default<VkPipelineColorBlendAttachmentState>()
{
    static const VkPipelineColorBlendAttachmentState DefaultPipelineColorBlendStateCreateInfo {
        /* .blendEnable         = */ VK_FALSE,
        /* .srcColorBlendFactor = */ VK_BLEND_FACTOR_SRC_ALPHA,
        /* .dstColorBlendFactor = */ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        /* .colorBlendOp        = */ VK_BLEND_OP_ADD,
        /* .srcAlphaBlendFactor = */ VK_BLEND_FACTOR_ONE,
        /* .dstAlphaBlendFactor = */ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        /* .alphaBlendOp        = */ VK_BLEND_OP_ADD,
        /* .colorWriteMask      = */ VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    return DefaultPipelineColorBlendStateCreateInfo;
}

template <>
const VkPipelineColorBlendStateCreateInfo& get_default<VkPipelineColorBlendStateCreateInfo>()
{
    static const VkPipelineColorBlendStateCreateInfo DefaultPipelineColorBlendStateCreateInfo {
        /* .sType           = */ get_stype<VkPipelineColorBlendStateCreateInfo>(),
        /* .pNext           = */ nullptr,
        /* .flags           = */ 0,
        /* .logicOpEnable   = */ VK_FALSE,
        /* .logicOp         = */ VK_LOGIC_OP_CLEAR,
        /* .attachmentCount = */ 1,
        /* .pAttachments    = */ &get_default<VkPipelineColorBlendAttachmentState>(),
        /* .blendConstants  = */ { 0.0f, 0.0f, 0.0f, 0.0f },
    };
    return DefaultPipelineColorBlendStateCreateInfo;
}

template <>
const VkPipelineDynamicStateCreateInfo& get_default<VkPipelineDynamicStateCreateInfo>()
{
    static const std::array<VkDynamicState, 2> DefaultDynamicStates { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
    static const VkPipelineDynamicStateCreateInfo DefaultPipelineDynamicStateCreateInfo {
        /* .sType             = */ get_stype<VkPipelineDynamicStateCreateInfo>(),
        /* .pNext             = */ nullptr,
        /* .flags             = */ 0,
        /* .dynamicStateCount = */ (uint32_t)DefaultDynamicStates.size(),
        /* .pDynamicStates    = */ DefaultDynamicStates.data(),
    };
    return DefaultPipelineDynamicStateCreateInfo;
}

template <>
const VkGraphicsPipelineCreateInfo& get_default<VkGraphicsPipelineCreateInfo>()
{
    static const VkGraphicsPipelineCreateInfo DefaultGraphicsPipelineCreateInfo {
        /* .sType               = */ get_stype<VkGraphicsPipelineCreateInfo>(),
        /* .pNext               = */ nullptr,
        /* .flags               = */ 0,
        /* .stageCount          = */ 0,
        /* .pStages             = */ nullptr,
        /* .pVertexInputState   = */ &get_default<VkPipelineVertexInputStateCreateInfo>(),
        /* .pInputAssemblyState = */ &get_default<VkPipelineInputAssemblyStateCreateInfo>(),
        /* .pTessellationState  = */ nullptr,
        /* .pViewportState      = */ &get_default<VkPipelineViewportStateCreateInfo>(),
        /* .pRasterizationState = */ &get_default<VkPipelineRasterizationStateCreateInfo>(),
        /* .pMultisampleState   = */ &get_default<VkPipelineMultisampleStateCreateInfo>(),
        /* .pDepthStencilState  = */ &get_default<VkPipelineDepthStencilStateCreateInfo>(),
        /* .pColorBlendState    = */ &get_default<VkPipelineColorBlendStateCreateInfo>(),
        /* .pDynamicState       = */ &get_default<VkPipelineDynamicStateCreateInfo>(),
        /* .layout              = */ VK_NULL_HANDLE,
        /* .renderPass          = */ VK_NULL_HANDLE,
        /* .subpass             = */ 0,
        /* .basePipelineHandle  = */ VK_NULL_HANDLE,
        /* .basePipelineIndex   = */ 0,
    };
    return DefaultGraphicsPipelineCreateInfo;
}

template <>
const VkComputePipelineCreateInfo& get_default<VkComputePipelineCreateInfo>()
{
    static const VkPipelineShaderStageCreateInfo DefaultPipelineShaderStageCreateInfo {
        /* .sType               = */ get_stype<VkPipelineShaderStageCreateInfo>(),
        /* .pNext               = */ nullptr,
        /* .flags               = */ 0,
        /* .stage               = */ VK_SHADER_STAGE_COMPUTE_BIT,
        /* .module              = */ VK_NULL_HANDLE,
        /* .pName               = */ "main",
        /* .pSpecializationInfo = */ nullptr,
    };
    static const VkComputePipelineCreateInfo DefaultComputePipelineCreateInfo {
        /* .sType               = */ get_stype<VkComputePipelineCreateInfo>(),
        /* .pNext               = */ nullptr,
        /* .flags               = */ 0,
        /* .stage               = */ DefaultPipelineShaderStageCreateInfo,
        /* .layout              = */ VK_NULL_HANDLE,
        /* .basePipelineHandle  = */ VK_NULL_HANDLE,
        /* .basePipelineIndex   = */ 0,
    };
    return DefaultComputePipelineCreateInfo;
}

} // namespace gvk

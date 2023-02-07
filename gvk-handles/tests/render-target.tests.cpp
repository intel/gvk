
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

#include "gvk-handles/context.hpp"
#include "gvk-handles/render-target.hpp"
#include "gvk-structures/to-string.hpp"
#include "gvk-handles/utilities.hpp"
#include "gvk-format-info.hpp"
#include "gvk-structures/defaults.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

#include <algorithm>
#include <iostream>
#include <sstream>

struct RenderTargetValidationCreateInfo
{
    VkExtent2D extent{ };
    VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };
};

VkResult create_validation_render_target(const gvk::Context& context, RenderTargetValidationCreateInfo createInfo, gvk::RenderTarget* pRenderTarget)
{
    assert(pRenderTarget);
    assert(createInfo.colorFormat);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // MSAA VkAttachmentDescription2 and VkAttachmentReference2
        auto msaaAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        msaaAttachmentDescription.format = createInfo.colorFormat;
        msaaAttachmentDescription.samples = createInfo.sampleCount;
        msaaAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        msaaAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        msaaAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        auto msaaAttachmentReference = gvk::get_default<VkAttachmentReference2>();
        msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        msaaAttachmentReference.aspectMask = gvk::get_image_aspect_flags(createInfo.colorFormat);

        // Color VkAttachmentDescription2 and VkAttachmentReference2
        auto colorAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        colorAttachmentDescription.format = msaaAttachmentDescription.format;
        colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        auto colorAttachmentReference = msaaAttachmentReference;

        // Depth VkAttachmentDescription2 and VkAttachmentReference2
        auto depthAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        depthAttachmentDescription.format = createInfo.depthFormat;
        depthAttachmentDescription.samples = createInfo.sampleCount;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        auto depthAttachmentReference = gvk::get_default<VkAttachmentReference2>();
        depthAttachmentReference.layout = depthAttachmentDescription.finalLayout;
        depthAttachmentReference.aspectMask = gvk::get_image_aspect_flags(createInfo.depthFormat);

        // Setup attachment descriptions and references
        uint32_t attachmentCount = 1;
        std::array<VkAttachmentDescription2, 3> attachmentDescriptions{
            msaaAttachmentDescription,
            colorAttachmentDescription,
            depthAttachmentDescription,
        };
        auto pAttachmentDescriptions = &attachmentDescriptions[1];
        if (VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount) {
            pAttachmentDescriptions = &attachmentDescriptions[0];
            colorAttachmentReference.attachment = 1;
            ++attachmentCount;
        }
        if (createInfo.depthFormat) {
            depthAttachmentReference.attachment = colorAttachmentReference.attachment + 1;
            ++attachmentCount;
        }

        // Setup VkSubpassDescription2
        auto subpassDescription = gvk::get_default<VkSubpassDescription2>();
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount ? &msaaAttachmentReference : &colorAttachmentReference;
        subpassDescription.pResolveAttachments = VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount ? &colorAttachmentReference : nullptr;
        subpassDescription.pDepthStencilAttachment = depthAttachmentDescription.format ? &depthAttachmentReference : nullptr;

        // Setup VkSubpassDependency2 for a gvk::RenderPass with a single, 1 sample, color attachment
        std::array<VkSubpassDependency2, 2> subpassDependencies{
            VkSubpassDependency2{
                /* .sType           = */ gvk::get_stype<VkSubpassDependency2>(),
                /* .pNext           = */ nullptr,
                /* .srcSubpass      = */ VK_SUBPASS_EXTERNAL,
                /* .dstSubpass      = */ 0,
                /* .srcStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                /* .dstStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                /* .srcAccessMask   = */ 0,
                /* .dstAccessMask   = */ VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                /* .dependencyFlags = */ VK_DEPENDENCY_BY_REGION_BIT,
                /* .viewOffset      = */ 0,
            }
        };

        // Setup VkSubpassDependency2 for a gvk::RenderPass with a multisample color attachment and a resolve attachment
        if (VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount) {
            subpassDependencies = {
                VkSubpassDependency2{
                    /* .sType           = */ gvk::get_stype<VkSubpassDependency2>(),
                    /* .pNext           = */ nullptr,
                    /* .srcSubpass      = */ VK_SUBPASS_EXTERNAL,
                    /* .dstSubpass      = */ 0,
                    /* .srcStageMask    = */ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    /* .dstStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    /* .srcAccessMask   = */ VK_ACCESS_MEMORY_READ_BIT,
                    /* .dstAccessMask   = */ VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    /* .dependencyFlags = */ VK_DEPENDENCY_BY_REGION_BIT,
                    /* .viewOffset      = */ 0,
                },
                VkSubpassDependency2{
                    /* .sType           = */ gvk::get_stype<VkSubpassDependency2>(),
                    /* .pNext           = */ nullptr,
                    /* .srcSubpass      = */ 0,
                    /* .dstSubpass      = */ VK_SUBPASS_EXTERNAL,
                    /* .srcStageMask    = */ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    /* .dstStageMask    = */ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    /* .srcAccessMask   = */ VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    /* .dstAccessMask   = */ VK_ACCESS_MEMORY_READ_BIT,
                    /* .dependencyFlags = */ VK_DEPENDENCY_BY_REGION_BIT,
                    /* .viewOffset      = */ 0,
                }
            };
        }

        // Create gvk::RenderPass
        auto renderPassCreateInfo = gvk::get_default<VkRenderPassCreateInfo2>();
        renderPassCreateInfo.attachmentCount = attachmentCount;
        renderPassCreateInfo.pAttachments = pAttachmentDescriptions;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = VK_SAMPLE_COUNT_1_BIT < createInfo.sampleCount ? 2 : 1;
        renderPassCreateInfo.pDependencies = subpassDependencies.data();
        gvk::RenderPass renderPass;
        gvk_result(gvk::RenderPass::create(context.get_devices()[0], &renderPassCreateInfo, nullptr, &renderPass));

        // Prepare VkFramebufferCreateInfo
        auto framebufferCreateInfo = gvk::get_default<VkFramebufferCreateInfo>();
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.width = createInfo.extent.width;
        framebufferCreateInfo.height = createInfo.extent.height;

        // Create gvk::RenderTarget
        auto renderTargetCreateInfo = gvk::get_default<gvk::RenderTarget::CreateInfo>();
        renderTargetCreateInfo.pFramebufferCreateInfo = &framebufferCreateInfo;
        gvk_result(gvk::RenderTarget::create(context.get_devices()[0], &renderTargetCreateInfo, nullptr, pRenderTarget));
    } gvk_result_scope_end
    return gvkResult;
}

void validate_render_target(
    const gvk::Context& context,
    const RenderTargetValidationCreateInfo& renderTargetCreateInfo,
    const std::vector<VkImageCreateInfo>& expectedImageCreateInfos
)
{
    gvk::RenderTarget renderTarget;
    create_validation_render_target(context, renderTargetCreateInfo, &renderTarget);
    const auto& imageViews = renderTarget.get_framebuffer().get<gvk::ImageViews>();
    ASSERT_EQ(imageViews.size(), expectedImageCreateInfos.size());
    for (size_t i = 0; i < imageViews.size(); ++i) {
        if (imageViews[i].get<gvk::Image>().get<VkImageCreateInfo>() != expectedImageCreateInfos[i]) {
            FAIL()
                << "==============================================================================="                              << std::endl
                << "VkSampleCountFlagBits : " << gvk::to_string(renderTargetCreateInfo.sampleCount, gvk::Printer::EnumIdentifier) << std::endl
                << "Color VkFormat        : " << gvk::to_string(renderTargetCreateInfo.colorFormat, gvk::Printer::EnumIdentifier) << std::endl
                << "Depth VkFormat        : " << gvk::to_string(renderTargetCreateInfo.depthFormat, gvk::Printer::EnumIdentifier) << std::endl
                << "-------------------------------------------------------------------------------"                              << std::endl
                << gvk::to_string(imageViews[i].get<gvk::Image>().get<VkImageCreateInfo>())                                       << std::endl
                << "-------------------------------------------------------------------------------"                              << std::endl
                << gvk::to_string(expectedImageCreateInfos[i])                                                                    << std::endl
                << "==============================================================================="                              << std::endl
                << std::endl;
        }
    }
}

TEST(RenderTarget, ResourceCreation)
{
    gvk::Context context;
    ASSERT_EQ(gvk::Context::create(&gvk::get_default<gvk::Context::CreateInfo>(), nullptr, &context), VK_SUCCESS);

    // Get color VkFormat
    auto colorFormat = VK_FORMAT_UNDEFINED;
    auto physicalDevice = context.get_devices()[0].get<gvk::PhysicalDevice>();
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT,
        [&](VkFormat format)
        {
            GvkFormatInfo formatInfo { };
            gvk::get_format_info(format, &formatInfo);
            if (gvk::get_bits_per_texel(format) == 32 &&
                formatInfo.componentCount == 4 &&
                formatInfo.compressionType == GVK_FORMAT_COMPRESSION_TYPE_NONE &&
                formatInfo.numericFormat == GVK_NUMERIC_FORMAT_UNORM &&
                !formatInfo.packed &&
                !formatInfo.chroma
            ) {
                colorFormat = format;
            }
            return colorFormat == VK_FORMAT_UNDEFINED;
        }
    );
    EXPECT_EQ(colorFormat, VK_FORMAT_R8G8B8A8_UNORM);

    // Get depth VkFormat
    auto depthFormat = VK_FORMAT_UNDEFINED;
    auto requestedDepthFormat = VK_FORMAT_D32_SFLOAT;
    GvkFormatInfo depthFormatInfo { };
    gvk::get_format_info(requestedDepthFormat, &depthFormatInfo);
    assert(depthFormatInfo.componentCount);
    assert(depthFormatInfo.pComponents);
    auto requestedDepthBits = depthFormatInfo.pComponents[0].bits;
    gvk::enumerate_formats(
        physicalDevice.get<gvk::DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
        physicalDevice,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
        [&](VkFormat format)
        {
            // TODO : Abstract this logic into a utility function
            if (format == requestedDepthFormat) {
                depthFormat = requestedDepthFormat;
            } else {
                uint32_t actualDepthBits = 0;
                if (depthFormat) {
                    gvk::get_format_info(depthFormat, &depthFormatInfo);
                    assert(depthFormatInfo.componentCount);
                    assert(depthFormatInfo.pComponents);
                    actualDepthBits = depthFormatInfo.pComponents[0].bits;
                }
                GvkFormatInfo formatInfo { };
                gvk::get_format_info(format, &formatInfo);
                assert(formatInfo.componentCount);
                assert(formatInfo.pComponents);
                auto formatDepthBits = formatInfo.pComponents[0].bits;
                if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                    depthFormat = format;
                }
            }
            return depthFormat != requestedDepthFormat;
        }
    );
    EXPECT_NE(depthFormat, VK_FORMAT_UNDEFINED);

    // Get VkSampleCountFlagBits
    auto sampleCount = gvk::get_max_framebuffer_sample_count(physicalDevice, VK_TRUE, VK_TRUE, VK_FALSE);

    // Prepare RenderTargetValidationCreateInfo
    auto renderTargetValidationCreateInfo = gvk::get_default<RenderTargetValidationCreateInfo>();
    renderTargetValidationCreateInfo.extent = { 1024, 1024 };

    // MSAA  : No
    // Color : Yes
    // Depth : No
    renderTargetValidationCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
    renderTargetValidationCreateInfo.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    renderTargetValidationCreateInfo.depthFormat = VK_FORMAT_UNDEFINED;
    validate_render_target(
        context,
        renderTargetValidationCreateInfo,
        {
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ VK_FORMAT_R8G8B8A8_UNORM,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ VK_SAMPLE_COUNT_1_BIT,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            }
        }
    );

    // MSAA  : Yes
    // Color : Yes
    // Depth : No
    renderTargetValidationCreateInfo.sampleCount = sampleCount;
    renderTargetValidationCreateInfo.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    renderTargetValidationCreateInfo.depthFormat = VK_FORMAT_UNDEFINED;
    validate_render_target(
        context,
        renderTargetValidationCreateInfo,
        {
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ VK_FORMAT_R8G8B8A8_UNORM,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ sampleCount,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ VK_FORMAT_R8G8B8A8_UNORM,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ VK_SAMPLE_COUNT_1_BIT,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            }
        }
    );

    // MSAA  : No
    // Color : Yes
    // Depth : Yes
    renderTargetValidationCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
    renderTargetValidationCreateInfo.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    renderTargetValidationCreateInfo.depthFormat = depthFormat;
    validate_render_target(
        context,
        renderTargetValidationCreateInfo,
        {
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ VK_FORMAT_R8G8B8A8_UNORM,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ VK_SAMPLE_COUNT_1_BIT,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ depthFormat,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ VK_SAMPLE_COUNT_1_BIT,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            }
        }
    );

    // MSAA  : Yes
    // Color : Yes
    // Depth : Yes
    renderTargetValidationCreateInfo.sampleCount = sampleCount;
    renderTargetValidationCreateInfo.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    renderTargetValidationCreateInfo.depthFormat = depthFormat;
    validate_render_target(
        context,
        renderTargetValidationCreateInfo,
        {
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ VK_FORMAT_R8G8B8A8_UNORM,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ sampleCount,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ VK_FORMAT_R8G8B8A8_UNORM,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ VK_SAMPLE_COUNT_1_BIT,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VkImageCreateInfo{
                /* .sType                 = */ gvk::get_stype<VkImageCreateInfo>(),
                /* .pNext                 = */ nullptr,
                /* .flags                 = */ 0,
                /* .imageType             = */ VK_IMAGE_TYPE_2D,
                /* .format                = */ depthFormat,
                /* .extent                = */ { 1024, 1024, 1 },
                /* .mipLevels             = */ 1,
                /* .arrayLayers           = */ 1,
                /* .samples               = */ sampleCount,
                /* .tiling                = */ VK_IMAGE_TILING_OPTIMAL,
                /* .usage                 = */ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                /* .sharingMode           = */ VK_SHARING_MODE_EXCLUSIVE,
                /* .queueFamilyIndexCount = */ 0,
                /* .pQueueFamilyIndices   = */ nullptr,
                /* .initialLayout         = */ VK_IMAGE_LAYOUT_UNDEFINED,
            }
        }
    );
}

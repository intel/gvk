
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

#include "gvk/render-target.hpp"
#include "gvk/format.hpp"

#include <array>
#include <cassert>

namespace gvk {

VkResult RenderTarget::create(const Device& device, const RenderTarget::CreateInfo* pRenderTargetCreateInfo, const VkAllocationCallbacks* pAllocator, RenderTarget* pRenderTarget)
{
    assert(device);
    assert(pRenderTargetCreateInfo);
    assert(pRenderTargetCreateInfo->pFramebufferCreateInfo);
    assert(pRenderTarget);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pRenderTarget->reset();
        auto framebufferCreateInfo = *pRenderTargetCreateInfo->pFramebufferCreateInfo;
        auto renderPass = RenderPass::get(framebufferCreateInfo.renderPass);
        assert(renderPass);
        auto renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo2>();
        if (renderPassCreateInfo.attachmentCount && renderPassCreateInfo.pAttachments) {
            std::vector<ImageView> imageViews;
            std::vector<VkImageView> vkImageViews;
            imageViews.reserve(renderPassCreateInfo.attachmentCount);
            for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
                if (i < framebufferCreateInfo.attachmentCount && framebufferCreateInfo.pAttachments && framebufferCreateInfo.pAttachments[i]) {
                    vkImageViews.push_back(framebufferCreateInfo.pAttachments[i]);
                } else {
                    // If framebufferCreateInfo doesn't provide a VkImageView for the attachment
                    //  we're processing, we need to use the renderPassCreateInfo to determine
                    //  how the attachment is used and create a gvk::Image and gvk::ImageView that
                    //  are compatible with the the attachment description...
                    auto imageCreateInfo = get_default<VkImageCreateInfo>();
                    const auto& attachmentDescription = renderPassCreateInfo.pAttachments[i];
                    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                    imageCreateInfo.format = attachmentDescription.format;
                    imageCreateInfo.extent = { framebufferCreateInfo.width, framebufferCreateInfo.height, 1 };
                    imageCreateInfo.samples = attachmentDescription.samples;
                    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO : User configuartion for VkImageTiling
                    auto imageAspectFlags = get_image_aspect_flags(attachmentDescription.format);
                    if (imageAspectFlags & VK_IMAGE_ASPECT_COLOR_BIT) { // TODO : User configuration for VkImageUsageFlags
                        imageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    } else if (imageAspectFlags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                        imageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    }
                    auto allocationCreateInfo = get_default<VmaAllocationCreateInfo>();
                    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO; // TODO : User configuration for VmaMemoryUsage
                    Image image;
                    gvk_result(Image::create(device, &imageCreateInfo, &allocationCreateInfo, &image));

                    // Create gvk::ImageView
                    auto imageViewCreateInfo = get_default<VkImageViewCreateInfo>();
                    imageViewCreateInfo.image = image;
                    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    imageViewCreateInfo.format = attachmentDescription.format;
                    imageViewCreateInfo.subresourceRange.aspectMask = get_image_aspect_flags(attachmentDescription.format);
                    ImageView imageView;
                    gvk_result(ImageView::create(device, &imageViewCreateInfo, pAllocator, &imageView));
                    imageViews.push_back(imageView);
                    vkImageViews.push_back(imageView);
                }
            }

            // Create gvk::Framebuffer
            framebufferCreateInfo.attachmentCount = (uint32_t)vkImageViews.size();
            framebufferCreateInfo.pAttachments = vkImageViews.data();
            gvk_result(Framebuffer::create(device, &framebufferCreateInfo, pAllocator, &pRenderTarget->mFramebuffer));
            
            // Prepare VkClearValue array
            pRenderTarget->mClearValues.reserve(pRenderTarget->mFramebuffer.get<ImageViews>().size());
            for (const auto& imageView : pRenderTarget->mFramebuffer.get<ImageViews>()) {
                auto imageAspectFlags = get_image_aspect_flags(imageView.get<VkImageViewCreateInfo>().format);
                if (imageAspectFlags & VK_IMAGE_ASPECT_COLOR_BIT) {
                    pRenderTarget->mClearValues.push_back({ .color = { 0, 0, 0, 1 } });
                } else {
                    pRenderTarget->mClearValues.push_back({ .depthStencil = { 1, 0 } });
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void RenderTarget::reset()
{
    mFramebuffer.reset();
    mClearValues.clear();
}

Image RenderTarget::get_image(uint32_t attachmentIndex) const
{
    assert(mFramebuffer);
    const auto& imageViews = mFramebuffer.get<ImageViews>();
    const auto& imageView = attachmentIndex < imageViews.size() ? imageViews[attachmentIndex] : VK_NULL_HANDLE;
    return imageView ? imageView.get<Image>() : VK_NULL_HANDLE;
}

const Framebuffer& RenderTarget::get_framebuffer() const
{
    return mFramebuffer;
}

RenderPass RenderTarget::get_render_pass() const
{
    assert(mFramebuffer);
    return mFramebuffer.get<RenderPass>();
}

VkRenderPassBeginInfo RenderTarget::get_render_pass_begin_info() const
{
    auto framebufferCreateInfo = mFramebuffer.get<VkFramebufferCreateInfo>();
    return VkRenderPassBeginInfo {
        .sType           = get_stype<VkRenderPassBeginInfo>(),
        .renderPass      = mFramebuffer.get<RenderPass>(),
        .framebuffer     = mFramebuffer,
        .renderArea      = VkRect2D { .extent = { framebufferCreateInfo.width, framebufferCreateInfo.height } },
        .clearValueCount = (uint32_t)mClearValues.size(),
        .pClearValues    = mClearValues.data(),
    };
}

VkImageMemoryBarrier RenderTarget::get_image_memory_barrier(uint32_t attachmentIndex) const
{
    auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
    auto image = get_image(attachmentIndex);
    if (image) {
        imageMemoryBarrier.image = image;
        auto renderPass = mFramebuffer.get<RenderPass>();
        auto renderPassCreateInfo = renderPass ? renderPass.get<VkRenderPassCreateInfo2>() : get_default<VkRenderPassCreateInfo2>();
        if (attachmentIndex < renderPassCreateInfo.attachmentCount && renderPassCreateInfo.pAttachments) {
            const auto& attachmentDescription = renderPassCreateInfo.pAttachments[attachmentIndex];
            imageMemoryBarrier.oldLayout = attachmentDescription.finalLayout;
            imageMemoryBarrier.newLayout = attachmentDescription.initialLayout ? attachmentDescription.initialLayout : attachmentDescription.finalLayout;
            imageMemoryBarrier.subresourceRange.aspectMask = get_image_aspect_flags(attachmentDescription.format);
        }
    }
    return imageMemoryBarrier;
}

VkImageMemoryBarrier2 RenderTarget::get_image_memory_barrier_2(uint32_t attachmentIndex) const
{
    auto imageMemoryBarrier = get_default<VkImageMemoryBarrier2>();
    auto image = get_image(attachmentIndex);
    if (image) {
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        auto renderPass = mFramebuffer.get<RenderPass>();
        auto renderPassCreateInfo = renderPass ? renderPass.get<VkRenderPassCreateInfo2>() : get_default<VkRenderPassCreateInfo2>();
        if (attachmentIndex < renderPassCreateInfo.attachmentCount && renderPassCreateInfo.pAttachments) {
            const auto& attachmentDescription = renderPassCreateInfo.pAttachments[attachmentIndex];
            imageMemoryBarrier.oldLayout = attachmentDescription.finalLayout;
            imageMemoryBarrier.newLayout = attachmentDescription.initialLayout ? attachmentDescription.initialLayout : attachmentDescription.finalLayout;
            imageMemoryBarrier.subresourceRange.aspectMask = get_image_aspect_flags(attachmentDescription.format);
        }
    }
    return imageMemoryBarrier;
}

VkSampleCountFlagBits get_max_framebuffer_sample_count(VkPhysicalDevice vkPhysicalDevice, VkBool32 color, VkBool32 depth, VkBool32 stencil)
{
    VkPhysicalDeviceProperties physicalDeviceProperties{ };
    auto dispatchTable = DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.gvkGetPhysicalDeviceProperties);
    dispatchTable.gvkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
    VkSampleCountFlags sampleCounts = (color || depth || stencil) ? (uint32_t)-1 : 0;
    if (color) {
        sampleCounts &= physicalDeviceProperties.limits.framebufferColorSampleCounts;
    }
    if (depth) {
        sampleCounts &= physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    }
    if (stencil) {
        sampleCounts &= physicalDeviceProperties.limits.framebufferStencilSampleCounts;
    }
    if (sampleCounts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

} // namespace gvk

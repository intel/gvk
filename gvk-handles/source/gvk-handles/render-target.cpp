
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

#include "gvk-handles/render-target.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-format-info.hpp"

#include <array>

namespace gvk {

template <typename RenderPassCreateInfoType>
static VkResult create_render_target_framebuffer(const Device& device, const RenderPassCreateInfoType& renderPassCreateInfo, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Framebuffer* pFramebuffer, std::vector<VkImageMemoryBarrier2>* pImageMemoryBarrier2s)
{
    assert(device);
    assert(pCreateInfo);
    assert(pFramebuffer);
    assert(pImageMemoryBarrier2s);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        std::vector<ImageView> imageViews;
        std::vector<VkImageView> vkImageViews;
        std::vector<VkImageMemoryBarrier2> imageMemoryBarrier2s;
        imageViews.reserve(renderPassCreateInfo.attachmentCount);
        vkImageViews.reserve(renderPassCreateInfo.attachmentCount);
        for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
            if (i < pCreateInfo->attachmentCount && pCreateInfo->pAttachments && pCreateInfo->pAttachments[i]) {
                vkImageViews.push_back(pCreateInfo->pAttachments[i]);
            } else {
                // Create gvk::Image
                auto imageCreateInfo = get_default<VkImageCreateInfo>();
                const auto& attachmentDescription = renderPassCreateInfo.pAttachments[i];
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = attachmentDescription.format;
                imageCreateInfo.extent = { pCreateInfo->width, pCreateInfo->height, 1 };
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
        auto framebufferCreateInfo = *pCreateInfo;
        framebufferCreateInfo.attachmentCount = (uint32_t)vkImageViews.size();
        framebufferCreateInfo.pAttachments = vkImageViews.data();
        gvk_result(Framebuffer::create(device, &framebufferCreateInfo, pAllocator, pFramebuffer));

        // Setup VkImageMemoryBarriers
        pImageMemoryBarrier2s->clear();
        pImageMemoryBarrier2s->reserve(framebufferCreateInfo.attachmentCount);
        for (uint32_t i = 0; i < framebufferCreateInfo.attachmentCount; ++i) {
            ImageView imageView({ device, framebufferCreateInfo.pAttachments[i] });
            auto imageMemoryBarrier2 = get_default<VkImageMemoryBarrier2>();
            imageMemoryBarrier2.image = imageView.get<VkImageViewCreateInfo>().image;
            imageMemoryBarrier2.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            imageMemoryBarrier2.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            if (i < renderPassCreateInfo.attachmentCount) {
                const auto& attachmentDescription = renderPassCreateInfo.pAttachments[i];
                imageMemoryBarrier2.oldLayout = attachmentDescription.finalLayout;
                imageMemoryBarrier2.newLayout = attachmentDescription.initialLayout ? attachmentDescription.initialLayout : attachmentDescription.finalLayout;
                imageMemoryBarrier2.subresourceRange.aspectMask = get_image_aspect_flags(attachmentDescription.format);
            }
            pImageMemoryBarrier2s->push_back(imageMemoryBarrier2);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult RenderTarget::create(const Device& device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, RenderTarget* pRenderTarget)
{
    assert(device);
    assert(pCreateInfo);
    assert(pRenderTarget);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Create gvk::Framebuffer
        Framebuffer framebuffer = VK_NULL_HANDLE;
        std::vector<VkImageMemoryBarrier2> imageMemoryBarrier2s;
        auto renderPass = RenderPass::get({ device, pCreateInfo->renderPass });
        if (renderPass.get<VkRenderPassCreateInfo2>().sType == get_stype<VkRenderPassCreateInfo2>()) {
            gvk_result(create_render_target_framebuffer(device, renderPass.get<VkRenderPassCreateInfo2>(), pCreateInfo, pAllocator, &framebuffer, &imageMemoryBarrier2s));
        } else if (renderPass.get<VkRenderPassCreateInfo>().sType == get_stype<VkRenderPassCreateInfo>()) {
            gvk_result(create_render_target_framebuffer(device, renderPass.get<VkRenderPassCreateInfo>(), pCreateInfo, pAllocator, &framebuffer, &imageMemoryBarrier2s));
        }

        // Create new Reference<>()
        pRenderTarget->mReference.reset(newref);
        auto& controlBlock = *pRenderTarget->mReference;

        // Set the gvk::Framebuffer
        controlBlock.mFramebuffer = framebuffer;

        // Prepare VkClearValue array
        std::vector<VkClearValue> clearValues;
        clearValues.reserve(controlBlock.mFramebuffer.get<ImageViews>().size());
        for (const auto& imageView : controlBlock.mFramebuffer.get<ImageViews>()) {
            auto clearValue = get_default<VkClearValue>();
            auto imageAspectFlags = get_image_aspect_flags(imageView.get<VkImageViewCreateInfo>().format);
            if (imageAspectFlags & VK_IMAGE_ASPECT_COLOR_BIT) {
                clearValue.color.float32[3] = 1;
            } else {
                clearValue.depthStencil.depth = 1;
            }
            clearValues.push_back(clearValue);
        }

        // Setup VkRenderPassBeginInfo
        auto renderPassBeginInfo = get_default<VkRenderPassBeginInfo>();
        renderPassBeginInfo.renderPass = controlBlock.mFramebuffer.get<RenderPass>();
        renderPassBeginInfo.framebuffer = controlBlock.mFramebuffer;
        renderPassBeginInfo.renderArea = { { }, { pCreateInfo->width, pCreateInfo->height } };
        renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassBeginInfo.pClearValues = !clearValues.empty() ? clearValues.data() : nullptr;
        controlBlock.mRenderPassBeginInfo = renderPassBeginInfo;

        // Setup VkImageMemoryBarriers
        controlBlock.mImageMemoryBarriers.reserve(imageMemoryBarrier2s.size());
        controlBlock.mImageMemoryBarrier2s.reserve(imageMemoryBarrier2s.size());
        for (const auto& imageMemoryBarrier2 : imageMemoryBarrier2s) {
            controlBlock.mImageMemoryBarrier2s.push_back(imageMemoryBarrier2);
            controlBlock.mImageMemoryBarriers.push_back(convert<VkImageMemoryBarrier2, VkImageMemoryBarrier>(imageMemoryBarrier2));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

RenderTarget::ControlBlock::~ControlBlock()
{
}

} // namespace gvk

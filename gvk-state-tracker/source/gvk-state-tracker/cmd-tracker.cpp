
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

#include "gvk-state-tracker/generated/state-tracked-handles.hpp"
#include "gvk-state-tracker/cmd-tracker.hpp"
#include "gvk-structures/get-stype.hpp"

namespace gvk {
namespace state_tracker {

const std::vector<const GvkCommandBaseStructure*>& CmdTracker::get_cmds() const
{
    return mCmds;
}

const std::unordered_map<VkImage, ImageLayoutTracker>& CmdTracker::get_image_layout_trackers() const
{
    return mImageLayoutTrackers;
}

////////////////////////////////////////////////////////////////////////////////
// RenderPass Cmds
void CmdTracker::record_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    assert(!mBeginRenderPass->sType);
    assert(!mBeginRenderPass2->sType);
    BasicCmdTracker::record_vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    assert(!mCmds.empty());
    assert(mCmds.back()->sType == get_stype<GvkCommandStructureCmdBeginRenderPass>());
    mBeginRenderPass = *(const GvkCommandStructureCmdBeginRenderPass*)mCmds.back();
}

void CmdTracker::record_vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfo* pSubpassBeginInfo)
{
    assert(!mBeginRenderPass->sType);
    assert(!mBeginRenderPass2->sType);
    BasicCmdTracker::record_vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    assert(!mCmds.empty());
    assert(mCmds.back()->sType == get_stype<GvkCommandStructureCmdBeginRenderPass2>());
    mBeginRenderPass2 = *(const GvkCommandStructureCmdBeginRenderPass2*)mCmds.back();
}

void CmdTracker::record_vkCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfo* pSubpassBeginInfo)
{
    record_vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void CmdTracker::record_vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo)
{
    BasicCmdTracker::record_vkCmdBeginRendering(commandBuffer, pRenderingInfo);
}

void CmdTracker::record_vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo)
{
    BasicCmdTracker::record_vkCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
}

template <typename RenderPassCreateInfoType, typename RecordImageLayoutTransitionFunctionType>
inline void record_render_pass_layout_transitions(VkDevice device, const RenderPassCreateInfoType& renderPassCreateInfo, const std::vector<ImageView>& framebufferAttachments, RecordImageLayoutTransitionFunctionType recordImageLayoutTransition)
{
    assert(renderPassCreateInfo.pAttachments);
    for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount && i < framebufferAttachments.size(); ++i) {
        auto gvkImageView = framebufferAttachments[i];
        assert(gvkImageView);
        auto imageViewCreateInfo = gvkImageView.get<VkImageViewCreateInfo>();
        recordImageLayoutTransition(device, imageViewCreateInfo.image, imageViewCreateInfo.subresourceRange, renderPassCreateInfo.pAttachments[i].finalLayout);
    }
}

void CmdTracker::record_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    BasicCmdTracker::record_vkCmdEndRenderPass(commandBuffer);
    CommandBuffer gvkCommandBuffer(commandBuffer);
    assert(gvkCommandBuffer);
    auto gvkDevice = gvkCommandBuffer.get<Device>();
    assert(gvkDevice);

    RenderPass gvkRenderPass;
    Framebuffer gvkFramebuffer;
    if (mBeginRenderPass->sType == get_stype<GvkCommandStructureCmdBeginRenderPass>()) {
        assert(!mBeginRenderPass2->sType);
        gvkRenderPass = RenderPass({ gvkDevice, mBeginRenderPass->pRenderPassBegin->renderPass });
        gvkFramebuffer = Framebuffer({ gvkDevice, mBeginRenderPass->pRenderPassBegin->framebuffer });
    } else {
        assert(mBeginRenderPass2->sType == get_stype< GvkCommandStructureCmdBeginRenderPass2>());
        gvkRenderPass = RenderPass({ gvkDevice, mBeginRenderPass2->pRenderPassBegin->renderPass });
        gvkFramebuffer = Framebuffer({ gvkDevice, mBeginRenderPass2->pRenderPassBegin->framebuffer });
    }
    assert(gvkRenderPass);
    assert(gvkFramebuffer);

    auto renderPassCreateInfo = gvkRenderPass.get<VkRenderPassCreateInfo>();
    if (renderPassCreateInfo.sType == get_stype<VkRenderPassCreateInfo>()) {
        record_render_pass_layout_transitions(gvkDevice, renderPassCreateInfo, gvkFramebuffer.get<const std::vector<ImageView>&>(),
            [&](VkDevice device, VkImage image, const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout imageLayout)
            {
                record_image_layout_transition(device, image, imageSubresourceRange, imageLayout);
            }
        );
    } else {
        auto renderPassCreateInfo2 = gvkRenderPass.get<VkRenderPassCreateInfo2>();
        assert(renderPassCreateInfo2.sType == get_stype<VkRenderPassCreateInfo2>());
        record_render_pass_layout_transitions(gvkDevice, renderPassCreateInfo2, gvkFramebuffer.get<const std::vector<ImageView>&>(),
            [&](VkDevice device, VkImage image, const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout imageLayout)
            {
                record_image_layout_transition(device, image, imageSubresourceRange, imageLayout);
            }
        );
    }

    mBeginRenderPass.reset();
    mBeginRenderPass2.reset();
}

void CmdTracker::record_vkCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo)
{
    BasicCmdTracker::record_vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void CmdTracker::record_vkCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo)
{
    record_vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void CmdTracker::record_vkCmdEndRendering(VkCommandBuffer commandBuffer)
{
    BasicCmdTracker::record_vkCmdEndRendering(commandBuffer);
}

void CmdTracker::record_vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer)
{
    BasicCmdTracker::record_vkCmdEndRenderingKHR(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////
// Descriptor Cmds
void CmdTracker::record_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
    BasicCmdTracker::record_vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void CmdTracker::record_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites)
{
    BasicCmdTracker::record_vkCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

void CmdTracker::record_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData)
{
    BasicCmdTracker::record_vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

////////////////////////////////////////////////////////////////////////////////
// Transfer Cmds
void CmdTracker::record_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    BasicCmdTracker::record_vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CmdTracker::record_vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo)
{
    BasicCmdTracker::record_vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
}

void CmdTracker::record_vkCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo)
{
    record_vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
}

void CmdTracker::record_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    BasicCmdTracker::record_vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void CmdTracker::record_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    BasicCmdTracker::record_vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void CmdTracker::record_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    BasicCmdTracker::record_vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void CmdTracker::record_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    BasicCmdTracker::record_vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void CmdTracker::record_vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo)
{
    BasicCmdTracker::record_vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}

void CmdTracker::record_vkCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo)
{
    record_vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}

void CmdTracker::record_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    BasicCmdTracker::record_vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void CmdTracker::record_vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo)
{
    BasicCmdTracker::record_vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}

void CmdTracker::record_vkCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo)
{
    record_vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}

void CmdTracker::record_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    BasicCmdTracker::record_vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CmdTracker::record_vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo)
{
    BasicCmdTracker::record_vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
}

void CmdTracker::record_vkCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo)
{
    record_vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
}

void CmdTracker::record_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    BasicCmdTracker::record_vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void CmdTracker::record_vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo)
{
    BasicCmdTracker::record_vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}

void CmdTracker::record_vkCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo)
{
    record_vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}

void CmdTracker::record_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
    BasicCmdTracker::record_vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void CmdTracker::record_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    BasicCmdTracker::record_vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CmdTracker::record_vkCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo)
{
    BasicCmdTracker::record_vkCmdResolveImage2(commandBuffer,pResolveImageInfo);
}

void CmdTracker::record_vkCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo)
{
    record_vkCmdResolveImage2(commandBuffer, pResolveImageInfo);
}

////////////////////////////////////////////////////////////////////////////////
// Barrier Cmds
void CmdTracker::record_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    BasicCmdTracker::record_vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    if (imageMemoryBarrierCount && pImageMemoryBarriers) {
        CommandBuffer gvkCommandBuffer;
        gvkCommandBuffer = CommandBuffer(commandBuffer);
        assert(gvkCommandBuffer);
        auto gvkDevice = gvkCommandBuffer.get<Device>();
        assert(gvkDevice);
        for (uint32_t imageMemoryBarrier_i = 0; imageMemoryBarrier_i < imageMemoryBarrierCount; ++imageMemoryBarrier_i) {
            const auto& imageMemoryBarrier = pImageMemoryBarriers[imageMemoryBarrier_i];
            record_image_layout_transition(gvkDevice, imageMemoryBarrier.image, imageMemoryBarrier.subresourceRange, imageMemoryBarrier.newLayout);
        }
    }
}

void CmdTracker::record_vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo)
{
    assert(pDependencyInfo);
    BasicCmdTracker::record_vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
    auto imageMemoryBarrierCount = pDependencyInfo->imageMemoryBarrierCount;
    auto pImageMemoryBarriers = pDependencyInfo->pImageMemoryBarriers;
    if (imageMemoryBarrierCount && pImageMemoryBarriers) {
        CommandBuffer gvkCommandBuffer;
        gvkCommandBuffer = CommandBuffer(commandBuffer);
        assert(gvkCommandBuffer);
        auto gvkDevice = gvkCommandBuffer.get<Device>();
        assert(gvkDevice);
        for (uint32_t imageMemoryBarrier_i = 0; imageMemoryBarrier_i < imageMemoryBarrierCount; ++imageMemoryBarrier_i) {
            const auto& imageMemoryBarrier = pImageMemoryBarriers[imageMemoryBarrier_i];
            record_image_layout_transition(gvkDevice, imageMemoryBarrier.image, imageMemoryBarrier.subresourceRange, imageMemoryBarrier.newLayout);
        }
    }
}

void CmdTracker::record_vkCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo)
{
    record_vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}

void CmdTracker::record_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    BasicCmdTracker::record_vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    if (imageMemoryBarrierCount && pImageMemoryBarriers) {
        CommandBuffer gvkCommandBuffer;
        gvkCommandBuffer = CommandBuffer(commandBuffer);
        assert(gvkCommandBuffer);
        auto gvkDevice = gvkCommandBuffer.get<Device>();
        assert(gvkDevice);
        for (uint32_t imageMemoryBarrier_i = 0; imageMemoryBarrier_i < imageMemoryBarrierCount; ++imageMemoryBarrier_i) {
            const auto& imageMemoryBarrier = pImageMemoryBarriers[imageMemoryBarrier_i];
            record_image_layout_transition(gvkDevice, imageMemoryBarrier.image, imageMemoryBarrier.subresourceRange, imageMemoryBarrier.newLayout);
        }
    }
}

void CmdTracker::record_vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfos)
{
    assert(pDependencyInfos);
    BasicCmdTracker::record_vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    for (uint32_t event_i = 0; event_i < eventCount; ++event_i) {
        auto imageMemoryBarrierCount = pDependencyInfos[event_i].imageMemoryBarrierCount;
        auto pImageMemoryBarriers = pDependencyInfos[event_i].pImageMemoryBarriers;
        if (imageMemoryBarrierCount && pImageMemoryBarriers) {
            CommandBuffer gvkCommandBuffer;
            gvkCommandBuffer = CommandBuffer(commandBuffer);
            assert(gvkCommandBuffer);
            auto gvkDevice = gvkCommandBuffer.get<Device>();
            assert(gvkDevice);
            for (uint32_t imageMemoryBarrier_i = 0; imageMemoryBarrier_i < imageMemoryBarrierCount; ++imageMemoryBarrier_i) {
                const auto& imageMemoryBarrier = pImageMemoryBarriers[imageMemoryBarrier_i];
                record_image_layout_transition(gvkDevice, imageMemoryBarrier.image, imageMemoryBarrier.subresourceRange, imageMemoryBarrier.newLayout);
            }
        }
    }
}

void CmdTracker::record_vkCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfos)
{
    record_vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

void CmdTracker::record_image_layout_transition(VkDevice device, VkImage image, const VkImageSubresourceRange& imageSubresourceRange, VkImageLayout imageLayout)
{
    // TODO : We should be tracking only image layout deltas, but we're currently
    //  writing all subresource layouts on queue submission.
    auto imageLayoutTrackerItr = mImageLayoutTrackers.find(image);
    if (imageLayoutTrackerItr == mImageLayoutTrackers.end()) {
        Image gvkImage({ device, image });
        assert(gvkImage);
        imageLayoutTrackerItr = mImageLayoutTrackers.insert({ image, gvkImage.get<const ImageLayoutTracker&>() }).first;
    }
    imageLayoutTrackerItr->second.set_image_layouts(imageSubresourceRange, imageLayout);
}

} // namespace state_tracker
} // namespace gvk

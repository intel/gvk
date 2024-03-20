
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

#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-layer/registry.hpp"

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence, VkResult gvkResult)
{
    (void)queue;
    (void)bindInfoCount;
    (void)pBindInfo;
    (void)fence;
    (void)gvkResult;
    assert(false && "VK_LAYER_INTEL_gvk_state_tracker::vkQueueBindSparse() unserviced; gvk maintenance required");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

VkResult StateTracker::post_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult gvkResult)
{
    (void)fence;
    if (submitCount && pSubmits) {
        Queue gvkQueue(queue);
        assert(gvkQueue);
        Device gvkDevice(gvkQueue.get<VkDevice>());
        assert(gvkDevice);
        for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
            const auto& submit = pSubmits[submit_i];
            if (submit.commandBufferCount && submit.pCommandBuffers) {
                for (uint32_t commandBuffer_i = 0; commandBuffer_i < submit.commandBufferCount; ++commandBuffer_i) {
                    auto commandBufferReference = CommandBuffer(submit.pCommandBuffers[commandBuffer_i]).mReference;
                    assert(commandBufferReference);
                    auto& commandBufferControlBlock = commandBufferReference.get_obj();
                    for (const auto& imageLayoutTrackerItr : commandBufferControlBlock.mCmdTracker.get_image_layout_trackers()) {
                        auto imageReference = Image({ commandBufferControlBlock.mDevice, imageLayoutTrackerItr.first }).mReference;
                        assert(imageReference);
                        imageReference.get_obj().mImageLayoutTracker = imageLayoutTrackerItr.second;
                    }
                    #if 0 // TODO : Acceleration structure history
                    for (auto buildAcclerationStructureCmdIndex : commandBufferControlBlock.mCmdTracker.get_build_acceleration_sturcture_cmd_indices()) {
                        const auto& cmds = commandBufferControlBlock.mCmdTracker.get_cmds();
                        assert(buildAcclerationStructureCmdIndex < cmds.size());
                        auto pCmd = (const GvkCommandStructureCmdBuildAccelerationStructuresKHR*)cmds[buildAcclerationStructureCmdIndex];
                        assert(pCmd->sType == get_stype<GvkCommandStructureCmdBuildAccelerationStructuresKHR>());
                        process_build_acceleration_structures(gvkDevice, VK_NULL_HANDLE, pCmd->infoCount, pCmd->pInfos, pCmd->ppBuildRangeInfos);
                    }
                    #endif
                    if (commandBufferControlBlock.mCommandbufferBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
                        commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_ALL_COMMAND_BUFFER_BIT;
                        commandBufferControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_INVALID_BIT;
                        commandBufferControlBlock.mCommandbufferBeginInfo.reset();
                        commandBufferControlBlock.mBeginEndCommandBufferResults = { VK_SUCCESS, VK_SUCCESS };
                        commandBufferControlBlock.mCmdTracker.reset();
                    }
                }
            }
            for (uint32_t wait_i = 0; wait_i < submit.waitSemaphoreCount; ++wait_i) {
                Semaphore gvkSemaphore({ gvkDevice, submit.pWaitSemaphores[wait_i] });
                assert(gvkSemaphore);
                gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
            }
            for (uint32_t signal_i = 0; signal_i < submit.signalSemaphoreCount; ++signal_i) {
                Semaphore gvkSemaphore({ gvkDevice, submit.pSignalSemaphores[signal_i] });
                assert(gvkSemaphore);
                gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
            }
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult)
{
    (void)fence;
    if (submitCount && pSubmits) {
        Queue gvkQueue(queue);
        assert(gvkQueue);
        Device gvkDevice(gvkQueue.get<VkDevice>());
        assert(gvkDevice);
        for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
            const auto& submit = pSubmits[submit_i];
            if (submit.commandBufferInfoCount && submit.pCommandBufferInfos) {
                for (uint32_t commandBufferInfo_i = 0; commandBufferInfo_i < submit.commandBufferInfoCount; ++commandBufferInfo_i) {
                    auto commandBufferReference = CommandBuffer(submit.pCommandBufferInfos[commandBufferInfo_i].commandBuffer).mReference;
                    assert(commandBufferReference);
                    auto& commandBufferControlBlock = commandBufferReference.get_obj();
                    for (const auto& imageLayoutTrackerItr : commandBufferControlBlock.mCmdTracker.get_image_layout_trackers()) {
                        auto imageReference = Image({ commandBufferControlBlock.mDevice, imageLayoutTrackerItr.first }).mReference;
                        assert(imageReference);
                        imageReference.get_obj().mImageLayoutTracker = imageLayoutTrackerItr.second;
                    }
                    #if 0 // TODO : Acceleration structure history
                    for (auto buildAcclerationStructureCmdIndex : commandBufferControlBlock.mCmdTracker.get_build_acceleration_sturcture_cmd_indices()) {
                        const auto& cmds = commandBufferControlBlock.mCmdTracker.get_cmds();
                        assert(buildAcclerationStructureCmdIndex < cmds.size());
                        auto pCmd = (const GvkCommandStructureCmdBuildAccelerationStructuresKHR*)cmds[buildAcclerationStructureCmdIndex];
                        assert(pCmd->sType == get_stype<GvkCommandStructureCmdBuildAccelerationStructuresKHR>());
                        process_build_acceleration_structures(gvkDevice, VK_NULL_HANDLE, pCmd->infoCount, pCmd->pInfos, pCmd->ppBuildRangeInfos);
                    }
                    #endif
                    if (commandBufferControlBlock.mCommandbufferBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
                        commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_ALL_COMMAND_BUFFER_BIT;
                        commandBufferControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_INVALID_BIT;
                        commandBufferControlBlock.mCommandbufferBeginInfo.reset();
                        commandBufferControlBlock.mBeginEndCommandBufferResults = { VK_SUCCESS, VK_SUCCESS };
                        commandBufferControlBlock.mCmdTracker.reset();
                    }
                }
            }
            for (uint32_t wait_i = 0; wait_i < submit.waitSemaphoreInfoCount; ++wait_i) {
                Semaphore gvkSemaphore({ gvkDevice, submit.pWaitSemaphoreInfos[wait_i].semaphore });
                assert(gvkSemaphore);
                gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
            }
            for (uint32_t signal_i = 0; signal_i < submit.signalSemaphoreInfoCount; ++signal_i) {
                Semaphore gvkSemaphore({ gvkDevice, submit.pSignalSemaphoreInfos[signal_i].semaphore });
                assert(gvkSemaphore);
                gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
            }
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult)
{
    return post_vkQueueSubmit2(queue, submitCount, pSubmits, fence, gvkResult);
}

VkResult StateTracker::post_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult gvkResult)
{
    Queue gvkQueue(queue);
    assert(gvkQueue);
    Device gvkDevice(gvkQueue.get<VkDevice>());
    assert(gvkDevice);

    for (uint32_t wait_i = 0; wait_i < pPresentInfo->waitSemaphoreCount; ++wait_i) {
        Semaphore gvkSemaphore({ gvkDevice, pPresentInfo->pWaitSemaphores[wait_i] });
        assert(gvkSemaphore);
        gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
    }

    const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(gvkDevice.get<VkDevice>()));
    assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
    const auto& dispatchTable = dispatchTableItr->second;
    assert(dispatchTable.gvkGetSwapchainImagesKHR);

    std::vector<VkImage> vkImages;
    assert(!pPresentInfo->swapchainCount == !pPresentInfo->pSwapchains);
    assert(!pPresentInfo->swapchainCount == !pPresentInfo->pImageIndices);
    for (uint32_t swapchain_i = 0; swapchain_i < pPresentInfo->swapchainCount; ++swapchain_i) {
        SwapchainKHR gvkSwapchain({ gvkDevice, pPresentInfo->pSwapchains[swapchain_i]});
        assert(gvkSwapchain);

        uint32_t imageCount = 0;
        gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(gvkDevice, gvkSwapchain, &imageCount, nullptr);
        assert(gvkResult == VK_SUCCESS);
        vkImages.clear();
        vkImages.resize(imageCount); // TODO : Scratchpad allocator
        gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(gvkDevice, gvkSwapchain, &imageCount, vkImages.data());
        assert(pPresentInfo->pImageIndices[swapchain_i] < imageCount);

        Image gvkImage({ gvkDevice, vkImages[pPresentInfo->pImageIndices[swapchain_i]]});
        assert(gvkImage);
        gvkImage.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_ACQUIRED_BIT;
        if (gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore) {
            gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
        }
        gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore.reset();
        gvkImage.mReference.get_obj().mSwapchainAcquisitionFence.reset();
    }

    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

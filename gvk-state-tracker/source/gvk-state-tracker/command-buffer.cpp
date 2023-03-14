
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

#include "gvk-state-tracker/cmd-tracker.hpp"
#include "gvk-state-tracker/state-tracker.hpp"

#define GVK_COMMAND_BUFFER_STATUS_FLAGS (GVK_STATE_TRACKER_OBJECT_STATUS_RECORDING_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_PENDING_BIT | GVK_STATE_TRACKER_OBJECT_STATUS_INVALID_BIT)

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult)
{
    (void)device;
    (void)commandPool;
    (void)flags;
    return gvkResult;
}

VkResult StateTracker::post_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        assert(pAllocateInfo);
        assert(pCommandBuffers);
        Device gvkDevice = device;
        assert(gvkDevice);
        CommandPool gvkCommandPool({ device, pAllocateInfo->commandPool });
        assert(gvkCommandPool);
        for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i) {
            CommandBuffer commandBuffer;
            commandBuffer.mReference.reset(gvk::newref, pCommandBuffers[i]);
            auto& controlBlock = commandBuffer.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkCommandBuffer = pCommandBuffers[i];
            controlBlock.mCommandPool = gvkCommandPool;
            controlBlock.mDevice = gvkDevice;
            controlBlock.mCommandBufferAllocateInfo = *pAllocateInfo;
            gvkCommandPool.mReference.get_obj().mCommandBufferTracker.insert(commandBuffer);
        }
    }
    return gvkResult;
}

void StateTracker::post_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    if (commandBufferCount && pCommandBuffers) {
        auto commandPoolReference = CommandPool({ device, commandPool }).mReference;
        assert(commandPoolReference);
        auto& commandPoolControlBlock = commandPoolReference.get_obj();
        for (uint32_t i = 0; i < commandBufferCount; ++i) {
            CommandBuffer commandBuffer(pCommandBuffers[i]);
            assert(commandBuffer);
            commandBuffer.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            commandBuffer.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
            commandPoolControlBlock.mCommandBufferTracker.erase(pCommandBuffers[i]);
        }
    }
}

VkResult StateTracker::post_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo, VkResult gvkResult)
{
    assert(pBeginInfo);
    auto commandBufferReference = CommandBuffer(commandBuffer).mReference;
    assert(commandBufferReference);
    auto& commandBufferControlBlock = commandBufferReference.get_obj();
    commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_COMMAND_BUFFER_STATUS_FLAGS;
    commandBufferControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_RECORDING_BIT;
    commandBufferControlBlock.mCommandbufferBeginInfo = *pBeginInfo;
    commandBufferControlBlock.mBeginEndCommandBufferResults = { gvkResult, VK_SUCCESS};
    commandBufferControlBlock.mCmdTracker.reset();
    return gvkResult;
}

VkResult StateTracker::post_vkEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult gvkResult)
{
    auto commandBufferReference = CommandBuffer(commandBuffer).mReference;
    assert(commandBufferReference);
    auto& commandBufferControlBlock = commandBufferReference.get_obj();
    commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_COMMAND_BUFFER_STATUS_FLAGS;
    commandBufferControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT;
    commandBufferControlBlock.mBeginEndCommandBufferResults.second = gvkResult;
    return gvkResult;
}

VkResult StateTracker::post_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult gvkResult)
{
    (void)fence;
    if (submitCount && pSubmits) {
        Queue gvkQueue(queue);
        assert(gvkQueue);
        auto gvkDevice = gvkQueue.get<Device>();
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
                    if (commandBufferControlBlock.mCommandbufferBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
                        commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_COMMAND_BUFFER_STATUS_FLAGS;
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
                gvkSemaphore.mReference.get_obj().mSignaled = VK_FALSE;
                gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
            }
            for (uint32_t signal_i = 0; signal_i < submit.signalSemaphoreCount; ++signal_i) {
                Semaphore gvkSemaphore({ gvkDevice, submit.pSignalSemaphores[signal_i] });
                assert(gvkSemaphore);
                gvkSemaphore.mReference.get_obj().mSignaled = VK_TRUE;
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
        auto gvkDevice = gvkQueue.get<Device>();
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
                    if (commandBufferControlBlock.mCommandbufferBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
                        commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_COMMAND_BUFFER_STATUS_FLAGS;
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
                gvkSemaphore.mReference.get_obj().mSignaled = VK_FALSE;
                gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
            }
            for (uint32_t signal_i = 0; signal_i < submit.signalSemaphoreInfoCount; ++signal_i) {
                Semaphore gvkSemaphore({ gvkDevice, submit.pSignalSemaphoreInfos[signal_i].semaphore });
                assert(gvkSemaphore);
                gvkSemaphore.mReference.get_obj().mSignaled = VK_TRUE;
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
    auto gvkDevice = gvkQueue.get<Device>();
    assert(gvkDevice);
    for (uint32_t wait_i = 0; wait_i < pPresentInfo->waitSemaphoreCount; ++wait_i) {
        Semaphore gvkSemaphore({ gvkDevice, pPresentInfo->pWaitSemaphores[wait_i] });
        assert(gvkSemaphore);
        gvkSemaphore.mReference.get_obj().mSignaled = VK_FALSE;
        gvkSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
    }
    return gvkResult;
}

VkResult StateTracker::post_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult)
{
    (void)flags;
    auto commandBufferReference = CommandBuffer(commandBuffer).mReference;
    assert(commandBufferReference);
    auto& commandBufferControlBlock = commandBufferReference.get_obj();
    commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_COMMAND_BUFFER_STATUS_FLAGS;
    commandBufferControlBlock.mCommandbufferBeginInfo.reset();
    commandBufferControlBlock.mBeginEndCommandBufferResults = { VK_SUCCESS, VK_SUCCESS };
    commandBufferControlBlock.mCmdTracker.reset();
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk


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

#include "gvk-layer/registry.hpp"
#include "gvk-state-tracker/cmd-tracker.hpp"
#include "gvk-state-tracker/state-tracker.hpp"

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult)
{
    (void)flags;
    if (gvkResult == VK_SUCCESS) {
        auto commandPoolReference = CommandPool({ device, commandPool }).mReference;
        assert(commandPoolReference);
        auto& commandPoolControlBlock = commandPoolReference.get_obj();
        commandPoolControlBlock.mCommandBufferTracker.enumerate(
            [&](CommandBuffer commandBuffer)
            {
                gvkResult = post_vkResetCommandBuffer(commandBuffer, 0, gvkResult);
                return gvkResult == VK_SUCCESS;
            }
        );
    }
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
    commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_ALL_COMMAND_BUFFER_BIT;
    commandBufferControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_RECORDING_BIT;
    commandBufferControlBlock.mCommandbufferBeginInfo = *pBeginInfo;
    commandBufferControlBlock.mBeginEndCommandBufferResults = { gvkResult, VK_SUCCESS };
    commandBufferControlBlock.mCmdTracker.reset();
    return gvkResult;
}

VkResult StateTracker::post_vkEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult gvkResult)
{
    auto commandBufferReference = CommandBuffer(commandBuffer).mReference;
    assert(commandBufferReference);
    auto& commandBufferControlBlock = commandBufferReference.get_obj();
    commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_ALL_COMMAND_BUFFER_BIT;
    commandBufferControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_EXECUTABLE_BIT;
    commandBufferControlBlock.mBeginEndCommandBufferResults.second = gvkResult;
    return gvkResult;
}

VkResult StateTracker::post_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult)
{
    (void)flags;
    auto commandBufferReference = CommandBuffer(commandBuffer).mReference;
    assert(commandBufferReference);
    auto& commandBufferControlBlock = commandBufferReference.get_obj();
    commandBufferControlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKER_OBJECT_STATUS_ALL_COMMAND_BUFFER_BIT;
    commandBufferControlBlock.mCommandbufferBeginInfo.reset();
    commandBufferControlBlock.mBeginEndCommandBufferResults = { VK_SUCCESS, VK_SUCCESS };
    commandBufferControlBlock.mCmdTracker.reset();
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

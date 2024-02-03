
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

#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"

namespace gvk {
namespace restore_point {

VkResult Creator::process_VkSemaphore(GvkSemaphoreRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert(restoreInfo.pSemaphoreCreateInfo);
        auto pSemaphoreTypeCreateInfo = get_pnext<VkSemaphoreTypeCreateInfo>(*restoreInfo.pSemaphoreCreateInfo);
        if (pSemaphoreTypeCreateInfo && pSemaphoreTypeCreateInfo->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE) {
            Device gvkDevice = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
            gvk_result(gvkDevice.get<DispatchTable>().gvkGetSemaphoreCounterValue(gvkDevice, restoreInfo.handle, &restoreInfo.value));
            if (restoreInfo.value) {
                restoreInfo.flags |= GVK_RESTORE_POINT_OBJECT_STATUS_SIGNALED_BIT;
            }
        } else {
            restoreInfo.value = restoreInfo.flags & GVK_RESTORE_POINT_OBJECT_STATUS_SIGNALED_BIT ? 1 : 0;
        }
        gvk_result(BasicCreator::process_VkSemaphore(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkSemaphore_state(const GvkRestorePointObject& restorePointObject, const GvkSemaphoreRestoreInfo& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkSemaphore(const GvkRestorePointObject& restorePointObject, const GvkSemaphoreRestoreInfo& restoreInfo)
{
    // Stateless restore
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApplier::process_VkSemaphore(restorePointObject, restoreInfo));
        auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
        auto vkSemaphore = (VkSemaphore)get_restored_object(restorePointObject).handle;
        auto pSemaphoreTypeCreateInfo = get_pnext<VkSemaphoreTypeCreateInfo>(*restoreInfo.pSemaphoreCreateInfo);
        if (pSemaphoreTypeCreateInfo && pSemaphoreTypeCreateInfo->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (restoreInfo.value != pSemaphoreTypeCreateInfo->initialValue) {
                auto semaphoreSignalInfo = get_default<VkSemaphoreSignalInfo>();
                semaphoreSignalInfo.semaphore = vkSemaphore;
                semaphoreSignalInfo.value = restoreInfo.value;
                gvk_result(Device(vkDevice).get<DispatchTable>().gvkSignalSemaphore(vkDevice, &semaphoreSignalInfo));
            }
        } else if (restoreInfo.value) {
            Device gvkDevice(vkDevice);
            auto vkCommandBuffer = mVkCommandBuffers[vkDevice];
            const auto& gvkFence = mFences[vkDevice];
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(gvkDevice.get<DispatchTable>().gvkBeginCommandBuffer(vkCommandBuffer, &commandBufferBeginInfo));
            gvk_result(gvkDevice.get<DispatchTable>().gvkEndCommandBuffer(vkCommandBuffer));
            auto submitInfo = get_default<VkSubmitInfo>();
            if (restoreInfo.flags & GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT) {
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = &vkSemaphore;
            }
            gvk_result(gvkDevice.get<DispatchTable>().gvkQueueSubmit(gvkDevice.get<QueueFamilies>()[0].queues[0], 1, &submitInfo, gvkFence));
            gvk_result(gvkDevice.get<DispatchTable>().gvkWaitForFences(gvkDevice, 1, &gvkFence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(gvkDevice.get<DispatchTable>().gvkResetFences(gvkDevice, 1, &gvkFence.get<VkFence>()));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

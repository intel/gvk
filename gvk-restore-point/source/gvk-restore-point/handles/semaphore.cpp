
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
#include "gvk-layer/registry.hpp"

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
                restoreInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_SIGNALED_BIT;
            }
        } else {
            restoreInfo.value = restoreInfo.flags & GVK_STATE_TRACKED_OBJECT_STATUS_SIGNALED_BIT ? 1 : 0;
        }
        gvk_result(BasicCreator::process_VkSemaphore(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::restore_VkSemaphore_state(const GvkStateTrackedObject& restorePointObject, const GvkSemaphoreRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        auto vkSemaphore = (VkSemaphore)get_restored_object(restorePointObject).handle;
        auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
        auto pSemaphoreTypeCreateInfo = get_pnext<VkSemaphoreTypeCreateInfo>(*restoreInfo.pSemaphoreCreateInfo);
        if (pSemaphoreTypeCreateInfo && pSemaphoreTypeCreateInfo->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE) {
            auto semaphoreSignalInfo = get_default<VkSemaphoreSignalInfo>();
            semaphoreSignalInfo.semaphore = vkSemaphore;
            semaphoreSignalInfo.value = restoreInfo.value;
            gvk_result(mApplyInfo.dispatchTable.gvkSignalSemaphore(vkDevice, &semaphoreSignalInfo));
        } else {
            uint64_t value = 0;
            if (!(mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT)) {
                GvkStateTrackedObjectInfo stateTrackedObjectInfo{ };
                gvkGetStateTrackedObjectInfo(&restorePointObject, &stateTrackedObjectInfo);
                value = stateTrackedObjectInfo.flags & GVK_STATE_TRACKED_OBJECT_STATUS_SIGNALED_BIT ? 1 : 0;
            }
            if (value != restoreInfo.value) {
                if (restoreInfo.value) {
                    const auto& deviceRestoreInfo = mDeviceRestoreInfos[(VkDevice)restorePointObject.dispatchableHandle];
                    gvk_result(deviceRestoreInfo->queueCount ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    gvk_result(deviceRestoreInfo->pQueues ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    auto vkQueue = (VkQueue)get_restored_object({ VK_OBJECT_TYPE_QUEUE, (uint64_t)deviceRestoreInfo->pQueues[0], (uint64_t)deviceRestoreInfo->pQueues[0] }).handle;
                    auto submitInfo = get_default<VkSubmitInfo>();
                    submitInfo.signalSemaphoreCount = 1;
                    submitInfo.pSignalSemaphores = &vkSemaphore;
                    gvk_result(mApplyInfo.dispatchTable.gvkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE));
                } else {
                    // NOTE : Wait operations are handled with the objects that need them
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

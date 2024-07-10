
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

VkResult Creator::process_VkFence(GvkFenceRestoreInfo& restoreInfo)
{
    Device device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
    restoreInfo.status = device.get<DispatchTable>().gvkGetFenceStatus(device, restoreInfo.handle);
    return BasicCreator::process_VkFence(restoreInfo);
}

VkResult Applier::restore_VkFence_state(const GvkStateTrackedObject& restorePointObject, const GvkFenceRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        auto vkFence = (VkFence)get_restored_object(restorePointObject).handle;
        auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
        auto status = restoreInfo.pFenceCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT ? VK_SUCCESS : VK_NOT_READY;
        if (!(mApplyInfo.flags & GVK_RESTORE_POINT_APPLY_SYNTHETIC_BIT)) {
            status = mApplyInfo.dispatchTable.gvkGetFenceStatus(vkDevice, vkFence);
        }
        if (status != restoreInfo.status) {
            const auto& deviceRestoreInfo = mDeviceRestoreInfos[(VkDevice)restorePointObject.dispatchableHandle];
            gvk_result(deviceRestoreInfo->queueCount ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(deviceRestoreInfo->pQueues ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            auto vkQueue = (VkQueue)get_restored_object({ VK_OBJECT_TYPE_QUEUE, (uint64_t)deviceRestoreInfo->pQueues[0], (uint64_t)deviceRestoreInfo->pQueues[0] }).handle;
            if (restoreInfo.status == VK_SUCCESS && !(restoreInfo.pFenceCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT)) {
                gvk_result(mApplyInfo.dispatchTable.gvkQueueSubmit(vkQueue, 0, nullptr, vkFence));
            } else if (restoreInfo.status != VK_SUCCESS && restoreInfo.pFenceCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) {
                gvk_result(mApplyInfo.dispatchTable.gvkResetFences(vkDevice, 1, &vkFence));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

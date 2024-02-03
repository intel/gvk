
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
    assert(device);
    restoreInfo.status = device.get<DispatchTable>().gvkGetFenceStatus(device, restoreInfo.handle);
    return BasicCreator::process_VkFence(restoreInfo);
}

VkResult Applier::restore_VkFence_state(const GvkRestorePointObject& restorePointObject, const GvkFenceRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // TODO : Check if dispatchTable is pointing at the implementation?
        // TODO : vkGetFenceStatus() won't work for deferred...
        bool liveRestore_HACK = true;
        if (liveRestore_HACK) {
            // TODO : get_restored_object() should always get the live object...
            Device gvkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
            auto vkFence = (VkFence)get_restored_object(restorePointObject).handle;
            auto status = mApplyInfo.dispatchTable.gvkGetFenceStatus(gvkDevice, vkFence);
            if (status != restoreInfo.status) {
                switch (restoreInfo.status) {
                case VK_SUCCESS: {
                    const auto& gvkQueue = gvkDevice.get<QueueFamilies>()[0].queues[0];
                    gvk_result(mApplyInfo.dispatchTable.gvkQueueSubmit(gvkQueue, 0, nullptr, vkFence));
                } break;
                case VK_NOT_READY: {
                    gvk_result(mApplyInfo.dispatchTable.gvkResetFences(gvkDevice, 1, &vkFence));
                } break;
                default: {
                    gvk_result(VK_ERROR_INITIALIZATION_FAILED);
                } break;
                }
            }
        } else {
            // TODO : Full restore based on restoreInfo...
        }
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkFence(const GvkRestorePointObject& restorePointObject, const GvkFenceRestoreInfo& restoreInfo)
{
    // Stateless restore
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApplier::process_VkFence(restorePointObject, restoreInfo));
        auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
        auto vkFence = (VkFence)get_restored_object(restorePointObject).handle;
        if (restoreInfo.status == VK_SUCCESS && !(restoreInfo.pFenceCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT)) {
            Device gvkDevice(vkDevice);
            gvk_result(gvkDevice.get<DispatchTable>().gvkQueueSubmit(gvkDevice.get<QueueFamilies>()[0].queues[0], 0, nullptr, vkFence));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

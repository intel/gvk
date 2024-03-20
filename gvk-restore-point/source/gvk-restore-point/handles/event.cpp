
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

VkResult Creator::process_VkEvent(GvkEventRestoreInfo& restoreInfo)
{
    Device device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
    assert(device);
    restoreInfo.status = device.get<DispatchTable>().gvkGetEventStatus(device, restoreInfo.handle);
    return BasicCreator::process_VkEvent(restoreInfo);
}

VkResult Applier::restore_VkEvent_state(const GvkRestorePointObject& restorePointObject, const GvkEventRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
        auto vkEvent = (VkEvent)get_restored_object(restorePointObject).handle;
        switch (restoreInfo.status) {
        case VK_EVENT_SET: {
            gvk_result(mApplyInfo.dispatchTable.gvkSetEvent(vkDevice, vkEvent));
        } break;
        case VK_EVENT_RESET: {
            gvk_result(mApplyInfo.dispatchTable.gvkResetEvent(vkDevice, vkEvent));
        } break;
        default: {
            gvk_result(VK_ERROR_INITIALIZATION_FAILED);
        } break;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkEvent(const GvkRestorePointObject& restorePointObject, const GvkEventRestoreInfo& restoreInfo)
{
    // Stateless restore
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApplier::process_VkEvent(restorePointObject, restoreInfo));
        if (restoreInfo.status == VK_EVENT_SET) {
            auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
            auto vkEvent = (VkEvent)get_restored_object(restorePointObject).handle;
            gvk_result(Device(vkDevice).get<DispatchTable>().gvkSetEvent(vkDevice, vkEvent));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

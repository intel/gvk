
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

#include <cassert>

namespace gvk {
namespace state_tracker {

VkResult StateTracker::pre_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult)
{
    (void)physicalDevice;
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pDevice;
    for (auto itr : layer::Registry::get().VkPhysicalDevices) {
        PhysicalDevice gvkPhysicalDevice(itr.second);
        assert(gvkPhysicalDevice);
        auto& physicalDeviceControlBlock = gvkPhysicalDevice.mReference.get_obj();
        assert(!physicalDeviceControlBlock.mApplicationHandle || physicalDeviceControlBlock.mApplicationHandle == (uint64_t)itr.first);
        physicalDeviceControlBlock.mApplicationHandle = (uint64_t)itr.first;
    }
    return gvkResult;
}

VkResult StateTracker::post_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        // TODO : Move to layer::Registry so it's handled for all layers...
        auto pNext = (VkBaseOutStructure*)pCreateInfo;
        while (pNext) {
            while (pNext->pNext && pNext->pNext->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO) {
                pNext->pNext = pNext->pNext->pNext;
            }
            pNext = pNext->pNext;
        }
        gvkResult = BasicStateTracker::post_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, gvkResult);
        assert(gvkResult == VK_SUCCESS);
        assert(pDevice);
        Device gvkDevice(*pDevice);
        assert(gvkDevice);
        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(*pDevice));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
        const auto& dispatchTable = dispatchTableItr->second;
        assert(dispatchTable.gvkGetDeviceQueue);
        for (uint32_t queueCreateInfo_i = 0; queueCreateInfo_i < pCreateInfo->queueCreateInfoCount; ++queueCreateInfo_i) {
            const auto& queueCreateInfo = pCreateInfo->pQueueCreateInfos[queueCreateInfo_i];
            for (uint32_t queue_i = 0; queue_i < queueCreateInfo.queueCount; ++queue_i) {
                VkQueue vkQueue = VK_NULL_HANDLE;
                dispatchTable.gvkGetDeviceQueue(*pDevice, queueCreateInfo.queueFamilyIndex, queue_i, &vkQueue);
                assert(vkQueue);
                Queue queue;
                queue.mReference.reset(newref, vkQueue);
                auto& controlBlock = queue.mReference.get_obj();
                controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
                controlBlock.mVkQueue = vkQueue;
                controlBlock.mVkDevice = *pDevice;
                controlBlock.mDeviceQueueCreateInfo = queueCreateInfo;
                gvkDevice.mReference.get_obj().mQueueTracker.insert(queue);
            }
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

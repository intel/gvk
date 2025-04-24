
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
#include "gvk-handles.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

VkResult StateTracker::pre_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult)
{
    (void)physicalDevice;
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pDevice;
    auto& layerRegistry = layer::Registry::get();
    for (auto itr : layerRegistry.VkPhysicalDevices) {
        PhysicalDevice gvkPhysicalDevice(itr.second);
        // NOTE : It would be _much_ nicer to be able to assume that every tracked
        //  VkPhysicalDevice is live, but there is a little bit of a dance that needs to
        //  be done to keep track of the application's VkPhysicalDevice handles vs the
        //  loader's, and on top of that have some multi-layered intialization and
        //  deinitialization interleaved with the "actual" persistent context objects
        //  the workload will use once it's up and running, so in one process, once a
        //  VkPhysicalDevice is encountered it's not disposed of...we assume that an
        //  application won't send down a bad VkPhysicalDevice (the app would be busted
        //  if it were doing that anyway).
        // TODO : It should be possible to setup "bulletproof" logic that would make
        //  disposing of unused VkPhysicalDevice handles work intuitively, but it will
        //  be time consuming and truly not much of "win" other than a cleaner context,
        //  which is important, but certainly lower priority at the moment.
        if (gvkPhysicalDevice) {
            auto& physicalDeviceControlBlock = gvkPhysicalDevice.mReference.get_obj();
            assert(!physicalDeviceControlBlock.mApplicationHandle || physicalDeviceControlBlock.mApplicationHandle == (uint64_t)itr.first);
            physicalDeviceControlBlock.mApplicationHandle = (uint64_t)itr.first;
        }
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

        gvk::state_tracker::Device gvkStateTrackedDevice(*pDevice);
        assert(gvkStateTrackedDevice);
       // NOTE : This assert() is here to ensure we have a live state tracked handle
        //  for the given VkPhysicalDevice.  See the note in the pre_vkCreateDevice()
        //  for more info.
        assert(gvkStateTrackedDevice.mReference->mPhysicalDevice);

        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(*pDevice));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
        const auto& dispatchTable = dispatchTableItr->second;

        gvk::Device gvkDevice;
        gvkResult = gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &dispatchTable, *pDevice, &gvkDevice);
        assert(gvkResult == VK_SUCCESS);
        mGvkDevices.insert(gvkDevice);

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
                gvkStateTrackedDevice.mReference.get_obj().mQueueTracker.insert(queue);
            }
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

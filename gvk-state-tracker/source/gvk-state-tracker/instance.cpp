
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

VkResult StateTracker::post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        auto pNext = (VkBaseOutStructure*)pCreateInfo;
        while (pNext) {
            while (pNext->pNext && pNext->pNext->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO) {
                pNext->pNext = pNext->pNext->pNext;
            }
            pNext = pNext->pNext;
        }
        gvkResult = BasicStateTracker::post_vkCreateInstance(pCreateInfo, pAllocator, pInstance, gvkResult);
        assert(gvkResult == VK_SUCCESS);
        assert(pInstance);
        Instance gvkInstance(*pInstance);
        assert(gvkInstance);
        const auto& dispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(*pInstance));
        assert(dispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end());
        const auto& dispatchTable = dispatchTableItr->second;
        assert(dispatchTable.gvkEnumeratePhysicalDevices);
        uint32_t physicalDeviceCount = 0;
        gvkResult = dispatchTable.gvkEnumeratePhysicalDevices(*pInstance, &physicalDeviceCount, nullptr);
        assert(gvkResult == VK_SUCCESS);
        std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
        gvkResult = dispatchTable.gvkEnumeratePhysicalDevices(*pInstance, &physicalDeviceCount, vkPhysicalDevices.data());
        assert(gvkResult == VK_SUCCESS);
        for (auto vkPhysicalDevice : vkPhysicalDevices) {
            PhysicalDevice physicalDevice;
            physicalDevice.mReference.reset(newref, vkPhysicalDevice);
            physicalDevice.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            physicalDevice.mReference.get_obj().mVkPhysicalDevice = vkPhysicalDevice;
            physicalDevice.mReference.get_obj().mVkInstance = *pInstance;
            gvkInstance.mReference.get_obj().mPhysicalDeviceTracker.insert(physicalDevice);
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk


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

#include "gvk-state-tracker/generated/state-tracked-handles.hpp"

namespace gvk {
namespace state_tracker {

void DisplayModeKHR::enumerate_dependencies(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        stateTrackedObject.handle = (uint64_t)get<VkDisplayModeKHR>();
        stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
        DisplayKHR(DisplayKHR::HandleIdType(mReference.get_obj().mPhysicalDevice, mReference.get_obj().mDisplayKHR)).enumerate_dependencies(pfnCallback, pUserData);
    }
}

void SwapchainKHR::enumerate_dependencies(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        stateTrackedObject.handle = (uint64_t)get<VkSwapchainKHR>();
        stateTrackedObject.dispatchableHandle = (uint64_t)get<VkDevice>();
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
        mReference.get_obj().mDevice.enumerate_dependencies(pfnCallback, pUserData);
        const auto& device = mReference.get_obj().mDevice;
        assert(device);
        auto physicalDevice = device ? device.get<PhysicalDevice>() : VK_NULL_HANDLE;
        assert(physicalDevice);
        auto instance = physicalDevice ? physicalDevice.get<Instance>() : VK_NULL_HANDLE;
        assert(instance);
        SurfaceKHR(SurfaceKHR::HandleIdType(instance, mReference.get_obj().mSurfaceKHR)).enumerate_dependencies(pfnCallback, pUserData);
    }
}

} // namespace state_tracker
} // namespace gvk

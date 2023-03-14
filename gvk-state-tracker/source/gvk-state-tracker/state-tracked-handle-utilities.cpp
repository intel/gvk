
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
#include "gvk-state-tracker/state-tracker.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

void PhysicalDevice::enumerate(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        switch (StateTracker::get_physical_device_enumeration_mode()) {
        case StateTracker::PhysicalDeviceEnumerationMode::Application: {
            stateTrackedObject.handle = get<uint64_t>();
            stateTrackedObject.dispatchableHandle = get<uint64_t>();
        } break;
        case StateTracker::PhysicalDeviceEnumerationMode::Loader: {
            stateTrackedObject.handle = (uint64_t)get<VkPhysicalDevice>();
            stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        } break;
        default: {
            assert(false);
        } break;
        }
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
        const auto& controlBlock = mReference.get_obj();
        controlBlock.mDeviceTracker.enumerate(pfnCallback, pUserData);
        controlBlock.mDisplayKHRTracker.enumerate(pfnCallback, pUserData);
    }
}

void PhysicalDevice::enumerate_dependencies(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        switch (StateTracker::get_physical_device_enumeration_mode()) {
        case StateTracker::PhysicalDeviceEnumerationMode::Application: {
            stateTrackedObject.handle = get<uint64_t>();
            stateTrackedObject.dispatchableHandle = get<uint64_t>();
        } break;
        case StateTracker::PhysicalDeviceEnumerationMode::Loader: {
            stateTrackedObject.handle = (uint64_t)get<VkPhysicalDevice>();
            stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        } break;
        default: {
            assert(false);
        } break;
        }
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
        get<Instance>().enumerate_dependencies(pfnCallback, pUserData);
    }
}

void DisplayKHR::enumerate(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        stateTrackedObject.handle = (uint64_t)get<VkDisplayKHR>();
        switch (StateTracker::get_physical_device_enumeration_mode()) {
        case StateTracker::PhysicalDeviceEnumerationMode::Application: {
            PhysicalDevice gvkPhysicalDevice(get<VkPhysicalDevice>());
            assert(gvkPhysicalDevice);
            stateTrackedObject.dispatchableHandle = gvkPhysicalDevice.get<uint64_t>();
        } break;
        case StateTracker::PhysicalDeviceEnumerationMode::Loader: {
            stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        } break;
        default: {
            assert(false);
        } break;
        }
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
        const auto& controlBlock = mReference.get_obj();
        controlBlock.mDisplayModeKHRTracker.enumerate(pfnCallback, pUserData);
    }
}

void DisplayKHR::enumerate_dependencies(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        stateTrackedObject.handle = (uint64_t)get<VkDisplayKHR>();
        switch (StateTracker::get_physical_device_enumeration_mode()) {
        case StateTracker::PhysicalDeviceEnumerationMode::Application: {
            PhysicalDevice gvkPhysicalDevice(get<VkPhysicalDevice>());
            assert(gvkPhysicalDevice);
            stateTrackedObject.dispatchableHandle = gvkPhysicalDevice.get<uint64_t>();
        } break;
        case StateTracker::PhysicalDeviceEnumerationMode::Loader: {
            stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        } break;
        default: {
            assert(false);
        } break;
        }
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
    }
}

void DisplayModeKHR::enumerate(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        stateTrackedObject.handle = (uint64_t)get<VkDisplayModeKHR>();
        switch (StateTracker::get_physical_device_enumeration_mode()) {
        case StateTracker::PhysicalDeviceEnumerationMode::Application: {
            PhysicalDevice gvkPhysicalDevice(get<VkPhysicalDevice>());
            assert(gvkPhysicalDevice);
            stateTrackedObject.dispatchableHandle = gvkPhysicalDevice.get<uint64_t>();
        } break;
        case StateTracker::PhysicalDeviceEnumerationMode::Loader: {
            stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        } break;
        default: {
            assert(false);
        } break;
        }
        pfnCallback(&stateTrackedObject, nullptr, pUserData);
    }
}

void DisplayModeKHR::enumerate_dependencies(PFN_gvkEnumerateStateTrackedObjectsCallback pfnCallback, void* pUserData) const
{
    assert(pfnCallback);
    if (mReference) {
        GvkStateTrackedObject stateTrackedObject { };
        stateTrackedObject.type = get<VkObjectType>();
        stateTrackedObject.handle = (uint64_t)get<VkDisplayModeKHR>();
        switch (StateTracker::get_physical_device_enumeration_mode()) {
        case StateTracker::PhysicalDeviceEnumerationMode::Application: {
            PhysicalDevice gvkPhysicalDevice(get<VkPhysicalDevice>());
            assert(gvkPhysicalDevice);
            stateTrackedObject.dispatchableHandle = gvkPhysicalDevice.get<uint64_t>();
        } break;
        case StateTracker::PhysicalDeviceEnumerationMode::Loader: {
            stateTrackedObject.dispatchableHandle = (uint64_t)get<VkPhysicalDevice>();
        } break;
        default: {
            assert(false);
        } break;
        }
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

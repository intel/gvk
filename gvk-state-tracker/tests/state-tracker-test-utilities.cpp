
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

#include "state-tracker-test-utilities.hpp"

PFN_gvkEnumerateStateTrackedObjects pfnGvkEnumerateStateTrackedObjects;
PFN_gvkEnumerateStateTrackedObjectDependencies pfnGvkEnumerateStateTrackedObjectDependencies;
PFN_gvkEnumerateStateTrackedObjectBindings pfnGvkEnumerateStateTrackedObjectBindings;
PFN_gvkGetStateTrackedObjectInfo pfnGvkGetStateTrackedObjectInfo;
PFN_gvkGetStateTrackedObjectCreateInfo pfnGvkGetStateTrackedObjectCreateInfo;
PFN_gvkGetStateTrackedObjectAllocateInfo pfnGvkGetStateTrackedObjectAllocateInfo;
PFN_gvkGetStateTrackedImageLayouts pfnGvkGetStateTrackedImageLayouts;
PFN_gvkSetStateTrackerPhysicalDevices pfnGvkSetStateTrackerPhysicalDevices;

namespace gvk {

template <> void print<GvkStateTrackedObjectStatusBits>(Printer& printer, const GvkStateTrackedObjectStatusBits& value)
{
    switch (value) {
    case GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT: printer.print_enum("GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT", value); break;
    case GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT: printer.print_enum("GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT", value); break;
    default: printer.print_enum("GvkStateTrackedObjectStatusBits_UNKNOWN", value);
    }
}

template <> void print<GvkStateTrackedObjectStatusBits>(Printer& printer, std::underlying_type_t<GvkStateTrackedObjectStatusBits> flags)
{
    std::stringstream strStrm;
    if (flags & GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT) strStrm << "GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT|";
    if (flags & GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT) strStrm << "GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT|";
    auto str = strStrm.str();
    if (!str.empty()) {
        str.pop_back();
    }
    printer.print_enum(str.c_str(), (VkFramebufferCreateFlagBits)flags);
}

template <> void print<GvkStateTrackedObject>(Printer& printer, const GvkStateTrackedObject& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("type", obj.type);
            printer.print_field("handle", obj.handle);
            printer.print_field("dispatchableHandle", obj.dispatchableHandle);
        }
    );
}

template <> void print<GvkStateTrackedObjectInfo>(Printer& printer, const GvkStateTrackedObjectInfo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_flags<GvkStateTrackedObjectStatusBits>("flags", obj.flags);
        }
    );
}

} // namespace gvk

VkResult StateTrackerValidationContext::create(StateTrackerValidationContext* pContext)
{
    assert(pContext);
    auto vkLayerPath = gvk::get_env_var("VK_LAYER_PATH");
    if (vkLayerPath.empty()) {
#if defined(_WIN32) || defined(_WIN64)
        gvk::set_vk_layer_path_from_windows_registry();
#endif
        gvk::append_value_to_env_var("VK_LAYER_PATH", GVK_STATE_TRACKER_LAYER_JSON_PATH);
    }
    std::array<const char*, 1> layers { VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME };
    auto instanceCreateInfo = gvk::get_default<VkInstanceCreateInfo>();
    instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
    instanceCreateInfo.ppEnabledLayerNames = layers.data();
    auto contextCreateInfo = gvk::get_default<gvk::Context::CreateInfo>();
    contextCreateInfo.loadValidationLayer = VK_TRUE;
    contextCreateInfo.loadWsiExtensions = VK_TRUE;
    contextCreateInfo.pInstanceCreateInfo = &instanceCreateInfo;
    return gvk::Context::create(&contextCreateInfo, nullptr, pContext);
}

VkResult StateTrackerValidationContext::create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks*)
{
    uint32_t physicalDeviceCount = 0;
    mInstance.get<gvk::DispatchTable>().gvkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
    mInstance.get<gvk::DispatchTable>().gvkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, vkPhysicalDevices.data());
    std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties(physicalDeviceCount);
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        mInstance.get<gvk::DispatchTable>().gvkGetPhysicalDeviceProperties(vkPhysicalDevices[i], &physicalDeviceProperties[i]);
    }

    auto dlStateTracker = gvk_dlopen(VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME);
    assert(dlStateTracker);
    pfnGvkSetStateTrackerPhysicalDevices = (PFN_gvkSetStateTrackerPhysicalDevices)gvk_dlsym(dlStateTracker, "gvkSetStateTrackerPhysicalDevices");
    assert(pfnGvkSetStateTrackerPhysicalDevices);
    pfnGvkSetStateTrackerPhysicalDevices(mInstance, physicalDeviceCount, vkPhysicalDevices.data(), physicalDeviceProperties.data());
    gvk_dlclose(dlStateTracker);

    assert(pDeviceCreateInfo);
    auto physicalDeviceSynchronization2Features = gvk::get_default<VkPhysicalDeviceSynchronization2Features>();
    auto availablePhysicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures2>();
    availablePhysicalDeviceFeatures.pNext = &physicalDeviceSynchronization2Features;
    auto dispatchTable = get_physical_devices()[0].get<gvk::DispatchTable>();
    assert(dispatchTable.gvkGetPhysicalDeviceFeatures2);
    dispatchTable.gvkGetPhysicalDeviceFeatures2(get_physical_devices()[0], &availablePhysicalDeviceFeatures);
    auto enabledPhysicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures2>();
    if (physicalDeviceSynchronization2Features.synchronization2) {
        enabledPhysicalDeviceFeatures.pNext = &physicalDeviceSynchronization2Features;
    }
    auto deviceCreateInfo = *pDeviceCreateInfo;
    deviceCreateInfo.pNext = &enabledPhysicalDeviceFeatures;
    mDevices.push_back({ });
    return gvk::Device::create(get_physical_devices()[0], &deviceCreateInfo, nullptr, &mDevices.back());
}

void load_gvk_state_tracker_entry_points()
{
    auto dlStateTracker = gvk_dlopen(VK_LAYER_INTEL_GVK_STATE_TRACKER_NAME);
    assert(dlStateTracker);
    pfnGvkEnumerateStateTrackedObjects = (PFN_gvkEnumerateStateTrackedObjects)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedObjects");
    assert(pfnGvkEnumerateStateTrackedObjects);
    pfnGvkEnumerateStateTrackedObjectDependencies = (PFN_gvkEnumerateStateTrackedObjectDependencies)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedObjectDependencies");
    assert(pfnGvkEnumerateStateTrackedObjectDependencies);
    pfnGvkEnumerateStateTrackedObjectBindings = (PFN_gvkEnumerateStateTrackedObjectBindings)gvk_dlsym(dlStateTracker, "gvkEnumerateStateTrackedObjectBindings");
    assert(pfnGvkEnumerateStateTrackedObjectBindings);
    pfnGvkGetStateTrackedObjectInfo = (PFN_gvkGetStateTrackedObjectInfo)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedObjectInfo");
    assert(pfnGvkGetStateTrackedObjectInfo);
    pfnGvkGetStateTrackedObjectCreateInfo = (PFN_gvkGetStateTrackedObjectCreateInfo)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedObjectCreateInfo");
    assert(pfnGvkGetStateTrackedObjectCreateInfo);
    pfnGvkGetStateTrackedObjectAllocateInfo = (PFN_gvkGetStateTrackedObjectAllocateInfo)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedObjectAllocateInfo");
    assert(pfnGvkGetStateTrackedObjectAllocateInfo);
    pfnGvkGetStateTrackedImageLayouts = (PFN_gvkGetStateTrackedImageLayouts)gvk_dlsym(dlStateTracker, "gvkGetStateTrackedImageLayouts");
    assert(pfnGvkGetStateTrackedImageLayouts);
    gvk_dlclose(dlStateTracker);
}

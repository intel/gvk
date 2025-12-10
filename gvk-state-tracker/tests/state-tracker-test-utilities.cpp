
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

// TODO : Straighten out layer interfaces...
#define VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

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

VkResult StateTrackerValidationContext::create(StateTrackerValidationContext* pContext, VkBool32 loadApiDumpLayer)
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
    contextCreateInfo.loadApiDumpLayer = loadApiDumpLayer;
    // TODO : Need to run tests with and without validation enabled, and fail if the
    //  run with validation has any validation errors
    contextCreateInfo.loadValidationLayer = VK_FALSE;
    contextCreateInfo.loadWsiExtensions = VK_TRUE;
    contextCreateInfo.pInstanceCreateInfo = &instanceCreateInfo;
    return gvk::Context::create(&contextCreateInfo, nullptr, pContext);
}

const VkPhysicalDevice8BitStorageFeatures& StateTrackerValidationContext::get_physical_device_8_bit_storage_features() const
{
    return mPhysicalDevice8BitStorageFeatures;
}

const VkPhysicalDeviceSynchronization2Features& StateTrackerValidationContext::get_physical_device_synchronization_2_features() const
{
    return mPhysicalDeviceSynchronization2Features;
}

const VkPhysicalDeviceAccelerationStructureFeaturesKHR& StateTrackerValidationContext::get_physical_device_acceleration_structure_features() const
{
    return mPhysicalDeviceAccelerationStructureFeatures;
}

const VkPhysicalDeviceBufferDeviceAddressFeatures& StateTrackerValidationContext::get_physical_device_buffer_device_address_features() const
{
    return mPhysicalDeviceBufferDeviceAddressFeatures;
}

// NOTE : Duplicated in...
//  gvk/gvk-pipeline-explorer/source/gvk-pipeline-explorer/backend/device.cpp
//  gvk/gvk-state-tracker/tests/state-tracker-test-utilities.cpp
//  gvk/gvk-spirv/tests/spirv-validation-context.hpp
// TODO : Move to a common location
template <typename PhysicalDeviceFeatures>
static inline PhysicalDeviceFeatures get_available_physical_device_features(const gvk::PhysicalDevice& gvkPhysicalDevice)
{
    assert(gvkPhysicalDevice);
    auto physicalDeviceFeatures = gvk::get_default<PhysicalDeviceFeatures>();
    auto physicalDeviceFeatures2 = gvk::get_default<VkPhysicalDeviceFeatures2>();
    physicalDeviceFeatures2.pNext = &physicalDeviceFeatures;
    gvkPhysicalDevice.GetPhysicalDeviceFeatures2(&physicalDeviceFeatures2);
    return physicalDeviceFeatures;
}

VkResult StateTrackerValidationContext::create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, std::vector<gvk::Device>* pDevices) const
{
    assert(pDeviceCreateInfo);
    gvk::state_tracker::load_layer_entry_points();

    const auto& gvkPhysicalDevices = get<gvk::PhysicalDevices>();
    assert(!gvkPhysicalDevices.empty());
    const auto& gvkPhysicalDevice = gvkPhysicalDevices[0];

    // TODO : Refactor all this to use PNextChainEditor and ExtensionsCollection
    //  from gvk-pipeline-explorer
    std::vector<const char*> extensions(pDeviceCreateInfo->ppEnabledExtensionNames, pDeviceCreateInfo->ppEnabledExtensionNames + pDeviceCreateInfo->enabledExtensionCount);
    auto enabledPhysicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures2>();

    // TODO : It is nice to have these factory functions return their result so they
    //  can be const, but these const_cast<>() are no good...gotta take the consts
    //  off these Context::create() functions.
    const_cast<VkPhysicalDeviceSynchronization2Features&>(mPhysicalDeviceSynchronization2Features) = get_available_physical_device_features<VkPhysicalDeviceSynchronization2Features>(gvkPhysicalDevice);
    if (mPhysicalDeviceSynchronization2Features.synchronization2) {
        const_cast<VkPhysicalDeviceSynchronization2Features&>(mPhysicalDeviceSynchronization2Features).pNext = enabledPhysicalDeviceFeatures.pNext;
        enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDeviceSynchronization2Features;
    }

    const_cast<VkPhysicalDevice8BitStorageFeatures&>(mPhysicalDevice8BitStorageFeatures) = get_available_physical_device_features<VkPhysicalDevice8BitStorageFeatures>(gvkPhysicalDevice);
    if (mPhysicalDevice8BitStorageFeatures.storageBuffer8BitAccess) {
        const_cast<VkPhysicalDevice8BitStorageFeatures&>(mPhysicalDevice8BitStorageFeatures).pNext = enabledPhysicalDeviceFeatures.pNext;
        enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDevice8BitStorageFeatures;
        enabledPhysicalDeviceFeatures.features.shaderInt64 = VK_TRUE;
    }

    const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR&>(mPhysicalDeviceAccelerationStructureFeatures) = get_available_physical_device_features<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(gvkPhysicalDevice);
    if (mPhysicalDeviceAccelerationStructureFeatures.accelerationStructure) {
        const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR&>(mPhysicalDeviceAccelerationStructureFeatures).pNext = enabledPhysicalDeviceFeatures.pNext;
        enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDeviceAccelerationStructureFeatures;
        extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    }

    const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures&>(mPhysicalDeviceBufferDeviceAddressFeatures) = get_available_physical_device_features<VkPhysicalDeviceBufferDeviceAddressFeatures>(gvkPhysicalDevice);
    if (mPhysicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress) {
        const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures&>(mPhysicalDeviceBufferDeviceAddressFeatures).pNext = enabledPhysicalDeviceFeatures.pNext;
        enabledPhysicalDeviceFeatures.pNext = (void*)&mPhysicalDeviceBufferDeviceAddressFeatures;
        extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    }

    auto deviceCreateInfo = *pDeviceCreateInfo;
    deviceCreateInfo.pNext = &enabledPhysicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)extensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = !extensions.empty() ? extensions.data() : nullptr;
    pDevices->push_back({ });
    return gvk::Device::create(get<gvk::PhysicalDevices>()[0], &deviceCreateInfo, nullptr, &pDevices->back());
}

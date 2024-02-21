
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

#include "gvk-restore-point/creator.hpp"
#include "gvk-environment.hpp"
#include "gvk-format-info.hpp"
#include "gvk-layer.hpp"
#include "gvk-structures.hpp"

#include <set>
#include <utility>
#include <vector>

namespace gvk {
namespace restore_point {

VkResult Creator::create_restore_point(const CreateInfo& createInfo)
{
    mLog.set_instance(createInfo.instance);
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Entered gvk::restore_point::Creator::create_restore_point()" << Log::Flush;
    mCreateInfo = createInfo;
    std::filesystem::create_directories(createInfo.path);
    GvkStateTrackedObject stateTrackedInstance{ };
    stateTrackedInstance.type = VK_OBJECT_TYPE_INSTANCE;
    stateTrackedInstance.handle = (uint64_t)createInfo.instance;
    stateTrackedInstance.dispatchableHandle = (uint64_t)createInfo.instance;
    process_object(&stateTrackedInstance, nullptr, this);
    GvkStateTrackedObjectEnumerateInfo enumerateInfo { };
    enumerateInfo.pfnCallback = process_object;
    enumerateInfo.pUserData = this;
    gvkEnumerateStateTrackedObjects(&stateTrackedInstance, &enumerateInfo);

    mRestorePointObjects.erase(
        std::remove_if(
            mRestorePointObjects.begin(),
            mRestorePointObjects.end(),
            [](const GvkRestorePointObject& restorePointObject)
            {
                return
                    restorePointObject.type == VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT ||
                    restorePointObject.type == VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT;
            }
        ),
        mRestorePointObjects.end()
    );

    auto restorePointManifest = get_default<GvkRestorePointManifest>();
    restorePointManifest.objectCount = (uint32_t)mRestorePointObjects.size();
    restorePointManifest.pObjects = mRestorePointObjects.data();
    if (mResult == VK_SUCCESS) {
        mResult = write_object_restore_info(mCreateInfo, { }, "GvkRestorePointManifest", restorePointManifest);
    }

    create_VkAccelerationStructure_restore_point();

    mInstance.reset();
    for (const auto& gvkDevice : mDevices) {
        gvkDevice.get<DispatchTable>().gvkDeviceWaitIdle(gvkDevice);
    }
    mDevices.clear();
    mDeviceQueueCreateInfos.clear();
    mCopyEngines.clear();
    mLog << VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    mLog << "Leaving gvk::restore_point::Creator::create_restore_point() " << gvk::to_string(mResult, Printer::Default & ~Printer::EnumValue) << Log::Flush;
    return mResult;
}

void Creator::create_VkAccelerationStructure_restore_point()
{
    // TODO : Documentation
    assert(mCopyEngines.size() == 1 && "Multidevice not yet supported");
    auto& copyEngine = mCopyEngines.begin()->second;

    // TODO : Documentation
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkDeviceSize maxAccelerationStructureSerializationSize = 0;
    std::vector<GvkRestorePointObject> capturedAccelerationStructures;
    for (uint32_t i = 0; i < mRestorePointObjects.size(); ++i) {
        const auto& restorePointObject = mRestorePointObjects[i];
        switch (restorePointObject.type) {
        case VK_OBJECT_TYPE_INSTANCE: {
            instance = (VkInstance)restorePointObject.handle;
        } break;
        case VK_OBJECT_TYPE_DEVICE: {
            device = (VkDevice)restorePointObject.handle;
            Auto<GvkDeviceRestoreInfo> deviceRestoreInfo;
            mResult = read_object_restore_info(mCreateInfo.path, "VkDevice", to_hex_string(restorePointObject.handle), deviceRestoreInfo);
            assert(mResult == VK_SUCCESS);
            physicalDevice = get_dependency<VkPhysicalDevice>(deviceRestoreInfo->dependencyCount, deviceRestoreInfo->pDependencies);
            instance = get_dependency<VkInstance>(deviceRestoreInfo->dependencyCount, deviceRestoreInfo->pDependencies);
        } break;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: {
            capturedAccelerationStructures.push_back(restorePointObject);
            VkDeviceSize accelerationStructureSerializationSize = 0;
            mResult = copyEngine.get_acceleration_structure_serialization_size((VkAccelerationStructureKHR)restorePointObject.handle, &accelerationStructureSerializationSize);
            assert(mResult == VK_SUCCESS);
            maxAccelerationStructureSerializationSize = std::max(maxAccelerationStructureSerializationSize, accelerationStructureSerializationSize);
        } break;
        default: {
        } break;
        }
    }

    // HACK : TODO : Documentation
    if (!maxAccelerationStructureSerializationSize) {
        return;
    }

    // HACK : TODO : Documentation
    assert(instance);
    assert(physicalDevice);
    assert(device);
    DispatchTable applicationDispatchTable{ };
    DispatchTable::load_global_entry_points(&applicationDispatchTable);
    DispatchTable::load_instance_entry_points(instance, &applicationDispatchTable);
    DispatchTable::load_device_entry_points(device, &applicationDispatchTable);
    const auto& layerDeviceDispatchTable = mDevices.begin()->get<DispatchTable>();
    (void)layerDeviceDispatchTable;

#if 0
    // TODO : Need to check if VkPhysicalDeviceMemoryProperties
    auto pfnGvkGetPhysicalDeviceMemoryProperties = applicationDispatchTable.gvkGetPhysicalDeviceMemoryProperties;
    applicationDispatchTable = layerDeviceDispatchTable;
    applicationDispatchTable.gvkGetPhysicalDeviceMemoryProperties = pfnGvkGetPhysicalDeviceMemoryProperties;
#endif

    // TODO : Documentation
    auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
    bufferCreateInfo.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    bufferCreateInfo.size = maxAccelerationStructureSerializationSize;
    bufferCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBuffer buffer = VK_NULL_HANDLE;
    mResult = applicationDispatchTable.gvkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
    assert(mResult == VK_SUCCESS);

    // TODO : Documentation
    VkMemoryRequirements memoryRequirements{ };
    applicationDispatchTable.gvkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    // TODO : Documentation
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{ };
    applicationDispatchTable.gvkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

    // TODO : Documentation
    auto memoryAllocateFlagsInfo = get_default<VkMemoryAllocateFlagsInfo>();
    memoryAllocateFlagsInfo.flags =
        VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT |
        VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;

    // TODO : Documentation
    auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    uint32_t memoryTypeCount = 0;
    // TODO : This works on Arc A770, but isn't guaranteed to be avaialable.
    auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
    assert(memoryTypeCount);
    memoryTypeCount = 1;
    get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryAllocateInfo.memoryTypeIndex);
    VkDeviceMemory memory = VK_NULL_HANDLE;
    mResult = applicationDispatchTable.gvkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);
    assert(mResult == VK_SUCCESS);

    // TODO : Documentation
    mResult = applicationDispatchTable.gvkBindBufferMemory(device, buffer, memory, 0);
    assert(mResult == VK_SUCCESS);

    // TODO : Documentation
    auto pApplicationInfo = mInstance.get<VkInstanceCreateInfo>().pApplicationInfo;
    auto apiVersion = pApplicationInfo ? pApplicationInfo->apiVersion : VK_API_VERSION_1_0;

    // TODO : Documentation
    auto bufferOpaqueCaptureAddressCreateInfo = get_default<VkBufferOpaqueCaptureAddressCreateInfo>();
    auto bufferDeviceAddressInfo = get_default<VkBufferDeviceAddressInfo>();
    bufferDeviceAddressInfo.buffer = buffer;
    if (apiVersion < VK_API_VERSION_1_2) {
        bufferOpaqueCaptureAddressCreateInfo.opaqueCaptureAddress = applicationDispatchTable.gvkGetBufferOpaqueCaptureAddressKHR(device, &bufferDeviceAddressInfo);
    } else {
        bufferOpaqueCaptureAddressCreateInfo.opaqueCaptureAddress = applicationDispatchTable.gvkGetBufferOpaqueCaptureAddress(device, &bufferDeviceAddressInfo);
    }

    // TODO : Documentation
    VkDeviceAddress bufferDeviceAddress = 0;
    if (apiVersion < VK_API_VERSION_1_2) {
        bufferDeviceAddress = applicationDispatchTable.gvkGetBufferDeviceAddressKHR(device, &bufferDeviceAddressInfo);
    } else {
        bufferDeviceAddress = applicationDispatchTable.gvkGetBufferDeviceAddress(device, &bufferDeviceAddressInfo);
    }

    // TODO : Doucmentation
    bufferOpaqueCaptureAddressCreateInfo.pNext = bufferCreateInfo.pNext;
    bufferCreateInfo.pNext = &bufferOpaqueCaptureAddressCreateInfo;

    // TODO : Doucmentation
    auto memoryOpaqueCaptureAddressAllocateInfo = get_default<VkMemoryOpaqueCaptureAddressAllocateInfo>();
    auto deviceMemoryOpaqueCaptureAddressInfo = get_default<VkDeviceMemoryOpaqueCaptureAddressInfo>();
    deviceMemoryOpaqueCaptureAddressInfo.memory = memory;
    if (apiVersion < VK_API_VERSION_1_2) {
        memoryOpaqueCaptureAddressAllocateInfo.opaqueCaptureAddress = applicationDispatchTable.gvkGetDeviceMemoryOpaqueCaptureAddressKHR(device, &deviceMemoryOpaqueCaptureAddressInfo);
    } else {
        memoryOpaqueCaptureAddressAllocateInfo.opaqueCaptureAddress = applicationDispatchTable.gvkGetDeviceMemoryOpaqueCaptureAddress(device, &deviceMemoryOpaqueCaptureAddressInfo);
    }

    // TODO : Doucmentation
    memoryOpaqueCaptureAddressAllocateInfo.pNext = memoryAllocateInfo.pNext;
    memoryAllocateInfo.pNext = &memoryOpaqueCaptureAddressAllocateInfo;

    // TODO : Documentation
    auto accelerationStructureSerializationInfo = get_default<GvkAccelerationStructureSerilizationInfoKHR>();
    accelerationStructureSerializationInfo.pMemoryAllocateInfo = &memoryAllocateInfo;
    accelerationStructureSerializationInfo.pBufferCreateInfo = &bufferCreateInfo;
    accelerationStructureSerializationInfo.bufferDeviceAddress = bufferDeviceAddress;

    // TODO : Documentation
    for (const auto& capturedAccelerationStructure : capturedAccelerationStructures) {
        // TODO : Documentation
        mResult = copyEngine.get_acceleration_structure_serialization_size((VkAccelerationStructureKHR)capturedAccelerationStructure.handle, &accelerationStructureSerializationInfo.size);
        assert(mResult == VK_SUCCESS);

        // TODO : Documentation
        Auto<GvkAccelerationStructureRestoreInfoKHR> accelerationStructureRestoreInfo;
        mResult = read_object_restore_info(mCreateInfo.path, "VkAccelerationStructureKHR", to_hex_string(capturedAccelerationStructure.handle), accelerationStructureRestoreInfo);
        assert(mResult == VK_SUCCESS);
        auto modifiedAccelerationStructureRestoreInfo = (GvkAccelerationStructureRestoreInfoKHR)accelerationStructureRestoreInfo;
        modifiedAccelerationStructureRestoreInfo.pSerializationInfo = &accelerationStructureSerializationInfo;
        modifiedAccelerationStructureRestoreInfo.serializedSize = accelerationStructureSerializationInfo.size;
        mResult = write_object_restore_info(mCreateInfo, "VkAccelerationStructureKHR", to_hex_string(modifiedAccelerationStructureRestoreInfo.handle), modifiedAccelerationStructureRestoreInfo);
        assert(mResult == VK_SUCCESS);

        // TODO : Documentation
        if (mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT) {
            auto downloadInfo = get_default<CopyEngine::DownloadAccelerationStructureInfo>();
            downloadInfo.accelerationStructure = accelerationStructureRestoreInfo->handle;
            downloadInfo.accelerationStructureCreateInfo = *accelerationStructureRestoreInfo->pAccelerationStructureCreateInfoKHR;
            downloadInfo.accelerationStructureSerializedSize = accelerationStructureSerializationInfo.size;
            downloadInfo.buffer = buffer;
            downloadInfo.bufferCreateInfo = bufferCreateInfo;
            downloadInfo.bufferDeviceAddress = bufferDeviceAddress;
            downloadInfo.memory = memory;
            downloadInfo.memoryAllocateInfo = memoryAllocateInfo;
            downloadInfo.device = device;
            downloadInfo.pUserData = this;
            downloadInfo.pfnCallback = process_downloaded_VkAccelerationStructureKHR;
            copyEngine.download(downloadInfo);
        }
    }

    // TODO : Documentation
    for (auto& copyEngineEx : mCopyEngines) {
        copyEngineEx.second.wait();
    }

    // TODO : Documentation
    applicationDispatchTable.gvkDestroyBuffer(device, buffer, nullptr);
    applicationDispatchTable.gvkFreeMemory(device, memory, nullptr);
}

const std::vector<GvkRestorePointObject>& Creator::get_restore_point_objects() const
{
    return mRestorePointObjects;
}

VkResult Creator::process_VkDebugReportCallbackEXT(GvkDebugReportCallbackRestoreInfoEXT& restoreInfo)
{
    (void)restoreInfo;
    // NOOP : VkDebugReportCallbackEXT excluded from restore point
    return VK_SUCCESS;
}

VkResult Creator::process_VkDebugUtilsMessengerEXT(GvkDebugUtilsMessengerRestoreInfoEXT& restoreInfo)
{
    (void)restoreInfo;
    // NOOP : VkDebugUtilsMessengerEXT excluded from restore point
    return VK_SUCCESS;
}

} // namespace restore_point
} // namespace gvk

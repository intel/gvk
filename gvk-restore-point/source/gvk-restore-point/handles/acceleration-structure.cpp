
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
#include "gvk-restore-point/layer.hpp"
#include "gvk-layer/registry.hpp"

namespace gvk {
namespace restore_point {

VkResult Layer::pre_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pAccelerationStructure;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfo);
        const_cast<VkAccelerationStructureCreateInfoKHR*>(pCreateInfo)->createFlags |= VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    }
    return gvkResult;
}

VkResult Creator::process_VkAccelerationStructureKHR(GvkAccelerationStructureRestoreInfoKHR& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        assert(restoreInfo.pAccelerationStructureCreateInfoKHR);
        assert(restoreInfo.pAccelerationStructureCreateInfoKHR->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR);
        Device gvkDevice = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto accelerationStructureDeviceAddressInfoKHR = gvk::get_default<VkAccelerationStructureDeviceAddressInfoKHR>();
        accelerationStructureDeviceAddressInfoKHR.accelerationStructure = restoreInfo.handle;
        auto deviceAddress = gvkDevice.get<DispatchTable>().gvkGetAccelerationStructureDeviceAddressKHR(gvkDevice, &accelerationStructureDeviceAddressInfoKHR);
        const_cast<VkAccelerationStructureCreateInfoKHR*>(restoreInfo.pAccelerationStructureCreateInfoKHR)->deviceAddress = deviceAddress;
        gvk_result(BasicCreator::process_VkAccelerationStructureKHR(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Creator::process_VkAccelerationStructureKHR_downloads()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // TODO : Documentation
        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        VkDevice vkDevice = VK_NULL_HANDLE;
        VkDeviceSize maxAccelerationStructureSerializationSize = 0;
        for (uint32_t i = 0; i < mCreateInfo.gvkRestorePoint->manifest->objectCount; ++i) {
            const auto& object = mCreateInfo.gvkRestorePoint->manifest->pObjects[i];
            if (object.type == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR) {
                // TODO : Documentation
                Auto<GvkAccelerationStructureRestoreInfoKHR> restoreInfo;
                gvk_result(read_object_restore_info(mCreateInfo.path, "VkAccelerationStructureKHR", to_hex_string(object.handle), restoreInfo));
                auto vkInstanceDependency = get_dependency<VkInstance>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
                auto vkPhysicalDeviceDependency = get_dependency<VkPhysicalDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
                auto vkDeviceDependency = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
                gvk_result(!vkInstance || vkInstance == vkInstanceDependency ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(!vkPhysicalDevice || vkPhysicalDevice == vkPhysicalDeviceDependency ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(!vkDevice || vkDevice == vkDeviceDependency ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                vkInstance = vkInstanceDependency;
                vkPhysicalDevice = vkPhysicalDeviceDependency;
                vkDevice = vkDeviceDependency;

                // TODO : Documentation
                VkDeviceSize acclerationStructureSerializationSize = 0;
                gvk_result(mCopyEngines[vkDevice].get_acceleration_structure_serialization_size((VkAccelerationStructureKHR)object.handle, &acclerationStructureSerializationSize));
                maxAccelerationStructureSerializationSize = std::max(maxAccelerationStructureSerializationSize, acclerationStructureSerializationSize);
            }
        }

        // TODO : Documentation
        if (maxAccelerationStructureSerializationSize) {
            gvk_result(vkInstance ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(vkPhysicalDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            vkPhysicalDevice = layer::Registry::get().VkPhysicalDevices[vkPhysicalDevice];
            gvk_result(vkPhysicalDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(vkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            const auto& layerPhysicalDeviceDispatchTable = layer::Registry::get().get_physical_device_dispatch_table(vkPhysicalDevice);
            const auto& layerDeviceDispatchTable = layer::Registry::get().get_device_dispatch_table(vkDevice);

            // TODO : Documentation
            auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
            bufferCreateInfo.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
            bufferCreateInfo.size = maxAccelerationStructureSerializationSize;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            VkBuffer vkBuffer = VK_NULL_HANDLE;
            gvk_result(layerDeviceDispatchTable.gvkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &vkBuffer));

            // TODO : Documentation
            VkMemoryRequirements memoryRequirements{ };
            layerDeviceDispatchTable.gvkGetBufferMemoryRequirements(vkDevice, vkBuffer, &memoryRequirements);

            // TODO : Documentation
            VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{ };
            layerPhysicalDeviceDispatchTable.gvkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &physicalDeviceMemoryProperties);

            // TODO : Documentation
            auto memoryAllocateFlagsInfo = get_default<VkMemoryAllocateFlagsInfo>();
            memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
            auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
            memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            uint32_t memoryTypeCount = 0;
            auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
            gvk_result(memoryTypeCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            memoryTypeCount = 1;
            get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryAllocateInfo.memoryTypeIndex);
            VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
            gvk_result(layerDeviceDispatchTable.gvkAllocateMemory(vkDevice, &memoryAllocateInfo, nullptr, &vkDeviceMemory));
            gvk_result(layerDeviceDispatchTable.gvkBindBufferMemory(vkDevice, vkBuffer, vkDeviceMemory, 0));

            // TODO : Documentation
            auto pfnGetBufferDeviceAddress = layerDeviceDispatchTable.gvkGetBufferDeviceAddress;
            auto pfnGetBufferOpaqueCaptureAddress = layerDeviceDispatchTable.gvkGetBufferOpaqueCaptureAddress;
            auto pfnGetDeviceMemoryOpaqueCaptureAddress = layerDeviceDispatchTable.gvkGetDeviceMemoryOpaqueCaptureAddress;
            if (layer::Registry::get().apiVersion < VK_API_VERSION_1_2) {
                pfnGetBufferDeviceAddress = layerDeviceDispatchTable.gvkGetBufferDeviceAddressKHR;
                pfnGetBufferOpaqueCaptureAddress = layerDeviceDispatchTable.gvkGetBufferOpaqueCaptureAddressKHR;
                pfnGetDeviceMemoryOpaqueCaptureAddress = layerDeviceDispatchTable.gvkGetDeviceMemoryOpaqueCaptureAddressKHR;
            }

            // TODO : Documentation
            auto bufferDeviceAddressInfo = get_default<VkBufferDeviceAddressInfo>();
            bufferDeviceAddressInfo.buffer = vkBuffer;
            VkDeviceAddress bufferDeviceAddress = pfnGetBufferDeviceAddress(vkDevice, &bufferDeviceAddressInfo);

            // TODO : Documentation
            auto bufferOpaqueCaptureAddressCreateInfo = get_default<VkBufferOpaqueCaptureAddressCreateInfo>();
            bufferOpaqueCaptureAddressCreateInfo.opaqueCaptureAddress = pfnGetBufferOpaqueCaptureAddress(vkDevice, &bufferDeviceAddressInfo);

            // TODO : Doucmentation
            auto deviceMemoryOpaqueCaptureAddressInfo = get_default<VkDeviceMemoryOpaqueCaptureAddressInfo>();
            deviceMemoryOpaqueCaptureAddressInfo.memory = vkDeviceMemory;
            auto memoryOpaqueCaptureAddressAllocateInfo = get_default<VkMemoryOpaqueCaptureAddressAllocateInfo>();
            memoryOpaqueCaptureAddressAllocateInfo.opaqueCaptureAddress = pfnGetDeviceMemoryOpaqueCaptureAddress(vkDevice, &deviceMemoryOpaqueCaptureAddressInfo);

            // TODO : Doucmentation
            bufferOpaqueCaptureAddressCreateInfo.pNext = bufferCreateInfo.pNext;
            bufferCreateInfo.pNext = &bufferOpaqueCaptureAddressCreateInfo;

            // TODO : Doucmentation
            memoryOpaqueCaptureAddressAllocateInfo.pNext = memoryAllocateInfo.pNext;
            memoryAllocateInfo.pNext = &memoryOpaqueCaptureAddressAllocateInfo;

            // TODO : Documentation
            auto accelerationStructureSerializationInfo = get_default<GvkAccelerationStructureSerilizationInfoKHR>();
            accelerationStructureSerializationInfo.pMemoryAllocateInfo = &memoryAllocateInfo;
            accelerationStructureSerializationInfo.pBufferCreateInfo = &bufferCreateInfo;
            accelerationStructureSerializationInfo.bufferDeviceAddress = bufferDeviceAddress;

            // TODO : Documentation
            for (uint32_t i = 0; i < mCreateInfo.gvkRestorePoint->manifest->objectCount; ++i) {
                const auto& object = mCreateInfo.gvkRestorePoint->manifest->pObjects[i];
                if (object.type == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR) {
                    // TODO : Documentation
                    gvk_result(mCopyEngines[vkDevice].get_acceleration_structure_serialization_size((VkAccelerationStructureKHR)object.handle, &accelerationStructureSerializationInfo.size));

                    // TODO : Documentation
                    Auto<GvkAccelerationStructureRestoreInfoKHR> restoreInfo;
                    gvk_result(read_object_restore_info(mCreateInfo.path, "VkAccelerationStructureKHR", to_hex_string(object.handle), restoreInfo));
                    auto modifiedAccelerationStructureRestoreInfo = (GvkAccelerationStructureRestoreInfoKHR)restoreInfo;
                    modifiedAccelerationStructureRestoreInfo.pSerializationInfo = &accelerationStructureSerializationInfo;
                    modifiedAccelerationStructureRestoreInfo.serializedSize = accelerationStructureSerializationInfo.size;
                    gvk_result(write_object_restore_info(mCreateInfo, "VkAccelerationStructureKHR", to_hex_string(modifiedAccelerationStructureRestoreInfo.handle), modifiedAccelerationStructureRestoreInfo));

                    // TODO : Documentation
                    if (mCreateInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT) {
                        auto downloadInfo = get_default<CopyEngine::DownloadAccelerationStructureInfo>();
                        downloadInfo.accelerationStructure = restoreInfo->handle;
                        downloadInfo.accelerationStructureCreateInfo = *restoreInfo->pAccelerationStructureCreateInfoKHR;
                        downloadInfo.accelerationStructureSerializedSize = accelerationStructureSerializationInfo.size;
                        downloadInfo.buffer = vkBuffer;
                        downloadInfo.bufferCreateInfo = bufferCreateInfo;
                        downloadInfo.bufferDeviceAddress = bufferDeviceAddress;
                        downloadInfo.memory = vkDeviceMemory;
                        downloadInfo.memoryAllocateInfo = memoryAllocateInfo;
                        downloadInfo.device = vkDevice;
                        downloadInfo.pUserData = this;
                        downloadInfo.pfnCallback = process_downloaded_VkAccelerationStructureKHR;
                        mCopyEngines[vkDevice].download(downloadInfo);
                    }
                }
            }

            // TODO : Documentation
            for (auto& copyEngine : mCopyEngines) {
                copyEngine.second.wait();
            }

            // TODO : Documentation
            layerDeviceDispatchTable.gvkDestroyBuffer(vkDevice, vkBuffer, nullptr);
            layerDeviceDispatchTable.gvkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Creator::process_downloaded_VkAccelerationStructureKHR(const CopyEngine::DownloadAccelerationStructureInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData)
{
    assert(downloadInfo.pUserData);
    assert(pData);
    const auto& creator = *(const Creator*)downloadInfo.pUserData;
    if (creator.mCreateInfo.gvkRestorePoint->createFlags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT) {
        if (creator.mCreateInfo.pfnProcessResourceDataCallback) {
            GvkStateTrackedObject restorePointObject{ };
            restorePointObject.type = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
            restorePointObject.handle = (uint64_t)downloadInfo.accelerationStructure;
            restorePointObject.dispatchableHandle = (uint64_t)downloadInfo.device;
            creator.mCreateInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, downloadInfo.accelerationStructureSerializedSize, pData);
        } else {
            auto path = creator.mCreateInfo.path / "VkAccelerationStructureKHR";
            std::filesystem::create_directories(path);
            path /= to_hex_string(downloadInfo.accelerationStructure);
            std::ofstream dataFile(path.replace_extension("data"), std::ios::binary);
            dataFile.write((char*)pData, downloadInfo.accelerationStructureSerializedSize);
        }
    }
}

Applier::AccelerationStructureSerializationResources::~AccelerationStructureSerializationResources()
{
    reset();
}

VkResult Applier::AccelerationStructureSerializationResources::validate(const DispatchTable& dispatchTable, VkDevice vkDevice, const GvkAccelerationStructureSerilizationInfoKHR& serializationInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        assert(vkDevice);
        assert(!mVkDevice == !mVkBuffer);
        assert(!mVkDevice == !mVkDeviceMemory);
        assert(!mVkDevice || mVkDevice == vkDevice);
        if (!mVkDevice) {
            mDispatchTable = dispatchTable;
            mVkDevice = vkDevice;
            assert(serializationInfo.pBufferCreateInfo);
            assert(serializationInfo.pMemoryAllocateInfo);

            // TODO : Documentation
            gvk_result(mDispatchTable.gvkCreateBuffer(mVkDevice, serializationInfo.pBufferCreateInfo, nullptr, &mVkBuffer));
            gvk_result(mDispatchTable.gvkAllocateMemory(mVkDevice, serializationInfo.pMemoryAllocateInfo, nullptr, &mVkDeviceMemory));
            gvk_result(mDispatchTable.gvkBindBufferMemory(mVkDevice, mVkBuffer, mVkDeviceMemory, 0));

            // TODO : Documentation
            auto bufferDeviceAddressInfo = get_default<VkBufferDeviceAddressInfo>();
            bufferDeviceAddressInfo.buffer = mVkBuffer;
            auto pfnGetBufferDeviceAddress = layer::Registry::get().apiVersion < VK_API_VERSION_1_2 ? mDispatchTable.gvkGetBufferDeviceAddressKHR : mDispatchTable.gvkGetBufferDeviceAddress;
            pfnGetBufferDeviceAddress(mVkDevice, &bufferDeviceAddressInfo);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkBuffer Applier::AccelerationStructureSerializationResources::get_buffer() const
{
    return mVkBuffer;
}

VkDeviceMemory Applier::AccelerationStructureSerializationResources::get_device_memory() const
{
    return mVkDeviceMemory;
}

void Applier::AccelerationStructureSerializationResources::reset()
{
    assert(!mVkDevice == !mVkBuffer);
    assert(!mVkDevice == !mVkDeviceMemory);
    if (mVkDevice) {
        auto vkResult = mDispatchTable.gvkDeviceWaitIdle(mVkDevice);
        (void)vkResult;
        assert(vkResult == VK_SUCCESS);
        mDispatchTable.gvkDestroyBuffer(mVkDevice, mVkBuffer, nullptr);
        mDispatchTable.gvkFreeMemory(mVkDevice, mVkDeviceMemory, nullptr);
    }
    mDispatchTable = { };
    mVkDevice = VK_NULL_HANDLE;
    mVkBuffer = VK_NULL_HANDLE;
    mVkDeviceMemory = VK_NULL_HANDLE;
}

VkResult Applier::restore_VkAccelerationStructureKHR_data(const GvkStateTrackedObject& restorePointObject, AccelerationStructureSerializationResources& accelerationStructureSerializationResources)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkAccelerationStructureRestoreInfoKHR> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkAccelerationStructureKHR", to_hex_string(restorePointObject.handle), restoreInfo));
        if (restoreInfo->flags & GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT) {
            auto vkDevice = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, restorePointObject.dispatchableHandle, restorePointObject.dispatchableHandle }).handle;
            gvk_result(restoreInfo->pSerializationInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            accelerationStructureSerializationResources.validate(mApplyInfo.dispatchTable, vkDevice, *restoreInfo->pSerializationInfo);
            CopyEngine::UploadAccelerationStructureInfo uploadInfo{ };
            uploadInfo.path = (mApplyInfo.path / "VkAccelerationStructureKHR" / to_hex_string(restorePointObject.handle)).replace_extension(".data");
            uploadInfo.accelerationStructure = (VkAccelerationStructureKHR)get_restored_object(restorePointObject).handle;
            uploadInfo.accelerationStructureCreateInfo = *restoreInfo->pAccelerationStructureCreateInfoKHR;
            uploadInfo.accelerationStructureSerializationInfo = *restoreInfo->pSerializationInfo;
            uploadInfo.accelerationStructureSerializedSize = restoreInfo->serializedSize;
            uploadInfo.buffer = accelerationStructureSerializationResources.get_buffer();
            uploadInfo.bufferCreateInfo = *restoreInfo->pSerializationInfo->pBufferCreateInfo;
            uploadInfo.bufferDeviceAddress = restoreInfo->pSerializationInfo->bufferDeviceAddress;
            uploadInfo.memory = mAccelerationStructureSerializationMemory;
            uploadInfo.memoryAllocateInfo = *restoreInfo->pSerializationInfo->pMemoryAllocateInfo;
            uploadInfo.device = vkDevice;
            uploadInfo.pUserData = this;
            uploadInfo.pfnCallback = process_VkAccelerationStructureKHR_data_upload;
            mCopyEngines[vkDevice].upload_ex(uploadInfo);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::process_VkAccelerationStructureKHR_data_upload(const CopyEngine::UploadAccelerationStructureInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData)
{
    (void)uploadInfo;
    (void)bindBufferMemoryInfo;
    (void)pData;
    gvk_result_scope_begin(VK_SUCCESS) {
        if (pData) {
            if (std::filesystem::exists(uploadInfo.path)) {
                std::ifstream dataFile(uploadInfo.path, std::ios::binary);
                gvk_result(dataFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                dataFile.read((char*)pData, uploadInfo.accelerationStructureSerializedSize);
            }
        } else {
            assert(uploadInfo.pUserData);
            const auto& applier = *(Applier*)uploadInfo.pUserData;
            if (applier.mApplyInfo.pfnProcessResourceDataCallback) {
                GvkStateTrackedObject restorePointObject{ };
                restorePointObject.type = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
                restorePointObject.handle = (uint64_t)uploadInfo.accelerationStructure;
                restorePointObject.dispatchableHandle = (uint64_t)uploadInfo.device;
                applier.mApplyInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, uploadInfo.accelerationStructureSerializedSize, pData);
            }
        }
    } gvk_result_scope_end;
    assert(gvkResult == VK_SUCCESS);
}

VkResult Applier::process_VkAccelerationStructureKHR_builds(const GvkStateTrackedObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkAccelerationStructureRestoreInfoKHR> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkAccelerationStructureKHR", to_hex_string(restorePointObject.handle), restoreInfo));
        if (restoreInfo->buildGeometryInfo.sType == get_stype<VkAccelerationStructureBuildGeometryInfoKHR>()) {
            auto device = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
            device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
            auto vkAccelerationStructure = (VkAccelerationStructureKHR)get_restored_object(restorePointObject).handle;
            CopyEngine::BuildAcclerationStructureInfo buildInfo{ };
            buildInfo.device = device;
            buildInfo.accelerationStructure = vkAccelerationStructure;
            buildInfo.buildGeometryInfo = restoreInfo->buildGeometryInfo;
            buildInfo.pBuildRangeInfos = restoreInfo->pBuildRangeInfos;
            mCopyEngines[device].build_acceleration_structure(buildInfo);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

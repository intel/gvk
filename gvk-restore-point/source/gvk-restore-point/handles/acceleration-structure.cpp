
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

namespace gvk {
namespace restore_point {

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

void Creator::process_downloaded_VkAccelerationStructureKHR(const CopyEngine::DownloadAccelerationStructureInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData)
{
    assert(downloadInfo.pUserData);
    assert(pData);
    const auto& creator = *(const Creator*)downloadInfo.pUserData;
    if (creator.mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_ACCELERATION_STRUCTURE_DATA_BIT) {
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

void Applier::apply_VkAccelerationStructure_restore_point(const std::vector<GvkRestorePointObject>& capturedAccelerationStructures)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        VkDevice device = VK_NULL_HANDLE;
        for (const auto& capturedAccelerationStructure : capturedAccelerationStructures) {
            Auto<GvkAccelerationStructureRestoreInfoKHR> restoreInfo;
            gvk_result(read_object_restore_info(mApplyInfo.path, "VkAccelerationStructureKHR", to_hex_string(capturedAccelerationStructure.handle), restoreInfo));
            assert(!mAccelerationStructureSerializationBuffer == !mAccelerationStructureSerializationMemory);
            if (!mAccelerationStructureSerializationBuffer) {
                auto pSerializationInfo = restoreInfo->pSerializationInfo;
                assert(pSerializationInfo);
                auto pBufferCreateInfo = pSerializationInfo->pBufferCreateInfo;
                assert(pBufferCreateInfo);
                auto pMemoryAllocateInfo = pSerializationInfo->pMemoryAllocateInfo;
                assert(pMemoryAllocateInfo);
                device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, capturedAccelerationStructure.dispatchableHandle, capturedAccelerationStructure.dispatchableHandle }).handle;
                gvk_result(mApplyInfo.dispatchTable.gvkCreateBuffer(device, pBufferCreateInfo, nullptr, &mAccelerationStructureSerializationBuffer));
                gvk_result(mApplyInfo.dispatchTable.gvkAllocateMemory(device, pMemoryAllocateInfo, nullptr, &mAccelerationStructureSerializationMemory));
                gvk_result(mApplyInfo.dispatchTable.gvkBindBufferMemory(device, mAccelerationStructureSerializationBuffer, mAccelerationStructureSerializationMemory, 0));
            }
            ///////////////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////
            #if 1
            if (restoreInfo->flags & GVK_RESTORE_POINT_OBJECT_STATUS_ACTIVE_BIT) {
                device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
                CopyEngine::UploadAccelerationStructureInfo uploadInfo{ };
                uploadInfo.path = (mApplyInfo.path / "VkAccelerationStructureKHR" / to_hex_string(capturedAccelerationStructure.handle)).replace_extension(".data");
                uploadInfo.accelerationStructure = (VkAccelerationStructureKHR)get_restored_object(capturedAccelerationStructure).handle;
                uploadInfo.accelerationStructureCreateInfo = *restoreInfo->pAccelerationStructureCreateInfoKHR;
                uploadInfo.accelerationStructureSerializationInfo = *restoreInfo->pSerializationInfo;
                uploadInfo.accelerationStructureSerializedSize = restoreInfo->serializedSize;
                uploadInfo.buffer = mAccelerationStructureSerializationBuffer;
                uploadInfo.bufferCreateInfo = *restoreInfo->pSerializationInfo->pBufferCreateInfo;
                uploadInfo.bufferDeviceAddress = restoreInfo->pSerializationInfo->bufferDeviceAddress;
                uploadInfo.memory = mAccelerationStructureSerializationMemory;
                uploadInfo.memoryAllocateInfo = *restoreInfo->pSerializationInfo->pMemoryAllocateInfo;
                uploadInfo.device = device;
                uploadInfo.pUserData = this;
                uploadInfo.pfnCallback = process_VkAccelerationStructureKHR_data_upload;
                mCopyEngines[device].upload_ex(uploadInfo);
            }
            #else
            process_VkAccelerationStructureKHR_data(capturedAccelerationStructure);
            #endif
            ///////////////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////
        }
        assert(!mAccelerationStructureSerializationBuffer == !mAccelerationStructureSerializationMemory);
        if (mAccelerationStructureSerializationBuffer) {
            assert(mAccelerationStructureSerializationMemory);
            mApplyInfo.dispatchTable.gvkDestroyBuffer(device, mAccelerationStructureSerializationBuffer, nullptr);
            mApplyInfo.dispatchTable.gvkFreeMemory(device, mAccelerationStructureSerializationMemory, nullptr);
        }

    } gvk_result_scope_end;
    assert(gvkResult == VK_SUCCESS);
}

VkResult Applier::process_VkAccelerationStructureKHR(const GvkRestorePointObject& restorePointObject, const GvkAccelerationStructureRestoreInfoKHR& restoreInfo)
{
    return BasicApplier::process_VkAccelerationStructureKHR(restorePointObject, restoreInfo);
}

VkResult Applier::process_VkAccelerationStructureKHR_data(const GvkRestorePointObject& restorePointObject)
{
    (void)restorePointObject;
    return VK_SUCCESS;
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

VkResult Applier::process_VkAccelerationStructureKHR_builds(const GvkRestorePointObject& restorePointObject)
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


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

VkResult Creator::process_VkDeviceMemory(GvkDeviceMemoryRestoreInfo& restoreInfo)
{
    // TODO : Filter downloads based on flags

    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Setup GvkStateTrackedObject
        auto device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto stateTrackedObject = get_default<GvkStateTrackedObject>();
        stateTrackedObject.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
        stateTrackedObject.handle = (uint64_t)restoreInfo.handle;
        stateTrackedObject.dispatchableHandle = (uint64_t)device;

        // Get VkDeviceMemory bindings
        class DeviceMemoryBindings final
        {
        public:
            std::vector<VkBindBufferMemoryInfo> bufferBindInfos;
            std::vector<VkBindImageMemoryInfo> imageBindInfos;
        };
        auto enumerateDeviceMemoryBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
        {
            assert(pInfo);
            assert(pUserData);
            switch (pInfo->sType) {
            case get_stype<VkBindBufferMemoryInfo>(): {
                ((DeviceMemoryBindings*)pUserData)->bufferBindInfos.push_back(*(VkBindBufferMemoryInfo*)pInfo);
            } break;
            case get_stype<VkBindImageMemoryInfo>(): {
                ((DeviceMemoryBindings*)pUserData)->imageBindInfos.push_back(*(VkBindImageMemoryInfo*)pInfo);
            } break;
            default: {
                assert(false && "Unserviced VkDeviceMemory binding type; gvk maintenance required");
            } break;
            }
        };
        DeviceMemoryBindings deviceMemoryBindings;
        auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
        enumerateInfo.pfnCallback = enumerateDeviceMemoryBindings;
        enumerateInfo.pUserData = &deviceMemoryBindings;
        gvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
        restoreInfo.bufferBindInfoCount = (uint32_t)deviceMemoryBindings.bufferBindInfos.size();
        restoreInfo.pBufferBindInfos = !deviceMemoryBindings.bufferBindInfos.empty() ? deviceMemoryBindings.bufferBindInfos.data() : nullptr;
        restoreInfo.imageBindInfoCount = (uint32_t)deviceMemoryBindings.imageBindInfos.size();
        restoreInfo.pImageBindInfos = !deviceMemoryBindings.imageBindInfos.empty() ? deviceMemoryBindings.imageBindInfos.data() : nullptr;

        // Sort bindings
        std::set<std::pair<VkDeviceSize, VkDeviceSize>> bindings;
        const auto& dispatchTable = Device(device).get<DispatchTable>();
        for (const auto& bindBufferMemoryInfo : deviceMemoryBindings.bufferBindInfos) {
            VkMemoryRequirements memoryRequirements{ };
            dispatchTable.gvkGetBufferMemoryRequirements(device, bindBufferMemoryInfo.buffer, &memoryRequirements);
            bindings.insert(std::make_pair(bindBufferMemoryInfo.memoryOffset, memoryRequirements.size));
        }
        for (const auto& bindImageMemoryInfo : deviceMemoryBindings.imageBindInfos) {
            VkMemoryRequirements memoryRequirements{ };
            dispatchTable.gvkGetImageMemoryRequirements(device, bindImageMemoryInfo.image, &memoryRequirements);
            bindings.insert(std::make_pair(bindImageMemoryInfo.memoryOffset, memoryRequirements.size));
        }

        // HACK :
        if (restoreInfo.flags & GVK_RESTORE_POINT_OBJECT_STATUS_ACTIVE_BIT) {
            // Get mapped memory
            auto mappedMemoryInfo = get_default<GvkMappedMemoryInfo>();
            gvkGetStateTrackedMappedMemory(&stateTrackedObject, &mappedMemoryInfo.offset, &mappedMemoryInfo.size, &mappedMemoryInfo.flags, (void**)&mappedMemoryInfo.dataHandle);
            restoreInfo.mappedMemoryInfo = mappedMemoryInfo;
        }

        const auto& memoryAllocateInfo = restoreInfo.pMemoryAllocateInfo ? *restoreInfo.pMemoryAllocateInfo : VkMemoryAllocateInfo{ };

        if (restoreInfo.flags & GVK_RESTORE_POINT_OBJECT_STATUS_ACTIVE_BIT) {
            // Submit for download
            if (mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT) {
                auto downloadInfo = get_default<CopyEngine::DownloadDeviceMemoryInfo>();
                downloadInfo.device = device;
                downloadInfo.memory = restoreInfo.handle;
                downloadInfo.memoryAllocateInfo = memoryAllocateInfo;
                downloadInfo.regionCount = 0;
                downloadInfo.pRegions = nullptr;
                downloadInfo.pUserData = this;
                downloadInfo.pfnCallback = process_downloaded_VkDeviceMemory;
                mCopyEngines[device].download(downloadInfo);
            }
        }

        VkMemoryAllocateFlags memoryAllocateFlags = 0;
        VkMemoryOpaqueCaptureAddressAllocateInfo* pMemoryOpaqueCaptureAddressAllocateInfo = nullptr;
        (void)pMemoryOpaqueCaptureAddressAllocateInfo;
        auto pNext = (VkBaseOutStructure*)memoryAllocateInfo.pNext;
        while (pNext) {
            switch (pNext->sType) {
            case get_stype<VkMemoryAllocateFlagsInfo>(): {
                auto pMemoryAllocateFlagsInfo = (VkMemoryAllocateFlagsInfo*)pNext;
                if (pMemoryAllocateFlagsInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) {
                    assert(pMemoryAllocateFlagsInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT);
                    memoryAllocateFlags |= pMemoryAllocateFlagsInfo->flags;
                }
            } break;
            case get_stype<VkMemoryOpaqueCaptureAddressAllocateInfo>(): {
                pMemoryOpaqueCaptureAddressAllocateInfo = (VkMemoryOpaqueCaptureAddressAllocateInfo*)pNext;
            } break;
            default: {
            } break;
            }
            pNext = pNext->pNext;
        }

        if (memoryAllocateFlags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) {
            assert(pMemoryOpaqueCaptureAddressAllocateInfo);
            assert(pMemoryOpaqueCaptureAddressAllocateInfo->opaqueCaptureAddress);
        }

        gvk_result(BasicCreator::process_VkDeviceMemory(restoreInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

void Creator::process_downloaded_VkDeviceMemory(const CopyEngine::DownloadDeviceMemoryInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData)
{
    assert(downloadInfo.pUserData);
    assert(pData);
    const auto& creator = *(const Creator*)downloadInfo.pUserData;
    if (creator.mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_DEVICE_MEMORY_DATA_BIT) {
        if (creator.mCreateInfo.pfnProcessResourceDataCallback) {
            GvkStateTrackedObject restorePointObject{ };
            restorePointObject.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
            restorePointObject.handle = (uint64_t)downloadInfo.memory;
            restorePointObject.dispatchableHandle = (uint64_t)downloadInfo.device;
            creator.mCreateInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, downloadInfo.memoryAllocateInfo.allocationSize, pData);
        } else {
            auto path = creator.mCreateInfo.path / "VkDeviceMemory";
            std::filesystem::create_directories(path);
            path /= to_hex_string(downloadInfo.memory);
            std::ofstream dataFile(path.replace_extension("data"), std::ios::binary);
            dataFile.write((char*)pData, downloadInfo.memoryAllocateInfo.allocationSize);
        }
    }
}

VkResult Applier::process_VkDeviceMemory(const GvkRestorePointObject& restorePointObject, const GvkDeviceMemoryRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        remove_pnext_entries(
            (VkBaseOutStructure*)restoreInfo.pMemoryAllocateInfo,
            {
                VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
                VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
                VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
                VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
            }
        );

        gvk_result(BasicApplier::process_VkDeviceMemory(restorePointObject, restoreInfo));
        auto device = (VkDevice)get_restored_object(get_restore_point_object_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies)).handle;
        auto restoredObject = get_restored_object(restorePointObject);
        auto deviceMemory = (VkDeviceMemory)restoredObject.handle;
        for (uint32_t i = 0; i < restoreInfo.bufferBindInfoCount; ++i) {
            const auto& bufferBindInfo = restoreInfo.pBufferBindInfos[i];
            auto bufferRestorePointObject = restorePointObject;
            bufferRestorePointObject.type = VK_OBJECT_TYPE_BUFFER;
            bufferRestorePointObject.handle = (uint64_t)bufferBindInfo.buffer;
            gvk_result(process_object(bufferRestorePointObject));
            auto buffer = (VkBuffer)get_restored_object(bufferRestorePointObject).handle;
            VkMemoryRequirements memoryRequirements{ };
            mApplyInfo.dispatchTable.gvkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
            gvk_result(mApplyInfo.dispatchTable.gvkBindBufferMemory(device, buffer, deviceMemory, bufferBindInfo.memoryOffset));

            // TODO : Is this the best place for this logic?
            Auto<GvkBufferRestoreInfo> bufferRestoreInfo;
            gvk_result(read_object_restore_info<GvkBufferRestoreInfo>(mApplyInfo.path, "VkBuffer", to_hex_string(bufferBindInfo.buffer), bufferRestoreInfo));
            if (bufferRestoreInfo->pBufferCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                auto bufferDeviceAddressInfo = get_default<VkBufferDeviceAddressInfo>();
                bufferDeviceAddressInfo.buffer = buffer;
                mApplyInfo.dispatchTable.gvkGetBufferDeviceAddress(device, &bufferDeviceAddressInfo);
            }
        }
        for (uint32_t i = 0; i < restoreInfo.imageBindInfoCount; ++i) {
            const auto& imageBindInfo = restoreInfo.pImageBindInfos[i];
            auto imageRestorePointObject = restorePointObject;
            imageRestorePointObject.type = VK_OBJECT_TYPE_IMAGE;
            imageRestorePointObject.handle = (uint64_t)imageBindInfo.image;
            gvk_result(process_object(imageRestorePointObject));
            auto image = (VkImage)get_restored_object(imageRestorePointObject).handle;
            VkMemoryRequirements memoryRequirements{ };
            mApplyInfo.dispatchTable.gvkGetImageMemoryRequirements(device, image, &memoryRequirements);
            gvk_result(mApplyInfo.dispatchTable.gvkBindImageMemory(device, image, deviceMemory, imageBindInfo.memoryOffset));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkDeviceMemory_data(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkDeviceMemoryRestoreInfo> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkDeviceMemory", to_hex_string(restorePointObject.handle), restoreInfo));
        auto device = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
        device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
        CopyEngine::UploadDeviceMemoryInfo uploadInfo{ };
        uploadInfo.path = (mApplyInfo.path / "VkDeviceMemory" / to_hex_string(restorePointObject.handle)).replace_extension(".data");
        uploadInfo.device = device;
        uploadInfo.memory = (VkDeviceMemory)get_restored_object(restorePointObject).handle;
        uploadInfo.memoryAllocateInfo = *restoreInfo->pMemoryAllocateInfo;
        // TODO : Handle regions...
        uploadInfo.pUserData = this;
        uploadInfo.pfnCallback = process_VkDeviceMemory_data_upload;
        mCopyEngines[device].upload(uploadInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::process_VkDeviceMemory_data_upload(const CopyEngine::UploadDeviceMemoryInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (pData) {
            if (std::filesystem::exists(uploadInfo.path)) {
                std::ifstream dataFile(uploadInfo.path, std::ios::binary);
                gvk_result(dataFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                // TODO : Handle regions...
                assert(uploadInfo.regionCount == 1);
                assert(uploadInfo.pRegions);
                dataFile.read((char*)pData, uploadInfo.pRegions[0].size);
            }
        } else {
            assert(uploadInfo.pUserData);
            const auto& applier = *(Applier*)uploadInfo.pUserData;
            if (applier.mApplyInfo.pfnProcessResourceDataCallback) {
                GvkStateTrackedObject restorePointObject{ };
                restorePointObject.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
                restorePointObject.handle = (uint64_t)uploadInfo.memory;
                restorePointObject.dispatchableHandle = (uint64_t)uploadInfo.device;
                // TODO : Handle regions...
                assert(uploadInfo.regionCount == 1);
                assert(uploadInfo.pRegions);
                applier.mApplyInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, uploadInfo.pRegions[0].size, pData);
            }
        }
    } gvk_result_scope_end;
    assert(gvkResult == VK_SUCCESS);
}

VkResult Applier::process_VkDeviceMemory_mapping(const GvkRestorePointObject& capturedDeviceMemory)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkDeviceMemoryRestoreInfo> deviceMemoryRestoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkDeviceMemory", to_hex_string(capturedDeviceMemory.handle), deviceMemoryRestoreInfo));
        if (deviceMemoryRestoreInfo->mappedMemoryInfo.size) {
            auto restoredDeviceMemory = get_restored_object(capturedDeviceMemory);
            auto device = (VkDevice)restoredDeviceMemory.dispatchableHandle;
            auto memory = (VkDeviceMemory)restoredDeviceMemory.handle;
            auto offset = deviceMemoryRestoreInfo->mappedMemoryInfo.offset;
            auto size = deviceMemoryRestoreInfo->mappedMemoryInfo.size;
            auto flags = deviceMemoryRestoreInfo->mappedMemoryInfo.flags;
            void* pData = nullptr;
            gvk_result(mApplyInfo.dispatchTable.gvkMapMemory(device, memory, offset, size, flags, (void**)&pData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

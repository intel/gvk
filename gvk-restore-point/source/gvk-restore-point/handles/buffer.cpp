
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

VkResult Creator::process_VkBuffer(GvkBufferRestoreInfo& restoreInfo)
{
    // TODO : Filter downloads based on flags
    assert(restoreInfo.pBufferCreateInfo);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Setup GvkStateTrackedObject
        auto device = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
        auto stateTrackedObject = get_default<GvkStateTrackedObject>();
        stateTrackedObject.type = VK_OBJECT_TYPE_BUFFER;
        stateTrackedObject.handle = (uint64_t)restoreInfo.handle;
        stateTrackedObject.dispatchableHandle = (uint64_t)device;

        // Get VkDeviceMemory bindings
        auto enumerateDeviceMemoryBindings = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
        {
            assert(pInfo);
            assert(pInfo->sType == get_stype<VkBindBufferMemoryInfo>());
            assert(pUserData);
            ((std::vector<VkBindBufferMemoryInfo>*)pUserData)->push_back(*(VkBindBufferMemoryInfo*)pInfo);
        };
        std::vector<VkBindBufferMemoryInfo> bindBufferMemoryInfos;
        auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
        enumerateInfo.pfnCallback = enumerateDeviceMemoryBindings;
        enumerateInfo.pUserData = &bindBufferMemoryInfos;
        gvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);
        restoreInfo.memoryBindInfoCount = (uint32_t)bindBufferMemoryInfos.size();
        restoreInfo.pMemoryBindInfos = !bindBufferMemoryInfos.empty() ? bindBufferMemoryInfos.data() : nullptr;

        if (restoreInfo.flags & GVK_RESTORE_POINT_OBJECT_STATUS_ACTIVE_BIT) {
            // Get VkMemoryRequirements
            // TODO : Cache memory requirements in resource control block
            restoreInfo.memoryRequirements = get_default<VkMemoryRequirements2>();
            Device(device).get<DispatchTable>().gvkGetBufferMemoryRequirements(device, restoreInfo.handle, &restoreInfo.memoryRequirements.memoryRequirements);

            // Submit for download
            if (mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT) {
                const auto& bufferCreateInfo = *restoreInfo.pBufferCreateInfo;
                auto downloadInfo = get_default<CopyEngine::DownloadBufferInfo>();
                downloadInfo.device = device;
                downloadInfo.buffer = restoreInfo.handle;
                downloadInfo.bufferCreateInfo = bufferCreateInfo;
                downloadInfo.size = bufferCreateInfo.size;
                downloadInfo.pUserData = this;
                downloadInfo.pfnCallback = process_downloaded_VkBuffer;
                mCopyEngines[device].download(downloadInfo);
            }
        } else {
            // TODO : This logic seems a bit kludgy...revisit transient dependency logic
            for (uint32_t i = 0; i < restoreInfo.memoryBindInfoCount; ++i) {
                GvkStateTrackedObject stateTrackedDeviceMemory{ };
                stateTrackedDeviceMemory.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
                stateTrackedDeviceMemory.handle = (uint64_t)restoreInfo.pMemoryBindInfos[i].memory;
                stateTrackedDeviceMemory.dispatchableHandle = (uint64_t)device;
                process_object(&stateTrackedDeviceMemory, nullptr, this);
            }
        }

        if (restoreInfo.pBufferCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            auto pBufferOpaqueCaptureAddressCreateInfo = const_cast<VkBufferOpaqueCaptureAddressCreateInfo*>(get_pnext<VkBufferOpaqueCaptureAddressCreateInfo>(*restoreInfo.pBufferCreateInfo));
            (void)pBufferOpaqueCaptureAddressCreateInfo;
            // assert(pBufferOpaqueCaptureAddressCreateInfo);
            //
            // TODO : IDGPU should have address, RTX does not
            //
            // TODO : Check device type to determine whether or not to assert here?
            // assert(pBufferOpaqueCaptureAddressCreateInfo->opaqueCaptureAddress);
#if 0
            if (!pBufferOpaqueCaptureAddressCreateInfo) {
                bufferOpaqueCaptureAddressCreateInfo.pNext = restoreInfo.pBufferCreateInfo->pNext;
                pBufferOpaqueCaptureAddressCreateInfo = &bufferOpaqueCaptureAddressCreateInfo;
                const_cast<VkBufferCreateInfo*>(restoreInfo.pBufferCreateInfo)->pNext = pBufferOpaqueCaptureAddressCreateInfo;
            }
            Device gvkDevice = device;
            assert(gvkDevice);
            auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfo>();
            bufferDeviceAddressInfo.buffer = restoreInfo.handle;
            // TODO : Double check if this check is actually necessary...
            auto pApplicationInfo = gvkDevice.get<Instance>().get<VkInstanceCreateInfo>().pApplicationInfo;
            auto apiVersion = pApplicationInfo ? pApplicationInfo->apiVersion : VK_API_VERSION_1_0;
            if (apiVersion < VK_API_VERSION_1_2) {
                pBufferOpaqueCaptureAddressCreateInfo->opaqueCaptureAddress = gvkDevice.get<DispatchTable>().gvkGetBufferOpaqueCaptureAddressKHR(device, &bufferDeviceAddressInfo);
            } else {
                pBufferOpaqueCaptureAddressCreateInfo->opaqueCaptureAddress = gvkDevice.get<DispatchTable>().gvkGetBufferOpaqueCaptureAddress(device, &bufferDeviceAddressInfo);
            }
#endif
        }

        gvk_result(BasicCreator::process_VkBuffer(restoreInfo));

#if 0
        if (restoreInfo.pBufferCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            auto pBufferOpaqueCaptureAddressCreateInfo = const_cast<VkBufferOpaqueCaptureAddressCreateInfo*>(get_pnext<VkBufferOpaqueCaptureAddressCreateInfo>(*restoreInfo.pBufferCreateInfo));
            if (pBufferOpaqueCaptureAddressCreateInfo == &bufferOpaqueCaptureAddressCreateInfo) {
                const_cast<VkBufferCreateInfo*>(restoreInfo.pBufferCreateInfo)->pNext = bufferOpaqueCaptureAddressCreateInfo.pNext;
            }
        }
#endif
    } gvk_result_scope_end;
    return gvkResult;
}

void Creator::process_downloaded_VkBuffer(const CopyEngine::DownloadBufferInfo& downloadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, const uint8_t* pData)
{
    assert(downloadInfo.pUserData);
    assert(pData);
    const auto& creator = *(const Creator*)downloadInfo.pUserData;
    if (creator.mCreateInfo.flags & GVK_RESTORE_POINT_CREATE_BUFFER_DATA_BIT) {
        if (creator.mCreateInfo.pfnProcessResourceDataCallback) {
            GvkStateTrackedObject restorePointObject{ };
            restorePointObject.type = VK_OBJECT_TYPE_BUFFER;
            restorePointObject.handle = (uint64_t)downloadInfo.buffer;
            restorePointObject.dispatchableHandle = (uint64_t)downloadInfo.device;
            creator.mCreateInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, downloadInfo.size, pData);
        } else {
            auto path = creator.mCreateInfo.path / "VkBuffer";
            std::filesystem::create_directories(path);
            path /= to_hex_string(downloadInfo.buffer);
            std::ofstream dataFile(path.replace_extension("data"), std::ios::binary);
            dataFile.write((char*)pData, downloadInfo.size);
        }
    }
}

VkResult Applier::process_VkBuffer(const GvkRestorePointObject& restorePointObject, const GvkBufferRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        const_cast<VkBufferCreateInfo*>(restoreInfo.pBufferCreateInfo)->usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        gvk_result(BasicApplier::process_VkBuffer(restorePointObject, restoreInfo));
        for (uint32_t i = 0; i < restoreInfo.memoryBindInfoCount; ++i) {
            auto deviceMemoryRestorePointObject = restorePointObject;
            deviceMemoryRestorePointObject.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
            deviceMemoryRestorePointObject.handle = (uint64_t)restoreInfo.pMemoryBindInfos[i].memory;
            gvk_result(process_object(deviceMemoryRestorePointObject));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Applier::process_VkBuffer_data(const GvkRestorePointObject& restorePointObject)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        Auto<GvkBufferRestoreInfo> restoreInfo;
        gvk_result(read_object_restore_info(mApplyInfo.path, "VkBuffer", to_hex_string(restorePointObject.handle), restoreInfo));
        if (restoreInfo->flags & GVK_RESTORE_POINT_OBJECT_STATUS_ACTIVE_BIT) {
            auto device = get_dependency<VkDevice>(restoreInfo->dependencyCount, restoreInfo->pDependencies);
            device = (VkDevice)get_restored_object({ VK_OBJECT_TYPE_DEVICE, (uint64_t)device, (uint64_t)device }).handle;
            CopyEngine::UploadBufferInfo uploadBufferInfo{ };
            uploadBufferInfo.path = (mApplyInfo.path / "VkBuffer" / to_hex_string(restorePointObject.handle)).replace_extension(".data");
            uploadBufferInfo.device = device;
            uploadBufferInfo.buffer = (VkBuffer)get_restored_object(restorePointObject).handle;
            uploadBufferInfo.bufferCreateInfo = *restoreInfo->pBufferCreateInfo;
            uploadBufferInfo.size = restoreInfo->pBufferCreateInfo->size;
            uploadBufferInfo.pUserData = this;
            uploadBufferInfo.pfnCallback = process_VkBuffer_data_upload;
            mCopyEngines[device].upload(uploadBufferInfo);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Applier::process_VkBuffer_data_upload(const CopyEngine::UploadBufferInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (pData) {
            if (std::filesystem::exists(uploadInfo.path)) {
                std::ifstream dataFile(uploadInfo.path, std::ios::binary);
                gvk_result(dataFile.is_open() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                dataFile.read((char*)pData, uploadInfo.size);
            }
        } else {
            assert(uploadInfo.pUserData);
            const auto& applier = *(Applier*)uploadInfo.pUserData;
            if (applier.mApplyInfo.pfnProcessResourceDataCallback) {
                GvkStateTrackedObject restorePointObject{ };
                restorePointObject.type = VK_OBJECT_TYPE_BUFFER;
                restorePointObject.handle = (uint64_t)uploadInfo.buffer;
                restorePointObject.dispatchableHandle = (uint64_t)uploadInfo.device;
                applier.mApplyInfo.pfnProcessResourceDataCallback(&restorePointObject, bindBufferMemoryInfo.memory, uploadInfo.size, pData);
            }
        }
    } gvk_result_scope_end;
    assert(gvkResult == VK_SUCCESS);
}

} // namespace restore_point
} // namespace gvk

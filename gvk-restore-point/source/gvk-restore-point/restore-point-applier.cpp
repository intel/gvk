
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

#include "gvk-restore-point.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-environment.hpp"
#include "gvk-format-info.hpp"
#include "gvk-runtime.hpp"
#include "gvk-structures.hpp"
#include "gvk-structures/generated/core-structure-enumerate-handles.hpp"
#include "gvk-restore-point/detail/restore-point-applier-base.hpp"

#include "gvk-restore-point/generated/basic-restore-point-applier.hpp"
#include "gvk-restore-point/generated/restore-info.h"
#include "gvk-restore-point/generated/restore-info-enumerations-to-string.hpp"
#include "gvk-restore-point/generated/restore-info-structure-comparison-operators.hpp"
#include "gvk-restore-point/generated/restore-info-structure-create-copy.hpp"
#include "gvk-restore-point/generated/restore-info-structure-deserialization.hpp"
#include "gvk-restore-point/generated/restore-info-structure-destroy-copy.hpp"
#include "gvk-restore-point/generated/restore-info-structure-get-stype.hpp"
#include "gvk-restore-point/generated/restore-info-structure-serialization.hpp"
#include "gvk-restore-point/generated/restore-info-structure-to-string.hpp"

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

#include <iostream>

namespace gvk {

template <typename PNextStructureType>
inline const PNextStructureType* get_pnext_structure(const void* pNext)
{
    auto pBaseInStructure = (const VkBaseInStructure*)pNext;
    while (pBaseInStructure) {
        if (pBaseInStructure->sType == get_stype<PNextStructureType>()) {
            return (const PNextStructureType*)pBaseInStructure;
        }
        pNext = pBaseInStructure->pNext;
    }
    return nullptr;
}

static const void* remove_pnext_entries(VkBaseOutStructure* pNext, const std::set<VkStructureType>& structureTypes)
{
    while (pNext) {
        while (pNext->pNext && structureTypes.count(pNext->pNext->sType)) {
            auto pRemove = pNext->pNext;
            pNext->pNext = pRemove->pNext;
            pRemove->pNext = nullptr;
            detail::destroy_pnext_copy(pRemove, nullptr);
        }
        pNext = pNext->pNext;
    }
    return pNext;
}

static const void* remove_pnext_entries(VkBaseOutStructure* pNext, VkStructureType structureType)
{
    return remove_pnext_entries(pNext, std::set<VkStructureType> { structureType });
}

template <typename HandleType>
std::string restored_handle_to_string(HandleType handle)
{
    std::stringstream strStrm;
    strStrm << "{ " << (uint64_t)handle << " : 0x" << std::hex << (uint64_t)handle << " }";
    return strStrm.str();
}

detail::RestorePointApplierBase::~RestorePointApplierBase()
{
}

detail::RestorePointApplierBase::CapturedHandle detail::RestorePointApplierBase::get_captured_handle(RestoredHandle restoredHandle) const
{
    if (restoredHandle) {
        auto capturedHandleItr = mCapturedHandles.find(restoredHandle);
        assert(capturedHandleItr != mCapturedHandles.end());
        return capturedHandleItr->second;
    }
    return 0;
}

detail::RestorePointApplierBase::RestoredHandle detail::RestorePointApplierBase::get_restored_handle(CapturedHandle capturedHandle) const
{
    if (capturedHandle) {
        auto restoredHandleItr = mRestoredHandles.find(capturedHandle);
        assert(restoredHandleItr != mRestoredHandles.end());
        return restoredHandleItr->second;
    }
    return 0;
}

GvkStateTrackedObject detail::RestorePointApplierBase::get_object(VkObjectType objectType, uint32_t objectCount, const GvkStateTrackedObject* pObjects)
{
    assert((objectCount == 0) == (pObjects == nullptr));
    for (uint32_t i = 0; i < objectCount; ++i) {
        if (pObjects[i].type == objectType) {
            return pObjects[i];
        }
    }
    return { };
}

const std::vector<GvkStateTrackedObject>& detail::RestorePointApplierBase::get_current_objects() const
{
    return mCurrentObjects;
}

std::pair<CommandBuffer, Fence> detail::RestorePointApplierBase::get_thread_command_buffer(const Device& gvkDevice)
{
    assert(gvkDevice);
    auto gvkQueue = get_queue_family(gvkDevice, 0).queues[0];
    assert(gvkQueue);
    std::unique_lock<std::mutex> lock(mThreadCommandBuffersMutex);
    auto& threadCommandBuffer = mThreadCommandBuffers[gvkDevice][std::this_thread::get_id()];
    lock.unlock();
    assert(!threadCommandBuffer.first == !threadCommandBuffer.second);
    if (!threadCommandBuffer.first) {
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        CommandPool commandPool;
        auto vkResult = CommandPool::create(gvkDevice, &commandPoolCreateInfo, nullptr, &commandPool);
        assert(vkResult == VK_SUCCESS);
        (void)vkResult;

        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        vkResult = CommandBuffer::allocate(gvkDevice, &commandBufferAllocateInfo, &threadCommandBuffer.first);
        assert(vkResult == VK_SUCCESS);
        vkResult = Fence::create(gvkDevice, &get_default<VkFenceCreateInfo>(), nullptr, &threadCommandBuffer.second);
        assert(vkResult == VK_SUCCESS);
    }
    return threadCommandBuffer;
}

void detail::RestorePointApplierBase::register_restored_handle(VkObjectType objectType, CapturedHandle capturedHandle, RestoredHandle restoredHandle)
{
    std::cout << "    register_restored_handle(" << to_string(objectType, Printer::EnumIdentifier) << ", " << restored_handle_to_string(capturedHandle) << ", " << restored_handle_to_string(restoredHandle) << ")" << std::endl;
    auto inserted = mRestoredHandles.insert({ capturedHandle, restoredHandle }).second;
    (void)inserted;
    assert(inserted);
    inserted = mCapturedHandles.insert({ restoredHandle, capturedHandle }).second;
    (void)inserted;
    assert(inserted);
    if (mApplyInfo.processObjectCallback) {
        mApplyInfo.processObjectCallback(objectType, capturedHandle, restoredHandle);
    }
}

VkResult detail::RestorePointApplierBase::restore_device_memory_bindings(const GvkStateTrackedObject& stateTrackedObject)
{
    auto vkResult = VK_SUCCESS;
    switch (stateTrackedObject.type) {
    case VK_OBJECT_TYPE_BUFFER: {
        auto restoreInfo = get_restore_info<GvkBufferRestoreInfo>("VkBuffer", stateTrackedObject.handle);
        assert(!restoreInfo->memoryBindInfoCount == !restoreInfo->pMemoryBindInfos);
        for (uint32_t i = 0; i < restoreInfo->memoryBindInfoCount; ++i) {
            const auto& memoryBindInfo = restoreInfo->pMemoryBindInfos[i];
            auto vkDevice = (VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle);
            auto vkBuffer = (VkBuffer)get_restored_handle(stateTrackedObject.handle);
            auto vkDeviceMemory = (VkDeviceMemory)get_restored_handle((uint64_t)memoryBindInfo.memory);
            VkMemoryRequirements memoryRequirements { };
            mDispatchTable.gvkGetBufferMemoryRequirements(vkDevice, vkBuffer, &memoryRequirements);
            vkResult = mDispatchTable.gvkBindBufferMemory(vkDevice, vkBuffer, vkDeviceMemory, memoryBindInfo.memoryOffset);
            assert(vkResult == VK_SUCCESS);
        }
    } break;
    case VK_OBJECT_TYPE_IMAGE: {
        auto restoreInfo = get_restore_info<GvkImageRestoreInfo>("VkImage", stateTrackedObject.handle);
        assert(!restoreInfo->memoryBindInfoCount == !restoreInfo->pMemoryBindInfos);
        for (uint32_t i = 0; i < restoreInfo->memoryBindInfoCount; ++i) {
            const auto& memoryBindInfo = restoreInfo->pMemoryBindInfos[i];
            auto vkDevice = (VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle);
            auto vkImage = (VkImage)get_restored_handle(stateTrackedObject.handle);
            auto vkDeviceMemory = (VkDeviceMemory)get_restored_handle((uint64_t)memoryBindInfo.memory);
            VkMemoryRequirements memoryRequirements { };
            mDispatchTable.gvkGetImageMemoryRequirements(vkDevice, vkImage, &memoryRequirements);
            vkResult = mDispatchTable.gvkBindImageMemory(vkDevice, vkImage, vkDeviceMemory, memoryBindInfo.memoryOffset);
            assert(vkResult == VK_SUCCESS);
        }
    } break;
    default: {
        assert(false && "TODO : Documentation");
    } break;
    }
    return vkResult;
}

VkResult detail::RestorePointApplierBase::restore_image_layouts(GvkStateTrackedObject stateTrackedObject)
{
    if (mApplyInfo.processImageLayoutsCallback) {
        auto restoreInfo = get_restore_info<GvkImageRestoreInfo>("VkImage", stateTrackedObject.handle);
        auto vkDevice = (VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle);
        auto vkImage = (VkImage)get_restored_handle(stateTrackedObject.handle);
        mApplyInfo.processImageLayoutsCallback(vkDevice, vkImage, *restoreInfo->pImageCreateInfo, restoreInfo->pImageLayouts);
    } else {
        asio::post(
            mThreadPool,
            [this, stateTrackedObject]()
            {
                auto restoreInfo = get_restore_info<GvkImageRestoreInfo>("VkImage", stateTrackedObject.handle);
                auto arrayLayerCount = restoreInfo->pImageCreateInfo->arrayLayers;
                auto mipLevelCount = restoreInfo->pImageCreateInfo->mipLevels;
                std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
                imageMemoryBarriers.reserve(arrayLayerCount * mipLevelCount);
                for (uint32_t arrayLayer_i = 0; arrayLayer_i < arrayLayerCount; ++arrayLayer_i) {
                    for (uint32_t mipLevel_i = 0; mipLevel_i < mipLevelCount; ++mipLevel_i) {
                        auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                        imageMemoryBarrier.oldLayout = restoreInfo->pImageCreateInfo->initialLayout;
                        imageMemoryBarrier.newLayout = restoreInfo->pImageLayouts[arrayLayer_i * mipLevelCount + mipLevel_i];
                        if (imageMemoryBarrier.oldLayout != imageMemoryBarrier.newLayout) {
                            imageMemoryBarrier.image = (VkImage)get_restored_handle(stateTrackedObject.handle);
                            imageMemoryBarrier.subresourceRange.aspectMask = get_image_aspect_flags(restoreInfo->pImageCreateInfo->format);
                            imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel_i;
                            imageMemoryBarrier.subresourceRange.levelCount = 1;
                            imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer_i;
                            imageMemoryBarrier.subresourceRange.layerCount = 1;
                            imageMemoryBarriers.push_back(imageMemoryBarrier);
                        }
                    }
                }
                if (!imageMemoryBarriers.empty()) {
                    Device gvkDevice = (VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle);
                    assert(gvkDevice);
                    auto [gvkCommandBuffer, gvkFence] = get_thread_command_buffer(gvkDevice);
                    auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
                    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    auto dispatchTable = gvkDevice.get<DispatchTable>();
                    assert(dispatchTable.gvkBeginCommandBuffer);
                    auto vkResult = dispatchTable.gvkBeginCommandBuffer(gvkCommandBuffer, &commandBufferBeginInfo);
                    assert(vkResult == VK_SUCCESS);
                    (void)vkResult;

                    dispatchTable.gvkCmdPipelineBarrier(
                        gvkCommandBuffer,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        (uint32_t)imageMemoryBarriers.size(),
                        imageMemoryBarriers.data()
                    );
                    vkResult = dispatchTable.gvkEndCommandBuffer(gvkCommandBuffer);
                    assert(vkResult == VK_SUCCESS);
                    std::lock_guard<std::mutex> lock(mThreadCommandBuffersMutex);
                    auto submitInfo = get_default<VkSubmitInfo>();
                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &gvkCommandBuffer.get<const VkCommandBuffer&>();
                    vkResult = dispatchTable.gvkQueueSubmit(get_queue_family(gvkDevice, 0).queues[0], 1, &submitInfo, VK_NULL_HANDLE);
                    assert(vkResult == VK_SUCCESS);
                }
            }
        );
    }
    return { };
}

VkResult detail::RestorePointApplierBase::restore_device_memory_data(const GvkStateTrackedObject& stateTrackedObject)
{
    auto restoreInfo = get_restore_info<GvkDeviceMemoryRestoreInfo>("VkDeviceMemory", stateTrackedObject.handle);
    auto vkDevice = (VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle);
    gvk::CopyEngine::DeviceMemoryCopyInfo deviceMemoryCopyInfo { };
    deviceMemoryCopyInfo.vkDeviceMemory = (VkDeviceMemory)get_restored_handle(stateTrackedObject.handle);
    deviceMemoryCopyInfo.allocateInfo = *restoreInfo->pMemoryAllocateInfo;
    deviceMemoryCopyInfo.path = (mApplyInfo.path / "VkDeviceMemory" / to_hex_string(stateTrackedObject.handle)).replace_extension(".bin");
    if (mApplyInfo.processDeviceMemoryDataCallback) {
        mApplyInfo.processDeviceMemoryDataCallback(vkDevice, deviceMemoryCopyInfo.vkDeviceMemory, deviceMemoryCopyInfo.allocateInfo, nullptr);
    } else {
        mCopyEngine.upload(vkDevice, deviceMemoryCopyInfo);
    }
    return { };
}

VkResult detail::RestorePointApplierBase::restore_descriptor_bindings(const GvkStateTrackedObject& stateTrackedObject)
{
    auto restoreInfo = get_restore_info<GvkDescriptorSetRestoreInfo>("VkDescriptorSet", stateTrackedObject.handle);
    auto vkDevice = (VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo->dependencyCount, (const GvkStateTrackedObject*)restoreInfo->pDependencies).handle);
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(restoreInfo->descriptorWriteCount);
    for (uint32_t i = 0; i < restoreInfo->descriptorWriteCount; ++i) {
        bool activeDescriptor = false;
        detail::enumerate_structure_handles(
            restoreInfo->pDescriptorWrites[i],
            [&](VkObjectType, const uint64_t& capturedHandle)
            {
                if (capturedHandle) {
                    auto restoredHandleItr = mRestoredHandles.find(capturedHandle);
                    if (restoredHandleItr != mRestoredHandles.end()) {
                        activeDescriptor |= capturedHandle != stateTrackedObject.handle;
                        const_cast<uint64_t&>(capturedHandle) = restoredHandleItr->second;
                    }
                }
            }
        );
        if (activeDescriptor) {
            descriptorWrites.push_back(restoreInfo->pDescriptorWrites[i]);
        }
    }
    if (!descriptorWrites.empty()) {
        mDispatchTable.gvkUpdateDescriptorSets(vkDevice, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
    return VK_SUCCESS;
}

#if 0
// NOTE : Implemented in "gvk-restore-point/source/generated/apply-command-bufer-restore-point.cpp"
VkResult detail::RestorePointApplierBase::restore_command_buffer_cmds(const GvkStateTrackedObject& stateTrackedObject)
{
    (void)stateTrackedObject;
    return { };
}
#endif

VkResult RestorePointApplier::apply_restore_point(const restore_point::ApplyInfo& restorePointInfo, const DispatchTable& dispatchTable, const DispatchTable& dynamicDispatchTable)
{
    (void)dynamicDispatchTable;
    std::cout << "Entered apply_restore_point()" << std::endl;
    auto vkResult = VK_SUCCESS;
    mApplyInfo = restorePointInfo;
    if (restorePointInfo.recording) {
        mCopyEngine.disable_multi_threading();
    }
    mDispatchTable = dispatchTable;
    mDynamicDispatchTable = dynamicDispatchTable;
    {
        std::ifstream manifestFile(mApplyInfo.path / "GvkRestorePointManifest.info", std::ios::binary);
        assert(manifestFile.is_open());
        deserialize(manifestFile, nullptr, mManifest);
    }
    assert(mManifest->objectCount);
    assert(mManifest->pObjects);
    for (uint32_t i = 0; i < mManifest->objectCount && vkResult == VK_SUCCESS; ++i) {
        const auto& stateTrackedObject = *(const GvkStateTrackedObject*)&mManifest->pObjects[i];
        std::cout << "[" << i << "/" << mManifest->objectCount << "]" << to_string(stateTrackedObject.type, Printer::EnumIdentifier) << " : " << restored_handle_to_string(stateTrackedObject.handle) << std::endl;
        vkResult = process_object(stateTrackedObject);
        assert(vkResult == VK_SUCCESS);
    }

    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Restoring image layouts" << std::endl;
    for (const auto& restoredImage : mRestoredImages) {
        auto restoreInfo = get_restore_info<GvkImageRestoreInfo>("VkImage", restoredImage.handle);
        auto vkDevice = (VkDevice)get_restored_handle(restoredImage.dispatchableHandle);
        CopyEngine::ImageCopyInfo imageCopyInfo { };
        imageCopyInfo.vkImage = (VkImage)get_restored_handle(restoredImage.handle);
        imageCopyInfo.createInfo = *restoreInfo->pImageCreateInfo;
        imageCopyInfo.newImageLayouts.reserve(restoreInfo->imageSubresourceCount);
        for (uint32_t i = 0; i < restoreInfo->imageSubresourceCount; ++i) {
            imageCopyInfo.newImageLayouts.push_back(restoreInfo->pImageLayouts[i]);
        }
        if (mApplyInfo.processImageLayoutsCallback) {
            mApplyInfo.processImageLayoutsCallback(vkDevice, imageCopyInfo.vkImage, *restoreInfo->pImageCreateInfo, restoreInfo->pImageLayouts);
        } else {
            mCopyEngine.transition_image_layouts(vkDevice, imageCopyInfo);
        }
    }
    std::cout << "Finished restoring image layouts" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Restoring device memory data" << std::endl;
    for (const auto& restoredDeviceMemory : mRestoredDeviceMemories) {
        vkResult = restore_device_memory_data(restoredDeviceMemory);
        assert(vkResult == VK_SUCCESS);
    }
    mCopyEngine.wait();
    std::cout << "Finished restoring device memory data" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Restoring device memory mappings" << std::endl;
    for (const auto& restoredDeviceMemory : mRestoredDeviceMemories) {
        auto restoreInfo = get_restore_info<GvkDeviceMemoryRestoreInfo>("VkDeviceMemory", restoredDeviceMemory.handle);
        const auto& mappedMemoryInfo = restoreInfo->mappedMemoryInfo;
        if (mappedMemoryInfo.size) {
            Device gvkDevice = (VkDevice)get_restored_handle(restoredDeviceMemory.dispatchableHandle);
            assert(gvkDevice);
            auto vkDeviceMemory = (VkDeviceMemory)get_restored_handle(restoredDeviceMemory.handle);
            void* pData = nullptr;
            vkResult = mDispatchTable.gvkMapMemory(gvkDevice, vkDeviceMemory, mappedMemoryInfo.offset, mappedMemoryInfo.size, mappedMemoryInfo.flags, &pData);
            assert(vkResult == VK_SUCCESS);
        }
    }
    std::cout << "Finished restoring device memory mappings" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Restoring descriptor set bindings" << std::endl;
    for (size_t i = 0; i < mRestoredDescriptorSets.size() && vkResult == VK_SUCCESS; ++i) {
        vkResult = restore_descriptor_bindings(mRestoredDescriptorSets[i]);
        assert(vkResult == VK_SUCCESS);
    }
    std::cout << "Finished restoring descriptor set bindings" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Restoring command buffer cmds" << std::endl;
    for (size_t i = 0; i < mRestoredCommandBuffers.size() && vkResult == VK_SUCCESS; ++i) {
        vkResult = restore_command_buffer_cmds(mRestoredCommandBuffers[i]);
        assert(vkResult == VK_SUCCESS);
    }
    std::cout << "Finished restoring command buffer cmds" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Leaving apply_restore_point()" << std::endl;
    assert(vkResult == VK_SUCCESS);
    return vkResult;
}

VkResult RestorePointApplier::process_VkBuffer(const GvkStateTrackedObject& stateTrackedObject, const GvkBufferRestoreInfo& restoreInfo)
{
    mRestoredBuffers.insert(stateTrackedObject);
    auto vkResult = BasicRestorePointApplier::process_VkBuffer(stateTrackedObject, restoreInfo);
    (void)vkResult;
    assert(!restoreInfo.memoryBindInfoCount == !restoreInfo.pMemoryBindInfos);
    for (uint32_t i = 0; i < restoreInfo.memoryBindInfoCount; ++i) {
        GvkStateTrackedObject stateTrackedDeviceMemory { };
        stateTrackedDeviceMemory.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
        stateTrackedDeviceMemory.handle = (uint64_t)restoreInfo.pMemoryBindInfos[i].memory;
        stateTrackedDeviceMemory.dispatchableHandle = stateTrackedObject.dispatchableHandle;
        vkResult = process_object(stateTrackedDeviceMemory);
        assert(vkResult == VK_SUCCESS);
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkImage(const GvkStateTrackedObject& stateTrackedObject, const GvkImageRestoreInfo& restoreInfo)
{
    auto vkResult = VK_SUCCESS;
    (void)vkResult;
    mRestoredImages.insert(stateTrackedObject);
    auto stateTrackedSwapchain = get_object(VK_OBJECT_TYPE_SWAPCHAIN_KHR, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies);
    if (stateTrackedSwapchain.type != VK_OBJECT_TYPE_SWAPCHAIN_KHR) {
        vkResult = BasicRestorePointApplier::process_VkImage(stateTrackedObject, restoreInfo);
        assert(vkResult == VK_SUCCESS);
        assert(!restoreInfo.memoryBindInfoCount == !restoreInfo.pMemoryBindInfos);
        for (uint32_t i = 0; i < restoreInfo.memoryBindInfoCount; ++i) {
            GvkStateTrackedObject stateTrackedDeviceMemory { };
            stateTrackedDeviceMemory.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
            stateTrackedDeviceMemory.handle = (uint64_t)restoreInfo.pMemoryBindInfos[i].memory;
            stateTrackedDeviceMemory.dispatchableHandle = stateTrackedObject.dispatchableHandle;
            vkResult = process_object(stateTrackedDeviceMemory);
            assert(vkResult == VK_SUCCESS);
        }
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkDeviceMemory(const GvkStateTrackedObject& stateTrackedObject, const GvkDeviceMemoryRestoreInfo& restoreInfo)
{
    mRestoredDeviceMemories.insert(stateTrackedObject);
    remove_pnext_entries((VkBaseOutStructure*)restoreInfo.pMemoryAllocateInfo, {
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
        VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
        VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
    });
    auto vkResult = BasicRestorePointApplier::process_VkDeviceMemory(stateTrackedObject, restoreInfo);
    assert(vkResult == VK_SUCCESS);
    assert(!restoreInfo.bufferBindInfoCount == !restoreInfo.pBufferBindInfos);
    for (uint32_t i = 0; i < restoreInfo.bufferBindInfoCount; ++i) {
        const auto& bufferBindInfo = restoreInfo.pBufferBindInfos[i];
        GvkStateTrackedObject stateTrackedBuffer { };
        stateTrackedBuffer.type = VK_OBJECT_TYPE_BUFFER;
        stateTrackedBuffer.handle = (uint64_t)bufferBindInfo.buffer;
        stateTrackedBuffer.dispatchableHandle = stateTrackedObject.dispatchableHandle;
        vkResult = process_object(stateTrackedBuffer);
        assert(vkResult == VK_SUCCESS);
        vkResult = restore_device_memory_bindings(stateTrackedBuffer);
        assert(vkResult == VK_SUCCESS);
    }
    assert(!restoreInfo.imageBindInfoCount == !restoreInfo.pImageBindInfos);
    for (uint32_t i = 0; i < restoreInfo.imageBindInfoCount; ++i) {
        const auto& imageBindInfo = restoreInfo.pImageBindInfos[i];
        GvkStateTrackedObject stateTrackedImage { };
        stateTrackedImage.type = VK_OBJECT_TYPE_IMAGE;
        stateTrackedImage.handle = (uint64_t)imageBindInfo.image;
        stateTrackedImage.dispatchableHandle = stateTrackedObject.dispatchableHandle;
        vkResult = process_object(stateTrackedImage);
        assert(vkResult == VK_SUCCESS);
        vkResult = restore_device_memory_bindings(stateTrackedImage);
        assert(vkResult == VK_SUCCESS);
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkInstance(const GvkStateTrackedObject& stateTrackedObject, const GvkInstanceRestoreInfo& restoreInfo)
{
    remove_pnext_entries((VkBaseOutStructure*)restoreInfo.pInstanceCreateInfo, VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT);
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    auto instanceCreateInfo = *restoreInfo.pInstanceCreateInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = 4;
    std::vector<const char*> extensions {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        "VK_KHR_external_memory_capabilities",
        "VK_KHR_get_physical_device_properties2",
    };
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    VkInstance vkInstance = mApplyInfo.recording ? (VkInstance)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkResult = mDispatchTable.gvkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_INSTANCE, stateTrackedObject.handle, (uint64_t)vkInstance);
        mCurrentObjects.clear();

        std::map<Auto<VkPhysicalDeviceProperties>, std::vector<VkPhysicalDevice>> capturedVkPhysicalDevices;
        for (uint32_t i = 0; i < restoreInfo.physicalDeviceCount; ++i) {
            auto capturedVkPhysicalDevice = restoreInfo.pPhysicalDevices[i];
            auto physicalDeviceRestoreInfo = get_restore_info<GvkPhysicalDeviceRestoreInfo>("VkPhysicalDevice", (uint64_t)capturedVkPhysicalDevice);
            capturedVkPhysicalDevices[physicalDeviceRestoreInfo->physicalDeviceProperties].push_back(capturedVkPhysicalDevice);
            GvkStateTrackedObject stateTrackedPhysicalDevice { };
            stateTrackedPhysicalDevice.type = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
            stateTrackedPhysicalDevice.handle = (uint64_t)capturedVkPhysicalDevice;
            stateTrackedPhysicalDevice.dispatchableHandle = (uint64_t)capturedVkPhysicalDevice;
            mCurrentObjects.push_back(stateTrackedPhysicalDevice);
        }

        uint32_t physicalDeviceCount = 0;
        std::vector<VkPhysicalDevice> vkPhysicalDevices;
        if (mApplyInfo.recording) {
            // TODO : Revisit this logic
            // TODO : Deal with dispatch tables
            mDispatchTable.gvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
            physicalDeviceCount = restoreInfo.physicalDeviceCount;
            vkPhysicalDevices.resize(physicalDeviceCount);
            for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
                vkPhysicalDevices[i] = restoreInfo.pPhysicalDevices[i];
            }
            mDispatchTable.gvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevices.data());
        } else {
            mDispatchTable.gvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
            vkPhysicalDevices.resize(physicalDeviceCount);
            mDispatchTable.gvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevices.data());
        }

        for (uint32_t i = 0; i < restoreInfo.physicalDeviceCount; ++i) {
            auto vkPhysicalDevice = vkPhysicalDevices[i];
            register_restored_handle(VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)restoreInfo.pPhysicalDevices[i], (uint64_t)vkPhysicalDevice);
            #if 0
            // TODO : Correlate VkPhysicalDeviceProperties from capture to playback.
            VkPhysicalDeviceProperties physicalDeviceProperties { };
            mDispatchTable.gvkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
            const auto& capturedVkPhysicalDeviceItr = capturedVkPhysicalDevices.find(physicalDeviceProperties);
            assert(capturedVkPhysicalDeviceItr != capturedVkPhysicalDevices.end());
            #endif
            ///////////////////////////////////////////////////////////////////////////////
            // NOTE : This call is here so that it triggers the deprecated state tracker's
            //  handler for this call; it's required for playback with state tracking (ie.
            //  repeat) to work correctly.  Once the deprecated state tracker is removed
            //  this can go away.
            VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties { };
            mDispatchTable.gvkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &physicalDeviceMemoryProperties);
            ///////////////////////////////////////////////////////////////////////////////
        }

        // TODO : Documentation
        Instance gvkInstance;
        vkResult = Instance::create_unmanaged(restoreInfo.pInstanceCreateInfo, nullptr, &mDynamicDispatchTable, vkInstance, &gvkInstance);
        if (vkResult == VK_SUCCESS) {
            mInstances.insert(gvkInstance);
        }
    }
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkPhysicalDevice(const GvkStateTrackedObject& stateTrackedObject, const GvkPhysicalDeviceRestoreInfo& restoreInfo)
{
    auto vkResult = BasicRestorePointApplier::process_VkPhysicalDevice(stateTrackedObject, restoreInfo);
    if (vkResult == VK_SUCCESS) {
        VkPhysicalDevice vkPhysicalDevice = mApplyInfo.recording ? (VkPhysicalDevice)stateTrackedObject.handle : VK_NULL_HANDLE;
        uint32_t queueFamilyPropertyCount = 0;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        if (mApplyInfo.recording) {
            // TODO : Revisit this logic
            // TODO : Deal with dispatch tables
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
            queueFamilyPropertyCount = restoreInfo.queueFamilyPropertyCount;
            queueFamilyProperties.resize(queueFamilyPropertyCount);
            for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i) {
                queueFamilyProperties[i] = restoreInfo.pQueueFamilyProperties[i];
            }
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
        } else {
            #if 0
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
            queueFamilyProperties.resize(queueFamilyPropertyCount);
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
            #endif
        }
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkDevice(const GvkStateTrackedObject& stateTrackedObject, const GvkDeviceRestoreInfo& restoreInfo)
{
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    auto vkPhysicalDevice = (VkPhysicalDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_PHYSICAL_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    VkDevice vkDevice = mApplyInfo.recording ? (VkDevice)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkResult = mDispatchTable.gvkCreateDevice(vkPhysicalDevice, restoreInfo.pDeviceCreateInfo, nullptr, &vkDevice);
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_DEVICE, stateTrackedObject.handle, (uint64_t)vkDevice);
        std::map<Auto<VkDeviceQueueCreateInfo>, std::vector<VkQueue>> capturedVkQueueFamilies;
        for (uint32_t i = 0; i < restoreInfo.queueCount; ++i) {
            auto queueRestoreInfo = get_restore_info<GvkQueueRestoreInfo>("VkQueue", (uint64_t)restoreInfo.pQueues[i]);
            capturedVkQueueFamilies[queueRestoreInfo->deviceQueueCreateInfo].push_back(restoreInfo.pQueues[i]);
        }
        uint32_t queueFamilyPropertyCount = 0;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        if (mApplyInfo.recording) {
            // TODO : Revisit this logic
            // TODO : Deal with dispatch tables
            #if 0
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
            queueFamilyProperties.resize(queueFamilyPropertyCount);
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
            #endif
        } else {
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
            queueFamilyProperties.resize(queueFamilyPropertyCount);
            mDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
        }
        for (uint32_t queueCreateInfo_i = 0; queueCreateInfo_i < restoreInfo.pDeviceCreateInfo->queueCreateInfoCount; ++queueCreateInfo_i) {
            const auto& deviceQueueCreateInfo = restoreInfo.pDeviceCreateInfo->pQueueCreateInfos[queueCreateInfo_i];
            const auto& capturedVkQueueFamilyItr = capturedVkQueueFamilies.find(deviceQueueCreateInfo);
            assert(capturedVkQueueFamilyItr != capturedVkQueueFamilies.end());
            const auto& capturedVkQueueFamily = capturedVkQueueFamilyItr->second;
            assert(capturedVkQueueFamily.size() == deviceQueueCreateInfo.queueCount);
            for (uint32_t queue_i = 0; queue_i < deviceQueueCreateInfo.queueCount; ++queue_i) {
                mCurrentObjects[0].type = VK_OBJECT_TYPE_QUEUE;
                mCurrentObjects[0].handle = (uint64_t)capturedVkQueueFamily[queue_i];
                mCurrentObjects[0].dispatchableHandle = (uint64_t)capturedVkQueueFamily[queue_i];
                #if 0
                // TODO : Revisit this logic
                // TODO : Deal with dispatch tables
                assert(deviceQueueCreateInfo.queueFamilyIndex < queueFamilyProperties.size());
                #endif
                VkQueue vkQueue = mApplyInfo.recording ? capturedVkQueueFamily[queue_i] : VK_NULL_HANDLE;
                mDispatchTable.gvkGetDeviceQueue(vkDevice, deviceQueueCreateInfo.queueFamilyIndex, queue_i, &vkQueue);
                register_restored_handle(VK_OBJECT_TYPE_QUEUE, (uint64_t)capturedVkQueueFamily[queue_i], (uint64_t)vkQueue);
            }
        }

        // TODO : Documentation
        Device gvkDevice;
        vkResult = Device::create_unmanaged(vkPhysicalDevice, restoreInfo.pDeviceCreateInfo, nullptr, &mDynamicDispatchTable, vkDevice, &gvkDevice);
        if (vkResult == VK_SUCCESS) {
            mDevices.insert(gvkDevice);
        }
    }
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkPipeline(const GvkStateTrackedObject& stateTrackedObject, const GvkPipelineRestoreInfo& restoreInfo)
{
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    auto vkResult = VK_ERROR_INITIALIZATION_FAILED;
    VkPipeline vkPipeline = mApplyInfo.recording ? (VkPipeline)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkDevice = (VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    if (restoreInfo.pComputePipelineCreateInfo) {
        update_handles(restoreInfo.pComputePipelineCreateInfo);
        vkResult = mDispatchTable.gvkCreateComputePipelines(vkDevice, VK_NULL_HANDLE, 1, restoreInfo.pComputePipelineCreateInfo, nullptr, &vkPipeline);
    }
    if (restoreInfo.pGraphicsPipelineCreateInfo) {
        // TODO : Need to cache data for VkSpecializationInfo
        update_handles(restoreInfo.pGraphicsPipelineCreateInfo);
        vkResult = mDispatchTable.gvkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, restoreInfo.pGraphicsPipelineCreateInfo, nullptr, &vkPipeline);
    }
    if (restoreInfo.pRayTracingPipelineCreateInfoKHR) {
        update_handles(restoreInfo.pRayTracingPipelineCreateInfoKHR);
        vkResult = mDispatchTable.gvkCreateRayTracingPipelinesKHR(vkDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, restoreInfo.pRayTracingPipelineCreateInfoKHR, nullptr, &vkPipeline);
    }
    if (restoreInfo.pRayTracingPipelineCreateInfoNV) {
        update_handles(restoreInfo.pRayTracingPipelineCreateInfoNV);
        vkResult = mDispatchTable.gvkCreateRayTracingPipelinesNV(vkDevice, VK_NULL_HANDLE, 1, restoreInfo.pRayTracingPipelineCreateInfoNV, nullptr, &vkPipeline);
    }
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_PIPELINE, stateTrackedObject.handle, (uint64_t)vkPipeline);
    }
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkCommandBuffer(const GvkStateTrackedObject& stateTrackedObject, const GvkCommandBufferRestoreInfo& restoreInfo)
{
    mRestoredCommandBuffers.push_back(stateTrackedObject);
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    VkCommandBuffer vkCommandBuffer = mApplyInfo.recording ? (VkCommandBuffer)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkDevice = (VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    update_handles(restoreInfo.pCommandBufferAllocateInfo);
    const_cast<VkCommandBufferAllocateInfo*>(restoreInfo.pCommandBufferAllocateInfo)->commandBufferCount = 1;
    auto vkResult = mDispatchTable.gvkAllocateCommandBuffers(vkDevice, restoreInfo.pCommandBufferAllocateInfo, &vkCommandBuffer);
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_COMMAND_BUFFER, stateTrackedObject.handle, (uint64_t)vkCommandBuffer);
    }
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkDescriptorSet(const GvkStateTrackedObject& stateTrackedObject, const GvkDescriptorSetRestoreInfo& restoreInfo)
{
    mRestoredDescriptorSets.push_back(stateTrackedObject);
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    VkDescriptorSet vkDescriptorSet = mApplyInfo.recording ? (VkDescriptorSet)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkDevice = (VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    auto descriptorSetAllocateInfo = *restoreInfo.pDescriptorSetAllocateInfo;
    auto capturedVkDescriptorSetLayout = (VkDescriptorSetLayout)get_object(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &capturedVkDescriptorSetLayout;
    update_handles(&descriptorSetAllocateInfo);
    auto vkResult = mDispatchTable.gvkAllocateDescriptorSets(vkDevice, &descriptorSetAllocateInfo, &vkDescriptorSet);
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_DESCRIPTOR_SET, stateTrackedObject.handle, (uint64_t)vkDescriptorSet);
    }
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkEvent(const GvkStateTrackedObject& stateTrackedObject, const GvkEventRestoreInfo& restoreInfo)
{
    auto vkResult = BasicRestorePointApplier::process_VkEvent(stateTrackedObject, restoreInfo);
    assert(vkResult == VK_SUCCESS);
    auto vkDevice = (VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    auto vkEvent = (VkEvent)get_restored_handle(stateTrackedObject.handle);
    switch (restoreInfo.status) {
    case VK_EVENT_SET: {
        vkResult = mDispatchTable.gvkSetEvent(vkDevice, vkEvent);
        assert(vkResult == VK_SUCCESS);
    } break;
    case VK_EVENT_RESET: {
        vkResult = mDispatchTable.gvkResetEvent(vkDevice, vkEvent);
        assert(vkResult == VK_SUCCESS);
    } break;
    default: {
        assert(false && "TODO : Documentation");
    } break;
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkFence(const GvkStateTrackedObject& stateTrackedObject, const GvkFenceRestoreInfo& restoreInfo)
{
    auto vkResult = BasicRestorePointApplier::process_VkFence(stateTrackedObject, restoreInfo);
    assert(vkResult == VK_SUCCESS);
    if (restoreInfo.signaled && !(restoreInfo.pFenceCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT)) {
        Device gvkDevice((VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle));
        assert(gvkDevice);
        auto gvkQueue = gvkDevice.get<QueueFamilies>()[0].queues[0];
        auto vkFence = (VkFence)get_restored_handle(stateTrackedObject.handle);
        vkResult = mDispatchTable.gvkQueueSubmit(gvkQueue, 0, nullptr, vkFence);
        assert(vkResult == VK_SUCCESS);
    } else if (!restoreInfo.signaled && restoreInfo.pFenceCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) {
        auto vkDevice = (VkDevice)get_restored_handle(stateTrackedObject.dispatchableHandle);
        auto vkFence = (VkFence)get_restored_handle(stateTrackedObject.handle);
        vkResult = mDispatchTable.gvkResetFences(vkDevice, 1, &vkFence);
        assert(vkResult == VK_SUCCESS);
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkSemaphore(const GvkStateTrackedObject& stateTrackedObject, const GvkSemaphoreRestoreInfo& restoreInfo)
{
    auto vkResult = BasicRestorePointApplier::process_VkSemaphore(stateTrackedObject, restoreInfo);
    assert(vkResult == VK_SUCCESS);
    if (!mApplyInfo.recording) {
        // TODO : Revisit this logic
        // TODO : Deal with dispatch tables
        if (restoreInfo.statusFlags & GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT) {
            Device gvkDevice((VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle));
            assert(gvkDevice);
            auto gvkQueue = gvkDevice.get<QueueFamilies>()[0].queues[0];
            const auto& dispatchTable = gvkDevice.get<DispatchTable>();
            auto [gvkCommandBuffer, gvkFence] = get_thread_command_buffer(gvkDevice);
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkResult = dispatchTable.gvkBeginCommandBuffer(gvkCommandBuffer, &commandBufferBeginInfo);
            assert(vkResult == VK_SUCCESS);
            vkResult = dispatchTable.gvkEndCommandBuffer(gvkCommandBuffer);
            assert(vkResult == VK_SUCCESS);
            auto submitInfo = get_default<VkSubmitInfo>();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &gvkCommandBuffer.get<const VkCommandBuffer&>();
            submitInfo.signalSemaphoreCount = 1;
            auto vkSemaphore = (VkSemaphore)get_restored_handle(stateTrackedObject.handle);
            submitInfo.pSignalSemaphores = &vkSemaphore;
            vkResult = dispatchTable.gvkQueueSubmit(gvkQueue, 1, &submitInfo, gvkFence);
            assert(vkResult == VK_SUCCESS);

            vkResult = dispatchTable.gvkWaitForFences(gvkDevice, 1, &gvkFence.get<const VkFence&>(), VK_TRUE, UINT64_MAX);
            assert(vkResult == VK_SUCCESS);
            vkResult = dispatchTable.gvkResetFences(gvkDevice, 1, &gvkFence.get<const VkFence&>());
            assert(vkResult == VK_SUCCESS);
        }
    }
    return vkResult;
}

VkResult RestorePointApplier::process_VkDisplayModeKHR(const GvkStateTrackedObject& stateTrackedObject, const GvkDisplayModeRestoreInfoKHR& restoreInfo)
{
    (void)stateTrackedObject;
    (void)restoreInfo;
    // NOOP : Nothing to do here
    return { };
}

VkResult RestorePointApplier::process_VkShaderEXT(const GvkStateTrackedObject& stateTrackedObject, const GvkShaderRestoreInfoEXT& restoreInfo)
{
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    auto vkResult = VK_ERROR_FEATURE_NOT_PRESENT;
    // TODO : RestorePointApplier::process_VkShaderEXT()
    (void)restoreInfo;
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkSurfaceKHR(const GvkStateTrackedObject& stateTrackedObject, const GvkSurfaceRestoreInfoKHR& restoreInfo)
{
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);
    auto vkResult = VK_ERROR_INITIALIZATION_FAILED;
    VkSurfaceKHR vkSurface = mApplyInfo.recording ? (VkSurfaceKHR)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkInstance = (VkInstance)get_restored_handle(get_object(VK_OBJECT_TYPE_INSTANCE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    if (restoreInfo.pDisplaySurfaceCreateInfoKHR) {
        update_handles(restoreInfo.pDisplaySurfaceCreateInfoKHR);
        vkResult = mDispatchTable.gvkCreateDisplayPlaneSurfaceKHR(vkInstance, restoreInfo.pDisplaySurfaceCreateInfoKHR, nullptr, &vkSurface);
    }
    if (restoreInfo.pHeadlessSurfaceCreateInfoEXT) {
        update_handles(restoreInfo.pHeadlessSurfaceCreateInfoEXT);
        vkResult = mDispatchTable.gvkCreateHeadlessSurfaceEXT(vkInstance, restoreInfo.pHeadlessSurfaceCreateInfoEXT, nullptr, &vkSurface);
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (restoreInfo.pWin32SurfaceCreateInfoKHR) {
        auto win32SurfaceCreateInfo = *restoreInfo.pWin32SurfaceCreateInfoKHR;
        if (mApplyInfo.processWin32SurfaceCreateInfoCallback) {
            mApplyInfo.processWin32SurfaceCreateInfoCallback(&win32SurfaceCreateInfo, restoreInfo.width, restoreInfo.height);
        }
        vkResult = mDispatchTable.gvkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfo, nullptr, &vkSurface);
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_SURFACE_KHR, stateTrackedObject.handle, (uint64_t)vkSurface);
    }
    mCurrentObjects.clear();
    return vkResult;
}

VkResult RestorePointApplier::process_VkSwapchainKHR(const GvkStateTrackedObject& stateTrackedObject, const GvkSwapchainRestoreInfoKHR& restoreInfo)
{
    // TODO : Image acquisition
    assert(mCurrentObjects.empty());
    mCurrentObjects.push_back(stateTrackedObject);

    ///////////////////////////////////////////////////////////////////////////////
    // TODO : Double check this call in live restore
    auto vkPhysicalDevice = (VkPhysicalDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_PHYSICAL_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    auto vkSurface = (VkSurfaceKHR)get_restored_handle(get_object(VK_OBJECT_TYPE_SURFACE_KHR, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    VkSurfaceCapabilitiesKHR surfaceCapabilites { };
    auto vkResult = mDispatchTable.gvkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &surfaceCapabilites);
    assert(vkResult == VK_SUCCESS);
    ///////////////////////////////////////////////////////////////////////////////

    VkSwapchainKHR vkSwapchain = mApplyInfo.recording ? (VkSwapchainKHR)stateTrackedObject.handle : VK_NULL_HANDLE;
    auto vkDevice = (VkDevice)get_restored_handle(get_object(VK_OBJECT_TYPE_DEVICE, restoreInfo.dependencyCount, (const GvkStateTrackedObject*)restoreInfo.pDependencies).handle);
    const_cast<VkSwapchainCreateInfoKHR*>(restoreInfo.pSwapchainCreateInfoKHR)->oldSwapchain = VK_NULL_HANDLE;
    update_handles(restoreInfo.pSwapchainCreateInfoKHR);
    vkResult = mDispatchTable.gvkCreateSwapchainKHR(vkDevice, restoreInfo.pSwapchainCreateInfoKHR, nullptr, &vkSwapchain);
    if (vkResult == VK_SUCCESS) {
        register_restored_handle(VK_OBJECT_TYPE_SWAPCHAIN_KHR, stateTrackedObject.handle, (uint64_t)vkSwapchain);
        mCurrentObjects.clear();
        for (uint32_t i = 0; i < restoreInfo.imageCount; ++i) {
            GvkStateTrackedObject stateTrackedImage { };
            stateTrackedImage.type = VK_OBJECT_TYPE_IMAGE;
            stateTrackedImage.handle = (uint64_t)restoreInfo.pImages[i].image;
            stateTrackedImage.dispatchableHandle = (uint64_t)restoreInfo.pImages[i].image;
            mCurrentObjects.push_back(stateTrackedImage);
        }
        uint32_t imageCount = 0;
        std::vector<VkImage> vkImages;
        if (mApplyInfo.recording) {
            // TODO : Revisit this logic
            // TODO : Deal with dispatch tables
            mDispatchTable.gvkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount, nullptr);
            imageCount = restoreInfo.imageCount;
            vkImages.resize(restoreInfo.imageCount);
            for (uint32_t i = 0; i < restoreInfo.imageCount; ++i) {
                vkImages[i] = restoreInfo.pImages[i].image;
            }
            mDispatchTable.gvkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount, vkImages.data());
        } else {
            mDispatchTable.gvkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount, nullptr);
            vkImages.resize(imageCount);
            mDispatchTable.gvkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount, vkImages.data());
        }
        for (size_t i = 0; i < vkImages.size(); ++i) {
            register_restored_handle(VK_OBJECT_TYPE_IMAGE, (uint64_t)restoreInfo.pImages[i].image, (uint64_t)vkImages[i]);
        }
    }
    mCurrentObjects.clear();
    return vkResult;
}

} // namespace gvk


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

#include "gvk-restore-point/copy-engine.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-format-info.hpp"
// TODO : Handle dispatch for vkAllocateCommandBuffers outside of CopyEngine
#include "gvk-layer.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include "stb/stb_image_write.h"

#include <filesystem>
#include <utility>

namespace gvk {
namespace restore_point {

VkResult CopyEngine::create(const gvk::Device& device, const CreateInfo* pCreateInfo, CopyEngine* pCopyEngine)
{
    assert(device);
    assert(pCreateInfo);
    assert(pCopyEngine);
    pCopyEngine->reset();
    pCopyEngine->mDevice = device;
    pCopyEngine->mpfnInitializeThreadCallback = pCreateInfo->pfnInitializeThreadCallback;
    switch (pCreateInfo->threadCount) {
    case 0: { pCopyEngine->mupThreadPool = std::make_unique<asio::thread_pool>(); } break;
    case 1: break;
    default: { pCopyEngine->mupThreadPool = std::make_unique<asio::thread_pool>(pCreateInfo->threadCount); } break;
    }
    uint32_t queueFamilyPropertyCount = 0;
    const auto& physicalDevice = device.get<PhysicalDevice>();
    physicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    physicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
    for (const auto& queueFamily : device.get<QueueFamilies>()) {
        assert(queueFamily.index < queueFamilyProperties.size());
        auto queueFlags = queueFamilyProperties[queueFamily.index].queueFlags;
        for (const auto& queue : queueFamily.queues) {
            if (queueFlags == VK_QUEUE_TRANSFER_BIT || (!pCopyEngine->mQueue && (queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)))) {
                pCopyEngine->mQueue = queue;
            }
        }
    }
    return VK_SUCCESS;
}

CopyEngine::CopyEngine(CopyEngine&& other)
{
    *this = std::move(other);
}

CopyEngine& CopyEngine::operator=(CopyEngine&& other)
{
    if (this != &other) {
        mDevice = std::move(other.mDevice);
        mQueue = std::move(other.mQueue);
        mpfnInitializeThreadCallback = std::move(other.mpfnInitializeThreadCallback);
        mupThreadPool = std::move(other.mupThreadPool);
        mTaskResources = std::move(other.mTaskResources);
        mAccelerationStructureTaskResources = std::move(other.mAccelerationStructureTaskResources);
        mAccelerationStrcutureSerializationInfoRetrieved = std::move(other.mAccelerationStrcutureSerializationInfoRetrieved);
    }
    return *this;
}

CopyEngine::~CopyEngine()
{
    reset();
}

void CopyEngine::reset()
{
    wait();
    mDevice.reset();
    mQueue.reset();
    mupThreadPool.reset();
    mTaskResources.clear();
}

void CopyEngine::wait()
{
    if (mupThreadPool) {
        mupThreadPool->wait();
    }
    if (mDevice) {
        auto vkResult = mDevice.get<DispatchTable>().gvkDeviceWaitIdle(mDevice);
        (void)vkResult;
        assert(vkResult == VK_SUCCESS);
    }
}

CopyEngine::operator bool() const
{
    return mDevice && mQueue;
}

void CopyEngine::download(DownloadDeviceMemoryInfo downloadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(downloadInfo.memory);
    std::vector<VkBufferCopy> copyRegions;
    if (downloadInfo.regionCount && downloadInfo.pRegions) {
        copyRegions.insert(copyRegions.end(), downloadInfo.pRegions, downloadInfo.pRegions + downloadInfo.regionCount);
    } else {
        auto copyRegion = get_default<VkBufferCopy>();
        copyRegion.size = downloadInfo.memoryAllocateInfo.allocationSize;
        copyRegions.push_back(copyRegion);
    }
    auto downloadMemory = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Calculate total size and set dstOffsets
            VkDeviceSize totalSize = 0;
            for (auto& copyRegion : copyRegions) {
                copyRegion.dstOffset = totalSize;
                totalSize += copyRegion.size;
            }

            // Get TaskResources
            TaskResources taskResources { };
            gvk_result(get_task_resources(totalSize, &taskResources));

            // Create Buffer and bind to target VkDeviceMemory
            auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
            bufferCreateInfo.size = downloadInfo.memoryAllocateInfo.allocationSize;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            Buffer buffer;
            gvk_result(Buffer::create(mDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &buffer));
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            gvk_result(dispatchTable.gvkBindBufferMemory(mDevice, buffer, downloadInfo.memory, 0));

            // Begin CommandBuffer
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Copy
            dispatchTable.gvkCmdCopyBuffer(taskResources.vkCommandBuffer, buffer, taskResources.buffer, (uint32_t)copyRegions.size(), copyRegions.data());

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(dispatchTable.gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            downloadInfo.regionCount = (uint32_t)copyRegions.size();
            downloadInfo.pRegions = copyRegions.data();
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            downloadInfo.pfnCallback(downloadInfo, bindBufferMemoryInfo, pData);
            dispatchTable.gvkUnmapMemory(mDevice, taskResources.memory);
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [downloadMemory]() mutable { downloadMemory(); });
    } else {
        downloadMemory();
    }
}

void CopyEngine::download(DownloadBufferInfo downloadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(downloadInfo.buffer);
    assert(downloadInfo.pfnCallback);
    const auto& bufferCreateInfo = downloadInfo.bufferCreateInfo;
    auto downloadBuffer = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources { };
            gvk_result(get_task_resources(bufferCreateInfo.size, &taskResources));

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Copy
            auto bufferCopy = get_default<VkBufferCopy>();
            bufferCopy.size = bufferCreateInfo.size;
            dispatchTable.gvkCmdCopyBuffer(taskResources.vkCommandBuffer, downloadInfo.buffer, taskResources.buffer, 1, &bufferCopy);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(dispatchTable.gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            downloadInfo.pfnCallback(downloadInfo, bindBufferMemoryInfo, pData);
            dispatchTable.gvkUnmapMemory(mDevice, taskResources.memory);
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [downloadBuffer]() mutable { downloadBuffer(); });
    } else {
        downloadBuffer();
    }
}

void CopyEngine::download(DownloadImageInfo downloadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(downloadInfo.image);
    assert(downloadInfo.pfnCallback);
    const auto& imageCreateInfo = downloadInfo.imageCreateInfo;
    const auto& imageSubresourceRange = downloadInfo.imageSubresourceRange;
    auto imageSubresourceCount = imageSubresourceRange.levelCount * imageSubresourceRange.layerCount;
    std::vector<VkImageLayout> imageLayouts(downloadInfo.pImageLayouts, downloadInfo.pImageLayouts + imageSubresourceCount);
    auto downloadImage = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources { };
            gvk_result(get_task_resources(get_image_data_size(imageCreateInfo, imageSubresourceRange), &taskResources));

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Transition Image layouts to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
            std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
            imageMemoryBarriers.reserve(imageSubresourceCount);
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                    auto subresource = arrayLayer * imageCreateInfo.mipLevels + mipLevel;
                    if (imageLayouts[subresource]) {
                        auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                        imageMemoryBarrier.srcAccessMask = 0;
                        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                        imageMemoryBarrier.oldLayout = imageLayouts[subresource];
                        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.image = downloadInfo.image;
                        imageMemoryBarrier.subresourceRange = imageSubresourceRange;
                        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel;
                        imageMemoryBarrier.subresourceRange.levelCount = 1;
                        imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer;
                        imageMemoryBarrier.subresourceRange.layerCount = 1;
                        imageMemoryBarriers.push_back(imageMemoryBarrier);
                    }
                }
            }
            dispatchTable.gvkCmdPipelineBarrier(
                taskResources.vkCommandBuffer,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );

            // Copy
            VkDeviceSize bufferOffset = 0;
            std::vector<VkBufferImageCopy> bufferImageCopies;
            bufferImageCopies.reserve(imageSubresourceCount);
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                    auto subresource = arrayLayer * imageCreateInfo.mipLevels + mipLevel;
                    if (imageLayouts[subresource]) {
                        auto bufferImageCopy = get_default<VkBufferImageCopy>();
                        bufferImageCopy.bufferOffset = bufferOffset;
                        bufferImageCopy.imageSubresource.aspectMask = get_image_aspect_flags(imageCreateInfo.format) & ~VK_IMAGE_ASPECT_STENCIL_BIT;
                        bufferImageCopy.imageSubresource.mipLevel = mipLevel;
                        bufferImageCopy.imageSubresource.baseArrayLayer = arrayLayer;
                        bufferImageCopy.imageSubresource.layerCount = 1;
                        bufferImageCopy.imageExtent = get_mip_level_extent(imageCreateInfo.extent, mipLevel);
                        bufferImageCopies.push_back(bufferImageCopy);
                    }
                    auto arrayLayerSubresourceRange = imageSubresourceRange;
                    arrayLayerSubresourceRange.baseMipLevel = mipLevel;
                    arrayLayerSubresourceRange.levelCount = 1;
                    arrayLayerSubresourceRange.baseArrayLayer = arrayLayer;
                    arrayLayerSubresourceRange.layerCount = 1;
                    bufferOffset += get_image_data_size(imageCreateInfo, arrayLayerSubresourceRange);
                }
            }
            dispatchTable.gvkCmdCopyImageToBuffer(
                taskResources.vkCommandBuffer,
                downloadInfo.image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                taskResources.buffer,
                (uint32_t)bufferImageCopies.size(),
                bufferImageCopies.data()
            );

            // Transition Image layouts back
            for (auto& imageMemoryBarrier : imageMemoryBarriers) {
                std::swap(imageMemoryBarrier.srcAccessMask, imageMemoryBarrier.dstAccessMask);
                std::swap(imageMemoryBarrier.oldLayout, imageMemoryBarrier.newLayout);
            }
            dispatchTable.gvkCmdPipelineBarrier(
                taskResources.vkCommandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(dispatchTable.gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            downloadInfo.pImageLayouts = imageLayouts.data();
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            downloadInfo.pfnCallback(downloadInfo, bindBufferMemoryInfo, pData);
            dispatchTable.gvkUnmapMemory(mDevice, taskResources.memory);
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [downloadImage]() mutable { downloadImage(); });
    } else {
        downloadImage();
    }
}

void CopyEngine::download(DownloadAccelerationStructureInfo downloadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(downloadInfo.accelerationStructure);
    assert(downloadInfo.buffer);
    assert(downloadInfo.memory);
    assert(downloadInfo.device);
    assert(downloadInfo.pfnCallback);
    auto downloadAccelerationStructureEx = [=]() mutable
    {
        // TODO : Need to handle host allocated acceleration structures
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            gvk_result(get_task_resources(downloadInfo.accelerationStructureSerializedSize, &taskResources));

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Copy VkAccelerationStructureKHR -> VkBuffer
            auto accelerationStructureCopy = get_default<VkCopyAccelerationStructureToMemoryInfoKHR>();
            accelerationStructureCopy.src = downloadInfo.accelerationStructure;
            accelerationStructureCopy.dst.deviceAddress = downloadInfo.bufferDeviceAddress;
            accelerationStructureCopy.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
            dispatchTable.gvkCmdCopyAccelerationStructureToMemoryKHR(taskResources.vkCommandBuffer, &accelerationStructureCopy);

            // TODO : Barrier?

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until the transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // Begin CommandBuffer
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // TODO : Documentation
            auto bufferCopy = get_default<VkBufferCopy>();
            bufferCopy.size = downloadInfo.accelerationStructureSerializedSize;
            dispatchTable.gvkCmdCopyBuffer(taskResources.vkCommandBuffer, downloadInfo.buffer, taskResources.buffer, 1, &bufferCopy);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until the transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(dispatchTable.gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            downloadInfo.pfnCallback(downloadInfo, bindBufferMemoryInfo, pData);
            dispatchTable.gvkUnmapMemory(mDevice, taskResources.memory);
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [downloadAccelerationStructureEx]() mutable { downloadAccelerationStructureEx(); });
    } else {
        downloadAccelerationStructureEx();
    }
}

void CopyEngine::upload(UploadDeviceMemoryInfo uploadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(uploadInfo.memory);
    assert(uploadInfo.pfnCallback);
    const auto& memoryAllocateInfo = uploadInfo.memoryAllocateInfo;
    std::vector<VkBufferCopy> copyRegions;
    if (uploadInfo.regionCount && uploadInfo.pRegions) {
        copyRegions.insert(copyRegions.end(), uploadInfo.pRegions, uploadInfo.pRegions + uploadInfo.regionCount);
    } else {
        auto copyRegion = get_default<VkBufferCopy>();
        copyRegion.size = uploadInfo.memoryAllocateInfo.allocationSize;
        copyRegions.push_back(copyRegion);
    }
    auto uploadDeviceMemory = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            // TODO : Handle regions
            gvk_result(get_task_resources(memoryAllocateInfo.allocationSize, &taskResources));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(mDevice.get<DispatchTable>().gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            uploadInfo.regionCount = (uint32_t)copyRegions.size();
            uploadInfo.pRegions = copyRegions.data();
            uploadInfo.pfnCallback(uploadInfo, bindBufferMemoryInfo, pData);
            mDevice.get<DispatchTable>().gvkUnmapMemory(mDevice, taskResources.memory);

            // Create dst Buffer and bind it to dst VkDeviceMemory
            auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
            bufferCreateInfo.size = memoryAllocateInfo.allocationSize;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            Buffer dstBuffer;
            gvk_result(Buffer::create(mDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &dstBuffer));
            gvk_result(mDevice.get<DispatchTable>().gvkBindBufferMemory(mDevice, dstBuffer, uploadInfo.memory, 0));

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Copy
            auto bufferCopy = get_default<VkBufferCopy>();
            bufferCopy.size = memoryAllocateInfo.allocationSize;
            dispatchTable.gvkCmdCopyBuffer(taskResources.vkCommandBuffer, taskResources.buffer, dstBuffer, 1, &bufferCopy);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [uploadDeviceMemory]() mutable { uploadDeviceMemory(); });
    } else {
        uploadDeviceMemory();
    }
}

void CopyEngine::upload(UploadBufferInfo uploadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(uploadInfo.buffer);
    assert(uploadInfo.pfnCallback);
    const auto& bufferCreateInfo = uploadInfo.bufferCreateInfo;
    auto uploadBuffer = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            gvk_result(get_task_resources(bufferCreateInfo.size, &taskResources));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(mDevice.get<DispatchTable>().gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            uploadInfo.pfnCallback(uploadInfo, bindBufferMemoryInfo, pData);
            mDevice.get<DispatchTable>().gvkUnmapMemory(mDevice, taskResources.memory);

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Copy
            auto bufferCopy = get_default<VkBufferCopy>();
            bufferCopy.size = bufferCreateInfo.size;
            dispatchTable.gvkCmdCopyBuffer(taskResources.vkCommandBuffer, taskResources.buffer, uploadInfo.buffer, 1, &bufferCopy);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [uploadBuffer]() mutable { uploadBuffer(); });
    } else {
        uploadBuffer();
    }
}

void CopyEngine::upload(UploadImageInfo uploadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(uploadInfo.image);
    assert(uploadInfo.pfnCallback);
    const auto& imageCreateInfo = uploadInfo.imageCreateInfo;
    const auto& imageSubresourceRange = uploadInfo.imageSubresourceRange;
    auto imageSubresourceCount = imageSubresourceRange.levelCount * imageSubresourceRange.layerCount;
    std::vector<VkImageLayout> oldImageLayouts(uploadInfo.pOldImageLayouts, uploadInfo.pOldImageLayouts + imageSubresourceCount);
    std::vector<VkImageLayout> newImageLayouts(uploadInfo.pNewImageLayouts, uploadInfo.pNewImageLayouts + imageSubresourceCount);
    auto uploadBuffer = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            gvk_result(get_task_resources(get_image_data_size(imageCreateInfo, imageSubresourceRange), &taskResources));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(mDevice.get<DispatchTable>().gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            uploadInfo.pfnCallback(uploadInfo, bindBufferMemoryInfo, pData);
            mDevice.get<DispatchTable>().gvkUnmapMemory(mDevice, taskResources.memory);

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Transition Image layouts to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
            std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
            imageMemoryBarriers.reserve(imageSubresourceCount);
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                    auto subresource = arrayLayer * imageCreateInfo.mipLevels + mipLevel;
                    if (newImageLayouts[subresource]) {
                        auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                        imageMemoryBarrier.srcAccessMask = 0;
                        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        imageMemoryBarrier.oldLayout = oldImageLayouts[subresource];
                        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.image = uploadInfo.image;
                        imageMemoryBarrier.subresourceRange = imageSubresourceRange;
                        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel;
                        imageMemoryBarrier.subresourceRange.levelCount = 1;
                        imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer;
                        imageMemoryBarrier.subresourceRange.layerCount = 1;
                        imageMemoryBarriers.push_back(imageMemoryBarrier);
                    }
                }
            }
            dispatchTable.gvkCmdPipelineBarrier(
                taskResources.vkCommandBuffer,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );

            // Copy
            VkDeviceSize bufferOffset = 0;
            std::vector<VkBufferImageCopy> bufferImageCopies;
            bufferImageCopies.reserve(imageSubresourceCount);
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                    auto subresource = arrayLayer * imageCreateInfo.mipLevels + mipLevel;
                    if (newImageLayouts[subresource]) {
                        auto bufferImageCopy = get_default<VkBufferImageCopy>();
                        bufferImageCopy.bufferOffset = bufferOffset;
                        bufferImageCopy.imageSubresource.aspectMask = get_image_aspect_flags(imageCreateInfo.format) & ~VK_IMAGE_ASPECT_STENCIL_BIT;
                        bufferImageCopy.imageSubresource.mipLevel = mipLevel;
                        bufferImageCopy.imageSubresource.baseArrayLayer = arrayLayer;
                        bufferImageCopy.imageSubresource.layerCount = 1;
                        bufferImageCopy.imageExtent = get_mip_level_extent(imageCreateInfo.extent, mipLevel);
                        bufferImageCopies.push_back(bufferImageCopy);
                    }
                    auto arrayLayerSubresourceRange = imageSubresourceRange;
                    arrayLayerSubresourceRange.baseMipLevel = mipLevel;
                    arrayLayerSubresourceRange.levelCount = 1;
                    arrayLayerSubresourceRange.baseArrayLayer = arrayLayer;
                    arrayLayerSubresourceRange.layerCount = 1;
                    bufferOffset += get_image_data_size(imageCreateInfo, arrayLayerSubresourceRange);
                }
            }

            // TODO : We kinda don't want to be here if we have no copies to perform...
            //  Need layout transition and transfer logic to be modular.
            if (!bufferImageCopies.empty()) {
                dispatchTable.gvkCmdCopyBufferToImage(
                    taskResources.vkCommandBuffer,
                    taskResources.buffer,
                    uploadInfo.image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    (uint32_t)bufferImageCopies.size(),
                    bufferImageCopies.data()
                );
            }

            // Transition Image layouts back
            imageMemoryBarriers.clear();
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                    auto subresource = arrayLayer * imageCreateInfo.mipLevels + mipLevel;
                    if (newImageLayouts[subresource]) {
                        auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        imageMemoryBarrier.dstAccessMask = 0;
                        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                        imageMemoryBarrier.newLayout = newImageLayouts[subresource];
                        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.image = uploadInfo.image;
                        imageMemoryBarrier.subresourceRange = imageSubresourceRange;
                        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel;
                        imageMemoryBarrier.subresourceRange.levelCount = 1;
                        imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer;
                        imageMemoryBarrier.subresourceRange.layerCount = 1;
                        imageMemoryBarriers.push_back(imageMemoryBarrier);
                    }
                }
            }
            dispatchTable.gvkCmdPipelineBarrier(
                taskResources.vkCommandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [uploadBuffer]() mutable { uploadBuffer(); });
    } else {
        uploadBuffer();
    }
}

void CopyEngine::upload(UploadAccelerationStructureInfo uploadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(uploadInfo.accelerationStructure);
    assert(uploadInfo.pfnCallback);
    const auto& accelerationStructureCreateInfo = uploadInfo.accelerationStructureCreateInfo;
    const auto& accelerationStructureSerializationInfo = uploadInfo.accelerationStructureSerializationInfo;
    auto uploadAccelerationStructure = [=]() mutable
    {
        (void)accelerationStructureCreateInfo;
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            AccelerationStructureTaskResources taskResources{ };
            gvk_result(get_acceleration_structure_task_resources(accelerationStructureSerializationInfo, &taskResources));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(mDevice.get<DispatchTable>().gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            uploadInfo.pfnCallback(uploadInfo, bindBufferMemoryInfo, pData);
            mDevice.get<DispatchTable>().gvkUnmapMemory(mDevice, taskResources.memory);

            // TODO : Need VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT path
            {
                // Begin CommandBuffer
                const auto& dispatchTable = mDevice.get<DispatchTable>();
                auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
                commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

                // TODO : Documentation
                auto bufferDeviceAddressInfo = get_default<VkBufferDeviceAddressInfo>();
                bufferDeviceAddressInfo.buffer = taskResources.buffer;
                auto pApplicationInfo = mDevice.get<Instance>().get<VkInstanceCreateInfo>().pApplicationInfo;
                auto apiVersion = pApplicationInfo ? pApplicationInfo->apiVersion : VK_API_VERSION_1_0;

                // TODO : Documentation
                auto copyInfo = get_default<VkCopyMemoryToAccelerationStructureInfoKHR>();
                if (apiVersion < VK_API_VERSION_1_2) {
                    copyInfo.src.deviceAddress = dispatchTable.gvkGetBufferDeviceAddressKHR(mDevice, &bufferDeviceAddressInfo);
                } else {
                    copyInfo.src.deviceAddress = dispatchTable.gvkGetBufferDeviceAddress(mDevice, &bufferDeviceAddressInfo);
                }
                copyInfo.dst = uploadInfo.accelerationStructure;
                copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
                dispatchTable.gvkCmdCopyMemoryToAccelerationStructureKHR(taskResources.vkCommandBuffer, &copyInfo);

                // End CommandBuffer
                gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

                // Submit CommandBuffer
                {
                    auto submitInfo = get_default<VkSubmitInfo>();
                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                    std::lock_guard<std::mutex> lock(mQueueMutex);
                    gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
                }

                // Hold this thread's execution until transfer is complete
                gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
                gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
            }
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
#if 0
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [uploadAccelerationStructure]() mutable { uploadAccelerationStructure(); });
    } else {
        uploadAccelerationStructure();
    }
#else
    uploadAccelerationStructure();
#endif
}

void CopyEngine::upload_ex(UploadAccelerationStructureInfo uploadInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(uploadInfo.accelerationStructure);
    assert(uploadInfo.pfnCallback);
    const auto& accelerationStructureCreateInfo = uploadInfo.accelerationStructureCreateInfo;
    const auto& accelerationStructureSerializationInfo = uploadInfo.accelerationStructureSerializationInfo;
    (void)accelerationStructureSerializationInfo;
    auto uploadAccelerationStructureEx = [=]() mutable
    {
        (void)accelerationStructureCreateInfo;
        // TODO : Need to handle host allocated acceleration structures
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            gvk_result(get_task_resources(uploadInfo.accelerationStructureSerializationInfo.size, &taskResources));

            // Map data, fire callback, unmap data
            uint8_t* pData = nullptr;
            gvk_result(mDevice.get<DispatchTable>().gvkMapMemory(mDevice, taskResources.memory, 0, VK_WHOLE_SIZE, 0, (void**)&pData));
            auto bindBufferMemoryInfo = get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = taskResources.buffer;
            bindBufferMemoryInfo.memory = taskResources.memory;
            uploadInfo.pfnCallback(uploadInfo, bindBufferMemoryInfo, pData);
            mDevice.get<DispatchTable>().gvkUnmapMemory(mDevice, taskResources.memory);

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // TODO : Documentation
            auto bufferCopy = get_default<VkBufferCopy>();
            bufferCopy.size = uploadInfo.accelerationStructureSerializationInfo.size;
            dispatchTable.gvkCmdCopyBuffer(taskResources.vkCommandBuffer, taskResources.buffer, uploadInfo.buffer, 1, &bufferCopy);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until the transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // Begin CommandBuffer
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // TODO : Documentation
            auto accelerationStructureCopy = get_default<VkCopyMemoryToAccelerationStructureInfoKHR>();
            accelerationStructureCopy.src.deviceAddress = uploadInfo.bufferDeviceAddress;
            accelerationStructureCopy.dst = uploadInfo.accelerationStructure;
            accelerationStructureCopy.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;
            dispatchTable.gvkCmdCopyMemoryToAccelerationStructureKHR(taskResources.vkCommandBuffer, &accelerationStructureCopy);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until the transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [uploadAccelerationStructureEx]() mutable { uploadAccelerationStructureEx(); });
    } else {
        uploadAccelerationStructureEx();
    }
}

void CopyEngine::transition_image_layouts(TransitionImageLayoutInfo transitionInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(transitionInfo.image);
    const auto& imageCreateInfo = transitionInfo.imageCreateInfo;
    const auto& imageSubresourceRange = transitionInfo.imageSubresourceRange;
    auto imageSubresourceCount = imageSubresourceRange.levelCount * imageSubresourceRange.layerCount;
    std::vector<VkImageLayout> oldImageLayouts(transitionInfo.pOldImageLayouts, transitionInfo.pOldImageLayouts + imageSubresourceCount);
    std::vector<VkImageLayout> newImageLayouts(transitionInfo.pNewImageLayouts, transitionInfo.pNewImageLayouts + imageSubresourceCount);
    auto transitionLayouts = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            gvk_result(get_task_resources(1, &taskResources));

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // Transition Image layouts
            std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
            imageMemoryBarriers.reserve(imageSubresourceCount);
            for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
                    auto subresource = arrayLayer * imageCreateInfo.mipLevels + mipLevel;
                    if (newImageLayouts[subresource]) {
                        auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                        imageMemoryBarrier.srcAccessMask = 0;
                        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        imageMemoryBarrier.oldLayout = oldImageLayouts[subresource];
                        imageMemoryBarrier.newLayout = newImageLayouts[subresource];
                        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imageMemoryBarrier.image = transitionInfo.image;
                        imageMemoryBarrier.subresourceRange = imageSubresourceRange;
                        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel;
                        imageMemoryBarrier.subresourceRange.levelCount = 1;
                        imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer;
                        imageMemoryBarrier.subresourceRange.layerCount = 1;
                        imageMemoryBarriers.push_back(imageMemoryBarrier);
                    }
                }
            }

            dispatchTable.gvkCmdPipelineBarrier(
                taskResources.vkCommandBuffer,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until transfer is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [transitionLayouts]() mutable { transitionLayouts(); });
    } else {
        transitionLayouts();
    }
}

void CopyEngine::build_acceleration_structure(BuildAcclerationStructureInfo buildInfo)
{
    assert(mDevice);
    assert(mQueue);
    assert(buildInfo.accelerationStructure);
    Auto<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometryInfo = buildInfo.buildGeometryInfo;
    std::vector<Auto<VkAccelerationStructureBuildRangeInfoKHR>> buildRangeInfos(buildGeometryInfo->geometryCount);
    for (uint32_t i = 0; i < buildGeometryInfo->geometryCount; ++i) {
        buildRangeInfos[i] = buildInfo.pBuildRangeInfos[i];
    }
    auto buildAccelerationStructure = [=]() mutable
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            // Get TaskResources
            TaskResources taskResources{ };
            gvk_result(get_task_resources(1, &taskResources));

            // Begin CommandBuffer
            const auto& dispatchTable = mDevice.get<DispatchTable>();
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            #if 0
            // TODO : VkAcclerationStructureKHR history
            std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfoPtrs(buildGeometryInfo->geometryCount);
            for (uint32_t i = 0; i < buildGeometryInfo->geometryCount; ++i) {
                buildRangeInfoPtrs[i] = &*buildRangeInfos[i];
            }
            // dispatchTable.gvkCmdBuildAccelerationStructuresKHR(taskResources.vkCommandBuffer, 1, &*buildGeometryInfo, buildRangeInfoPtrs.data());
            #endif

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until task is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));
        } gvk_result_scope_end;
        // TODO : Report errors
        assert(gvkResult == VK_SUCCESS);
    };
    if (mupThreadPool) {
        asio::post(*mupThreadPool, [buildAccelerationStructure]() mutable { buildAccelerationStructure(); });
    } else {
        buildAccelerationStructure();
    }
}

VkResult CopyEngine::get_acceleration_structure_serialization_size(VkAccelerationStructureKHR accelerationStructure, VkDeviceSize* pSize)
{
    assert(pSize);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // TODO : Documentation
        GvkStateTrackedObject stateTrackedAccelerationStructure{ };
        stateTrackedAccelerationStructure.type = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        stateTrackedAccelerationStructure.handle = (uint64_t)accelerationStructure;
        stateTrackedAccelerationStructure.dispatchableHandle = (uint64_t)mDevice.get<VkDevice>();

        // TODO : Documentation
        auto accelerationStructureCreateInfoType = get_stype<VkAccelerationStructureCreateInfoKHR>();
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{ };
        gvkGetStateTrackedObjectCreateInfo(&stateTrackedAccelerationStructure, &accelerationStructureCreateInfoType, (VkBaseOutStructure*)&accelerationStructureCreateInfo);
        gvk_result(accelerationStructureCreateInfoType == get_stype<VkAccelerationStructureCreateInfoKHR>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(accelerationStructureCreateInfo.sType == get_stype<VkAccelerationStructureCreateInfoKHR>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        GvkStateTrackedObject stateTrackedBuffer{ };
        stateTrackedBuffer.type = VK_OBJECT_TYPE_BUFFER;
        stateTrackedBuffer.handle = (uint64_t)accelerationStructureCreateInfo.buffer;
        stateTrackedBuffer.dispatchableHandle = (uint64_t)mDevice.get<VkDevice>();

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
        gvkEnumerateStateTrackedObjectBindings(&stateTrackedBuffer, &enumerateInfo);
        gvk_result(bindBufferMemoryInfos.size() == 1 ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        GvkStateTrackedObject stateTrackedDeviceMemory{ };
        stateTrackedDeviceMemory.type = VK_OBJECT_TYPE_DEVICE_MEMORY;
        stateTrackedDeviceMemory.handle = (uint64_t)bindBufferMemoryInfos[0].memory;
        stateTrackedDeviceMemory.dispatchableHandle = stateTrackedBuffer.dispatchableHandle;

        // TODO : Documentation
        auto memoryAllocateInfoType = get_stype<VkMemoryAllocateInfo>();
        VkMemoryAllocateInfo memoryAllocateInfo{ };
        gvkGetStateTrackedObjectAllocateInfo(&stateTrackedDeviceMemory, &memoryAllocateInfoType, (VkBaseOutStructure*)&memoryAllocateInfo);
        gvk_result(memoryAllocateInfoType == get_stype<VkMemoryAllocateInfo>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(memoryAllocateInfo.sType == get_stype<VkMemoryAllocateInfo>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        const auto& physicalDevice = mDevice.get<PhysicalDevice>();
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{ };
        physicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

        // TODO : Documentation
        const auto& dispatchTable = mDevice.get<DispatchTable>();

        // TODO : Documentation
        auto queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
        gvk_result(memoryAllocateInfo.memoryTypeIndex < physicalDeviceMemoryProperties.memoryTypeCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto memoryTypePropertyFlags = physicalDeviceMemoryProperties.memoryTypes[memoryAllocateInfo.memoryTypeIndex].propertyFlags;
        if (memoryTypePropertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            // Get TaskResources
            AccelerationStructureTaskResources taskResources{ };
            gvk_result(get_acceleration_structure_task_resources(0, &taskResources));

            // Begin CommandBuffer
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(dispatchTable.gvkBeginCommandBuffer(taskResources.vkCommandBuffer, &commandBufferBeginInfo));

            // TODO : Documentation
            dispatchTable.gvkCmdResetQueryPool(taskResources.vkCommandBuffer, taskResources.queryPool, 0, 1);
            dispatchTable.gvkCmdWriteAccelerationStructuresPropertiesKHR(taskResources.vkCommandBuffer, 1, &accelerationStructure, queryType, taskResources.queryPool, 0);

            // End CommandBuffer
            gvk_result(dispatchTable.gvkEndCommandBuffer(taskResources.vkCommandBuffer));

            // Submit CommandBuffer
            {
                auto submitInfo = get_default<VkSubmitInfo>();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &taskResources.vkCommandBuffer;
                std::lock_guard<std::mutex> lock(mQueueMutex);
                gvk_result(dispatchTable.gvkQueueSubmit(mQueue, 1, &submitInfo, taskResources.fence));
            }

            // Hold this thread's execution until the query is complete
            gvk_result(dispatchTable.gvkWaitForFences(mDevice, 1, &taskResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
            gvk_result(dispatchTable.gvkResetFences(mDevice, 1, &taskResources.fence.get<VkFence>()));

            // TODO : Documentation
            auto queryResultFlags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;
            gvk_result(dispatchTable.gvkGetQueryPoolResults(mDevice, taskResources.queryPool, 0, 1, sizeof(*pSize), pSize, sizeof(*pSize), queryResultFlags));
        } else if (memoryTypePropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            gvk_result(dispatchTable.gvkWriteAccelerationStructuresPropertiesKHR(mDevice, 1, &accelerationStructure, queryType, sizeof(*pSize), &pSize, sizeof(*pSize)));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CopyEngine::get_task_resources(VkDeviceSize taskSize, TaskResources* pTaskResources)
{
    assert(mDevice);
    assert(taskSize);
    assert(pTaskResources);
    std::unique_lock<std::mutex> lock(mTaskResourcesMutex);
    auto taskResourcesItr = mTaskResources.find(std::this_thread::get_id());
    if (taskResourcesItr == mTaskResources.end()) {
        taskResourcesItr = mTaskResources.insert({ std::this_thread::get_id(), { }}).first;
        if (mpfnInitializeThreadCallback /* && mupThreadPool */) {
            mpfnInitializeThreadCallback();
        }
    }
    lock.unlock();
    gvk_result_scope_begin(VK_SUCCESS) {
        // HACK : TODO : Documentation
        DispatchTable applicationDispatchTable{ };
        DispatchTable::load_global_entry_points(&applicationDispatchTable);
        DispatchTable::load_instance_entry_points(mDevice.get<PhysicalDevice>().get<VkInstance>(), &applicationDispatchTable);
        DispatchTable::load_device_entry_points(mDevice, &applicationDispatchTable);
        const auto& layerDeviceDispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(mDevice.get<VkDevice>()));
        assert(layerDeviceDispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerDeviceDispatchTable = layerDeviceDispatchTableItr->second;
        const auto& layerInstanceDispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(mDevice.get<PhysicalDevice>().get<VkInstance>()));
        assert(layerInstanceDispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerInstanceDispatchTable = layerInstanceDispatchTableItr->second;

        if (!taskResourcesItr->second.buffer || taskResourcesItr->second.buffer.get<VkBufferCreateInfo>().size < taskSize) {
            auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
            bufferCreateInfo.size = taskSize;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            gvk_result(Buffer::create(mDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &taskResourcesItr->second.buffer));

            // HACK : TODO : Documentation
            VkBuffer proxyBuffer = VK_NULL_HANDLE;
            gvk_result(layerDeviceDispatchTable.gvkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &proxyBuffer));
            VkMemoryRequirements memoryRequirements{ };
            layerDeviceDispatchTable.gvkGetBufferMemoryRequirements(mDevice, proxyBuffer, &memoryRequirements);
            VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{ };
            VkPhysicalDevice stateTrackerPhysicalDevice = VK_NULL_HANDLE;
            gvkGetStateTrackerPhysicalDevice(mDevice.get<PhysicalDevice>().get<VkInstance>(), mDevice.get<PhysicalDevice>(), &stateTrackerPhysicalDevice);
            auto physicalDevice = stateTrackerPhysicalDevice ? stateTrackerPhysicalDevice : mDevice.get<PhysicalDevice>().get<VkPhysicalDevice>();
            layerInstanceDispatchTable.gvkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
            // mDevice.get<DispatchTable>().gvkGetBufferMemoryRequirements(mDevice, proxyBuffer, &memoryRequirements);
            layerDeviceDispatchTable.gvkDestroyBuffer(mDevice, proxyBuffer, nullptr);

            // TODO : Documentation
            auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            uint32_t memoryTypeCount = 0;
            auto memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
            gvk_result(memoryTypeCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            memoryTypeCount = 1;
            get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryAllocateInfo.memoryTypeIndex);
            gvk_result(DeviceMemory::allocate(mDevice, &memoryAllocateInfo, nullptr, &taskResourcesItr->second.memory));
            gvk_result(mDevice.get<DispatchTable>().gvkBindBufferMemory(mDevice, taskResourcesItr->second.buffer, taskResourcesItr->second.memory, 0));
        }
        if (!taskResourcesItr->second.fence) {
            gvk_result(Fence::create(mDevice, &get_default<VkFenceCreateInfo>(), nullptr, &taskResourcesItr->second.fence));
        }
        if (!taskResourcesItr->second.vkCommandBuffer) {
            auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = mQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
            gvk_result(CommandPool::create(mDevice, &commandPoolCreateInfo, nullptr, &taskResourcesItr->second.commandPool));
            auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
            commandBufferAllocateInfo.commandPool = taskResourcesItr->second.commandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;
            gvk_result(mDevice.get<DispatchTable>().gvkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &taskResourcesItr->second.vkCommandBuffer));
            // HACK : TODO : Documentation
            if (mDevice.get<DispatchTable>().gvkAllocateCommandBuffers == applicationDispatchTable.gvkAllocateCommandBuffers ||
                mDevice.get<DispatchTable>().gvkAllocateCommandBuffers == layerDeviceDispatchTable.gvkAllocateCommandBuffers) {
                *(void**)taskResourcesItr->second.vkCommandBuffer = *(void**)mDevice.get<VkDevice>();
            }
        }
        *pTaskResources = taskResourcesItr->second;
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CopyEngine::get_acceleration_structure_task_resources(VkDeviceSize taskSize, AccelerationStructureTaskResources* pTaskResources)
{
    assert(mDevice);
    assert(pTaskResources);
    std::unique_lock<std::mutex> lock(mTaskResourcesMutex);
    auto taskResourcesItr = mAccelerationStructureTaskResources.find(std::this_thread::get_id());
    if (taskResourcesItr == mAccelerationStructureTaskResources.end()) {
        taskResourcesItr = mAccelerationStructureTaskResources.insert({ std::this_thread::get_id(), { }}).first;
#if 0
        if (mpfnInitializeThreadCallback /* && mupThreadPool */) {
            mpfnInitializeThreadCallback();
        }
#endif
    }
    lock.unlock();
    gvk_result_scope_begin(VK_SUCCESS) {
        // HACK : TODO : Documentation
        DispatchTable applicationDispatchTable{ };
        DispatchTable::load_global_entry_points(&applicationDispatchTable);
        DispatchTable::load_instance_entry_points(mDevice.get<PhysicalDevice>().get<VkInstance>(), &applicationDispatchTable);
        DispatchTable::load_device_entry_points(mDevice, &applicationDispatchTable);
        const auto& layerDeviceDispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(mDevice.get<VkDevice>()));
        assert(layerDeviceDispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerDeviceDispatchTable = layerDeviceDispatchTableItr->second;
        const auto& layerInstanceDispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(mDevice.get<PhysicalDevice>().get<VkInstance>()));
        assert(layerInstanceDispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerInstanceDispatchTable = layerInstanceDispatchTableItr->second;

        if (taskSize && (!taskResourcesItr->second.buffer || taskResourcesItr->second.buffer.get<VkBufferCreateInfo>().size < taskSize)) {
            assert(!mAccelerationStrcutureSerializationInfoRetrieved);
            auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
            bufferCreateInfo.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
            bufferCreateInfo.size = taskSize;
            bufferCreateInfo.usage =
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            gvk_result(Buffer::create(mDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &taskResourcesItr->second.buffer));

            // HACK : TODO : Documentation
            VkBuffer proxyBuffer = VK_NULL_HANDLE;
            gvk_result(layerDeviceDispatchTable.gvkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &proxyBuffer));
            VkMemoryRequirements memoryRequirements{ };
            layerDeviceDispatchTable.gvkGetBufferMemoryRequirements(mDevice, proxyBuffer, &memoryRequirements);
            VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{ };
            VkPhysicalDevice stateTrackerPhysicalDevice = VK_NULL_HANDLE;
            gvkGetStateTrackerPhysicalDevice(mDevice.get<PhysicalDevice>().get<VkInstance>(), mDevice.get<PhysicalDevice>(), &stateTrackerPhysicalDevice);
            auto physicalDevice = stateTrackerPhysicalDevice ? stateTrackerPhysicalDevice : mDevice.get<PhysicalDevice>().get<VkPhysicalDevice>();
            layerInstanceDispatchTable.gvkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
            // mDevice.get<DispatchTable>().gvkGetBufferMemoryRequirements(mDevice, proxyBuffer, &memoryRequirements);
            layerDeviceDispatchTable.gvkDestroyBuffer(mDevice, proxyBuffer, nullptr);

            // TODO : This isn't state tracked from here so need special handling
            auto memoryAllocateFlagsInfo = get_default<VkMemoryAllocateFlagsInfo>();
            memoryAllocateFlagsInfo.flags =
                VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT |
                VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;

            // TODO : Documentation
            auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
            memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            uint32_t memoryTypeCount = 0;
            auto memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
            gvk_result(memoryTypeCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            memoryTypeCount = 1;
            get_compatible_memory_type_indices(&physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryAllocateInfo.memoryTypeIndex);
            gvk_result(DeviceMemory::allocate(mDevice, &memoryAllocateInfo, nullptr, &taskResourcesItr->second.memory));
            gvk_result(mDevice.get<DispatchTable>().gvkBindBufferMemory(mDevice, taskResourcesItr->second.buffer, taskResourcesItr->second.memory, 0));
        }
        if (!taskResourcesItr->second.fence) {
            assert(!mAccelerationStrcutureSerializationInfoRetrieved);
            gvk_result(Fence::create(mDevice, &get_default<VkFenceCreateInfo>(), nullptr, &taskResourcesItr->second.fence));
        }
        if (!taskResourcesItr->second.vkCommandBuffer) {
            assert(!mAccelerationStrcutureSerializationInfoRetrieved);
            auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = mQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
            gvk_result(CommandPool::create(mDevice, &commandPoolCreateInfo, nullptr, &taskResourcesItr->second.commandPool));
            auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
            commandBufferAllocateInfo.commandPool = taskResourcesItr->second.commandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;
            gvk_result(mDevice.get<DispatchTable>().gvkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &taskResourcesItr->second.vkCommandBuffer));
            // HACK : TODO : Documentation
            if (mDevice.get<DispatchTable>().gvkAllocateCommandBuffers == applicationDispatchTable.gvkAllocateCommandBuffers ||
                mDevice.get<DispatchTable>().gvkAllocateCommandBuffers == layerDeviceDispatchTable.gvkAllocateCommandBuffers) {
                *(void**)taskResourcesItr->second.vkCommandBuffer = *(void**)mDevice.get<VkDevice>();
            }
        }
        if (!taskResourcesItr->second.queryPool) {
            assert(!mAccelerationStrcutureSerializationInfoRetrieved);
            auto queryPoolCreateInfo = get_default<VkQueryPoolCreateInfo>();
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
            queryPoolCreateInfo.queryCount = 1;
            gvk_result(QueryPool::create(mDevice, &queryPoolCreateInfo, nullptr, &taskResourcesItr->second.queryPool));
        }
        *pTaskResources = taskResourcesItr->second;
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CopyEngine::get_acceleration_structure_task_resources(const GvkAccelerationStructureSerilizationInfoKHR& accelerationStructureSerializationInfo, AccelerationStructureTaskResources* pTaskResources)
{
    assert(mDevice);
    assert(pTaskResources);
    std::unique_lock<std::mutex> lock(mTaskResourcesMutex);
    auto taskResourcesItr = mAccelerationStructureTaskResources.find(std::this_thread::get_id());
    if (taskResourcesItr == mAccelerationStructureTaskResources.end()) {
        taskResourcesItr = mAccelerationStructureTaskResources.insert({ std::this_thread::get_id(), { }}).first;
#if 0
        if (mpfnInitializeThreadCallback /* && mupThreadPool */) {
            mpfnInitializeThreadCallback();
        }
#endif
    }
    lock.unlock();
    gvk_result_scope_begin(VK_SUCCESS) {
        // HACK : TODO : Documentation
        DispatchTable applicationDispatchTable{ };
        DispatchTable::load_global_entry_points(&applicationDispatchTable);
        DispatchTable::load_instance_entry_points(mDevice.get<PhysicalDevice>().get<VkInstance>(), &applicationDispatchTable);
        DispatchTable::load_device_entry_points(mDevice, &applicationDispatchTable);
        const auto& layerDeviceDispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(mDevice.get<VkDevice>()));
        assert(layerDeviceDispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerDeviceDispatchTable = layerDeviceDispatchTableItr->second;
        const auto& layerInstanceDispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(mDevice.get<PhysicalDevice>().get<VkInstance>()));
        assert(layerInstanceDispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        const auto& layerInstanceDispatchTable = layerInstanceDispatchTableItr->second;

        if (!taskResourcesItr->second.buffer) {
            // TODO : Documentation
            gvk_result(Buffer::create(mDevice, accelerationStructureSerializationInfo.pBufferCreateInfo, (VkAllocationCallbacks*)nullptr, &taskResourcesItr->second.buffer));

            // HACK : TODO : Documentation
            VkBuffer proxyBuffer = VK_NULL_HANDLE;
            gvk_result(layerDeviceDispatchTable.gvkCreateBuffer(mDevice, accelerationStructureSerializationInfo.pBufferCreateInfo, nullptr, &proxyBuffer));
            VkMemoryRequirements memoryRequirements{ };
            layerDeviceDispatchTable.gvkGetBufferMemoryRequirements(mDevice, proxyBuffer, &memoryRequirements);
            VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{ };
            VkPhysicalDevice stateTrackerPhysicalDevice = VK_NULL_HANDLE;
            gvkGetStateTrackerPhysicalDevice(mDevice.get<PhysicalDevice>().get<VkInstance>(), mDevice.get<PhysicalDevice>(), &stateTrackerPhysicalDevice);
            auto physicalDevice = stateTrackerPhysicalDevice ? stateTrackerPhysicalDevice : mDevice.get<PhysicalDevice>().get<VkPhysicalDevice>();
            layerInstanceDispatchTable.gvkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
            // mDevice.get<DispatchTable>().gvkGetBufferMemoryRequirements(mDevice, proxyBuffer, &memoryRequirements);
            layerDeviceDispatchTable.gvkDestroyBuffer(mDevice, proxyBuffer, nullptr);

            // TODO : Documentation
            gvk_result(DeviceMemory::allocate(mDevice, accelerationStructureSerializationInfo.pMemoryAllocateInfo, nullptr, &taskResourcesItr->second.memory));
            gvk_result(mDevice.get<DispatchTable>().gvkBindBufferMemory(mDevice, taskResourcesItr->second.buffer, taskResourcesItr->second.memory, 0));
        } else {
            // TODO : Validate resource sizes
        }
        if (!taskResourcesItr->second.fence) {
            gvk_result(Fence::create(mDevice, &get_default<VkFenceCreateInfo>(), nullptr, &taskResourcesItr->second.fence));
        }
        if (!taskResourcesItr->second.vkCommandBuffer) {
            auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = mQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
            gvk_result(CommandPool::create(mDevice, &commandPoolCreateInfo, nullptr, &taskResourcesItr->second.commandPool));
            auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
            commandBufferAllocateInfo.commandPool = taskResourcesItr->second.commandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;
            gvk_result(mDevice.get<DispatchTable>().gvkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &taskResourcesItr->second.vkCommandBuffer));
            // HACK : TODO : Documentation
            if (mDevice.get<DispatchTable>().gvkAllocateCommandBuffers == applicationDispatchTable.gvkAllocateCommandBuffers ||
                mDevice.get<DispatchTable>().gvkAllocateCommandBuffers == layerDeviceDispatchTable.gvkAllocateCommandBuffers) {
                *(void**)taskResourcesItr->second.vkCommandBuffer = *(void**)mDevice.get<VkDevice>();
            }
        }
        if (!taskResourcesItr->second.queryPool) {
            auto queryPoolCreateInfo = get_default<VkQueryPoolCreateInfo>();
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
            queryPoolCreateInfo.queryCount = 1;
            gvk_result(QueryPool::create(mDevice, &queryPoolCreateInfo, nullptr, &taskResourcesItr->second.queryPool));
        }
        *pTaskResources = taskResourcesItr->second;
    } gvk_result_scope_end;
    return gvkResult;
}

uint32_t get_image_data_size(const VkImageCreateInfo& imageCreateInfo, const VkImageSubresourceRange& imageSubresourceRange)
{
    assert(imageSubresourceRange.baseMipLevel + imageSubresourceRange.levelCount <= imageCreateInfo.mipLevels);
    assert(imageSubresourceRange.baseArrayLayer + imageSubresourceRange.layerCount <= imageCreateInfo.arrayLayers);
    return get_image_data_size(imageCreateInfo.format, imageCreateInfo.extent, imageSubresourceRange);
}

uint32_t get_image_data_size(VkFormat format, const VkExtent3D& extent, const VkImageSubresourceRange& imageSubresourceRange)
{
    // TODO : Revisit get_bytes_per_texel() for compressed formats...
    uint32_t imageDataSize = 0;
    GvkFormatInfo formatInfo{ };
    get_format_info(format, &formatInfo);
    auto imageAspectFlags = get_image_aspect_flags(format);
    for (uint32_t arrayLayer = imageSubresourceRange.baseArrayLayer; arrayLayer < imageSubresourceRange.layerCount; ++arrayLayer) {
        for (uint32_t mipLevel = imageSubresourceRange.baseMipLevel; mipLevel < imageSubresourceRange.levelCount; ++mipLevel) {
            auto mipLevelExtent = get_mip_level_extent(extent, mipLevel);
            if (!formatInfo.compressionType) {
                imageDataSize += get_bytes_per_texel(format) * mipLevelExtent.width * mipLevelExtent.height * mipLevelExtent.depth * imageSubresourceRange.layerCount;
            } else {
                auto blockCount = (mipLevelExtent.width / formatInfo.blockExtent[0]) * (mipLevelExtent.height / formatInfo.blockExtent[1]) * (mipLevelExtent.depth / formatInfo.blockExtent[2]);
                imageDataSize += blockCount * formatInfo.blockSize * imageSubresourceRange.layerCount;
            }
            // FROM : https://vulkan.lunarg.com/doc/view/1.3.261.1/windows/1.3-extensions/vkspec.html#VUID-vkCmdCopyImageToBuffer-srcImage-07978
            //  If srcImage has a depth/stencil format, the bufferOffset member of any element of pRegions must be a multiple of 4
            if (imageAspectFlags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                imageDataSize += imageDataSize % 4;
            }
        }
    }
    return imageDataSize;
}

} // namespace restore_point
} // namespace gvk

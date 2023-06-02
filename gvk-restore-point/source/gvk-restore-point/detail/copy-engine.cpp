
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

#include "gvk-restore-point/detail/copy-engine.hpp"
#include "gvk-format-info.hpp"

#include <cassert>
#include <fstream>

namespace gvk {

CopyEngine::~CopyEngine()
{
    reset();
}

void CopyEngine::reset()
{
    wait();
    mMultiThreaded = true;
    mInitializedThreads.clear();
    mOnThreadInitialized = nullptr;
    mTaskResources.clear();
    mDispatchTable = { };
}

void CopyEngine::disable_multi_threading()
{
    mMultiThreaded = false;
}

void CopyEngine::set_thread_initialization_callback(std::function<void(std::thread::id)> onThreadInitialized)
{
    mOnThreadInitialized = onThreadInitialized;
}

void CopyEngine::download(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo)
{
    if (mMultiThreaded) {
        asio::post(
            mThreadPool,
            [this, vkDevice, deviceMemoryCopyInfo]()
            {
                initialize_thread();
                download_impl(vkDevice, deviceMemoryCopyInfo);
            }
        );
    } else {
        download_impl(vkDevice, deviceMemoryCopyInfo);
    }
}

void CopyEngine::upload(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo)
{
    if (mMultiThreaded) {
        asio::post(
            mThreadPool,
            [this, vkDevice, deviceMemoryCopyInfo]()
            {
                initialize_thread();
                upload_impl(vkDevice, deviceMemoryCopyInfo);
            }
        );
    } else {
        upload_impl(vkDevice, deviceMemoryCopyInfo);
    }
}

void CopyEngine::transition_image_layouts(VkDevice vkDevice, ImageCopyInfo imageCopyInfo)
{
    if (mMultiThreaded) {
        asio::post(
            mThreadPool,
            [this, vkDevice, imageCopyInfo]()
            {
                initialize_thread();
                transition_image_layouts_impl(vkDevice, imageCopyInfo);
            }
        );
    } else {
        transition_image_layouts_impl(vkDevice, imageCopyInfo);
    }
}

void CopyEngine::wait()
{
    if (mMultiThreaded) {
        mThreadPool.wait();
    }
    for (const auto& itr : mTaskResources) {
        const auto& gvkDevice = itr.first;
        auto vkResult = gvkDevice.get<DispatchTable>().gvkDeviceWaitIdle(gvkDevice);
        assert(vkResult == VK_SUCCESS);
        (void)vkResult;
    }
}

void CopyEngine::initialize_thread()
{
    std::lock_guard<std::mutex> lock(mTaskResourceMutex);
    if (mInitializedThreads.insert(std::this_thread::get_id()).second && mOnThreadInitialized) {
        mOnThreadInitialized(std::this_thread::get_id());
    }
}

const CopyEngine::TaskResources& CopyEngine::get_task_resources(const Device& gvkDevice, VkDeviceSize taskSize)
{
    assert(gvkDevice);
    auto gvkQueue = get_queue_family(gvkDevice, 0).queues[0];
    assert(gvkQueue);
    std::unique_lock<std::mutex> lock(mTaskResourceMutex);
    auto& taskResources = mTaskResources[gvkDevice][std::this_thread::get_id()];
    lock.unlock();
    assert(!taskResources.gvkCommandBuffer == !taskResources.gvkFence);
    if (!taskResources.gvkCommandBuffer) {
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        CommandPool gvkCommandPool;
        auto vkResult = CommandPool::create(gvkDevice, &commandPoolCreateInfo, nullptr, &gvkCommandPool);
        assert(vkResult == VK_SUCCESS);
        (void)vkResult;

        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = gvkCommandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        vkResult = CommandBuffer::allocate(gvkDevice, &commandBufferAllocateInfo, &taskResources.gvkCommandBuffer);
        assert(vkResult == VK_SUCCESS);
        vkResult = Fence::create(gvkDevice, &get_default<VkFenceCreateInfo>(), nullptr, &taskResources.gvkFence);
        assert(vkResult == VK_SUCCESS);
    }
    if (!taskResources.gvkBuffer || taskResources.gvkBuffer.get<VkBufferCreateInfo>().size < taskSize) {
        auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = taskSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        auto vkResult = Buffer::create(gvkDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &taskResources.gvkBuffer);
        assert(vkResult == VK_SUCCESS);
        (void)vkResult;

        auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
        memoryAllocateInfo.allocationSize = taskSize;
        uint32_t memoryTypeCount = 0;
        auto memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkMemoryRequirements memoryRequirements { };
        gvkDevice.get<DispatchTable>().gvkGetBufferMemoryRequirements(gvkDevice, taskResources.gvkBuffer, &memoryRequirements);
        gvk::get_compatible_memory_type_indices(gvkDevice.get<gvk::PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
        assert(memoryTypeCount);
        memoryTypeCount = 1;
        gvk::get_compatible_memory_type_indices(gvkDevice.get<gvk::PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryAllocateInfo.memoryTypeIndex);
        vkResult = DeviceMemory::allocate(gvkDevice, &memoryAllocateInfo, nullptr, &taskResources.gvkDeviceMemory);
        assert(vkResult == VK_SUCCESS);

        vkResult = gvkDevice.get<DispatchTable>().gvkBindBufferMemory(gvkDevice, taskResources.gvkBuffer, taskResources.gvkDeviceMemory, 0);
        assert(vkResult == VK_SUCCESS);
    }
    return taskResources;
}

void CopyEngine::download_impl(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo)
{
    Device gvkDevice(vkDevice);
    assert(gvkDevice);
    const auto& dispatchTable = gvkDevice.get<DispatchTable>();
    const auto& taskResources = get_task_resources(gvkDevice, deviceMemoryCopyInfo.allocateInfo->allocationSize);

    auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
    bufferCreateInfo.size = deviceMemoryCopyInfo.allocateInfo->allocationSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    Buffer gvkBuffer;
    auto vkResult = Buffer::create(gvkDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &gvkBuffer);
    assert(vkResult == VK_SUCCESS);
    assert(dispatchTable.gvkBindBufferMemory);
    vkResult = dispatchTable.gvkBindBufferMemory(vkDevice, gvkBuffer, deviceMemoryCopyInfo.vkDeviceMemory, 0);
    assert(vkResult == VK_SUCCESS);
    (void)vkResult;

    auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkResult = dispatchTable.gvkBeginCommandBuffer(taskResources.gvkCommandBuffer, &commandBufferBeginInfo);
    assert(vkResult == VK_SUCCESS);

    auto bufferCopy = get_default<VkBufferCopy>();
    bufferCopy.size = deviceMemoryCopyInfo.allocateInfo->allocationSize;
    dispatchTable.gvkCmdCopyBuffer(taskResources.gvkCommandBuffer, gvkBuffer, taskResources.gvkBuffer, 1, &bufferCopy);

    vkResult = dispatchTable.gvkEndCommandBuffer(taskResources.gvkCommandBuffer);
    assert(vkResult == VK_SUCCESS);

    auto submitInfo = get_default<VkSubmitInfo>();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &taskResources.gvkCommandBuffer.get<const VkCommandBuffer&>();
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        vkResult = dispatchTable.gvkQueueSubmit(get_queue_family(gvkDevice, 0).queues[0], 1, &submitInfo, taskResources.gvkFence);
        assert(vkResult == VK_SUCCESS);
    }

    vkResult = dispatchTable.gvkWaitForFences(gvkDevice, 1, &taskResources.gvkFence.get<const VkFence&>(), VK_TRUE, UINT64_MAX);
    assert(vkResult == VK_SUCCESS);
    vkResult = dispatchTable.gvkResetFences(gvkDevice, 1, &taskResources.gvkFence.get<const VkFence&>());
    assert(vkResult == VK_SUCCESS);

    const char* pData = nullptr;
    vkResult = dispatchTable.gvkMapMemory(gvkDevice, taskResources.gvkDeviceMemory, 0, VK_WHOLE_SIZE, 0, (void**)&pData);
    assert(vkResult == VK_SUCCESS);
    auto path = deviceMemoryCopyInfo.path / "VkDeviceMemory";
    std::filesystem::create_directories(path);
    auto binPath = (path / to_hex_string(deviceMemoryCopyInfo.vkDeviceMemory)).replace_extension("bin");
    std::ofstream file(binPath, std::ios::binary);
    assert(file.is_open());
    file.write(pData, deviceMemoryCopyInfo.allocateInfo->allocationSize);
    if (deviceMemoryCopyInfo.onProcessData) {
        deviceMemoryCopyInfo.onProcessData(vkDevice, deviceMemoryCopyInfo.vkDeviceMemory, *deviceMemoryCopyInfo.allocateInfo, (void*)pData);
    }
    dispatchTable.gvkUnmapMemory(gvkDevice, taskResources.gvkDeviceMemory);
}

void CopyEngine::upload_impl(VkDevice vkDevice, DeviceMemoryCopyInfo deviceMemoryCopyInfo)
{
    if (deviceMemoryCopyInfo.onProcessData) {
        deviceMemoryCopyInfo.onProcessData(vkDevice, deviceMemoryCopyInfo.vkDeviceMemory, *deviceMemoryCopyInfo.allocateInfo, nullptr);
    } else {
        Device gvkDevice(vkDevice);
        assert(gvkDevice);
        const auto& dispatchTable = gvkDevice.get<DispatchTable>();
        const auto& taskResources = get_task_resources(gvkDevice, deviceMemoryCopyInfo.allocateInfo->allocationSize);

        char* pData = nullptr;
        auto vkResult = dispatchTable.gvkMapMemory(gvkDevice, taskResources.gvkDeviceMemory, 0, VK_WHOLE_SIZE, 0, (void**)&pData);
        assert(vkResult == VK_SUCCESS);
        (void)vkResult;

        std::ifstream file(deviceMemoryCopyInfo.path, std::ios::binary);
        assert(file.is_open());
        file.read(pData, deviceMemoryCopyInfo.allocateInfo->allocationSize);
        dispatchTable.gvkUnmapMemory(gvkDevice, taskResources.gvkDeviceMemory);

        auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = deviceMemoryCopyInfo.allocateInfo->allocationSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        Buffer gvkBuffer;
        vkResult = Buffer::create(gvkDevice, &bufferCreateInfo, (VkAllocationCallbacks*)nullptr, &gvkBuffer);
        assert(vkResult == VK_SUCCESS);
        assert(dispatchTable.gvkBindBufferMemory);
        vkResult = dispatchTable.gvkBindBufferMemory(vkDevice, gvkBuffer, deviceMemoryCopyInfo.vkDeviceMemory, 0);
        assert(vkResult == VK_SUCCESS);
        (void)vkResult;

        auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkResult = dispatchTable.gvkBeginCommandBuffer(taskResources.gvkCommandBuffer, &commandBufferBeginInfo);
        assert(vkResult == VK_SUCCESS);

        auto bufferCopy = get_default<VkBufferCopy>();
        bufferCopy.size = deviceMemoryCopyInfo.allocateInfo->allocationSize;
        dispatchTable.gvkCmdCopyBuffer(taskResources.gvkCommandBuffer, taskResources.gvkBuffer, gvkBuffer, 1, &bufferCopy);

        vkResult = dispatchTable.gvkEndCommandBuffer(taskResources.gvkCommandBuffer);
        assert(vkResult == VK_SUCCESS);

        auto submitInfo = get_default<VkSubmitInfo>();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &taskResources.gvkCommandBuffer.get<const VkCommandBuffer&>();
        {
            std::lock_guard<std::mutex> lock(mQueueMutex);
            vkResult = dispatchTable.gvkQueueSubmit(get_queue_family(gvkDevice, 0).queues[0], 1, &submitInfo, taskResources.gvkFence);
            assert(vkResult == VK_SUCCESS);
        }

        vkResult = dispatchTable.gvkWaitForFences(gvkDevice, 1, &taskResources.gvkFence.get<const VkFence&>(), VK_TRUE, UINT64_MAX);
        assert(vkResult == VK_SUCCESS);
        vkResult = dispatchTable.gvkResetFences(gvkDevice, 1, &taskResources.gvkFence.get<const VkFence&>());
        assert(vkResult == VK_SUCCESS);
    }
}

void CopyEngine::transition_image_layouts_impl(VkDevice vkDevice, ImageCopyInfo imageCopyInfo)
{
    if (imageCopyInfo.onProcessLayouts) {
        imageCopyInfo.onProcessLayouts(imageCopyInfo.vkImage);
    } else {
        auto arrayLayerCount = imageCopyInfo.createInfo->arrayLayers;
        auto mipLevelCount = imageCopyInfo.createInfo->mipLevels;
        std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
        imageMemoryBarriers.reserve(arrayLayerCount * mipLevelCount);
        for (uint32_t arrayLayer_i = 0; arrayLayer_i < arrayLayerCount; ++arrayLayer_i) {
            for (uint32_t mipLevel_i = 0; mipLevel_i < mipLevelCount; ++mipLevel_i) {
                auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                imageMemoryBarrier.oldLayout = imageCopyInfo.createInfo->initialLayout;
                imageMemoryBarrier.newLayout = imageCopyInfo.newImageLayouts[arrayLayer_i * mipLevelCount + mipLevel_i];
                if (imageMemoryBarrier.oldLayout != imageMemoryBarrier.newLayout) {
                    imageMemoryBarrier.image = imageCopyInfo.vkImage;
                    imageMemoryBarrier.subresourceRange.aspectMask = get_image_aspect_flags(imageCopyInfo.createInfo->format);
                    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel_i;
                    imageMemoryBarrier.subresourceRange.levelCount = 1;
                    imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer_i;
                    imageMemoryBarrier.subresourceRange.layerCount = 1;
                    imageMemoryBarriers.push_back(imageMemoryBarrier);
                }
            }
        }
        if (!imageMemoryBarriers.empty()) {
            Device gvkDevice(vkDevice);
            assert(gvkDevice);
            auto dispatchTable = gvkDevice.get<DispatchTable>();

            auto& taskResources = get_task_resources(gvkDevice, 1);
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            assert(dispatchTable.gvkBeginCommandBuffer);
            auto vkResult = dispatchTable.gvkBeginCommandBuffer(taskResources.gvkCommandBuffer, &commandBufferBeginInfo);
            assert(vkResult == VK_SUCCESS);
            (void)vkResult;

            dispatchTable.gvkCmdPipelineBarrier(
                taskResources.gvkCommandBuffer,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)imageMemoryBarriers.size(),
                imageMemoryBarriers.data()
            );

            vkResult = dispatchTable.gvkEndCommandBuffer(taskResources.gvkCommandBuffer);
            assert(vkResult == VK_SUCCESS);

            auto submitInfo = get_default<VkSubmitInfo>();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &taskResources.gvkCommandBuffer.get<const VkCommandBuffer&>();
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                vkResult = dispatchTable.gvkQueueSubmit(get_queue_family(gvkDevice, 0).queues[0], 1, &submitInfo, taskResources.gvkFence);
                assert(vkResult == VK_SUCCESS);
            }

            assert(dispatchTable.gvkWaitForFences);
            vkResult = dispatchTable.gvkWaitForFences(gvkDevice, 1, &taskResources.gvkFence.get<const VkFence&>(), VK_TRUE, UINT64_MAX);
            assert(dispatchTable.gvkResetFences);
            vkResult = dispatchTable.gvkResetFences(gvkDevice, 1, &taskResources.gvkFence.get<const VkFence&>());
            assert(vkResult == VK_SUCCESS);
        }
    }
}

} // namespace gvk


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

#include "gvk-virtual-swapchain/layer.hpp"
#include "gvk-handles/utilities.hpp"

#include <utility>

namespace gvk {
namespace virtual_swapchain {

Swapchain::Swapchain(Swapchain&& other)
{
    *this = std::move(other);
}

Swapchain& Swapchain::operator=(Swapchain&& other)
{
    if (this != &other) {
        mGvkDevice = std::move(other.mGvkDevice);
        mVkSwapchain = std::exchange(other.mVkSwapchain, VK_NULL_HANDLE);
        mExtent = std::move(other.mExtent);
        mGvkDeviceMemory = std::move(other.mGvkDeviceMemory);
        mActualImages = std::move(other.mActualImages);
        mImageLayouts = std::move(other.mImageLayouts);
        mVirtualVkImages = std::move(other.mVirtualVkImages);
        mAvailableVkImages = std::move(other.mAvailableVkImages);
        mAcquiredVkImages = std::move(other.mAcquiredVkImages);
        mPendingAcquisition = std::exchange(other.mPendingAcquisition, UINT32_MAX);
    }
    return *this;
}

VkResult Swapchain::post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    (void)pAllocator;
    assert(device);
    assert(pCreateInfo);
    assert(pSwapchain);
    assert(*pSwapchain);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        mGvkDevice = device;
        mVkSwapchain = *pSwapchain;
        mExtent = pCreateInfo->imageExtent;

        uint32_t imageCount = 0;
        gvk_result(mGvkDevice.get<DispatchTable>().gvkGetSwapchainImagesKHR(device, mVkSwapchain, &imageCount, nullptr));
        mActualImages.resize(imageCount);
        gvk_result(mGvkDevice.get<DispatchTable>().gvkGetSwapchainImagesKHR(device, mVkSwapchain, &imageCount, mActualImages.data()));
        mImageLayouts.resize(imageCount);

        auto imageCreateInfo = get_default<VkImageCreateInfo>();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = pCreateInfo->imageFormat;
        imageCreateInfo.extent.width = pCreateInfo->imageExtent.width;
        imageCreateInfo.extent.height = pCreateInfo->imageExtent.height;
        imageCreateInfo.arrayLayers = pCreateInfo->imageArrayLayers;
        imageCreateInfo.usage = pCreateInfo->imageUsage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCreateInfo.sharingMode = pCreateInfo->imageSharingMode;
        imageCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
        imageCreateInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
        mVirtualVkImages.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; ++i) {
            gvk_result(mGvkDevice.get<DispatchTable>().gvkCreateImage(device, &imageCreateInfo, nullptr, &mVirtualVkImages[i]));
            mAvailableVkImages.insert(i);
        }

        VkMemoryRequirements memoryRequirements{ };
        mGvkDevice.get<DispatchTable>().gvkGetImageMemoryRequirements(device, mVirtualVkImages[0], &memoryRequirements);

        auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memoryTypeCount = 0;
        get_compatible_memory_type_indices(mGvkDevice.get<PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
        gvk_result(memoryTypeCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        memoryTypeCount = 1;
        uint32_t memoryTypeIndex = 0;
        get_compatible_memory_type_indices(mGvkDevice.get<PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryTypeIndex);

        auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
        auto padding = memoryRequirements.size % (memoryRequirements.alignment ? memoryRequirements.alignment : 1);
        memoryAllocateInfo.allocationSize = memoryRequirements.size * imageCount + padding * (imageCount - 1);
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        gvk_result(DeviceMemory::allocate(mGvkDevice, &memoryAllocateInfo, nullptr, &mGvkDeviceMemory));

        VkDeviceSize offset = 0;
        for (uint32_t i = 0; i < imageCount && gvkResult == VK_SUCCESS; ++i) {
            gvk_result(mGvkDevice.get<DispatchTable>().gvkBindImageMemory(device, mVirtualVkImages[i], mGvkDeviceMemory, offset));
            offset += memoryRequirements.size + padding;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Swapchain::pre_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    if (swapchain) {
        assert(mGvkDevice);
        assert(mGvkDevice == device);
        for (auto virtualImage : mVirtualVkImages) {
            mGvkDevice.get<DispatchTable>().gvkDestroyImage(device, virtualImage, pAllocator);
        }
        mGvkDevice.reset();
        mVkSwapchain = VK_NULL_HANDLE;
        mExtent = { };
        mGvkDeviceMemory.reset();
        mActualImages.clear();
        mImageLayouts.clear();
        mVirtualVkImages.clear();
        mAvailableVkImages.clear();
        mAcquiredVkImages.clear();
        mPendingAcquisition = UINT32_MAX;
    }
}

VkResult Swapchain::post_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
{
    (void)device;
    (void)swapchain;
    if (pSwapchainImageCount && pSwapchainImages) {
        memcpy(pSwapchainImages, mVirtualVkImages.data(), *pSwapchainImageCount * sizeof(VkImage));
    }
    return VK_SUCCESS;
}

VkResult Swapchain::pre_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    (void)device;
    (void)swapchain;
    (void)timeout;
    (void)semaphore;
    (void)fence;
    assert(pImageIndex);
    assert(*pImageIndex < mVirtualVkImages.size());
    mPendingAcquisition = *pImageIndex;
    return VK_SUCCESS;
}

VkResult Swapchain::post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    (void)device;
    (void)swapchain;
    (void)timeout;
    (void)semaphore;
    (void)fence;
    assert(pImageIndex);
    assert(*pImageIndex < mActualImages.size());
    auto erased = mAvailableVkImages.erase(mPendingAcquisition);
    (void)erased;
    assert(erased);
    auto inserted = mAcquiredVkImages.insert({ mPendingAcquisition, *pImageIndex }).second;
    (void)inserted;
    assert(inserted);
    *pImageIndex = mPendingAcquisition;
    mPendingAcquisition = UINT32_MAX;
    return VK_SUCCESS;
}

VkResult Swapchain::pre_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex)
{
    (void)device;
    (void)pAcquireInfo;
    (void)pImageIndex;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkAcquireNextImage2KHR() is unserviced; gvk maintenance required");
    return VK_ERROR_UNKNOWN;
}

VkResult Swapchain::post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex)
{
    (void)device;
    (void)pAcquireInfo;
    (void)pImageIndex;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkAcquireNextImage2KHR() is unserviced; gvk maintenance required");
    return VK_ERROR_UNKNOWN;
}

void Swapchain::pre_vkQueuePresentKHR(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    assert(commandBuffer);

    auto actualImage = mActualImages[imageIndex];
    auto itr = mAcquiredVkImages.find(imageIndex);
    assert(itr != mAcquiredVkImages.end());
    assert(itr->second < mVirtualVkImages.size());
    auto virtualImage = mVirtualVkImages[itr->second];

    // Actual VkImage VK_IMAGE_LAYOUT_UNDEFINED/VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // Virtual VkImage VK_IMAGE_LAYOUT_PRESENT_SRC_KHR -> VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers{ };
    auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = mImageLayouts[imageIndex];
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = actualImage;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarriers[0] = imageMemoryBarrier;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarrier.image = virtualImage;
    imageMemoryBarriers[1] = imageMemoryBarrier;
    mGvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        (uint32_t)imageMemoryBarriers.size(),
        imageMemoryBarriers.data()
    );

    // Copy virtual VkImage -> actual VkImage
    auto imageCopy = get_default<VkImageCopy>();
    imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopy.srcSubresource.layerCount = 1;
    imageCopy.dstSubresource = imageCopy.srcSubresource;
    imageCopy.extent.width = mExtent.width;
    imageCopy.extent.height = mExtent.height;
    imageCopy.extent.depth = 1;
    mGvkDevice.get<DispatchTable>().gvkCmdCopyImage(
        commandBuffer,
        virtualImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        actualImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopy
    );

    // Actual VkImage VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    // Virtual VkImage VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    std::swap(imageMemoryBarriers[0].srcAccessMask, imageMemoryBarriers[0].dstAccessMask);
    std::swap(imageMemoryBarriers[0].oldLayout, imageMemoryBarriers[0].newLayout);
    imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    mImageLayouts[imageIndex] = imageMemoryBarriers[0].newLayout;
    std::swap(imageMemoryBarriers[1].srcAccessMask, imageMemoryBarriers[1].dstAccessMask);
    std::swap(imageMemoryBarriers[1].oldLayout, imageMemoryBarriers[1].newLayout);
    mGvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        (uint32_t)imageMemoryBarriers.size(),
        imageMemoryBarriers.data()
    );

    auto inserted = mAvailableVkImages.insert(itr->first).second;
    (void)inserted;
    assert(inserted);
    mAcquiredVkImages.erase(itr);
}

VkResult Layer::post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult)
{
    (void)pAllocator;
    if (gvkResult == VK_SUCCESS) {
        const auto& dispatchTableItr = layer::Registry::get().VkInstanceDispatchTables.find(layer::get_dispatch_key(*pInstance));
        assert(dispatchTableItr != layer::Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        gvkResult = Instance::create_unmanaged(pCreateInfo, nullptr, &dispatchTableItr->second, *pInstance, &mGvkInstance);
    }
    return gvkResult;
}

void Layer::pre_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    if (instance) {
        assert(mGvkInstance == instance);
        mGvkInstance.reset();
    }
}

VkResult Layer::post_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult)
{
    (void)pAllocator;
    if (gvkResult == VK_SUCCESS) {
        assert(pDevice);
        assert(*pDevice);
        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(*pDevice));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        Device gvkDevice;
        gvkResult = Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &dispatchTableItr->second, *pDevice, &gvkDevice);
        if (gvkResult == VK_SUCCESS) {
            std::lock_guard<std::mutex> lock(mMutex);
            auto inserted = mGvkDevices.insert(gvkDevice).second;
            (void)inserted;
            assert(inserted);
        }
    }
    return gvkResult;
}

void Layer::pre_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    if (device) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto erased = mCommandBuffers.erase(device);
        (void)erased;
        assert(erased);
        erased = mGvkDevices.erase(device);
        (void)erased;
        assert(erased);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Layer::pre_vkAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkAcquireFullScreenExclusiveModeEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkAcquireFullScreenExclusiveModeEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult Layer::pre_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult vkResult)
{
    if (vkResult == VK_SUCCESS) {
        assert(pAcquireInfo);
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(pAcquireInfo->swapchain);
        assert(itr != mSwapchains.end());
        vkResult = itr->second.pre_vkAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    }
    return vkResult;
}

VkResult Layer::post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult vkResult)
{
    if (vkResult == VK_SUCCESS) {
        assert(pAcquireInfo);
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(pAcquireInfo->swapchain);
        assert(itr != mSwapchains.end());
        vkResult = itr->second.post_vkAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    }
    return vkResult;
}

VkResult Layer::pre_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.pre_vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }
    return gvkResult;
}

VkResult Layer::post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.post_vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }
    return gvkResult;
}

VkResult Layer::pre_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult gvkResult)
{
    (void)device;
    (void)swapchainCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pSwapchains;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkCreateSharedSwapchainsKHR() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult gvkResult)
{
    (void)device;
    (void)swapchainCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pSwapchains;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkCreateSharedSwapchainsKHR() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pSwapchain;
    assert(pCreateInfo);
    const_cast<VkSwapchainCreateInfoKHR*>(pCreateInfo)->imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return gvkResult;
}

VkResult Layer::post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult vkResult)
{
    if (vkResult == VK_SUCCESS) {
        std::lock_guard<std::mutex> lock(mMutex);
        assert(!mSwapchains.count(*pSwapchain));
        vkResult = mSwapchains[*pSwapchain].post_vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }
    return vkResult;
}

void Layer::pre_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    (void)device;
    (void)pAllocator;
    if (swapchain) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(swapchain);
        assert(itr != mSwapchains.end());
        itr->second.pre_vkDestroySwapchainKHR(device, swapchain, pAllocator);
        mSwapchains.erase(itr);
    }
}

void Layer::post_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    (void)device;
    (void)swapchain;
    (void)pAllocator;
    // NOOP :
}

void Layer::pre_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo)
{
    (void)device;
    (void)swapchain;
    (void)pLatencyMarkerInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetLatencyTimingsNV() is unserviced; gvk maintenance required");
}

void Layer::post_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo)
{
    (void)device;
    (void)swapchain;
    (void)pLatencyMarkerInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetLatencyTimingsNV() is unserviced; gvk maintenance required");
}

VkResult Layer::pre_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pPresentationTimingCount;
    (void)pPresentationTimings;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetPastPresentationTimingGOOGLE() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pPresentationTimingCount;
    (void)pPresentationTimings;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetPastPresentationTimingGOOGLE() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::pre_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pDisplayTimingProperties;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetRefreshCycleDurationGOOGLE() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pDisplayTimingProperties;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetRefreshCycleDurationGOOGLE() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::pre_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)counter;
    (void)pCounterValue;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetSwapchainCounterEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)counter;
    (void)pCounterValue;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetSwapchainCounterEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::pre_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pSwapchainImageCount;
    (void)pSwapchainImages;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages, VkResult gvkResult)
{
    (void)device;
    if (gvkResult == VK_SUCCESS && pSwapchainImageCount && pSwapchainImages) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.post_vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }
    return gvkResult;
}

VkResult Layer::pre_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetSwapchainStatusKHR() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkGetSwapchainStatusKHR() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::pre_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pSleepInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkLatencySleepNV() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pSleepInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkLatencySleepNV() is unserviced; gvk maintenance required");
    return gvkResult;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Layer::pre_vkReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkReleaseFullScreenExclusiveModeEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkReleaseFullScreenExclusiveModeEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult Layer::pre_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo, VkResult gvkResult)
{
    (void)device;
    (void)pReleaseInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkReleaseSwapchainImagesEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo, VkResult gvkResult)
{
    (void)device;
    (void)pReleaseInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkReleaseSwapchainImagesEXT() is unserviced; gvk maintenance required");
    return gvkResult;
}

void Layer::pre_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata)
{
    (void)device;
    (void)swapchainCount;
    (void)pSwapchains;
    (void)pMetadata;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetHdrMetadataEXT() is unserviced; gvk maintenance required");
}

void Layer::post_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata)
{
    (void)device;
    (void)swapchainCount;
    (void)pSwapchains;
    (void)pMetadata;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetHdrMetadataEXT() is unserviced; gvk maintenance required");
}

void Layer::pre_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo)
{
    (void)device;
    (void)swapchain;
    (void)pLatencyMarkerInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetLatencyMarkerNV() is unserviced; gvk maintenance required");
}

void Layer::post_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo)
{
    (void)device;
    (void)swapchain;
    (void)pLatencyMarkerInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetLatencyMarkerNV() is unserviced; gvk maintenance required");
}

VkResult Layer::pre_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pSleepModeInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetLatencySleepModeNV() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pSleepModeInfo;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetLatencySleepModeNV() is unserviced; gvk maintenance required");
    return gvkResult;
}

void Layer::pre_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapchain, VkBool32 localDimmingEnable)
{
    (void)device;
    (void)swapchain;
    (void)localDimmingEnable;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetLocalDimmingAMD() is unserviced; gvk maintenance required");
}

void Layer::post_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapchain, VkBool32 localDimmingEnable)
{
    (void)device;
    (void)swapchain;
    (void)localDimmingEnable;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkSetLocalDimmingAMD() is unserviced; gvk maintenance required");
}

VkResult Layer::pre_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)presentId;
    (void)timeout;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkWaitForPresentKHR() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::post_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)presentId;
    (void)timeout;
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkWaitForPresentKHR() is unserviced; gvk maintenance required");
    return gvkResult;
}

VkResult Layer::pre_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult vkResult)
{
    if (vkResult == VK_SUCCESS) {
        gvk_result_scope_begin(vkResult) {
            Queue gvkQueue = queue;
            Device gvkDevice = gvkQueue.get<VkDevice>();
            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            gvk_result(get_command_buffer(queue, &commandBuffer));
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(gvkDevice.get<DispatchTable>().gvkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
            for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
                std::lock_guard<std::mutex> lock(mMutex);
                auto swapchainItr = mSwapchains.find(pPresentInfo->pSwapchains[i]);
                assert(swapchainItr != mSwapchains.end());
                swapchainItr->second.pre_vkQueuePresentKHR(commandBuffer, pPresentInfo->pImageIndices[i]);
            }
            gvk_result(gvkDevice.get<DispatchTable>().gvkEndCommandBuffer(commandBuffer));
            // TODO : Hook up app's VkSemaphores
            auto submitInfo = get_default<VkSubmitInfo>();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            gvk_result(gvkDevice.get<DispatchTable>().gvkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
        } gvk_result_scope_end;
    }
    return vkResult;
}

VkResult Layer::post_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult gvkResult)
{
    (void)queue;
    (void)pPresentInfo;
    // NOOP :
    return gvkResult;
}

VkResult Layer::get_command_buffer(const Queue& gvkQueue, VkCommandBuffer* pCommandBuffer)
{
    assert(gvkQueue);
    assert(pCommandBuffer);
    *pCommandBuffer = VK_NULL_HANDLE;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        Device gvkDevice = gvkQueue.get<VkDevice>();
        assert(gvkDevice);
        std::lock_guard<std::mutex> lock(mMutex);
        auto& commandResources = mCommandBuffers[gvkDevice][gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex];
        assert(!commandResources.first == !commandResources.second);
        if (!commandResources.first) {
            auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
            gvk_result(CommandPool::create(gvkDevice, &commandPoolCreateInfo, nullptr, &commandResources.first));
            auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
            commandBufferAllocateInfo.commandPool = commandResources.first;
            commandBufferAllocateInfo.commandBufferCount = 1;
            // TODO : Detect layer and automate dispatch table update so gvk::CommandBuffer
            //  can be allocated in layers
            gvk_result(gvkDevice.get<DispatchTable>().gvkAllocateCommandBuffers(gvkDevice, &commandBufferAllocateInfo, &commandResources.second));
            *(void**)commandResources.second = *(void**)gvkDevice.get<VkDevice>();
        }
        *pCommandBuffer = commandResources.second;
        gvk_result(*pCommandBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace virtual_swapchain
} // namespace gvk

namespace gvk {
namespace layer {

void on_load(Registry& registry)
{
    registry.layers.push_back(std::make_unique<virtual_swapchain::Layer>());
}

} // namespace layer
} // namespace gvk

extern "C" {

VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

} // extern "C"

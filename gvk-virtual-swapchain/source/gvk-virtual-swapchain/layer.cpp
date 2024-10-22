
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

VirtualSwapchain::VirtualSwapchain(VirtualSwapchain&& other)
{
    *this = std::move(other);
}

VirtualSwapchain& VirtualSwapchain::operator=(VirtualSwapchain&& other)
{
    if (this != &other) {
        mGvkDevice = std::move(other.mGvkDevice);
        mVkSwapchain = std::exchange(other.mVkSwapchain, VK_NULL_HANDLE);
        mExtent = std::move(other.mExtent);
        mGvkDeviceMemory = std::move(other.mGvkDeviceMemory);
        mActualVkImages = std::move(other.mActualVkImages);
        mVirtualImages = std::move(other.mVirtualImages);
        mAvailableImageIndices = std::move(other.mAvailableImageIndices);
        mAcquiredImages = std::move(other.mAcquiredImages);
    }
    return *this;
}

VkResult VirtualSwapchain::post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
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

        // Get the actual VkSwapchainKHR VkImages
        uint32_t imageCount = 0;
        gvk_result(mGvkDevice.get<DispatchTable>().gvkGetSwapchainImagesKHR(device, mVkSwapchain, &imageCount, nullptr));
        mActualVkImages.resize(imageCount);
        gvk_result(mGvkDevice.get<DispatchTable>().gvkGetSwapchainImagesKHR(device, mVkSwapchain, &imageCount, mActualVkImages.data()));

        // Create VirtualImages
        mVirtualImages.resize(imageCount);
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
        for (uint32_t i = 0; i < imageCount; ++i) {
            gvk_result(Image::create(device, &imageCreateInfo, nullptr, &mVirtualImages[i].image));
            gvk_result(Semaphore::create(device, &get_default<VkSemaphoreCreateInfo>(), nullptr, &mVirtualImages[i].imageTransferedSemaphore));
            mAvailableImageIndices.insert(i);
        }

        // Allocate backing DeviceMemory for the virtual VkImages
        VkMemoryRequirements memoryRequirements{ };
        mGvkDevice.get<DispatchTable>().gvkGetImageMemoryRequirements(device, mVirtualImages[0].image, &memoryRequirements);
        auto memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memoryTypeCount = 0;
        get_compatible_memory_type_indices(mGvkDevice.get<PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, nullptr);
        gvk_result(memoryTypeCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        memoryTypeCount = 1;
        uint32_t memoryTypeIndex = 0;
        get_compatible_memory_type_indices(mGvkDevice.get<PhysicalDevice>(), memoryRequirements.memoryTypeBits, memoryPropertyFlags, &memoryTypeCount, &memoryTypeIndex);
        auto memoryAllocateInfo = get_default<VkMemoryAllocateInfo>();
        auto padding = memoryRequirements.alignment ? memoryRequirements.size % memoryRequirements.alignment : 0;
        if (padding) {
            padding = memoryRequirements.alignment - padding;
        }
        memoryAllocateInfo.allocationSize = memoryRequirements.size * imageCount + padding * (imageCount - 1);
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        gvk_result(DeviceMemory::allocate(mGvkDevice, &memoryAllocateInfo, nullptr, &mGvkDeviceMemory));

        // Bind Images to DeviceMemory
        VkDeviceSize offset = 0;
        for (uint32_t i = 0; i < imageCount && gvkResult == VK_SUCCESS; ++i) {
            gvk_result(mGvkDevice.get<DispatchTable>().gvkBindImageMemory(device, mVirtualImages[i].image, mGvkDeviceMemory, offset));
            offset += memoryRequirements.size + padding;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void VirtualSwapchain::pre_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    (void)device;
    (void)swapchain;
    (void)pAllocator;
    // NOOP :
}

VkResult VirtualSwapchain::post_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
{
    (void)device;
    (void)swapchain;
    if (pSwapchainImageCount && pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount && i < mVirtualImages.size(); ++i) {
            pSwapchainImages[i] = mVirtualImages[i].image;
        }
    }
    return VK_SUCCESS;
}

VkResult VirtualSwapchain::pre_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)timeout;
    (void)fence;
    assert(pImageIndex);
    assert(*pImageIndex < mVirtualImages.size());
    assert(gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR);

    // Update mPendingImageAcquisition.  The application provides the index of the
    //  image it wants to acquire.
    mPendingImageAcquisition = { };
    mPendingImageAcquisition.imageAcquiredSemaphore = semaphore;
    mPendingImageAcquisition.virtualImageIndex = *pImageIndex;
    mPendingImageAcquisition.pVirtualImage = &mVirtualImages[*pImageIndex];
    auto erased = mAvailableImageIndices.erase(*pImageIndex);
    return erased ? gvkResult : VK_NOT_READY;
}

VkResult VirtualSwapchain::post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)timeout;
    (void)semaphore;
    (void)fence;
    assert(pImageIndex);
    assert(*pImageIndex < mActualVkImages.size());
    assert(gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR);

    // In the post call handler the live Vulkan call has updated the index to refer
    //  to the actual image acquired.  Update mPendingImageAcquisition with the
    //  actual index and image, map mPendingImageAcquisition, reset the pImageIndex
    //  to the virtual image index, then clear mPendingImageAcquisition.
    mPendingImageAcquisition.actualImageIndex = *pImageIndex;
    mPendingImageAcquisition.actualImage = mActualVkImages[*pImageIndex];
    auto inserted = mAcquiredImages.insert({ mPendingImageAcquisition.virtualImageIndex, mPendingImageAcquisition }).second;
    *pImageIndex = mPendingImageAcquisition.virtualImageIndex;
    mPendingImageAcquisition = { };
    return  inserted ? gvkResult : VK_NOT_READY;
}

VkResult VirtualSwapchain::pre_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult)
{
    (void)device;
    (void)pAcquireInfo;
    (void)pImageIndex;
    (void)gvkResult;
    assert(gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR);
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkAcquireNextImage2KHR() is unserviced; gvk maintenance required");
    return VK_ERROR_UNKNOWN;
}

VkResult VirtualSwapchain::post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult)
{
    (void)device;
    (void)pAcquireInfo;
    (void)pImageIndex;
    (void)gvkResult;
    assert(gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR);
    assert(false && "VK_LAYER_INTEL_gvk_virtual_swapchain vkAcquireNextImage2KHR() is unserviced; gvk maintenance required");
    return VK_ERROR_UNKNOWN;
}

VkResult VirtualSwapchain::pre_vkQueuePresentKHR(VkCommandBuffer commandBuffer, uint32_t* pImageIndex, VkSemaphore* pImageAcquiredSemaphore, VkSemaphore* pImageTransferedSemaphore)
{
    assert(commandBuffer);
    assert(pImageIndex);
    assert(pImageAcquiredSemaphore);
    assert(pImageTransferedSemaphore);

    // Get the AcquiredImage associated with the given image index
    auto acquiredImageItr = mAcquiredImages.find(*pImageIndex);
    assert(acquiredImageItr != mAcquiredImages.end());
    const auto& acquiredImage = acquiredImageItr->second;
    assert(acquiredImage.virtualImageIndex == *pImageIndex);
    assert(acquiredImage.pVirtualImage);

    // Populate semaphore out parameters
    *pImageAcquiredSemaphore = acquiredImage.imageAcquiredSemaphore;
    *pImageTransferedSemaphore = acquiredImage.pVirtualImage->imageTransferedSemaphore;

    // Prepare VkImageMemoryBarriers
    std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers{ };
    auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarriers.fill(imageMemoryBarrier);

    // Virtual VkImage VK_IMAGE_LAYOUT_PRESENT_SRC_KHR -> VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    auto& virtualImageMemoryBarrier = imageMemoryBarriers[0];
    virtualImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    virtualImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    virtualImageMemoryBarrier.image = acquiredImage.pVirtualImage->image;

    // Actual VkImage VK_IMAGE_LAYOUT_UNDEFINED/VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    auto& actualImageMemoryBarrier = imageMemoryBarriers[1];
    actualImageMemoryBarrier.oldLayout = acquiredImage.pVirtualImage->layout;
    actualImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    actualImageMemoryBarrier.image = acquiredImage.actualImage;

    // Record barriers
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
        acquiredImage.pVirtualImage->image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        acquiredImage.actualImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopy
    );

    // Virtual VkImage VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    std::swap(virtualImageMemoryBarrier.srcAccessMask, virtualImageMemoryBarrier.dstAccessMask);
    std::swap(virtualImageMemoryBarrier.oldLayout, virtualImageMemoryBarrier.newLayout);

    // Actual VkImage VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    std::swap(actualImageMemoryBarrier.srcAccessMask, actualImageMemoryBarrier.dstAccessMask);
    actualImageMemoryBarrier.oldLayout = actualImageMemoryBarrier.newLayout;
    actualImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Update the VirtualImage layout
    acquiredImage.pVirtualImage->layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Record barriers
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

    // After presentation, we're done with this VirtualImage until it's reacquired.
    //  insert() the index back into mAvailableImageIndices, set the out parameter
    //  to acquiredImage.actualImageIndex so that the present will use the actual
    //  VkSwapchainKHR VkImage that is used as TRANSFER_DST in the cmds above, then
    //  erase the AcquiredImage.
    auto inserted = mAvailableImageIndices.insert(*pImageIndex).second;
    *pImageIndex = acquiredImage.actualImageIndex;
    mAcquiredImages.erase(acquiredImageItr);
    return inserted ? VK_SUCCESS : VK_NOT_READY;
}

VkResult Layer::post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult)
{
    // TODO : gvk-handles needs to hook up VkAllocationCallbacks
    (void)pAllocator;
    if (gvkResult == VK_SUCCESS) {
        mLog.set_instance(*pInstance);
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
    // TODO : gvk-handles needs to hook up VkAllocationCallbacks
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
            gvkResult = inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
        }
    }
    return gvkResult;
}

void Layer::pre_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    if (device) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto erased = mCommandResources.erase(device);
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

VkResult Layer::pre_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.pre_vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, gvkResult);
    }
    return gvkResult;
}

VkResult Layer::post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.post_vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, gvkResult);
    }
    return gvkResult;
}

VkResult Layer::pre_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR) {
        assert(pAcquireInfo);
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(pAcquireInfo->swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.pre_vkAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, gvkResult);
    }
    return gvkResult;
}

VkResult Layer::post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS || gvkResult == VK_SUBOPTIMAL_KHR) {
        assert(pAcquireInfo);
        std::lock_guard<std::mutex> lock(mMutex);
        auto itr = mSwapchains.find(pAcquireInfo->swapchain);
        assert(itr != mSwapchains.end());
        gvkResult = itr->second.post_vkAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, gvkResult);
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

thread_local VkSwapchainCreateInfoKHR tlApplicationSwapchinCreateInfoKHR;
VkResult Layer::pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult)
{
    (void)device;
    (void)pAllocator;
    (void)pSwapchain;
    assert(pCreateInfo);

    // Cache the application's VkSwapchainCreateInfoKHR then set the TRANSFER_DST
    //  bit.  This needs to be on so that we can copy from the virtual swapchain
    //  images to the actual images.
    tlApplicationSwapchinCreateInfoKHR = *pCreateInfo;
    const_cast<VkSwapchainCreateInfoKHR*>(pCreateInfo)->imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return gvkResult;
}

VkResult Layer::post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult vkResult)
{
    // Create the virtual swapchain
    if (vkResult == VK_SUCCESS) {
        assert(pSwapchain);
        std::lock_guard<std::mutex> lock(mMutex);
        assert(!mSwapchains.count(*pSwapchain));
        vkResult = mSwapchains[*pSwapchain].post_vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }

    // Revert the application's VkSwapchainCreateInfoKHR
    *const_cast<VkSwapchainCreateInfoKHR*>(pCreateInfo) = tlApplicationSwapchinCreateInfoKHR;
    tlApplicationSwapchinCreateInfoKHR = { };
    return vkResult;
}

void Layer::pre_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
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
    if (gvkResult == VK_SUCCESS && pSwapchainImageCount && pSwapchainImages) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto swapchainItr = mSwapchains.find(swapchain);
        assert(swapchainItr != mSwapchains.end());
        gvkResult = swapchainItr->second.post_vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
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

thread_local VkPresentInfoKHR tlApplicationPresentInfo;
thread_local std::vector<uint32_t> tlImageIndices;
thread_local std::vector<VkSemaphore> tlImageAcquiredVkSemaphores;
thread_local std::vector<VkPipelineStageFlags> tlWaitDstStageMask;
thread_local std::vector<VkSemaphore> tlImageTransferedVkSemaphores;
thread_local std::vector<VkSemaphore> tlPresentWaitVkSemaphores;
VkResult Layer::pre_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult vkResult)
{
    assert(pPresentInfo);
    assert(pPresentInfo->swapchainCount);
    assert(pPresentInfo->pSwapchains);
    assert(pPresentInfo->pImageIndices);

    std::lock_guard<std::mutex> lock(mMutex);

    // Prepare storage used for modifying pPresentInfo across pre/post handlers
    tlApplicationPresentInfo = { };
    tlImageIndices.clear();
    tlImageIndices.insert(tlImageIndices.end(), pPresentInfo->pImageIndices, pPresentInfo->pImageIndices + pPresentInfo->swapchainCount);
    tlImageAcquiredVkSemaphores.clear();
    tlImageAcquiredVkSemaphores.reserve(pPresentInfo->swapchainCount);
    tlImageTransferedVkSemaphores.clear();
    tlImageTransferedVkSemaphores.reserve(pPresentInfo->swapchainCount);
    tlPresentWaitVkSemaphores.clear();
    tlPresentWaitVkSemaphores.reserve(pPresentInfo->waitSemaphoreCount + pPresentInfo->swapchainCount);
    if (pPresentInfo->waitSemaphoreCount && pPresentInfo->pWaitSemaphores) {
        tlPresentWaitVkSemaphores.insert(tlPresentWaitVkSemaphores.end(), pPresentInfo->pWaitSemaphores, pPresentInfo->pWaitSemaphores + pPresentInfo->waitSemaphoreCount);
    }

    if (vkResult == VK_SUCCESS) {
        gvk_result_scope_begin(vkResult) {
            Queue gvkQueue = queue;
            gvk_result(gvkQueue ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            Device gvkDevice = gvkQueue.get<VkDevice>();
            gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // Get CommandResources and prepare the VkCommandBuffer for recording
            CommandResources commandResources{ };
            gvk_result(get_command_resources(lock, queue, &commandResources));
            auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            gvk_result(gvkDevice.get<DispatchTable>().gvkBeginCommandBuffer(commandResources.vkCommandBuffer, &commandBufferBeginInfo));

            // For each VkSwapchainKHR used in this present record commands to transfer the
            //  contents of each virtual image to the actual image that will be presented.
            for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
                VkSemaphore imageAcquiredSemaphore = VK_NULL_HANDLE;
                VkSemaphore imageTransferedSemaphore = VK_NULL_HANDLE;
                auto swapchainItr = mSwapchains.find(pPresentInfo->pSwapchains[i]);
                assert(swapchainItr != mSwapchains.end());
                swapchainItr->second.pre_vkQueuePresentKHR(commandResources.vkCommandBuffer, &tlImageIndices[i], &imageAcquiredSemaphore, &imageTransferedSemaphore);
                if (imageAcquiredSemaphore) {
                    tlImageAcquiredVkSemaphores.push_back(imageAcquiredSemaphore);
                }
                if (imageTransferedSemaphore) {
                    tlImageTransferedVkSemaphores.push_back(imageTransferedSemaphore);
                    tlPresentWaitVkSemaphores.push_back(imageTransferedSemaphore);
                }
            }

            // End VkCommandBuffer recording.
            gvk_result(gvkDevice.get<DispatchTable>().gvkEndCommandBuffer(commandResources.vkCommandBuffer));

            // Prepare a VkSubmitInfo to execute the recorded VkCommandBuffer.  When
            //  execution completes, the VkSemphores waiting on the transfer to complete
            //  will be signaled.
            auto submitInfo = get_default<VkSubmitInfo>();
            // NOTE : The application should be waiting on the VkSemaphores provided at
            //  vkAcquireNextImageKHR() time.  We can't wait on them as well.  Some use
            //  cases may require introducing waits on these VkSemaphores.  This can be
            //  addressed when a use case presents itself.
            #if 0
            submitInfo.waitSemaphoreCount = (uint32_t)tlImageAcquiredVkSemaphores.size();
            submitInfo.pWaitSemaphores = !tlImageAcquiredVkSemaphores.empty() ? tlImageAcquiredVkSemaphores.data() : nullptr;
            tlWaitDstStageMask.resize(tlImageAcquiredVkSemaphores.size(), VK_PIPELINE_STAGE_TRANSFER_BIT);
            submitInfo.pWaitDstStageMask = !tlWaitDstStageMask.empty() ? tlWaitDstStageMask.data() : nullptr;
            #endif
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandResources.vkCommandBuffer;
            submitInfo.signalSemaphoreCount = (uint32_t)tlImageTransferedVkSemaphores.size();
            submitInfo.pSignalSemaphores = !tlImageTransferedVkSemaphores.empty() ? tlImageTransferedVkSemaphores.data() : nullptr;
            gvk_result(gvkQueue.get<DispatchTable>().gvkQueueSubmit(queue, 1, &submitInfo, commandResources.gvkFence));

            // Cache the application's VkPresentInfoKHR so it can be reverted in the post
            //  handler, then const_cast<>() and update
            tlApplicationPresentInfo = *pPresentInfo;
            auto pMutablePresentInfo = const_cast<VkPresentInfoKHR*>(pPresentInfo);
            pMutablePresentInfo->waitSemaphoreCount = (uint32_t)tlPresentWaitVkSemaphores.size();
            pMutablePresentInfo->pWaitSemaphores = !tlPresentWaitVkSemaphores.empty() ? tlPresentWaitVkSemaphores.data() : nullptr;
            pMutablePresentInfo->pImageIndices = tlImageIndices.data();
        } gvk_result_scope_end;
        vkResult = gvkResult;
    }
    return vkResult;
}

VkResult Layer::post_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult gvkResult)
{
    (void)queue;
    assert(pPresentInfo);
    *const_cast<VkPresentInfoKHR*>(pPresentInfo) = tlApplicationPresentInfo;
    tlApplicationPresentInfo = { };
    tlImageIndices.clear();
    tlImageAcquiredVkSemaphores.clear();
    tlWaitDstStageMask.clear();
    tlImageTransferedVkSemaphores.clear();
    tlPresentWaitVkSemaphores.clear();
    return gvkResult;
}

VkResult Layer::get_command_resources(const std::lock_guard<std::mutex>&, const Queue& gvkQueue, CommandResources* pCommandResources)
{
    assert(gvkQueue);
    assert(pCommandResources);
    *pCommandResources = { };
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        Device gvkDevice = gvkQueue.get<VkDevice>();
        auto& commandResources = mCommandResources[gvkDevice][gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex];
        assert(!commandResources.gvkCommandPool == !commandResources.vkCommandBuffer);
        assert(!commandResources.gvkCommandPool == !commandResources.gvkFence);
        if (!commandResources.gvkCommandPool) {

            // Create CommandPool
            auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = gvkQueue.get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
            gvk_result(CommandPool::create(gvkDevice, &commandPoolCreateInfo, nullptr, &commandResources.gvkCommandPool));

            // Allocate VkCommandBuffer
            auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
            commandBufferAllocateInfo.commandPool = commandResources.gvkCommandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;
            // TODO : Detect layer and automate dispatch table update so gvk::CommandBuffer
            //  can be allocated in layers
            gvk_result(gvkDevice.get<DispatchTable>().gvkAllocateCommandBuffers(gvkDevice, &commandBufferAllocateInfo, &commandResources.vkCommandBuffer));
            *(void**)commandResources.vkCommandBuffer = *(void**)gvkDevice.get<VkDevice>();

            // Create Fence, signaled since it will be waited on right away
            auto fenceCreateInfo = get_default<VkFenceCreateInfo>();
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            gvk_result(Fence::create(gvkDevice, &fenceCreateInfo, nullptr, &commandResources.gvkFence));
        }

        // Populate out parameter
        *pCommandResources = commandResources;
        gvk_result(pCommandResources->gvkCommandPool ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommandResources->vkCommandBuffer ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommandResources->gvkFence ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Wait on Fence to ensure resources aren't still in use, then reset Fence
        gvk_result(gvkDevice.get<DispatchTable>().gvkWaitForFences(gvkDevice, 1, &commandResources.gvkFence.get<VkFence>(), VK_TRUE, UINT64_MAX));
        gvk_result(gvkDevice.get<DispatchTable>().gvkResetFences(gvkDevice, 1, &commandResources.gvkFence.get<VkFence>()));
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

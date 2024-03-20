
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

#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-layer/registry.hpp"

#include <cassert>
#include <vector>

namespace gvk {
namespace state_tracker {

// NOTE : Cache the info arg so any changes made by this layer, or layers down
//  the chain, can be reverted before returning control to the application.
thread_local VkSwapchainCreateInfoKHR tlApplicationSwapchainCreateInfo;
VkResult StateTracker::pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult)
{
    assert(pCreateInfo);
    tlApplicationSwapchainCreateInfo = *pCreateInfo;
    return BasicStateTracker::pre_vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, gvkResult);
}

VkResult StateTracker::post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult)
{
    assert(pCreateInfo);
    gvkResult = BasicStateTracker::post_vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, gvkResult);
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        const auto& gvkPhysicalDevice = gvkDevice.get<PhysicalDevice>();
        assert(gvkPhysicalDevice);
        assert(pCreateInfo);
        assert(pSwapchain);
        assert(*pSwapchain);
        SwapchainKHR gvkSwapchain;
        gvkSwapchain.mReference.reset(newref, HandleId<VkDevice, VkSwapchainKHR>(device, *pSwapchain));
        auto& swapchainControlBlock = gvkSwapchain.mReference.get_obj();
        swapchainControlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
        swapchainControlBlock.mVkSwapchainKHR = *pSwapchain;
        swapchainControlBlock.mDevice = gvkDevice;
        swapchainControlBlock.mSurfaceKHR = SurfaceKHR({ gvkPhysicalDevice.get<VkInstance>(), pCreateInfo->surface });
        assert(swapchainControlBlock.mSurfaceKHR);
        swapchainControlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
        swapchainControlBlock.mSwapchainCreateInfoKHR = *pCreateInfo;
        const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
        assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
        const auto& dispatchTable = dispatchTableItr->second;
        assert(dispatchTable.gvkGetSwapchainImagesKHR);
        uint32_t imageCount = 0;
        gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(gvkDevice, *pSwapchain, &imageCount, nullptr);
        assert(gvkResult == VK_SUCCESS);
        std::vector<VkImage> vkImages(imageCount); // TODO : Scratchpad allocator
        gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(gvkDevice, *pSwapchain, &imageCount, vkImages.data());
        assert(gvkResult == VK_SUCCESS);
        for (auto vkImage : vkImages) {
            Image gvkImage;
            gvkImage.mReference.reset(newref, { gvkDevice, vkImage });
            auto& imageControlBlock = gvkImage.mReference.get_obj();
            imageControlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            imageControlBlock.mVkImage = vkImage;
            imageControlBlock.mDevice = gvkDevice;
            imageControlBlock.mVkSwapchainKHR = *pSwapchain;
            imageControlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            auto imageCreateInfo = get_default<VkImageCreateInfo>();
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = pCreateInfo->imageFormat;
            imageCreateInfo.extent.width = pCreateInfo->imageExtent.width;
            imageCreateInfo.extent.height = pCreateInfo->imageExtent.height;
            imageCreateInfo.arrayLayers = pCreateInfo->imageArrayLayers;
            imageCreateInfo.usage = pCreateInfo->imageUsage;
            imageCreateInfo.sharingMode = pCreateInfo->imageSharingMode;
            imageCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
            imageCreateInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
            imageControlBlock.mImageCreateInfo = imageCreateInfo;
            imageControlBlock.mImageLayoutTracker = ImageLayoutTracker(1, pCreateInfo->imageArrayLayers, VK_IMAGE_LAYOUT_UNDEFINED);
            swapchainControlBlock.mImages.insert(gvkImage);
        }
        gvkDevice.mReference.get_obj().mSwapchainKHRTracker.insert(gvkSwapchain);
    }
    *const_cast<VkSwapchainCreateInfoKHR*>(pCreateInfo) = tlApplicationSwapchainCreateInfo;
    return gvkResult;
}

VkResult StateTracker::post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult)
{
    (void)timeout;
    assert(pImageIndex);

    const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
    assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
    const auto& dispatchTable = dispatchTableItr->second;
    assert(dispatchTable.gvkGetSwapchainImagesKHR);

    uint32_t imageCount = 0;
    gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    assert(gvkResult == VK_SUCCESS);
    std::vector<VkImage> images(imageCount); // TODO : Scratchpad allocator
    gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
    assert(*pImageIndex < imageCount);
    Image gvkImage({ device, images[*pImageIndex] });
    assert(gvkImage);

    gvkImage.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_ACQUIRED_BIT;
    gvkImage.mReference.get_obj().mSwapchainAcquisitionFence = Fence({ device, fence });
    gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore = Semaphore({ device, semaphore });
    if (gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore) {
        gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
    }
    return gvkResult;
}

VkResult StateTracker::post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult)
{
    (void)pImageIndex;
    assert(pAcquireInfo);
    assert(pImageIndex);

    const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
    assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
    const auto& dispatchTable = dispatchTableItr->second;
    assert(dispatchTable.gvkGetSwapchainImagesKHR);

    uint32_t imageCount = 0;
    gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(device, pAcquireInfo->swapchain, &imageCount, nullptr);
    assert(gvkResult == VK_SUCCESS);
    std::vector<VkImage> images(imageCount); // TODO : Scratchpad allocator
    gvkResult = dispatchTable.gvkGetSwapchainImagesKHR(device, pAcquireInfo->swapchain, &imageCount, images.data());
    assert(*pImageIndex < imageCount);
    Image gvkImage({ device, images[*pImageIndex] });
    assert(gvkImage);

    gvkImage.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_ACQUIRED_BIT;
    gvkImage.mReference.get_obj().mSwapchainAcquisitionFence = Fence({ device, pAcquireInfo->fence });
    gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore = Semaphore({ device, pAcquireInfo->semaphore });
    if (gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore) {
        gvkImage.mReference.get_obj().mSwapchainAcquisitionSemaphore.mReference.get_obj().mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKER_OBJECT_STATUS_SIGNALED_BIT;
    }
    return gvkResult;
}

VkResult StateTracker::post_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)presentId;
    (void)timeout;
    (void)gvkResult;
    assert(false && "VK_LAYER_INTEL_gvk_state_tracker vkWaitForPresentKHR() unserviced; gvk maintenance required");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

VkResult StateTracker::post_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo, VkResult gvkResult)
{
    (void)device;
    (void)swapchain;
    (void)pSleepInfo;
    (void)gvkResult;
    assert(false && "VK_LAYER_INTEL_gvk_state_tracker vkLatencySleepNV() unserviced; gvk maintenance required");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

void StateTracker::post_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    auto gvkSwapchain = SwapchainKHR({ device, swapchain });
    assert(gvkSwapchain);
    auto& controlBlock = gvkSwapchain.mReference.get_obj();
    controlBlock.mStateTrackedObjectInfo.flags &= ~GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
    controlBlock.mStateTrackedObjectInfo.flags |= GVK_STATE_TRACKED_OBJECT_STATUS_DESTROYED_BIT;
    controlBlock.mDevice.mReference.get_obj().mSwapchainKHRTracker.erase(gvkSwapchain);
}

} // namespace state_tracker
} // namespace gvk

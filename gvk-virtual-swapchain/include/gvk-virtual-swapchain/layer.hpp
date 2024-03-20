
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

#pragma once

#include "gvk-defines.hpp"
#include "gvk-handles.hpp"
#include "gvk-layer.hpp"

#include <set>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gvk {
namespace virtual_swapchain {

using ActualVkImage = VkImage;
using VirtualVkImage = VkImage;

class Swapchain final
{
public:
    Swapchain() = default;
    Swapchain(Swapchain&& other);
    Swapchain& operator=(Swapchain&& other);
    VkResult post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
    void pre_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
    VkResult post_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
    VkResult pre_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
    VkResult post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
    VkResult pre_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
    VkResult post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
    void pre_vkQueuePresentKHR(VkCommandBuffer commandBuffer, uint32_t imageIndex);

private:
    Device mGvkDevice;
    VkSwapchainKHR mVkSwapchain{ };
    VkExtent2D mExtent{ };
    DeviceMemory mGvkDeviceMemory;
    std::vector<ActualVkImage> mActualImages;
    std::vector<VkImageLayout> mImageLayouts;
    std::vector<VirtualVkImage> mVirtualVkImages;
    std::unordered_set<uint32_t> mAvailableVkImages;
    std::unordered_map<uint32_t, uint32_t> mAcquiredVkImages;
    uint32_t mPendingAcquisition{ UINT32_MAX };

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;
};

class Layer final
    : public layer::BasicLayer
{
public:
    VkResult post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult) override final;
    void pre_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) override final;
    VkResult post_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult) override final;
    void pre_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override final;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult pre_vkAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult) override final;
    VkResult post_vkAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult) override final;
#endif // VK_USE_PLATFORM_WIN32_KHR
    VkResult pre_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult) override final;
    VkResult post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult) override final;
    VkResult pre_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult) override final;
    VkResult post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult) override final;
    VkResult pre_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult gvkResult) override final;
    VkResult post_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult gvkResult) override final;
    VkResult pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult) override final;
    VkResult post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult) override final;
    void pre_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) override final;
    void post_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) override final;
    void pre_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) override final;
    void post_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) override final;
    VkResult pre_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings, VkResult gvkResult) override final;
    VkResult post_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings, VkResult gvkResult) override final;
    VkResult pre_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties, VkResult gvkResult) override final;
    VkResult post_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties, VkResult gvkResult) override final;
    VkResult pre_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue, VkResult gvkResult) override final;
    VkResult post_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue, VkResult gvkResult) override final;
    VkResult pre_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages, VkResult gvkResult) override final;
    VkResult post_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages, VkResult gvkResult) override final;
    VkResult pre_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult) override final;
    VkResult post_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult) override final;
    VkResult pre_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo, VkResult gvkResult) override final;
    VkResult post_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo, VkResult gvkResult) override final;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult pre_vkReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult) override final;
    VkResult post_vkReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult gvkResult) override final;
#endif // VK_USE_PLATFORM_WIN32_KHR
    VkResult pre_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo, VkResult gvkResult) override final;
    VkResult post_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo, VkResult gvkResult) override final;
    void pre_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) override final;
    void post_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) override final;
    void pre_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) override final;
    void post_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) override final;
    VkResult pre_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo, VkResult gvkResult) override final;
    VkResult post_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo, VkResult gvkResult) override final;
    void pre_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapchain, VkBool32 localDimmingEnable) override final;
    void post_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapchain, VkBool32 localDimmingEnable) override final;
    VkResult pre_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout, VkResult gvkResult) override final;
    VkResult post_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout, VkResult gvkResult) override final;
    VkResult pre_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult gvkResult) override final;
    VkResult post_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult gvkResult) override final;

private:
    VkResult get_command_buffer(const Queue& gvkQueue, VkCommandBuffer* pCommandBuffer);

    std::mutex mMutex;
    Instance mGvkInstance;
    std::set<Device> mGvkDevices;
    std::unordered_map<VkSwapchainKHR, Swapchain> mSwapchains;
    using QueueFamilyIndex = uint32_t;
    std::unordered_map<VkDevice, std::unordered_map<QueueFamilyIndex, std::pair<CommandPool, VkCommandBuffer>>> mCommandBuffers;
};

} // namespace virtual_swapchain
} // namespace gvk

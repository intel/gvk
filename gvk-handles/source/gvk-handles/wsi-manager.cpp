
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

#include "gvk-handles/wsi-manager.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-handles/context.hpp"
#include "gvk-handles/handles.hpp"
#include "gvk-handles/utilities.hpp"
#include "gvk-format-info.hpp"
#include "gvk-structures/defaults.hpp"

#include <algorithm>
#include <array>

namespace gvk {

VkResult WsiManager::create(const Device& device, const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, WsiManager* pWsiManager)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (device && pCreateInfo && pWsiManager) {
            pWsiManager->reset();
            pWsiManager->mDevice = device;
            pWsiManager->mAllocator = pAllocator ? *pAllocator : get_default<VkAllocationCallbacks>();
            gvk_result(pWsiManager->create_surface(pCreateInfo, validate_allocator(pWsiManager->mAllocator)));

            // Get VkPresentModeKHR
            uint32_t presentModeCount = 0;
            const auto& dispatchTable = device.get<PhysicalDevice>().get<DispatchTable>();
            assert(dispatchTable.gvkGetPhysicalDeviceSurfacePresentModesKHR);
            dispatchTable.gvkGetPhysicalDeviceSurfacePresentModesKHR(device.get<PhysicalDevice>(), pWsiManager->get_surface(), &presentModeCount, nullptr);
            std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
            dispatchTable.gvkGetPhysicalDeviceSurfacePresentModesKHR(device.get<PhysicalDevice>(), pWsiManager->get_surface(), &presentModeCount, availablePresentModes.data());
            pWsiManager->mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            for (auto availablePresentMode : availablePresentModes) {
                if (availablePresentMode == pCreateInfo->presentMode) {
                    pWsiManager->mPresentMode = pCreateInfo->presentMode;
                    break;
                }
            }

            // Get depth VkFormat
            if (pCreateInfo->depthFormat) {
                GvkFormatInfo formatInfo { };
                get_format_info(pCreateInfo->depthFormat, &formatInfo);
                assert(formatInfo.componentCount);
                assert(formatInfo.pComponents);
                auto requestedDepthBits = formatInfo.pComponents[0].bits;
                const auto& physicalDevice = device.get<PhysicalDevice>();
                enumerate_formats(
                    physicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceFormatProperties2,
                    physicalDevice,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
                    [&](VkFormat format)
                    {
                        // TODO : Abstract this logic into a utility function
                        if (format == pCreateInfo->depthFormat) {
                            pWsiManager->mDepthFormat = format;
                        }
                        if (pWsiManager->mDepthFormat != pCreateInfo->depthFormat) {
                            uint32_t actualDepthBits = 0;
                            if (pWsiManager->mDepthFormat) {
                                get_format_info(pWsiManager->mDepthFormat, &formatInfo);
                                assert(formatInfo.componentCount);
                                assert(formatInfo.pComponents);
                                actualDepthBits = formatInfo.pComponents[0].bits;
                            }
                            get_format_info(format, &formatInfo);
                            assert(formatInfo.componentCount);
                            assert(formatInfo.pComponents);
                            auto formatDepthBits = formatInfo.pComponents[0].bits;
                            if (actualDepthBits < formatDepthBits && formatDepthBits <= requestedDepthBits) {
                                pWsiManager->mDepthFormat = format;
                            }
                        }
                        return pWsiManager->mDepthFormat != pCreateInfo->depthFormat;
                    }
                );
            }

            // Get VkSampleCountFlagBits
            if (VK_SAMPLE_COUNT_1_BIT < pCreateInfo->sampleCount) {
                auto maxSampleCount = get_max_framebuffer_sample_count(device.get<gvk::PhysicalDevice>(), VK_TRUE, pWsiManager->mDepthFormat != VK_FORMAT_UNDEFINED, VK_FALSE);
                pWsiManager->mSampleCount = std::min(pCreateInfo->sampleCount, maxSampleCount);
            }

            // validate() causes gvk::WsiManager to create/recreate any resources that are
            //  invalid and returns a VkResult to indicate resource status
            pWsiManager->mStatus = pWsiManager->validate();
        }
    } gvk_result_scope_end;
    return gvkResult;
}

WsiManager::~WsiManager()
{
    reset();
}

void WsiManager::reset()
{
    invalidate();
    mDevice.reset();
    mSurface.reset();
}

const SurfaceKHR& WsiManager::get_surface() const
{
    return mSurface;
}

const SwapchainKHR& WsiManager::get_swapchain() const
{
    return mSwapchain;
}

const RenderPass& WsiManager::get_render_pass() const
{
    return mRenderPass;
}

const std::vector<RenderTarget>& WsiManager::get_render_targets() const
{
    return mRenderTargets;
}

const std::vector<Fence>& WsiManager::get_fences() const
{
    return mFences;
}

const std::vector<VkFence>& WsiManager::get_vk_fences() const
{
    return mVkFences;
}

const Semaphore& WsiManager::get_image_acquired_semaphore() const
{
    return mImageAcquiredSemaphore;
}

const Semaphore& WsiManager::get_image_rendered_semaphore() const
{
    return mImageRenderedSemaphore;
}

const std::vector<CommandBuffer>& WsiManager::get_command_buffers() const
{
    return mCommandBuffers;
}

VkResult WsiManager::get_status() const
{
    return mStatus;
}

VkBool32 WsiManager::is_enabled() const
{
    assert((mSwapchain == VK_NULL_HANDLE) == (mRenderPass == VK_NULL_HANDLE));
    assert((mSwapchain == VK_NULL_HANDLE) == mCommandBuffers.empty());
    assert((mSwapchain == VK_NULL_HANDLE) == mRenderTargets.empty());
    assert((mSwapchain == VK_NULL_HANDLE) == mFences.empty());
    assert((mSwapchain == VK_NULL_HANDLE) == mVkFences.empty());
    assert((mSwapchain == VK_NULL_HANDLE) == (mImageAcquiredSemaphore == VK_NULL_HANDLE));
    assert((mSwapchain == VK_NULL_HANDLE) == (mImageRenderedSemaphore == VK_NULL_HANDLE));
    return mSurface && mSwapchain;
}

uint32_t WsiManager::get_queue_family_index() const
{
    return mQueueFamilyIndex;
}

VkPresentModeKHR WsiManager::get_present_mode() const
{
    return mPresentMode;
}

VkSampleCountFlagBits WsiManager::get_sample_count() const
{
    return mSampleCount;
}

VkFormat WsiManager::get_color_format() const
{
    return mSwapchain ? mSwapchain.get<VkSwapchainCreateInfoKHR>().imageFormat : VK_FORMAT_UNDEFINED;
}

VkFormat WsiManager::get_depth_format() const
{
    return mDepthFormat;
}

VkBool32 WsiManager::update()
{
    return (validate() == VK_ERROR_OUT_OF_DATE_KHR || get_status() == VK_ERROR_OUT_OF_DATE_KHR) && is_enabled();
}

VkResult WsiManager::acquire_next_image(uint64_t timeout, VkFence vkFence, uint32_t* pImageIndex)
{
    assert(pImageIndex);
    assert(mImageAcquiredSemaphore);
    const auto& dispatchTable = mDevice.get<DispatchTable>();
    assert(dispatchTable.gvkAcquireNextImageKHR);
    mStatus = dispatchTable.gvkAcquireNextImageKHR(mDevice, mSwapchain, timeout, mImageAcquiredSemaphore, vkFence, pImageIndex);
    return mStatus;
}

VkSubmitInfo WsiManager::get_submit_info(uint32_t imageIndex) const
{
    assert(imageIndex < mCommandBuffers.size());
    static const VkPipelineStageFlags sWaitStage[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    auto submitInfo = get_default<VkSubmitInfo>();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mImageAcquiredSemaphore.get<VkSemaphore>();
    submitInfo.pWaitDstStageMask = sWaitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex].get<VkCommandBuffer>();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mImageRenderedSemaphore.get<VkSemaphore>();
    return submitInfo;
}

VkPresentInfoKHR WsiManager::get_present_info(const uint32_t* pImageIndex) const
{
    assert(pImageIndex);
    assert(mImageRenderedSemaphore);
    auto presentInfo = get_default<VkPresentInfoKHR>();
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mImageRenderedSemaphore.get<VkSemaphore>();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain.get<VkSwapchainKHR>();
    presentInfo.pImageIndices = pImageIndex;
    return presentInfo;
}

VkResult WsiManager::validate()
{
    assert(mSurface);
    VkSurfaceCapabilitiesKHR surfaceCapabilities { };
    const auto& dispatchTable = mDevice.get<PhysicalDevice>().get<DispatchTable>();
    assert(dispatchTable.gvkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    auto vkResult = dispatchTable.gvkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice.get<PhysicalDevice>(), mSurface, &surfaceCapabilities);
    bool swapchainImageExtentMatchesSurfaceCapabilities = mSwapchain && mSwapchain.get<VkSwapchainCreateInfoKHR>().imageExtent == surfaceCapabilities.currentExtent;
    if (mStatus != VK_SUCCESS || vkResult != VK_SUCCESS ||
        !surfaceCapabilities.currentExtent.width ||
        !surfaceCapabilities.currentExtent.height ||
        !swapchainImageExtentMatchesSurfaceCapabilities
    ) {
        invalidate();
    }
    if (surfaceCapabilities.currentExtent.width &&
        surfaceCapabilities.currentExtent.height &&
        !swapchainImageExtentMatchesSurfaceCapabilities
    ) {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            gvk_result(create_swapchain());
            gvk_result(create_render_pass());
            gvk_result(create_command_buffers());
            gvk_result(create_render_targets());
            gvk_result(create_synchronization_primitives());
        } gvk_result_scope_end
        vkResult = gvkResult;
        mStatus = VK_ERROR_OUT_OF_DATE_KHR;
    }
    if (vkResult == VK_SUCCESS) {
        std::swap(vkResult, mStatus);
    }
    return vkResult;
}

void WsiManager::invalidate()
{
    if (mDevice) {
        const auto& dispatchTable = mDevice.get<DispatchTable>();
        assert(dispatchTable.gvkDeviceWaitIdle);
        dispatchTable.gvkDeviceWaitIdle(mDevice);
    }
    mSwapchain.reset();
    mRenderPass.reset();
    mCommandBuffers.clear();
    mRenderTargets.clear();
    mFences.clear();
    mVkFences.clear();
    mImageAcquiredSemaphore.reset();
    mImageRenderedSemaphore.reset();
    mStatus = VK_ERROR_OUT_OF_DATE_KHR;
}

VkResult WsiManager::create_surface(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator)
{
    assert(mDevice);
    assert(pCreateInfo);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        #ifdef VK_USE_PLATFORM_ANDROID_KHR
        if (pCreateInfo->pAndroidSurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pAndroidSurfaceCreateInfoKHR, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_ANDROID_KHR
        #ifdef VK_USE_PLATFORM_DIRECTFB_EXT
        if (pCreateInfo->pWin32SurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pDirectFBSurfaceCreateInfoEXT, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_DIRECTFB_EXT
        if (pCreateInfo->pDisplaySurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pDisplaySurfaceCreateInfoKHR, pAllocator, &mSurface));
        }
        if (pCreateInfo->pHeadlessSurfaceCreateInfoEXT) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pHeadlessSurfaceCreateInfoEXT, pAllocator, &mSurface));
        }
        #ifdef VK_USE_PLATFORM_IOS_MVK
        if (pCreateInfo->pIOSSurfaceCreateInfoMVK) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pIOSSurfaceCreateInfoMVK, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_IOS_MVK
        #ifdef VK_USE_PLATFORM_FUCHSIA
        if (pCreateInfo->pImagePipeSurfaceCreateInfoFUCHSIA) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pImagePipeSurfaceCreateInfoFUCHSIA, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_FUCHSIA
        #ifdef VK_USE_PLATFORM_MACOS_MVK
        if (pCreateInfo->pMacOSSurfaceCreateInfoMVK) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pMacOSSurfaceCreateInfoMVK, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_MACOS_MVK
        #ifdef VK_USE_PLATFORM_METAL_EXT
        if (pCreateInfo->pMetalSurfaceCreateInfoEXT) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pMetalSurfaceCreateInfoEXT, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_METAL_EXT
        #ifdef VK_USE_PLATFORM_SCREEN_QNX
        if (pCreateInfo->pScreenSurfaceCreateInfoQNX) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pScreenSurfaceCreateInfoQNX, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_SCREEN_QNX
        #ifdef VK_USE_PLATFORM_GGP
        if (pCreateInfo->pStreamDescriptorSurfaceCreateInfoGGP) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pStreamDescriptorSurfaceCreateInfoGGP, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_GGP
        #ifdef VK_USE_PLATFORM_VI_NN
        if (pCreateInfo->pViSurfaceCreateInfoNN) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pViSurfaceCreateInfoNN, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_VI_NN
        #ifdef VK_USE_PLATFORM_WAYLAND_KHR
        if (pCreateInfo->pWaylandSurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pWaylandSurfaceCreateInfoKHR, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_WAYLAND_KHR
        #ifdef VK_USE_PLATFORM_WIN32_KHR
        if (pCreateInfo->pWin32SurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pWin32SurfaceCreateInfoKHR, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_WIN32_KHR
        #ifdef VK_USE_PLATFORM_XCB_KHR
        if (pCreateInfo->pXcbSurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pXcbSurfaceCreateInfoKHR, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_XCB_KHR
        #ifdef VK_USE_PLATFORM_XLIB_KHR
        if (pCreateInfo->pXlibSurfaceCreateInfoKHR) {
            gvk_result(SurfaceKHR::create(mDevice.get<Instance>(), pCreateInfo->pXlibSurfaceCreateInfoKHR, pAllocator, &mSurface));
        }
        #endif // VK_USE_PLATFORM_XLIB_KHR
        mQueueFamilyIndex = pCreateInfo->queueFamilyIndex;
        const auto& physicalDevice = mDevice.get<PhysicalDevice>();
        VkBool32 physicalDeviceSurfaceSupport = VK_FALSE;
        const auto& dispatchTable = physicalDevice.get<DispatchTable>();
        assert(dispatchTable.gvkGetPhysicalDeviceSurfaceSupportKHR);
        gvk_result(dispatchTable.gvkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mQueueFamilyIndex, mSurface, &physicalDeviceSurfaceSupport));
        gvk_result(gvkResult != VK_SUCCESS ? gvkResult : physicalDeviceSurfaceSupport ? gvkResult : VK_ERROR_FEATURE_NOT_PRESENT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult WsiManager::create_swapchain()
{
    assert(mDevice);
    assert(mSurface);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
        const auto& dispatchTable = mDevice.get<PhysicalDevice>().get<DispatchTable>();
        assert(dispatchTable.gvkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        gvk_result(dispatchTable.gvkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice.get<PhysicalDevice>(), mSurface, &surfaceCapabilities));
        if (surfaceCapabilities.currentExtent.width && surfaceCapabilities.currentExtent.height) {

            // Get VkSurfaceFormatKHR
            uint32_t surfaceFormatCount = 0;
            assert(dispatchTable.gvkGetPhysicalDeviceSurfaceFormatsKHR);
            dispatchTable.gvkGetPhysicalDeviceSurfaceFormatsKHR(mDevice.get<PhysicalDevice>(), mSurface, &surfaceFormatCount, nullptr);
            std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
            dispatchTable.gvkGetPhysicalDeviceSurfaceFormatsKHR(mDevice.get<PhysicalDevice>(), mSurface, &surfaceFormatCount, surfaceFormats.data());
            auto surfaceFormat = get_default<VkSurfaceFormatKHR>();
            if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
                surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
                surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            }
            if (!surfaceFormat.format) {
                for (const auto& candidateSurfaceFormat : surfaceFormats) {
                    if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
                        surfaceFormat = candidateSurfaceFormat;
                        break;
                    }
                }
            }
            if (!surfaceFormat.format) {
                surfaceFormat = surfaceFormats[0];
            }

            // Create gvk::SwapchainKHR
            auto minImageCount = surfaceCapabilities.minImageCount;
            auto maxImageCount = surfaceCapabilities.maxImageCount;
            auto swapchainCreateInfo = get_default<VkSwapchainCreateInfoKHR>();
            swapchainCreateInfo.surface = mSurface;
            swapchainCreateInfo.minImageCount = maxImageCount ? std::clamp(minImageCount + 1, minImageCount, maxImageCount) : minImageCount + 1;
            swapchainCreateInfo.imageFormat = surfaceFormat.format;
            swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
            swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
            swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
            swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swapchainCreateInfo.presentMode = mPresentMode;
            swapchainCreateInfo.clipped = VK_TRUE;
            gvk_result(SwapchainKHR::create(mDevice, &swapchainCreateInfo, validate_allocator(mAllocator), &mSwapchain));
        } else {
            invalidate();
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult WsiManager::create_render_pass()
{
    assert(mDevice);
    assert(mSurface);

    // MSAA VkAttachmentDescription2 and VkAttachmentReference2
    auto msaaAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    msaaAttachmentDescription.format = get_color_format();
    msaaAttachmentDescription.samples = mSampleCount;
    msaaAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    msaaAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    msaaAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    auto msaaAttachmentReference = gvk::get_default<VkAttachmentReference2>();
    msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    msaaAttachmentReference.aspectMask = get_image_aspect_flags(get_color_format());

    // Color VkAttachmentDescription2 and VkAttachmentReference2
    auto colorAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    colorAttachmentDescription.format = msaaAttachmentDescription.format;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    auto colorAttachmentReference = msaaAttachmentReference;

    // Depth VkAttachmentDescription2 and VkAttachmentReference2
    auto depthAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
    depthAttachmentDescription.format = mDepthFormat;
    depthAttachmentDescription.samples = mSampleCount;
    depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    auto depthAttachmentReference = gvk::get_default<VkAttachmentReference2>();
    depthAttachmentReference.layout = depthAttachmentDescription.finalLayout;
    depthAttachmentReference.aspectMask = get_image_aspect_flags(mDepthFormat);

    // Setup attachment descriptions and references
    uint32_t attachmentCount = 1;
    std::array<VkAttachmentDescription2, 3> attachmentDescriptions{
        msaaAttachmentDescription,
        colorAttachmentDescription,
        depthAttachmentDescription,
    };
    auto pAttachmentDescriptions = &attachmentDescriptions[1];
    if (VK_SAMPLE_COUNT_1_BIT < mSampleCount) {
        pAttachmentDescriptions = &attachmentDescriptions[0];
        colorAttachmentReference.attachment = 1;
        ++attachmentCount;
    }
    if (mDepthFormat) {
        depthAttachmentReference.attachment = colorAttachmentReference.attachment + 1;
        ++attachmentCount;
    }

    // Setup VkSubpassDescription2
    auto subpassDescription = get_default<VkSubpassDescription2>();
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = VK_SAMPLE_COUNT_1_BIT < mSampleCount ? &msaaAttachmentReference : &colorAttachmentReference;
    subpassDescription.pResolveAttachments = VK_SAMPLE_COUNT_1_BIT < mSampleCount ? &colorAttachmentReference : nullptr;
    subpassDescription.pDepthStencilAttachment = depthAttachmentDescription.format ? &depthAttachmentReference : nullptr;

    // Create gvk::RenderPass
    auto renderPassCreateInfo = get_default<VkRenderPassCreateInfo2>();
    renderPassCreateInfo.attachmentCount = attachmentCount;
    renderPassCreateInfo.pAttachments = pAttachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    return RenderPass::create(mDevice, &renderPassCreateInfo, validate_allocator(mAllocator), &mRenderPass);
}

VkResult WsiManager::create_command_buffers()
{
    assert(mDevice);
    assert(mSwapchain);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
        CommandPool commandPool;
        gvk_result(CommandPool::create(mDevice, &commandPoolCreateInfo, validate_allocator(mAllocator), &commandPool));
        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = (uint32_t)mSwapchain.get<Images>().size();
        mCommandBuffers.resize(commandBufferAllocateInfo.commandBufferCount);
        gvk_result(CommandBuffer::allocate(mDevice, &commandBufferAllocateInfo, mCommandBuffers.data()));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult WsiManager::create_render_targets()
{
    assert(mDevice);
    assert(mSwapchain);
    assert(mRenderPass);
    assert(!mCommandBuffers.empty());
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        mRenderTargets.clear();
        VkImageView msaaImageView = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;
        const auto& images = mSwapchain.get<Images>();
        if (!images.empty()) {
            mRenderTargets.reserve(images.size());
            for (size_t i = 0; i < images.size(); ++i) {
                const auto& image = images[i];

                // Create gvk::ImageView
                auto imageViewCreateInfo = get_default<VkImageViewCreateInfo>();
                imageViewCreateInfo.image = image;
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                imageViewCreateInfo.format = image.get<VkImageCreateInfo>().format;
                imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                ImageView imageView;
                gvk_result(ImageView::create(mDevice, &imageViewCreateInfo, validate_allocator(mAllocator), &imageView));

                // Prepare attachment array
                std::array<VkImageView, 3> attachments{ imageView, depthImageView };
                if (VK_SAMPLE_COUNT_1_BIT < get_sample_count()) {
                    attachments[0] = msaaImageView;
                    attachments[1] = imageView;
                    attachments[2] = depthImageView;
                }

                // Prepare VkFramebufferCreateInfo
                auto framebufferCreateInfo = get_default<VkFramebufferCreateInfo>();
                framebufferCreateInfo.renderPass = mRenderPass;
                framebufferCreateInfo.attachmentCount = mRenderPass.get<VkRenderPassCreateInfo2>().attachmentCount;
                framebufferCreateInfo.pAttachments = attachments.data();
                framebufferCreateInfo.width = image.get<VkImageCreateInfo>().extent.width;
                framebufferCreateInfo.height = image.get<VkImageCreateInfo>().extent.height;
                framebufferCreateInfo.layers = 1;

                // Create gvk::RenderTarget
                auto renderTargetCreateInfo = get_default<RenderTarget::CreateInfo>();
                renderTargetCreateInfo.pFramebufferCreateInfo = &framebufferCreateInfo;
                RenderTarget renderTarget;
                gvk_result(RenderTarget::create(mDevice, &renderTargetCreateInfo, validate_allocator(mAllocator), &renderTarget));

                // Cache handles for shared resources
                mRenderTargets.push_back(renderTarget);
                if (VK_SAMPLE_COUNT_1_BIT < get_sample_count()) {
                    msaaImageView = mRenderTargets.back().get_framebuffer().get<ImageViews>().front();
                }
                if (get_depth_format()) {
                    depthImageView = mRenderTargets.back().get_framebuffer().get<ImageViews>().back();
                }
            }

            // Transition shared resources to correct VkImageLayouts
            uint32_t imageMemoryBarrierCount = 0;
            std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers{ };
            if (msaaImageView) {
                auto& imageMemoryBarrier = imageMemoryBarriers[imageMemoryBarrierCount++];
                imageMemoryBarrier = mRenderTargets[0].get_image_memory_barrier(0);
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            if (depthImageView) {
                auto& imageMemoryBarrier = imageMemoryBarriers[imageMemoryBarrierCount++];
                auto attachmentIndex = mRenderPass.get<VkRenderPassCreateInfo2>().attachmentCount - 1;
                imageMemoryBarrier = mRenderTargets[0].get_image_memory_barrier(attachmentIndex);
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            auto queue = gvk::get_queue_family(mDevice, mQueueFamilyIndex).queues[0];
            gvk_result(gvk::execute_immediately(mDevice, queue, mCommandBuffers[0], VK_NULL_HANDLE,
                [&](auto)
                {
                    const auto& dispatchTable = mDevice.get<DispatchTable>();
                    assert(dispatchTable.gvkCmdPipelineBarrier);
                    dispatchTable.gvkCmdPipelineBarrier(
                        mCommandBuffers[0],
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        imageMemoryBarrierCount, imageMemoryBarriers.data()
                    );
                }
            ));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult WsiManager::create_synchronization_primitives()
{
    assert(mDevice);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto fenceCreateInfo = gvk::get_default<VkFenceCreateInfo>();
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        mFences.resize(mRenderTargets.size());
        mVkFences.resize(mRenderTargets.size());
        for (size_t i = 0; i < mRenderTargets.size(); ++i) {
            gvk_result(Fence::create(mDevice, &fenceCreateInfo, validate_allocator(mAllocator), &mFences[i]));
            mVkFences[i] = mFences[i];
        }
        gvk_result(Semaphore::create(mDevice, &get_default<VkSemaphoreCreateInfo>(), validate_allocator(mAllocator), &mImageAcquiredSemaphore));
        gvk_result(Semaphore::create(mDevice, &get_default<VkSemaphoreCreateInfo>(), validate_allocator(mAllocator), &mImageRenderedSemaphore));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk


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

#include "gvk-handles/wsi-context.hpp"
#include "gvk-handles/context.hpp"
#include "gvk-handles/handles.hpp"
#include "gvk-handles/utilities.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-format-info.hpp"

#include <algorithm>
#include <array>

namespace gvk {
namespace wsi {

VkResult Context::create(const Device& device, const SurfaceKHR& surface, const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext)
{
    (void)pAllocator;
    assert(device);
    assert(surface);
    assert(pCreateInfo);
    assert(pContext);
    pContext->mReference.reset(newref);
    auto& controlBlock = pContext->mReference.get_obj();
    controlBlock.mCreateInfo = *pCreateInfo;
    controlBlock.mDevice = device;
    controlBlock.mSurface = surface;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        VkBool32 physicalDeviceSurfaceSupport = VK_FALSE;
        gvk_result(device.get<PhysicalDevice>().GetPhysicalDeviceSurfaceSupportKHR(controlBlock.mCreateInfo.queueFamilyIndex, surface, &physicalDeviceSurfaceSupport));
        gvk_result(gvkResult != VK_SUCCESS ? gvkResult : physicalDeviceSurfaceSupport ? gvkResult : VK_ERROR_FEATURE_NOT_PRESENT);
        controlBlock.mInfo.queueFamilyIndex = controlBlock.mCreateInfo.queueFamilyIndex;
        controlBlock.mInfo.maxFramesInFlight = std::max(1u, controlBlock.mCreateInfo.maxFramesInFlight);
        gvk_result(pContext->create_command_resources(&controlBlock.mCommandResources));
        gvk_result(pContext->validate_swapchain_resources());
    } gvk_result_scope_end;
    if (gvkResult != VK_SUCCESS) {
        *pContext = nullref;
    }
    return gvkResult;
}

Context::~Context()
{
}

VkResult Context::acquire_next_image(uint64_t timeout, VkFence vkFence, AcquiredImageInfo* pAcquiredImageInfo, RenderTarget* pRenderTarget)
{
    assert(pAcquiredImageInfo);
    *pAcquiredImageInfo = { };
    if (pRenderTarget) {
        *pRenderTarget = gvk::nullref;
    }

    // Get CommandResources
    auto frameResourceIndex = get<Info>().frameCount % get<Info>().maxFramesInFlight;
    assert(frameResourceIndex < get<std::vector<CommandResources>>().size());
    const auto& commandResources = get<std::vector<CommandResources>>()[frameResourceIndex];

    gvk_result_scope_begin(VK_INCOMPLETE) {
        const auto& device = get<Device>();
        const auto& swapchain = get<SwapchainKHR>();

        // Wait on VkFence to ensure resources aren't in use
        gvk_result(device.WaitForFences(1, &commandResources.fence.get<VkFence>(), VK_TRUE, UINT64_MAX));

        // Call vkAcquireNextImageKHR(), if VK_ERROR_OUT_OF_DATE_KHR recreate resources
        pAcquiredImageInfo->swapchain = swapchain;
        pAcquiredImageInfo->status = device.AcquireNextImageKHR(swapchain, timeout, commandResources.imageAcquiredSemaphore, vkFence, &pAcquiredImageInfo->index);
        switch (pAcquiredImageInfo->status) {
        case VK_ERROR_OUT_OF_DATE_KHR: { gvk_result(validate_swapchain_resources()); gvk_result_scope_break(pAcquiredImageInfo->status); } break;
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:        { gvkResult = pAcquiredImageInfo->status; } break;
        default:                       { gvk_result(pAcquiredImageInfo->status); } break;
        }

        // Reset VkFence
        gvk_result(device.ResetFences(1, &commandResources.fence.get<VkFence>()));

        // Populate the AcquiredImageInfo
        pAcquiredImageInfo->image = swapchain.get<Images>()[pAcquiredImageInfo->index];
        pAcquiredImageInfo->commandBuffer = commandResources.commandBuffer;
        pAcquiredImageInfo->imageAcquiredSemaphore = commandResources.imageAcquiredSemaphore;
        pAcquiredImageInfo->presentWaitSemaphore = commandResources.presentWaitSemaphore;
        pAcquiredImageInfo->fence = commandResources.fence;

        // Populate the gvk::RenderTarget if argument is providedS
        if (pRenderTarget) {
            *pRenderTarget = get<RenderTargets>()[pAcquiredImageInfo->index];
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Context::queue_present(const Queue& queue, const AcquiredImageInfo* pAcquiredImageInfo)
{
    assert(queue);
    assert(pAcquiredImageInfo);
    gvk_result_scope_begin(VK_INCOMPLETE) {

        // Call vkQueuePresentKHR()
        auto status = queue.QueuePresentKHR(&get<VkPresentInfoKHR>(*pAcquiredImageInfo));

        // Get VkSurfaceCapabilitiesKHR
        VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
        gvk_result(get<Device>().get<PhysicalDevice>().GetPhysicalDeviceSurfaceCapabilitiesKHR(get<SurfaceKHR>(), &surfaceCapabilities));

        // If status is anything but VK_SUCCESS (and isn't an actual failure), or if the
        //  gvk::SurfaceKHR and gvk::SwapchainKHR extents don't match recreate resources.
        if (status == VK_SUBOPTIMAL_KHR ||
            status == VK_ERROR_OUT_OF_DATE_KHR ||
            pAcquiredImageInfo->status == VK_SUBOPTIMAL_KHR ||
            pAcquiredImageInfo->status == VK_ERROR_OUT_OF_DATE_KHR ||
            get<SwapchainKHR>().get<VkSwapchainCreateInfoKHR>().imageExtent != surfaceCapabilities.currentExtent) {
            gvk_result(validate_swapchain_resources());
            gvkResult = status;
        }
    } gvk_result_scope_end;
    ++mReference->mInfo.frameCount;
    return gvkResult;
}

VkPresentModeKHR Context::select_present_mode(VkPresentModeKHR requestedPresentMode) const
{
    uint32_t availablePresentModeCount = 0;
    get<Device>().get<PhysicalDevice>().GetPhysicalDeviceSurfacePresentModesKHR(get<SurfaceKHR>(), &availablePresentModeCount, nullptr);
    std::vector<VkPresentModeKHR> availablePresentModes(availablePresentModeCount);
    get<Device>().get<PhysicalDevice>().GetPhysicalDeviceSurfacePresentModesKHR(get<SurfaceKHR>(), &availablePresentModeCount, availablePresentModes.data());
    for (const auto& avaialablePresentMode : availablePresentModes) {
        if (avaialablePresentMode == requestedPresentMode) {
            return avaialablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Context::select_surface_format(const VkSurfaceFormatKHR& requestedSurfaceFormat) const
{
    gvk_result_scope_begin(VK_INCOMPLETE) {
        uint32_t surfaceFormatCount = 0;
        get<Device>().get<PhysicalDevice>().GetPhysicalDeviceSurfaceFormatsKHR(get<SurfaceKHR>(), &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        get<Device>().get<PhysicalDevice>().GetPhysicalDeviceSurfaceFormatsKHR(get<SurfaceKHR>(), &surfaceFormatCount, surfaceFormats.data());
        gvk_result(surfaceFormats.size() ? VK_SUCCESS : VK_INCOMPLETE);
        for (const auto& surfaceFormat : surfaceFormats) {
            if (requestedSurfaceFormat.format) {
                if (requestedSurfaceFormat.format == surfaceFormat.format &&
                    requestedSurfaceFormat.colorSpace == surfaceFormat.colorSpace) {
                    return surfaceFormat;
                }
            } else if(surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                GvkFormatInfo formatInfo{ };
                get_format_info(surfaceFormat.format, &formatInfo);
                if (formatInfo.componentCount == 4 &&
                    formatInfo.numericFormat == GVK_NUMERIC_FORMAT_UNORM &&
                    formatInfo.classCount == 1 && formatInfo.pClasses[0] == GVK_FORMAT_CLASS_32_BIT) {
                    return surfaceFormat;
                }
            }
        }
        return surfaceFormats[0];
    } gvk_result_scope_end;
    return { };
}

VkFormat Context::select_depth_format(VkFormat requestedDepthFormat) const
{
    return get_max_depth_format(get<Device>().get<PhysicalDevice>(), requestedDepthFormat);
}

VkSampleCountFlagBits Context::select_sample_count(VkFormat depthFormat, VkSampleCountFlagBits requestedSampleCount) const
{
    if (VK_SAMPLE_COUNT_1_BIT < requestedSampleCount) {
        auto maxSampleCount = get_max_framebuffer_sample_count(get<Device>().get<PhysicalDevice>(), VK_TRUE, depthFormat, VK_FALSE);
        return std::min(requestedSampleCount, maxSampleCount);
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

VkResult Context::create_command_resources(std::vector<CommandResources>* pCommandResources) const
{
    assert(pCommandResources);
    auto& commandResources = *pCommandResources;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Create gvk::CommandPool
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = get<Info>().queueFamilyIndex;
        CommandPool commandPool = VK_NULL_HANDLE;
        gvk_result(CommandPool::create(get<Device>(), &commandPoolCreateInfo, nullptr, &commandPool));

        commandResources.resize(get<Info>().maxFramesInFlight);
        for (uint32_t i = 0; i < get<Info>().maxFramesInFlight; ++i) {
            // Allocate gvk::CommandBuffer
            auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
            commandBufferAllocateInfo.commandPool = commandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;
            gvk_result(CommandBuffer::allocate(get<Device>(), &commandBufferAllocateInfo, &commandResources[i].commandBuffer));

            // Create synchronization primitives
            auto fenceCreateInfo = gvk::get_default<VkFenceCreateInfo>();
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            gvk_result(Fence::create(get<Device>(), &fenceCreateInfo, nullptr, &commandResources[i].fence));
            gvk_result(Semaphore::create(get<Device>(), &get_default<VkSemaphoreCreateInfo>(), nullptr, &commandResources[i].imageAcquiredSemaphore));
            gvk_result(Semaphore::create(get<Device>(), &get_default<VkSemaphoreCreateInfo>(), nullptr, &commandResources[i].presentWaitSemaphore));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Context::create_swapchain(const VkSurfaceCapabilitiesKHR* pSurfaceCapabilities, SwapchainKHR* pSwapchain) const
{
    assert(pSurfaceCapabilities);
    assert(pSwapchain);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto minImageCount = pSurfaceCapabilities->minImageCount;
        auto maxImageCount = pSurfaceCapabilities->maxImageCount;
        auto swapchainCreateInfo = get_default<VkSwapchainCreateInfoKHR>();
        swapchainCreateInfo.surface = get<SurfaceKHR>();
        swapchainCreateInfo.minImageCount = maxImageCount ? std::clamp(minImageCount + 1, minImageCount, maxImageCount) : minImageCount + 1;
        swapchainCreateInfo.imageFormat = get<Info>().surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = get<Info>().surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = get<Info>().extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = get<Info>().imageUsage;
        swapchainCreateInfo.imageSharingMode = { };
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        swapchainCreateInfo.preTransform = get<Info>().transform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = get<Info>().presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = *pSwapchain;
        gvk_result(SwapchainKHR::create(get<Device>(), &swapchainCreateInfo, nullptr, pSwapchain));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Context::create_render_pass(RenderPass* pRenderPass) const
{
    assert(pRenderPass);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto colorFormat = get<Info>().surfaceFormat.format;
        auto depthFormat = get<Info>().depthFormat;
        auto sampleCount = get<Info>().sampleCount;

        // MSAA VkAttachmentDescription2 and VkAttachmentReference2
        auto msaaAttachmentDescription = gvk::get_default<VkAttachmentDescription2>();
        msaaAttachmentDescription.format = colorFormat;
        msaaAttachmentDescription.samples = sampleCount;
        msaaAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        msaaAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        msaaAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        auto msaaAttachmentReference = gvk::get_default<VkAttachmentReference2>();
        msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        msaaAttachmentReference.aspectMask = get_image_aspect_flags(msaaAttachmentDescription.format);

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
        depthAttachmentDescription.format = depthFormat;
        depthAttachmentDescription.samples = sampleCount;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        auto depthAttachmentReference = gvk::get_default<VkAttachmentReference2>();
        depthAttachmentReference.layout = depthAttachmentDescription.finalLayout;
        depthAttachmentReference.aspectMask = get_image_aspect_flags(depthAttachmentDescription.format);

        // Setup attachment descriptions and references
        uint32_t attachmentCount = 1;
        std::array<VkAttachmentDescription2, 3> attachmentDescriptions{
            msaaAttachmentDescription,
            colorAttachmentDescription,
            depthAttachmentDescription,
        };
        auto pAttachmentDescriptions = &attachmentDescriptions[1];
        if (VK_SAMPLE_COUNT_1_BIT < sampleCount) {
            pAttachmentDescriptions = &attachmentDescriptions[0];
            colorAttachmentReference.attachment = 1;
            ++attachmentCount;
        }
        if (depthAttachmentDescription.format) {
            depthAttachmentReference.attachment = colorAttachmentReference.attachment + 1;
            ++attachmentCount;
        }

        // Setup VkSubpassDescription2
        auto subpassDescription = get_default<VkSubpassDescription2>();
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = VK_SAMPLE_COUNT_1_BIT < sampleCount ? &msaaAttachmentReference : &colorAttachmentReference;
        subpassDescription.pResolveAttachments = VK_SAMPLE_COUNT_1_BIT < sampleCount ? &colorAttachmentReference : nullptr;
        subpassDescription.pDepthStencilAttachment = depthAttachmentDescription.format ? &depthAttachmentReference : nullptr;

        // Create gvk::RenderPass
        auto renderPassCreateInfo = get_default<VkRenderPassCreateInfo2>();
        renderPassCreateInfo.attachmentCount = attachmentCount;
        renderPassCreateInfo.pAttachments = pAttachmentDescriptions;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        gvk_result(RenderPass::create(get<Device>(), &renderPassCreateInfo, nullptr, pRenderPass));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Context::create_render_targets(std::vector<RenderTarget>* pRenderTargets) const
{
    assert(pRenderTargets);
    pRenderTargets->clear();
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        VkImageView msaaImageView = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;
        const auto& images = get<SwapchainKHR>().get<Images>();
        pRenderTargets->reserve(images.size());
        for (uint32_t i = 0; i < images.size(); ++i) {

            // Create gvk::ImageView
            auto imageViewCreateInfo = get_default<VkImageViewCreateInfo>();
            imageViewCreateInfo.image = images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = images[i].get<VkImageCreateInfo>().format;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ImageView imageView;
            gvk_result(ImageView::create(get<Device>(), &imageViewCreateInfo, nullptr, &imageView));

            // Prepare attachment array
            auto sampleCount = get<Info>().sampleCount;
            std::array<VkImageView, 3> attachments{ imageView, depthImageView };
            if (VK_SAMPLE_COUNT_1_BIT < sampleCount) {
                attachments[0] = msaaImageView;
                attachments[1] = imageView;
                attachments[2] = depthImageView;
            }

            // Create gvk::RenderTarget
            auto renderTargetCreateInfo = get_default<VkFramebufferCreateInfo>();
            renderTargetCreateInfo.renderPass = get<RenderPass>();
            renderTargetCreateInfo.attachmentCount = get<RenderPass>().get<VkRenderPassCreateInfo2>().attachmentCount;
            renderTargetCreateInfo.pAttachments = attachments.data();
            renderTargetCreateInfo.width = images[i].get<VkImageCreateInfo>().extent.width;
            renderTargetCreateInfo.height = images[i].get<VkImageCreateInfo>().extent.height;
            renderTargetCreateInfo.layers = 1;
            RenderTarget renderTarget;
            gvk_result(RenderTarget::create(get<Device>(), &renderTargetCreateInfo, nullptr, &renderTarget));
            pRenderTargets->push_back(renderTarget);

            // Cache handles for shared resources
            if (!msaaImageView && VK_SAMPLE_COUNT_1_BIT < sampleCount) {
                msaaImageView = renderTarget.get<Framebuffer>().get<ImageViews>().front();
            }
            if (!depthImageView && get<Info>().depthFormat) {
                depthImageView = renderTarget.get<Framebuffer>().get<ImageViews>().back();
            }
        }

        // Transition shared resources to correct VkImageLayouts
        uint32_t imageMemoryBarrierCount = 0;
        std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers{ };
        if (msaaImageView) {
            auto& imageMemoryBarrier = imageMemoryBarriers[imageMemoryBarrierCount++];
            imageMemoryBarrier = get<RenderTargets>()[0].get<VkImageMemoryBarrier>(0);
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        if (depthImageView) {
            auto& imageMemoryBarrier = imageMemoryBarriers[imageMemoryBarrierCount++];
            auto attachmentIndex = get<RenderPass>().get<VkRenderPassCreateInfo2>().attachmentCount - 1;
            imageMemoryBarrier = get<RenderTargets>()[0].get<VkImageMemoryBarrier>(attachmentIndex);
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        auto queue = gvk::get_queue_family(get<Device>(), get<Info>().queueFamilyIndex).queues[0];
        const auto& commandBuffer = get<std::vector<CommandResources>>()[0].commandBuffer;
        gvk_result(gvk::execute_immediately(get<Device>(), queue, commandBuffer, VK_NULL_HANDLE,
            [&](auto)
            {
                commandBuffer.CmdPipelineBarrier(
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    imageMemoryBarrierCount, imageMemoryBarriers.data()
                );
            }
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Context::validate_swapchain_resources()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
        gvk_result(get<Device>().get<PhysicalDevice>().GetPhysicalDeviceSurfaceCapabilitiesKHR(get<SurfaceKHR>(), &surfaceCapabilities));
        if (surfaceCapabilities.currentExtent.width && surfaceCapabilities.currentExtent.height) {
            auto& controlBlock = mReference.get_obj();
            controlBlock.mInfo.presentMode = select_present_mode(controlBlock.mCreateInfo.presentMode);
            controlBlock.mInfo.surfaceFormat = select_surface_format(controlBlock.mCreateInfo.surfaceFormat);
            controlBlock.mInfo.depthFormat = select_depth_format(controlBlock.mCreateInfo.depthFormat);
            controlBlock.mInfo.sampleCount = select_sample_count(controlBlock.mInfo.depthFormat, controlBlock.mCreateInfo.sampleCount);
            controlBlock.mInfo.imageUsage = surfaceCapabilities.supportedUsageFlags & controlBlock.mCreateInfo.imageUsage;
            controlBlock.mInfo.extent = surfaceCapabilities.currentExtent;
            controlBlock.mInfo.transform = surfaceCapabilities.currentTransform;
            gvk_result(get<Device>().DeviceWaitIdle());
            gvk_result(create_swapchain(&surfaceCapabilities, &controlBlock.mSwapchain));
            gvk_result(create_render_pass(&controlBlock.mRenderPass));
            gvk_result(create_render_targets(&controlBlock.mRenderTargets));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

Context::ControlBlock::~ControlBlock()
{
}

} // namespace wsi
} // namespace gvk

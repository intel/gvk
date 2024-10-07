
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
#include "gvk-handles/handles.hpp"
#include "gvk-handles/render-target.hpp"

namespace gvk {
namespace wsi {

/**
Info regarding a VkImage acquired from a VkSwapchainKHR
    @note Once acquired via gvk::wsi::Context::acquire_next_image(), presentation must be performed via gvk::wsi::Context::queue_present()
*/
struct AcquiredImageInfo
{
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    VkResult status{ VK_INCOMPLETE };
    uint32_t index{ UINT32_MAX };
    VkImage image{ VK_NULL_HANDLE };
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkSemaphore imageAcquiredSemaphore{ VK_NULL_HANDLE };
    VkSemaphore presentWaitSemaphore{ VK_NULL_HANDLE };
    VkFence fence{ VK_NULL_HANDLE };
};

/**
Provides high level control over window system integration
*/
class Context
{
public:
    /**
    gvk::wsi::Context creation parameters
    */
    struct CreateInfo
    {
        /**
        The family index of the Queue that gvk::wsi::Context CommandBuffer objects will be submitted to
        */
        uint32_t queueFamilyIndex{ };

        /**
        The max number of frames allowed in flight at one time
        */
        uint32_t maxFramesInFlight{ 2 };

        /**
        The VkPresentModeKHR to request for the gvk::wsi::Context SwapchainKHR
            @note If the requested VkPresentModeKHR is unavailable, VK_PRESENT_MODE_FIFO_KHR will be selected
        */
        VkPresentModeKHR presentMode{ VK_PRESENT_MODE_MAILBOX_KHR };

        /**
        The VkSurfaceFormatKHR to request for the gvk::wsi::Context SwapchainKHR
            @note If the requested VkSurfaceFormatKHR is unavailable, the first 4 channel UNORM format with sRGB nonilnear color space will be selected, if unavailable the first available VkSurfaceFormatKHR will be selected
        */
        VkSurfaceFormatKHR surfaceFormat{ };

        /**
        The VkFormat to request for depth buffering
            @note VK_FORMAT_UNDEFINED will result in no depth buffer
            @note The supported VkFormat with the highest bitcount that is less than or equal to the requested VkFormat will be selected
        */
        VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

        /**
        The VkSampleCountFlagBits to request for multisample anti-aliasing (MSAA)
            @note The supported VkSampleCountFlagBits with the highest sample count that is less than or equal to the requested VkSampleCountFlagBits will be selected
        */
        VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };

        /**
        The VkImageUsageFlags to request
            @note Any flags that are unsupported will be omitted
        */
        VkImageUsageFlags imageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
    };

    /**
    Current gvk::wsi::Context properties
        @note These values may differ from CreateInfo values if requestsed parameters were unavailable
    */
    struct Info
    {
        uint32_t maxFramesInFlight{ };
        uint32_t queueFamilyIndex{ };
        VkPresentModeKHR presentMode{ };
        VkSurfaceFormatKHR surfaceFormat{ };
        VkFormat depthFormat{ };
        VkSampleCountFlagBits sampleCount{ };
        VkImageUsageFlags imageUsage{ };
        VkExtent2D extent{ };
        VkSurfaceTransformFlagBitsKHR transform{ };
        uint64_t frameCount{ };
    };

public:
    static VkResult create(const Device& device, const SurfaceKHR& surface, const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext);
    virtual ~Context();

    template <typename T>
    inline const T& get() const
    {
        assert(mReference && "Attempting to dereference nullref gvk::wsi::Context");
        if constexpr (std::is_same_v<T, Info>) { return mReference->mInfo; }
        if constexpr (std::is_same_v<T, Device>) { return mReference->mDevice; }
        if constexpr (std::is_same_v<T, SurfaceKHR>) { return mReference->mSurface; }
        if constexpr (std::is_same_v<T, std::vector<CommandResources>>) { return mReference->mCommandResources; }
        if constexpr (std::is_same_v<T, SwapchainKHR>) { return mReference->mSwapchain; }
        if constexpr (std::is_same_v<T, RenderPass>) { return mReference->mRenderPass; }
        if constexpr (std::is_same_v<T, RenderTargets>) { return mReference->mRenderTargets; }
    }

    template <typename T>
    inline const T& get(const AcquiredImageInfo& acquiredImageInfo) const
    {
        assert(mReference && "Attempting to dereference nullref gvk::wsi::Context");
        if constexpr (std::is_same_v<T, VkSubmitInfo>)
        {
            thread_local VkPipelineStageFlags tlWaitStage[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            thread_local auto tlSubmitInfo = get_default<VkSubmitInfo>();
            tlSubmitInfo.waitSemaphoreCount = 1;
            tlSubmitInfo.pWaitSemaphores = &acquiredImageInfo.imageAcquiredSemaphore;
            tlSubmitInfo.pWaitDstStageMask = tlWaitStage;
            tlSubmitInfo.commandBufferCount = 1;
            tlSubmitInfo.pCommandBuffers = &acquiredImageInfo.commandBuffer;
            tlSubmitInfo.signalSemaphoreCount = 1;
            tlSubmitInfo.pSignalSemaphores = &acquiredImageInfo.presentWaitSemaphore;
            return tlSubmitInfo;
        }
        if constexpr (std::is_same_v<T, VkPresentInfoKHR>)
        {
            thread_local auto tlPresentInfo = get_default<VkPresentInfoKHR>();
            tlPresentInfo.waitSemaphoreCount = 1;
            tlPresentInfo.pWaitSemaphores = &acquiredImageInfo.presentWaitSemaphore;
            tlPresentInfo.swapchainCount = 1;
            tlPresentInfo.pSwapchains = &acquiredImageInfo.swapchain;
            tlPresentInfo.pImageIndices = &acquiredImageInfo.index;
            return tlPresentInfo;
        }
    }

    VkResult acquire_next_image(uint64_t timeout, VkFence vkFence, AcquiredImageInfo* pAcquiredImageInfo, RenderTarget* pRenderTarget = nullptr);
    VkResult queue_present(const Queue& queue, const AcquiredImageInfo* pAcquiredImage);

    class CommandResources final
    {
    public:
        CommandBuffer commandBuffer{ VK_NULL_HANDLE };
        Semaphore imageAcquiredSemaphore{ VK_NULL_HANDLE };
        Semaphore presentWaitSemaphore{ VK_NULL_HANDLE };
        Fence fence{ VK_NULL_HANDLE };
    };

protected:
    virtual VkPresentModeKHR select_present_mode(VkPresentModeKHR requestedPresentMode) const;
    virtual VkSurfaceFormatKHR select_surface_format(const VkSurfaceFormatKHR& requestedSurfaceFormat) const;
    virtual VkFormat select_depth_format(VkFormat requestedDepthFormat) const;
    virtual VkSampleCountFlagBits select_sample_count(VkFormat depthFormat, VkSampleCountFlagBits requestedSampleCount) const;
    virtual VkResult create_command_resources(std::vector<CommandResources>* pCommandResources) const;
    virtual VkResult create_swapchain(const VkSurfaceCapabilitiesKHR* pSurfaceCapabilities, SwapchainKHR* pSwapchain) const;
    virtual VkResult create_render_pass(RenderPass* pRenderPass) const;
    virtual VkResult create_render_targets(std::vector<RenderTarget>* pRenderTargets) const;

private:
    VkResult validate_swapchain_resources();

    class ControlBlock final
    {
    public:
        ControlBlock() = default;
        ~ControlBlock();
        CreateInfo mCreateInfo{ };
        Info mInfo{ };
        Device mDevice{ VK_NULL_HANDLE };
        SurfaceKHR mSurface{ VK_NULL_HANDLE };
        std::vector<CommandResources> mCommandResources;
        SwapchainKHR mSwapchain{ VK_NULL_HANDLE };
        RenderPass mRenderPass{ VK_NULL_HANDLE };
        std::vector<RenderTarget> mRenderTargets;
    private:
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock& operator=(const ControlBlock&) = delete;
    };

    gvk_reference_type(Context)
};

} // namespace wsi
} // namespace gvk

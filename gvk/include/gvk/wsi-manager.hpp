
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include "gvk/defines.hpp"
#include "gvk/handles.hpp"
#include "gvk/render-target.hpp"

namespace gvk {

/**
Provides high level control over window system integration
*/
class WsiManager final
{
public:
    /**
    Creation parameters for gvk::WsiManager
    */
    struct CreateInfo
    {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        const VkAndroidSurfaceCreateInfoKHR* pAndroidSurfaceCreateInfoKHR{ };
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
        const VkDirectFBSurfaceCreateInfoEXT* pDirectFBSurfaceCreateInfoEXT{ };
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
        const VkDisplaySurfaceCreateInfoKHR* pDisplaySurfaceCreateInfoKHR{ };
        const VkHeadlessSurfaceCreateInfoEXT* pHeadlessSurfaceCreateInfoEXT{ };
#ifdef VK_USE_PLATFORM_IOS_MVK
        const VkIOSSurfaceCreateInfoMVK* pIOSSurfaceCreateInfoMVK{ };
#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_FUCHSIA
        const VkImagePipeSurfaceCreateInfoFUCHSIA* pImagePipeSurfaceCreateInfoFUCHSIA{ };
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_MACOS_MVK
        const VkMacOSSurfaceCreateInfoMVK* pMacOSSurfaceCreateInfoMVK{ };
#endif // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_METAL_EXT
        const VkMetalSurfaceCreateInfoEXT* pMetalSurfaceCreateInfoEXT{ };
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        const VkScreenSurfaceCreateInfoQNX* pScreenSurfaceCreateInfoQNX{ };
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_GGP
        const VkStreamDescriptorSurfaceCreateInfoGGP* pStreamDescriptorSurfaceCreateInfoGGP{ };
#endif // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_VI_NN
        const VkViSurfaceCreateInfoNN* pViSurfaceCreateInfoNN{ };
#endif // VK_USE_PLATFORM_VI_NN
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        const VkWaylandSurfaceCreateInfoKHR* pWaylandSurfaceCreateInfoKHR{ };
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        const VkWin32SurfaceCreateInfoKHR* pWin32SurfaceCreateInfoKHR{ };
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
        const VkXcbSurfaceCreateInfoKHR* pXcbSurfaceCreateInfoKHR{ };
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
        const VkXlibSurfaceCreateInfoKHR* pXlibSurfaceCreateInfoKHR{ };
#endif // VK_USE_PLATFORM_XLIB_KHR

        /**
        The family index of the gvk::Queue that gvk::WsiManager gvk::CommandBuffer objects will be submitted to
        */
        uint32_t queueFamilyIndex{ };

        /**
        The VkPresentModeKHR to request for the gvk::WsiManager gvk::SwapchainKHR
            @note If the requested VkPresentModeKHR is unavailable, VK_PRESENT_MODE_FIFO_KHR will be selected
        */
        VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };

        /**
        The VkSampleCountFlagBits to request for multisample anti-aliasing (MSAA)
            @note The supported VkSampleCountFlagBits with the highest sample count that is less than or equal to the requested VkSampleCountFlagBits will be selected
        */
        VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };

        /**
        The VkFormat to request for depth buffering
            @note VK_FORMAT_UNDEFINED will result in no depth buffer
            @note The supported VkFormat with the highest bitcount that is less than or equal to the requested VkFormat will be selected
        */
        VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    };

    /**
    Creates an instance of gvk::WsiManager
    @param [in] device The gvk::Device used to create gvk::WsiManager resources
    @param [in] pCreateInfo A pointer to the gvk::WsiManager creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @param [in,out] pRenderTarget A pointer to the gvk::WsiManager to create
    @return The VkResult
    */
    static VkResult create(const Device& device, const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, WsiManager* pWsiManager);

    /**
    Creates an instance of WsiManager
    */
    WsiManager() = default;

    /**
    Moves an instance of gvk::WsiManager
    @param [in] other The gvk::WsiManager to move from
    */
    WsiManager(WsiManager&& other) = default;

    /**
    Moves an instance of gvk::WsiManager
    @param [in] other The gvk::WsiManager to move from
    @return A reference to this gvk::WsiManager
    */
    WsiManager& operator=(WsiManager&& other) = default;

    /**
    Destroys this instance of gvk::WsiManager
    */
    virtual ~WsiManager();

    /**
    Destroys this instance of gvk::WsiManager
    */
    void reset();

    /**
    Gets this gvk::WsiManager object's gvk::SurfaceKHR
    @return This gvk::WsiManager object's gvk::SurfaceKHR object
    */
    const SurfaceKHR& get_surface() const;

    /**
    Gets this gvk::WsiManager object's gvk::SwapchainKHR
    @return This gvk::WsiManager object's gvk::SwapchainKHR object
    */
    const SwapchainKHR& get_swapchain() const;

    /**
    Gets this gvk::WsiManager object's gvk::RenderPass
    @return This gvk::WsiManager object's gvk::RenderPass object
    */
    const RenderPass& get_render_pass() const;

    /**
    Gets this gvk::WsiManager object's gvk::CommandBuffer objects
    @return This gvk::WsiManager object's gvk::CommandBuffer objects
    */
    const std::vector<CommandBuffer>& get_command_buffers() const;

    /**
    Gets this gvk::WsiManager object's gvk::RenderTarget objects
    @return This gvk::WsiManager object's gvk::RenderTarget objects
    */
    const std::vector<RenderTarget>& get_render_targets() const;

    /**
    Gets this gvk::WsiManager object's gvk::Fence objects
    @return This gvk::WsiManager object's gvk::Fence objects
    */
    const std::vector<Fence>& get_fences() const;

    /**
    Gets this gvk::WsiManager object's image acquired gvk::Semaphore
    @return This gvk::WsiManager object's image acquired gvk::Semaphore
    */
    const Semaphore& get_image_acquired_semaphore() const;

    /**
    Gets this gvk::WsiManager object's image rendered gvk::Semaphore
    @return This gvk::WsiManager object's image rendered gvk::Semaphore
    */
    const Semaphore& get_image_rendered_semaphore() const;

    /**
    Gets a value indicating whether or not this gvk::WsiManager object's resources are up to date
    @return This gvk::WsiManager object's status
        @note gvk::WsiManager resources may become out of date when the gvk::sys::Surface is resized, minimized, etc.
    */
    VkResult get_status() const;

    /**
    Gets a value indicating whether or not this gvk::WsiManager object's resources are valid
    @return A value indicating whether or not this gvk::WsiManager object's resources are valid
        @note gvk::WsiManager resource may become invalid when the gvk::sys::Surface is resized, minimzed, etc.
    */
    VkBool32 is_enabled() const;

    /**
    Gets this gvk::WsiManager object's gvk::CommandPool queue family index
    @return This gvk::WsiManager object's gvk::CommandPool queue family index
    */
    uint32_t get_queue_family_index() const;

    /**
    Gets this gvk::WsiManager object's VkPresentModeKHR
    @return This gvk::WsiManager object's VkPresentModeKHR
    */
    VkPresentModeKHR get_present_mode() const;

    /**
    Gets this gvk::WsiManager object's VkSampleCountFlagBits
    @return This gvk::WsiManager object's VkSampleCountFlagBits
    */
    VkSampleCountFlagBits get_sample_count() const;

    /**
    Gets this gvk::WsiManager object's color attachment VkFormat
    @return This gvk::WsiManager object's color attachment VkFormat
    */
    VkFormat get_color_format() const;

    /**
    Gets this gvk::WsiManager object's depth attachment VkFormat
    @return This gvk::WsiManager object's depth attachment VkFormat
    */
    VkFormat get_depth_format() const;

    /**
    Updates this gvk::WsiManager
    @return Whether or not this gvk::WsiManager object is enabled and its resources have been recreated since the last call to update()
    */
    VkBool32 update();

    /**
    Acquires the next gvk::SwapchainKHR image
    @param [in] timeout How long to wait, in nanoseconds, if no image is available
    @param [in] vkFence The VkFence to signal or VK_NULL_HANDLE
    @param [out] pImageIndex A pointer to a uint32_t to populate with the acquired image index
    @return The VkResult
    */
    VkResult acquire_next_image(uint64_t timeout, VkFence vkFence, uint32_t* pImageIndex);

    /**
    Gets the VkSubmitInfo for the gvk::SwapchainKHR image at the specified index
    @param [in] imageIndex The index of the gvk::SwapchainKHR image to get the VkSubmitInfo for
    @return The VkSubmitInfo of the gvk::SwapchainKHR image at the specified index
    */
    VkSubmitInfo get_submit_info(uint32_t imageIndex) const;

    /**
    Gets the VkPresentInfoKHR for the gvk::SwapchainKHR image at the specified index
    @param [in] pImageIndex A pointer to a uint32_t populated with the index of the gvk::SwapchainKHR image to get the VkPresentInfoKHR for
    @return The VkPresentInfoKHR of the gvk::SwapchainKHR image at the specified index
    */
    VkPresentInfoKHR get_present_info(const uint32_t* pImageIndex) const;

private:
    VkResult validate();
    void invalidate();
    VkResult create_surface(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator);
    VkResult create_swapchain();
    VkResult create_render_pass();
    VkResult create_command_buffers();
    VkResult create_render_targets();
    VkResult create_synchronization_primitives();

    Device mDevice;
    SurfaceKHR mSurface;
    SwapchainKHR mSwapchain;
    RenderPass mRenderPass;
    std::vector<CommandBuffer> mCommandBuffers;
    std::vector<RenderTarget> mRenderTargets;
    std::vector<Fence> mFences;
    Semaphore mImageAcquiredSemaphore;
    Semaphore mImageRenderedSemaphore;
    uint32_t mQueueFamilyIndex{ };
    VkPresentModeKHR mPresentMode{ VK_PRESENT_MODE_FIFO_KHR };
    VkSampleCountFlagBits mSampleCount{ VK_SAMPLE_COUNT_1_BIT };
    VkFormat mDepthFormat{ VK_FORMAT_UNDEFINED };
    VkResult mStatus{ VK_ERROR_OUT_OF_DATE_KHR };
    VkAllocationCallbacks mAllocator{ gvk::get_default<VkAllocationCallbacks>() };

    WsiManager(const WsiManager&) = delete;
    WsiManager& operator=(const WsiManager&) = delete;
};

} // namespace gvk


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

#include "gvk/handles.hpp"
#include "gvk/defines.hpp"

#include <utility>

namespace gvk {

/**
Provides high level control over gvk::RenderPass, gvk::Framebuffer, gvk::ImageView, and gvk::Image objects
*/
class RenderTarget final
{
public:
    /**
    Creation parameters for gvk::RenderTarget
    */
    struct CreateInfo
    {
        /**
        Framebuffer creation parameters
            @note The renderPass member of pFramebufferCreateInfo must refer to a gvk::RenderPass
            @note Any attachments not provided via pFramebufferCreateInfo will be created
        */
        const VkFramebufferCreateInfo* pFramebufferCreateInfo{ nullptr };
    };

    /**
    Creates an instance of gvk::RenderTarget
    @param [in] device The gvk::Device used to create gvk::RenderTarget resources
    @param [in] pCreateInfo A pointer to the gvk::RenderTarget creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @param [out] pRenderTarget A pointer to the gvk::RenderTarget to create
    @return the VkResult
    */
    static VkResult create(const Device& device, const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, RenderTarget* pRenderTarget);

    /**
    Destroys this instance of gvk::RenderTarget
    */
    void reset();

    /**
    Gets this gvk::RenderTarget object's gvk::Framebuffer
    @return This gvk::RenderTarget object's gvk::Framebuffer
    */
    const Framebuffer& get_framebuffer() const;

    /**
    Gets this gvk::RenderTarget object's gvk::RenderPass
    @return This gvk::RenderTarget object's gvk::RenderPass
    */
    RenderPass get_render_pass() const;

    /**
    Gets this gvk::RenderTarget object's gvk::Image for a specified attachment index
    @param [in] attachmentIndex The index of the attachment to return the gvk::Image for
    @return This gvk::RenderTarget object's gvk::Image for the specified attachment index
    */
    Image get_image(uint32_t attachmentIndex) const;

    /**
    Gets this gvk::RenderTarget object's VkRenderPassBeginInfo
    @return This gvk::RenderTarget object's VkRenderPassBeginInfo
    */
    VkRenderPassBeginInfo get_render_pass_begin_info() const;

    /**
    Gets this gvk::RenderTarget object's VkImageMemoryBarrier for a specified attachment index
    @param [in] attachmentIndex The index of the attachment to return the VkImageMemoryBarrier for
    @return This gvk::RenderTarget object's VkImageMemoryBarrier for the specified attachment index
        @note The VkImageMemoryBarrier2 will have its oldLayout member set to the corresponding VkAttachmentDescription2 object's finalLayout
        @note The VkImageMemoryBarrier2 will have its newLayout member set to the corresponding VkAttachmentDescription2 object's initialLayout
    */
    VkImageMemoryBarrier get_image_memory_barrier(uint32_t attachmentIndex) const;

    /**
    Gets this gvk::RenderTarget object's VkImageMemoryBarrier2 for a specified attachment index
    @param [in] attachmentIndex The index of the attachment to return the VkImageMemoryBarrier2 for
    @return This gvk::RenderTarget object's VkImageMemoryBarrier2 for the specified attachment index
        @note The VkImageMemoryBarrier2 will have its oldLayout member set to the corresponding VkAttachmentDescription2 object's finalLayout
        @note The VkImageMemoryBarrier2 will have its newLayout member set to the corresponding VkAttachmentDescription2 object's initialLayout
    */
    VkImageMemoryBarrier2 get_image_memory_barrier_2(uint32_t attachmentIndex) const;

private:
    Framebuffer mFramebuffer;
    std::vector<VkClearValue> mClearValues;
};

/**
Gets the max VkSampleCountFlagBits for a gvk::Framebuffer with specified attachment types
@param [in] vkPhysicalDevice
@param [in] color A value indicating whether or not the gvk::Framebuffer has a color attachment
@param [in] depth A value indicating whether or not the gvk::Framebuffer has a depth attachment
@param [in] stencil A value indicating whether or not the gvk::Framebuffer has a stencil attachment
@return The max VkSampleCountFlagBits for a gvk::Framebuffer with the specified attachment types
*/
VkSampleCountFlagBits get_max_framebuffer_sample_count(VkPhysicalDevice vkPhysicalDevice, VkBool32 color, VkBool32 depth, VkBool32 stencil);

} // namespace gvk

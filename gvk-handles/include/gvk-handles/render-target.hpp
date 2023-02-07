
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

#include "gvk-handles/handles.hpp"
#include "gvk-defines.hpp"

#include <utility>

namespace gvk {

/**
Provides high level control over RenderPass, Framebuffer, ImageView, and Image objects
*/
class RenderTarget final
{
public:
    /**
    Creation parameters for RenderTarget
    */
    struct CreateInfo
    {
        /**
        Framebuffer creation parameters
            @note The renderPass member of pFramebufferCreateInfo must refer to a RenderPass
            @note Any attachments not provided via pFramebufferCreateInfo will be created
        */
        const VkFramebufferCreateInfo* pFramebufferCreateInfo{ nullptr };
    };

    /**
    Creates an instance of RenderTarget
    @param [in] device The Device used to create RenderTarget resources
    @param [in] pCreateInfo A pointer to the RenderTarget creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @param [out] pRenderTarget A pointer to the RenderTarget to create
    @return the VkResult
    */
    static VkResult create(const Device& device, const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, RenderTarget* pRenderTarget);

    /**
    Destroys this instance of RenderTarget
    */
    void reset();

    /**
    Gets this RenderTarget object's Framebuffer
    @return This RenderTarget object's Framebuffer
    */
    const Framebuffer& get_framebuffer() const;

    /**
    Gets this RenderTarget object's RenderPass
    @return This RenderTarget object's RenderPass
    */
    RenderPass get_render_pass() const;

    /**
    Gets this RenderTarget object's Image for a specified attachment index
    @param [in] attachmentIndex The index of the attachment to return the Image for
    @return This RenderTarget object's Image for the specified attachment index
    */
    Image get_image(uint32_t attachmentIndex) const;

    /**
    Gets this RenderTarget object's VkRenderPassBeginInfo
    @return This RenderTarget object's VkRenderPassBeginInfo
    */
    VkRenderPassBeginInfo get_render_pass_begin_info() const;

    /**
    Gets this RenderTarget object's VkImageMemoryBarrier for a specified attachment index
    @param [in] attachmentIndex The index of the attachment to return the VkImageMemoryBarrier for
    @return This RenderTarget object's VkImageMemoryBarrier for the specified attachment index
        @note The VkImageMemoryBarrier2 will have its oldLayout member set to the corresponding VkAttachmentDescription2 object's finalLayout
        @note The VkImageMemoryBarrier2 will have its newLayout member set to the corresponding VkAttachmentDescription2 object's initialLayout
    */
    VkImageMemoryBarrier get_image_memory_barrier(uint32_t attachmentIndex) const;

    /**
    Gets this RenderTarget object's VkImageMemoryBarrier2 for a specified attachment index
    @param [in] attachmentIndex The index of the attachment to return the VkImageMemoryBarrier2 for
    @return This RenderTarget object's VkImageMemoryBarrier2 for the specified attachment index
        @note The VkImageMemoryBarrier2 will have its oldLayout member set to the corresponding VkAttachmentDescription2 object's finalLayout
        @note The VkImageMemoryBarrier2 will have its newLayout member set to the corresponding VkAttachmentDescription2 object's initialLayout
    */
    VkImageMemoryBarrier2 get_image_memory_barrier_2(uint32_t attachmentIndex) const;

private:
    Framebuffer mFramebuffer;
    std::vector<VkClearValue> mClearValues;
};

} // namespace gvk

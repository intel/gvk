
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
    static VkResult create(const Device& device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, RenderTarget* pRenderTarget);

    template <typename T>
    const T& get() const
    {
        assert(mReference && "Attempting to dereference nullref RenderTarget");
        if constexpr (std::is_same_v<T, Device>) { return mReference->mFramebuffer.get<Device>(); }
        if constexpr (std::is_same_v<T, Framebuffer>) { return mReference->mFramebuffer; }
        if constexpr (std::is_same_v<T, VkFramebufferCreateInfo>) { return mReference->mFramebuffer.get<VkFramebufferCreateInfo>(); }
        if constexpr (std::is_same_v<T, VkRenderPassBeginInfo>) { return mReference->mRenderPassBeginInfo; }
    }

    template <typename T>
    const T& get(uint32_t attachmentIndex) const
    {
        assert(mReference && "Attempting to dereference nullref RenderTarget");

        /**
        Gets this gvk::RenderTarget object's VkImageMemoryBarrier for a specified attachment index
        @param [in] attachmentIndex The index of the attachment to return the VkImageMemoryBarrier for
        @return This gvk::RenderTarget object's VkImageMemoryBarrier for the specified attachment index
            @note The VkImageMemoryBarrier2 will have its oldLayout member set to the corresponding VkAttachmentDescription2 object's finalLayout
            @note The VkImageMemoryBarrier2 will have its newLayout member set to the corresponding VkAttachmentDescription2 object's initialLayout
        */
        if constexpr (std::is_same_v<T, VkImageMemoryBarrier>) { return mReference->mImageMemoryBarriers[attachmentIndex]; }

        /**
        Gets this gvk::RenderTarget object's VkImageMemoryBarrier2 for a specified attachment index
        @param [in] attachmentIndex The index of the attachment to return the VkImageMemoryBarrier2 for
        @return This gvk::RenderTarget object's VkImageMemoryBarrier2 for the specified attachment index
            @note The VkImageMemoryBarrier2 will have its oldLayout member set to the corresponding VkAttachmentDescription2 object's finalLayout
            @note The VkImageMemoryBarrier2 will have its newLayout member set to the corresponding VkAttachmentDescription2 object's initialLayout
        */
        if constexpr (std::is_same_v<T, VkImageMemoryBarrier2>) { return mReference->mImageMemoryBarrier2s[attachmentIndex]; }
    }

private:
    class ControlBlock final
    {
    public:
        ControlBlock() = default;
        ~ControlBlock();
        Framebuffer mFramebuffer;
        Auto<VkRenderPassBeginInfo> mRenderPassBeginInfo{ };
        std::vector<Auto<VkImageMemoryBarrier>> mImageMemoryBarriers;
        std::vector<Auto<VkImageMemoryBarrier2>> mImageMemoryBarrier2s;
    private:
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock& operator=(const ControlBlock&) = delete;
    };

    gvk_reference_type(RenderTarget)
};

} // namespace gvk

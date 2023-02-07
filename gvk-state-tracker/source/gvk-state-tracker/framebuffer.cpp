
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
#include "gvk-layer/registry.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        gvkResult = BasicStateTracker::post_vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, gvkResult);
        assert(gvkResult == VK_SUCCESS);
        Framebuffer gvkFramebuffer({ device, *pFramebuffer });
        assert(gvkFramebuffer);
        auto& imageViews = gvkFramebuffer.mReference.get_obj().mImageViews;
        imageViews.resize(pCreateInfo->attachmentCount);
        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            imageViews[i] = ImageView({ device, pCreateInfo->pAttachments[i] });
            assert(imageViews[i]);
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

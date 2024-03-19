
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

// NOTE : Cache the info arg so any changes made by this layer, or layers down
//  the chain, can be reverted before returning control to the application.
thread_local VkImageCreateInfo tlApplicationImageCreateInfo;
VkResult StateTracker::pre_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult gvkResult)
{
    assert(pCreateInfo);
    tlApplicationImageCreateInfo = *pCreateInfo;
    return BasicStateTracker::pre_vkCreateImage(device, pCreateInfo, pAllocator, pImage, gvkResult);
}

VkResult StateTracker::post_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult gvkResult)
{
    assert(pCreateInfo);
    gvkResult = BasicStateTracker::post_vkCreateImage(device, pCreateInfo, pAllocator, pImage, gvkResult);
    if (gvkResult == VK_SUCCESS) {
        Image gvkImage({ device, *pImage });
        assert(gvkImage);
        gvkImage.mReference.get_obj().mImageLayoutTracker = ImageLayoutTracker(pCreateInfo->mipLevels, pCreateInfo->arrayLayers, pCreateInfo->initialLayout);
    }
    *const_cast<VkImageCreateInfo*>(pCreateInfo) = tlApplicationImageCreateInfo;
    return gvkResult;
}

void StateTracker::post_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
    Image gvkImage({ device, image });
    assert(gvkImage);
    auto& gvkImageControlBlock = gvkImage.mReference.get_obj();
    if (gvkImageControlBlock.mBindImageMemoryInfo->sType == VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO) {
        if (!gvkImageControlBlock.mVkDeviceMemoryBindings.empty()) {
            assert(gvkImageControlBlock.mVkDeviceMemoryBindings.size() == 1);
            DeviceMemory gvkDeviceMemory({ device, *gvkImageControlBlock.mVkDeviceMemoryBindings.begin() });
            assert(gvkDeviceMemory);
            gvkDeviceMemory.mReference.get_obj().mVkImageBindings.erase(image);
        }
    }
    BasicStateTracker::post_vkDestroyImage(device, image, pAllocator);
}

} // namespace state_tracker
} // namespace gvk

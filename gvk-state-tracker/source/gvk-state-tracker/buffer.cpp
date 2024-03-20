
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
thread_local VkBufferCreateInfo tlApplicationBufferCreateInfo;
VkResult StateTracker::pre_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult gvkResult)
{
    assert(pCreateInfo);
    tlApplicationBufferCreateInfo = *pCreateInfo;
    return BasicStateTracker::pre_vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, gvkResult);
}

VkResult StateTracker::post_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult gvkResult)
{
    assert(pCreateInfo);
    gvkResult = BasicStateTracker::post_vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, gvkResult);
    *const_cast<VkBufferCreateInfo*>(pCreateInfo) = tlApplicationBufferCreateInfo;
    return gvkResult;
}

VkDeviceAddress StateTracker::post_vkGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress gvkResult)
{
    return BasicStateTracker::post_vkGetBufferDeviceAddress(device, pInfo, gvkResult);
}

VkDeviceAddress StateTracker::post_vkGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress gvkResult)
{
    return post_vkGetBufferDeviceAddress(device, pInfo, gvkResult);
}

VkDeviceAddress StateTracker::post_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress gvkResult)
{
    return post_vkGetBufferDeviceAddress(device, pInfo, gvkResult);
}

void StateTracker::post_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    Buffer gvkBuffer({ device, buffer });
    assert(gvkBuffer);
    auto& bufferControlBlock = gvkBuffer.mReference.get_obj();
    if (bufferControlBlock.mBindBufferMemoryInfo->sType == VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO) {
        if (!bufferControlBlock.mVkDeviceMemoryBindings.empty()) {
            assert(bufferControlBlock.mVkDeviceMemoryBindings.size() == 1);
            DeviceMemory gvkDeviceMemory({ device, *bufferControlBlock.mVkDeviceMemoryBindings.begin() });
            assert(gvkDeviceMemory);
            gvkDeviceMemory.mReference.get_obj().mVkBufferBindings.erase(buffer);
            assert(!bufferControlBlock.mDeviceMemoryRecord || bufferControlBlock.mDeviceMemoryRecord == gvkDeviceMemory);
            if (!gvkDeviceMemory.mReference.get_obj().mDedicatedBuffer) {
                bufferControlBlock.mDeviceMemoryRecord = gvkDeviceMemory;
            }
        }
    }
    BasicStateTracker::post_vkDestroyBuffer(device, buffer, pAllocator);
}

} // namespace state_tracker
} // namespace gvk


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
#include "gvk-structures/defaults.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult)
{
    gvkResult = BasicStateTracker::post_vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, gvkResult);
    if (gvkResult == VK_SUCCESS) {
        auto pNext = (const VkBaseInStructure*)pAllocateInfo->pNext;
        while (pNext) {
            switch (pNext->sType) {
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO: {
                const auto& memoryDedicatedAllocateInfo = (const VkMemoryDedicatedAllocateInfo*)pNext;
                DeviceMemory gvkDeviceMemory({ device, *pMemory });
                assert(gvkDeviceMemory);
                auto& deviceMemoryControlBlock = gvkDeviceMemory.mReference.get_obj();
                if (memoryDedicatedAllocateInfo->buffer) {
                    deviceMemoryControlBlock.mDedicatedBuffer = Buffer({ device, memoryDedicatedAllocateInfo->buffer });
                } else if (memoryDedicatedAllocateInfo->image) {
                    deviceMemoryControlBlock.mDedicatedImage = Image({ device, memoryDedicatedAllocateInfo->image });
                }
            } break;
            default: {
            } break;
            }
            pNext = pNext->pNext;
        }
    }
    return gvkResult;
}

void StateTracker::post_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    DeviceMemory gvkDeviceMemory({ device, memory });
    assert(gvkDeviceMemory);
    auto& deviceMemoryControlBlock = gvkDeviceMemory.mReference.get_obj();
    for (auto vkBuffer : deviceMemoryControlBlock.mVkBufferBindings) {
        Buffer gvkBuffer({ device, vkBuffer });
        assert(gvkBuffer);
        gvkBuffer.mReference.get_obj().mVkDeviceMemoryBindings.erase(memory);
    }
    for (auto vkImage : deviceMemoryControlBlock.mVkImageBindings) {
        Image gvkImage({ device, vkImage });
        assert(gvkImage);
        gvkImage.mReference.get_obj().mVkDeviceMemoryBindings.erase(memory);
    }
    deviceMemoryControlBlock.mVkBufferBindings.clear();
    deviceMemoryControlBlock.mVkImageBindings.clear();
    deviceMemoryControlBlock.mDedicatedBuffer.reset();
    deviceMemoryControlBlock.mDedicatedImage.reset();
    BasicStateTracker::post_vkFreeMemory(device, memory, pAllocator);
}

VkResult StateTracker::post_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        DeviceMemory gvkDeviceMemory({ device, memory });
        assert(gvkDeviceMemory);
        auto& deviceMemoryControlBlock = gvkDeviceMemory.mReference.get_obj();
        deviceMemoryControlBlock.mMemoryMapInfo.offset = offset;
        deviceMemoryControlBlock.mMemoryMapInfo.size = size;
        deviceMemoryControlBlock.mMemoryMapInfo.flags = flags;
        deviceMemoryControlBlock.mMemoryMapInfo.pData = ppData ? *ppData : nullptr;
    }
    return gvkResult;
}

void StateTracker::post_vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
    DeviceMemory gvkDeviceMemory({ device, memory });
    assert(gvkDeviceMemory);
    gvkDeviceMemory.mReference.get_obj().mMemoryMapInfo = { };
}

VkResult StateTracker::post_vkBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos, VkResult gvkResult)
{
    assert(false && "Unsupported entry point; gvk maintenance required");
    (void)device;
    (void)bindInfoCount;
    (void)pBindInfos;
    (void)gvkResult;
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

VkResult StateTracker::post_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult gvkResult)
{
    auto bindBufferMemoryInfo = gvk::get_default<VkBindBufferMemoryInfo>();
    bindBufferMemoryInfo.buffer = buffer;
    bindBufferMemoryInfo.memory = memory;
    bindBufferMemoryInfo.memoryOffset = memoryOffset;
    return post_vkBindBufferMemory2(device, 1, &bindBufferMemoryInfo, gvkResult);
}

VkResult StateTracker::post_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS && bindInfoCount && pBindInfos) {
        for (uint32_t i = 0; i < bindInfoCount; ++i) {
            Buffer gvkBuffer({ device, pBindInfos[i].buffer });
            assert(gvkBuffer);
            DeviceMemory gvkDeviceMemory({ device, pBindInfos[i].memory });
            assert(gvkDeviceMemory);
            gvkBuffer.mReference.get_obj().mBindBufferMemoryInfo = pBindInfos[i];
            gvkBuffer.mReference.get_obj().mVkDeviceMemoryBindings.insert(gvkDeviceMemory);
            gvkDeviceMemory.mReference.get_obj().mVkBufferBindings.insert(gvkBuffer);
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult gvkResult)
{
    return post_vkBindBufferMemory2(device, bindInfoCount, pBindInfos, gvkResult);
}

VkResult StateTracker::post_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult gvkResult)
{
    auto bindImageMemoryInfo = gvk::get_default<VkBindImageMemoryInfo>();
    bindImageMemoryInfo.image = image;
    bindImageMemoryInfo.memory = memory;
    bindImageMemoryInfo.memoryOffset = memoryOffset;
    return post_vkBindImageMemory2(device, 1, &bindImageMemoryInfo, gvkResult);
}

VkResult StateTracker::post_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS && bindInfoCount && pBindInfos) {
        for (uint32_t i = 0; i < bindInfoCount; ++i) {
            Image gvkImage({ device, pBindInfos[i].image });
            assert(gvkImage);
            DeviceMemory gvkDeviceMemory({ device, pBindInfos[i].memory });
            assert(gvkDeviceMemory);
            gvkImage.mReference.get_obj().mBindImageMemoryInfo = pBindInfos[i];
            gvkImage.mReference.get_obj().mVkDeviceMemoryBindings.insert(gvkDeviceMemory);
            gvkDeviceMemory.mReference.get_obj().mVkImageBindings.insert(gvkImage);
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult gvkResult)
{
    return post_vkBindImageMemory2(device, bindInfoCount, pBindInfos, gvkResult);
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
VkResult StateTracker::post_vkBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos, VkResult gvkResult)
{
    assert(false && "Unsupported entry point; gvk maintenance required");
    (void)device;
    (void)videoSession;
    (void)bindSessionMemoryInfoCount;
    (void)pBindSessionMemoryInfos;
    (void)gvkResult;
    return VK_ERROR_FEATURE_NOT_PRESENT;
}
#endif // VK_ENABLE_BETA_EXTENSIONS

} // namespace state_tracker
} // namespace gvk

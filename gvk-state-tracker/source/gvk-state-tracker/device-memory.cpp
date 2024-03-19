
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

// NOTE : Cache the info arg so any changes made by this layer, or layers down
//  the chain, can be reverted before returning control to the application.
thread_local VkMemoryAllocateInfo tlApplicationMemoryAllocateInfo;
VkResult StateTracker::pre_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult)
{
    assert(pAllocateInfo);
    tlApplicationMemoryAllocateInfo = *pAllocateInfo;
    return BasicStateTracker::pre_vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, gvkResult);
}

VkResult StateTracker::post_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult)
{
    assert(pAllocateInfo);
    gvkResult = BasicStateTracker::post_vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, gvkResult);
    if (gvkResult == VK_SUCCESS) {
        DeviceMemory gvkDeviceMemory({ device, *pMemory });
        assert(gvkDeviceMemory);
        auto& allocateInfo = const_cast<VkMemoryAllocateInfo&>(*gvkDeviceMemory.mReference.get_obj().mMemoryAllocateInfo);

        VkMemoryAllocateFlagsInfo* pMemoryAllocateFlagsInfo = nullptr;
        VkMemoryOpaqueCaptureAddressAllocateInfo* pMemoryOpaqueCaptureAddressAllocateInfo = nullptr;
        auto pNext = (const VkBaseInStructure*)allocateInfo.pNext;
        while (pNext) {
            switch (pNext->sType) {
            case get_stype<VkMemoryAllocateFlagsInfo>(): {
                pMemoryAllocateFlagsInfo = (VkMemoryAllocateFlagsInfo*)pNext;
            } break;
            case get_stype<VkMemoryDedicatedAllocateInfo>(): {
                const auto& memoryDedicatedAllocateInfo = (const VkMemoryDedicatedAllocateInfo*)pNext;
                auto& deviceMemoryControlBlock = gvkDeviceMemory.mReference.get_obj();
                if (memoryDedicatedAllocateInfo->buffer) {
                    deviceMemoryControlBlock.mDedicatedBuffer = Buffer({ device, memoryDedicatedAllocateInfo->buffer });
                } else if (memoryDedicatedAllocateInfo->image) {
                    deviceMemoryControlBlock.mDedicatedImage = Image({ device, memoryDedicatedAllocateInfo->image });
                }
            } break;
            case get_stype<VkMemoryOpaqueCaptureAddressAllocateInfo>(): {
                pMemoryOpaqueCaptureAddressAllocateInfo = (VkMemoryOpaqueCaptureAddressAllocateInfo*)pNext;
            } break;
            default: {
            } break;
            }
            pNext = pNext->pNext;
        }

        if (pMemoryAllocateFlagsInfo && pMemoryAllocateFlagsInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) {
            const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
            assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
            const auto& dispatchTable = dispatchTableItr->second;

            auto deviceMemoryCaptureAddressInfo = get_default<VkDeviceMemoryOpaqueCaptureAddressInfo>();
            deviceMemoryCaptureAddressInfo.memory = *pMemory;

            if (!pMemoryOpaqueCaptureAddressAllocateInfo) {
                pMemoryOpaqueCaptureAddressAllocateInfo = (VkMemoryOpaqueCaptureAddressAllocateInfo*)detail::create_pnext_copy(&get_default<VkMemoryOpaqueCaptureAddressAllocateInfo>(), nullptr);
                pMemoryOpaqueCaptureAddressAllocateInfo->pNext = allocateInfo.pNext;
                allocateInfo.pNext = pMemoryOpaqueCaptureAddressAllocateInfo;
            }

            uint64_t opaqueCaptureAddress = 0;
            if (layer::Registry::get().apiVersion < VK_API_VERSION_1_2) {
                opaqueCaptureAddress = dispatchTable.gvkGetDeviceMemoryOpaqueCaptureAddressKHR(device, &deviceMemoryCaptureAddressInfo);
            } else {
                opaqueCaptureAddress = dispatchTable.gvkGetDeviceMemoryOpaqueCaptureAddress(device, &deviceMemoryCaptureAddressInfo);
            }
            assert(!pMemoryOpaqueCaptureAddressAllocateInfo->opaqueCaptureAddress || pMemoryOpaqueCaptureAddressAllocateInfo->opaqueCaptureAddress == opaqueCaptureAddress);
            pMemoryOpaqueCaptureAddressAllocateInfo->opaqueCaptureAddress = opaqueCaptureAddress;
        }
    }
    *const_cast<VkMemoryAllocateInfo*>(pAllocateInfo) = tlApplicationMemoryAllocateInfo;
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

        // TODO : Double check this logic
        if (!deviceMemoryControlBlock.mDedicatedBuffer) {
            gvkBuffer.mReference.get_obj().mDeviceMemoryRecord = gvkDeviceMemory;
        }
    }
    // TODO : Treat Images the same as Buffers wrt binding tracking
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

            const auto& bufferCreateInfo = *gvkBuffer.mReference.get_obj().mBufferCreateInfo;
            if (bufferCreateInfo.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) {
                const auto& dispatchTableItr = layer::Registry::get().VkDeviceDispatchTables.find(layer::get_dispatch_key(device));
                assert(dispatchTableItr != layer::Registry::get().VkDeviceDispatchTables.end());
                const auto& dispatchTable = dispatchTableItr->second;
                (void)dispatchTable;

#if 0 // TODO : Sort out buffer opaque capture addresses
                // TODO : Documentation
                auto pBufferOpaqueCaptureAddressCreateInfo = const_cast<VkBufferOpaqueCaptureAddressCreateInfo*>(get_pnext<VkBufferOpaqueCaptureAddressCreateInfo>(bufferCreateInfo));
                if (!pBufferOpaqueCaptureAddressCreateInfo) {
                    pBufferOpaqueCaptureAddressCreateInfo = (VkBufferOpaqueCaptureAddressCreateInfo*)detail::create_pnext_copy(&get_default<VkBufferOpaqueCaptureAddressCreateInfo>(), nullptr);
                    pBufferOpaqueCaptureAddressCreateInfo->pNext = bufferCreateInfo.pNext;
                    const_cast<VkBufferCreateInfo&>(bufferCreateInfo).pNext = pBufferOpaqueCaptureAddressCreateInfo;
                }

                // TODO : Documentation
                auto bufferDeviceAddressInfo = get_default<VkBufferDeviceAddressInfo>();
                bufferDeviceAddressInfo.buffer = gvkBuffer;

                // TODO : Documentation
                uint64_t opaqueCaptureAddress = 0;
                if (layer::Registry::get().apiVersion < VK_API_VERSION_1_2) {
                    opaqueCaptureAddress = dispatchTable.gvkGetBufferOpaqueCaptureAddressKHR(device, &bufferDeviceAddressInfo);
                } else {
                    opaqueCaptureAddress = dispatchTable.gvkGetBufferOpaqueCaptureAddress(device, &bufferDeviceAddressInfo);
                }

                assert(
                    !pBufferOpaqueCaptureAddressCreateInfo->opaqueCaptureAddress ||
                    pBufferOpaqueCaptureAddressCreateInfo->opaqueCaptureAddress == opaqueCaptureAddress &&
                    "TODO : Documentation"
                );

                // TODO : Documentation
                pBufferOpaqueCaptureAddressCreateInfo->opaqueCaptureAddress = opaqueCaptureAddress;
#endif
            }
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

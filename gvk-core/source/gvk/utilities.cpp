
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

#include "gvk/utilities.hpp"

#include <algorithm>
#include <cmath>

namespace gvk {

void get_compatible_memory_type_indices(VkPhysicalDevice vkPhysicalDevice, uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryPropertyFlags, uint32_t* pMemoryTypeCount, uint32_t* pMemoryTypeIndices)
{
    assert(vkPhysicalDevice);
    assert(pMemoryTypeCount);
    uint32_t memoryTypeCount = 0;
    auto dispatchTable = DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.gvkGetPhysicalDeviceMemoryProperties);
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties { };
    dispatchTable.gvkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &physicalDeviceMemoryProperties);
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        if (memoryTypeBits & (1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
            if (++memoryTypeCount <= *pMemoryTypeCount && pMemoryTypeIndices) {
                *pMemoryTypeIndices = i;
                pMemoryTypeIndices += 1;
            }
        }
    }
    *pMemoryTypeCount = memoryTypeCount;
}

uint32_t get_mip_level_count(const VkExtent3D& imageExtent)
{
    return (uint32_t)std::floor(std::log2(std::max(std::max(imageExtent.width, imageExtent.height), imageExtent.depth))) + 1;
}

VkExtent3D get_mip_level_extent(const VkExtent3D& imageExtent, uint32_t mipLevel)
{
    return { imageExtent.width >> mipLevel, imageExtent.height >> mipLevel, imageExtent.depth >> mipLevel };
}

VkSampleCountFlagBits get_max_framebuffer_sample_count(VkPhysicalDevice vkPhysicalDevice, VkBool32 color, VkBool32 depth, VkBool32 stencil)
{
    assert(vkPhysicalDevice);
    VkPhysicalDeviceProperties physicalDeviceProperties { };
    auto dispatchTable = DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.gvkGetPhysicalDeviceProperties);
    dispatchTable.gvkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
    VkSampleCountFlags sampleCounts = (color || depth || stencil) ? (uint32_t)-1 : 0;
    if (color) {
        sampleCounts &= physicalDeviceProperties.limits.framebufferColorSampleCounts;
    }
    if (depth) {
        sampleCounts &= physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    }
    if (stencil) {
        sampleCounts &= physicalDeviceProperties.limits.framebufferStencilSampleCounts;
    }
    if (sampleCounts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    } else if (sampleCounts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

} // namespace gvk

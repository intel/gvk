
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

#include "gvk-structures.hpp"
#include "gvk-command-structures/generated/command-structure-create-copy.hpp"
#include "gvk-command-structures/generated/command-structure-destroy-copy.hpp"

#include <algorithm>

namespace gvk {
namespace detail {

GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureAllocateCommandBuffers)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureAllocateDescriptorSets)
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureBuildAccelerationStructuresKHR
template <>
GvkCommandStructureBuildAccelerationStructuresKHR create_structure_copy<GvkCommandStructureBuildAccelerationStructuresKHR>(const GvkCommandStructureBuildAccelerationStructuresKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pInfos = create_dynamic_array_copy(obj.infoCount, obj.pInfos, pAllocator);
    pAllocator = validate_allocation_callbacks(pAllocator);
    auto ppBuildRangeInfos = (VkAccelerationStructureBuildRangeInfoKHR**)pAllocator->pfnAllocation(pAllocator->pUserData, obj.infoCount * sizeof(VkAccelerationStructureBuildRangeInfoKHR*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        ppBuildRangeInfos[i] = create_dynamic_array_copy(obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i], pAllocator);
    }
    result.ppBuildRangeInfos = ppBuildRangeInfos;
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureBuildAccelerationStructuresKHR>(const GvkCommandStructureBuildAccelerationStructuresKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        destroy_dynamic_array_copy(obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i], pAllocator);
    }
    destroy_dynamic_array_copy(obj.infoCount, obj.pInfos, pAllocator);
    destroy_dynamic_array_copy(obj.infoCount, obj.ppBuildRangeInfos, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR
template <>
GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR create_structure_copy<GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR>(const GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pInfos = create_dynamic_array_copy(obj.infoCount, obj.pInfos, pAllocator);
    result.pIndirectDeviceAddresses = create_dynamic_array_copy(obj.infoCount, obj.pIndirectDeviceAddresses, pAllocator);
    result.pIndirectStrides = create_dynamic_array_copy(obj.infoCount, obj.pIndirectStrides, pAllocator);
    pAllocator = validate_allocation_callbacks(pAllocator);
    auto ppMaxPrimitiveCounts = (uint32_t**)pAllocator->pfnAllocation(pAllocator->pUserData, obj.infoCount * sizeof(uint32_t*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        ppMaxPrimitiveCounts[i] = create_dynamic_array_copy(obj.pInfos[i].geometryCount, obj.ppMaxPrimitiveCounts[i], pAllocator);
    }
    result.ppMaxPrimitiveCounts = ppMaxPrimitiveCounts;
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR>(const GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_dynamic_array_copy(obj.infoCount, obj.pIndirectDeviceAddresses, pAllocator);
    destroy_dynamic_array_copy(obj.infoCount, obj.pIndirectStrides, pAllocator);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        destroy_dynamic_array_copy(obj.pInfos[i].geometryCount, obj.ppMaxPrimitiveCounts[i], pAllocator);
    }
    destroy_dynamic_array_copy(obj.infoCount, obj.pInfos, pAllocator);
    destroy_dynamic_array_copy(obj.infoCount, obj.ppMaxPrimitiveCounts, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdBuildAccelerationStructuresKHR
template <>
GvkCommandStructureCmdBuildAccelerationStructuresKHR create_structure_copy<GvkCommandStructureCmdBuildAccelerationStructuresKHR>(const GvkCommandStructureCmdBuildAccelerationStructuresKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pInfos = create_dynamic_array_copy(obj.infoCount, obj.pInfos, pAllocator);
    pAllocator = validate_allocation_callbacks(pAllocator);
    auto ppBuildRangeInfos = (VkAccelerationStructureBuildRangeInfoKHR**)pAllocator->pfnAllocation(pAllocator->pUserData, obj.infoCount * sizeof(VkAccelerationStructureBuildRangeInfoKHR*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        ppBuildRangeInfos[i] = create_dynamic_array_copy(obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i], pAllocator);
    }
    result.ppBuildRangeInfos = ppBuildRangeInfos;
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdBuildAccelerationStructuresKHR>(const GvkCommandStructureCmdBuildAccelerationStructuresKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        destroy_dynamic_array_copy(obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i], pAllocator);
    }
    destroy_dynamic_array_copy(obj.infoCount, obj.pInfos, pAllocator);
    destroy_dynamic_array_copy(obj.infoCount, obj.ppBuildRangeInfos, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdPushConstants
template <>
GvkCommandStructureCmdPushConstants create_structure_copy<GvkCommandStructureCmdPushConstants>(const GvkCommandStructureCmdPushConstants& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pValues = create_dynamic_array_copy(obj.size, (const uint8_t*)obj.pValues, pAllocator);
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdPushConstants>(const GvkCommandStructureCmdPushConstants& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_dynamic_array_copy(obj.size, (const uint8_t*)obj.pValues, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetBlendConstants
template <>
GvkCommandStructureCmdSetBlendConstants create_structure_copy<GvkCommandStructureCmdSetBlendConstants>(const GvkCommandStructureCmdSetBlendConstants& obj, const VkAllocationCallbacks*)
{
    // NOOP :
    return obj;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdSetBlendConstants>(const GvkCommandStructureCmdSetBlendConstants&, const VkAllocationCallbacks*)
{
    // NOOP :
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetFragmentShadingRateEnumNV
template <>
GvkCommandStructureCmdSetFragmentShadingRateEnumNV create_structure_copy<GvkCommandStructureCmdSetFragmentShadingRateEnumNV>(const GvkCommandStructureCmdSetFragmentShadingRateEnumNV& obj, const VkAllocationCallbacks*)
{
    // NOOP :
    return obj;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdSetFragmentShadingRateEnumNV>(const GvkCommandStructureCmdSetFragmentShadingRateEnumNV&, const VkAllocationCallbacks*)
{
    // NOOP :
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetFragmentShadingRateKHR
template <>
GvkCommandStructureCmdSetFragmentShadingRateKHR create_structure_copy<GvkCommandStructureCmdSetFragmentShadingRateKHR>(const GvkCommandStructureCmdSetFragmentShadingRateKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pFragmentSize = create_dynamic_array_copy(1, obj.pFragmentSize, pAllocator);
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdSetFragmentShadingRateKHR>(const GvkCommandStructureCmdSetFragmentShadingRateKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_dynamic_array_copy(1, obj.pFragmentSize, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetSampleMaskEXT
template <>
GvkCommandStructureCmdSetSampleMaskEXT create_structure_copy<GvkCommandStructureCmdSetSampleMaskEXT>(const GvkCommandStructureCmdSetSampleMaskEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pSampleMask = create_dynamic_array_copy(std::max(1, obj.samples / 32), obj.pSampleMask, pAllocator);
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureCmdSetSampleMaskEXT>(const GvkCommandStructureCmdSetSampleMaskEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_dynamic_array_copy(std::max(1, obj.samples / 32), obj.pSampleMask, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureGetAccelerationStructureBuildSizesKHR
template <>
GvkCommandStructureGetAccelerationStructureBuildSizesKHR create_structure_copy<GvkCommandStructureGetAccelerationStructureBuildSizesKHR>(const GvkCommandStructureGetAccelerationStructureBuildSizesKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pBuildInfo = create_dynamic_array_copy(1, obj.pBuildInfo, pAllocator);
    result.pMaxPrimitiveCounts = create_dynamic_array_copy(obj.pBuildInfo->geometryCount, obj.pMaxPrimitiveCounts, pAllocator);
    result.pSizeInfo = create_dynamic_array_copy(1, obj.pSizeInfo, pAllocator);
    return result;
}

template <>
void destroy_structure_copy<GvkCommandStructureGetAccelerationStructureBuildSizesKHR>(const GvkCommandStructureGetAccelerationStructureBuildSizesKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_dynamic_array_copy(obj.pBuildInfo->geometryCount, obj.pMaxPrimitiveCounts, pAllocator);
    destroy_dynamic_array_copy(1, obj.pBuildInfo, pAllocator);
    destroy_dynamic_array_copy(1, obj.pSizeInfo, pAllocator);
}

} // namespace detail
} // namespace gvk

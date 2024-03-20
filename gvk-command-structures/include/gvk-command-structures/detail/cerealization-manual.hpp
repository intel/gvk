
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

#include "gvk-defines.hpp"
#include "gvk-command-structures/generated/command.h"
#include "gvk-structures/detail/cerealization-manual.hpp"

#include <algorithm>

namespace cereal {

GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureAllocateCommandBuffers)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureAllocateDescriptorSets)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureBuildAccelerationStructuresKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetAccelerationStructureBuildSizesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetDeviceProcAddr)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetInstanceProcAddr)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetMemoryRemoteAddressNV)
#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetMemoryWin32HandlePropertiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetFenceWin32HandleKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetMemoryWin32HandleKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetMemoryWin32HandleNV)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetSemaphoreWin32HandleKHR)
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    archive(obj.infoCount);
    gvk::detail::cerealize_dynamic_array(archive, obj.infoCount, obj.pInfos);
    gvk::detail::cerealize_dynamic_array(archive, obj.infoCount, obj.pIndirectDeviceAddresses);
    gvk::detail::cerealize_dynamic_array(archive, obj.infoCount, obj.pIndirectStrides);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        gvk::detail::cerealize_dynamic_array(archive, obj.pInfos[i].geometryCount, obj.ppMaxPrimitiveCounts[i]);
    }
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    archive(obj.infoCount);
    obj.pInfos = gvk::detail::decerealize_dynamic_array<VkAccelerationStructureBuildGeometryInfoKHR>(archive);
    obj.pIndirectDeviceAddresses = gvk::detail::decerealize_dynamic_array<VkDeviceAddress>(archive);
    obj.pIndirectStrides = gvk::detail::decerealize_dynamic_array<uint32_t>(archive);
    assert(gvk::detail::tlpDecerealizationAllocator);
    auto pAllocator = gvk::detail::tlpDecerealizationAllocator;
    auto ppMaxPrimitiveCounts = (uint32_t**)pAllocator->pfnAllocation(pAllocator->pUserData, obj.infoCount * sizeof(uint32_t*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        ppMaxPrimitiveCounts[i] = gvk::detail::decerealize_dynamic_array<uint32_t>(archive);
    }
    obj.ppMaxPrimitiveCounts = ppMaxPrimitiveCounts;
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdBuildAccelerationStructuresKHR
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdBuildAccelerationStructuresKHR& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    archive(obj.infoCount);
    gvk::detail::cerealize_dynamic_array(archive, obj.infoCount, obj.pInfos);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        gvk::detail::cerealize_dynamic_array(archive, obj.pInfos[i].geometryCount, obj.ppBuildRangeInfos[i]);
    }
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdBuildAccelerationStructuresKHR& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    archive(obj.infoCount);
    obj.pInfos = gvk::detail::decerealize_dynamic_array<VkAccelerationStructureBuildGeometryInfoKHR>(archive);
    assert(gvk::detail::tlpDecerealizationAllocator);
    auto pAllocator = gvk::detail::tlpDecerealizationAllocator;
    auto ppBuildRangeInfos = (VkAccelerationStructureBuildRangeInfoKHR**)pAllocator->pfnAllocation(pAllocator->pUserData, obj.infoCount * sizeof(VkAccelerationStructureBuildRangeInfoKHR*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    for (uint32_t i = 0; i < obj.infoCount; ++i) {
        ppBuildRangeInfos[i] = gvk::detail::decerealize_dynamic_array<VkAccelerationStructureBuildRangeInfoKHR>(archive);
    }
    obj.ppBuildRangeInfos = ppBuildRangeInfos;
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdPushConstants
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdPushConstants& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    gvk::detail::cerealize_handle(archive, obj.layout);
    archive(obj.stageFlags);
    archive(obj.offset);
    archive(obj.size);
    gvk::detail::cerealize_dynamic_array(archive, obj.size, (const uint8_t*)obj.pValues);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdPushConstants& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    obj.layout = gvk::detail::decerealize_handle<VkPipelineLayout>(archive);
    archive(obj.stageFlags);
    archive(obj.offset);
    archive(obj.size);
    obj.pValues = gvk::detail::decerealize_dynamic_array<uint8_t>(archive);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetBlendConstants
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdSetBlendConstants& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    gvk::detail::cerealize_static_array<4>(archive, obj.blendConstants);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdSetBlendConstants& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    gvk::detail::decerealize_static_array<4>(archive, obj.blendConstants);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetFragmentShadingRateEnumNV
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdSetFragmentShadingRateEnumNV& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    archive(obj.shadingRate);
    gvk::detail::cerealize_static_array<2>(archive, obj.combinerOps);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdSetFragmentShadingRateEnumNV& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    archive(obj.shadingRate);
    gvk::detail::decerealize_static_array<2>(archive, obj.combinerOps);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetFragmentShadingRateKHR
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdSetFragmentShadingRateKHR& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    gvk::detail::cerealize_dynamic_array(archive, 1, obj.pFragmentSize);
    gvk::detail::cerealize_static_array<2>(archive, obj.combinerOps);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdSetFragmentShadingRateKHR& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    obj.pFragmentSize = gvk::detail::decerealize_dynamic_array<VkExtent2D>(archive);
    gvk::detail::decerealize_static_array<2>(archive, obj.combinerOps);
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructureCmdSetSampleMaskEXT
template <typename ArchiveType>
inline void save(ArchiveType& archive, const GvkCommandStructureCmdSetSampleMaskEXT& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_handle(archive, obj.commandBuffer);
    archive(obj.samples);
    gvk::detail::cerealize_dynamic_array(archive, std::max(1, obj.samples / 32), obj.pSampleMask);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, GvkCommandStructureCmdSetSampleMaskEXT& obj)
{
    archive(obj.sType);
    obj.commandBuffer = gvk::detail::decerealize_handle<VkCommandBuffer>(archive);
    archive(obj.samples);
    obj.pSampleMask = gvk::detail::decerealize_dynamic_array<VkSampleMask>(archive);
}

} // namespace cereal

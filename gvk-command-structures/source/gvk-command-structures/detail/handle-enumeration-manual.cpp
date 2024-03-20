
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

#include "gvk-command-structures/generated/command-structure-enumerate-handles.hpp"
#include "gvk-structures/generated/core-structure-enumerate-handles.hpp"
#include "gvk-structures/detail/get-count.hpp"

namespace gvk {
namespace detail {

#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

template <>
void enumerate_structure_handles<GvkCommandStructureAllocateCommandBuffers>(const GvkCommandStructureAllocateCommandBuffers& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.device, callback);
    enumerate_dynamic_structure_array_handles(1, obj.pAllocateInfo, callback);
    enumerate_dynamic_handle_array(gvk::detail::get_count(obj.pAllocateInfo ? obj.pAllocateInfo->commandBufferCount : 0), obj.pCommandBuffers, callback);
}

template <>
void enumerate_structure_handles<GvkCommandStructureAllocateDescriptorSets>(const GvkCommandStructureAllocateDescriptorSets& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.device, callback);
    enumerate_dynamic_structure_array_handles(1, obj.pAllocateInfo, callback);
    enumerate_dynamic_handle_array(gvk::detail::get_count(obj.pAllocateInfo ? obj.pAllocateInfo->descriptorSetCount : 0), obj.pDescriptorSets, callback);
}

template <>
void enumerate_structure_handles<GvkCommandStructureBuildAccelerationStructuresKHR>(const GvkCommandStructureBuildAccelerationStructuresKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.device, callback);
    enumerate_handle(obj.deferredOperation, callback);
    enumerate_dynamic_structure_array_handles(obj.infoCount, obj.pInfos, callback);
    // Skipping obj.ppBuildRangeInfos; VkAccelerationStructureBuildRangeInfoKHR contains no handles
}

template <>
void enumerate_structure_handles<GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR>(const GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
    enumerate_dynamic_structure_array_handles(obj.infoCount, obj.pInfos, callback);
}

template <>
void enumerate_structure_handles<GvkCommandStructureCmdBuildAccelerationStructuresKHR>(const GvkCommandStructureCmdBuildAccelerationStructuresKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
    enumerate_dynamic_structure_array_handles(obj.infoCount, obj.pInfos, callback);
    // Skipping obj.ppBuildRangeInfos; VkAccelerationStructureBuildRangeInfoKHR contains no handles
}

template <>
void enumerate_structure_handles<GvkCommandStructureCmdPushConstants>(const GvkCommandStructureCmdPushConstants& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
    enumerate_handle(obj.layout, callback);
}

template <> void enumerate_structure_handles<GvkCommandStructureCmdSetBlendConstants>(const GvkCommandStructureCmdSetBlendConstants& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
}

template <> void enumerate_structure_handles<GvkCommandStructureCmdSetFragmentShadingRateEnumNV>(const GvkCommandStructureCmdSetFragmentShadingRateEnumNV& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
}

template <>
void enumerate_structure_handles<GvkCommandStructureCmdSetFragmentShadingRateKHR>(const GvkCommandStructureCmdSetFragmentShadingRateKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
}

template <>
void enumerate_structure_handles<GvkCommandStructureCmdSetSampleMaskEXT>(const GvkCommandStructureCmdSetSampleMaskEXT& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.commandBuffer, callback);
}

template <>
void enumerate_structure_handles<GvkCommandStructureGetAccelerationStructureBuildSizesKHR>(const GvkCommandStructureGetAccelerationStructureBuildSizesKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_handle(obj.device, callback);
    enumerate_dynamic_structure_array_handles(1, obj.pBuildInfo, callback);
    // Skipping obj.pSizeInfo; VkAccelerationStructureBuildSizesInfoKHR contains no handles
}

} // namespace detail
} // namespace gvk


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

#include "gvk-structures/defines.hpp"
#include "gvk-structures/generated/command-structures.h"
#include "gvk-structures/detail/make-tuple-utilities.hpp"

#include <tuple>

namespace gvk {

#define GVK_STUB_MAKE_TUPLE_DEFINITION(VK_STRUCTURE_TYPE) \
inline auto make_tuple(const VK_STRUCTURE_TYPE&) { return std::make_tuple(0); }

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_MAKE_TUPLE_DEFINITION(VkXlibSurfaceCreateInfoKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_MAKE_TUPLE_DEFINITION(SECURITY_ATTRIBUTES)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkExportFenceWin32HandleInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkExportMemoryWin32HandleInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkExportMemoryWin32HandleInfoNV)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkExportSemaphoreWin32HandleInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkImportFenceWin32HandleInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkImportMemoryWin32HandleInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkImportMemoryWin32HandleInfoNV)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkImportSemaphoreWin32HandleInfoKHR)
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264ProfileInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264CapabilitiesEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersCreateInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264PictureInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264DpbSlotInfoEXT)
// Decode H265
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265ProfileInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265CapabilitiesEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersCreateInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265PictureInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265DpbSlotInfoEXT)
// Encode H264
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264CapabilitiesEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersCreateInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264ReferenceListsInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264NaluSliceInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264EmitPictureParametersInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264ProfileInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264RateControlInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264QpEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264FrameSizeEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264RateControlLayerInfoEXT)
// Encode H265
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265CapabilitiesEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersCreateInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265ReferenceListsInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265NaluSliceSegmentInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265VclFrameInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265EmitPictureParametersInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265ProfileInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265RateControlInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265QpEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265FrameSizeEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265RateControlLayerInfoEXT)

////////////////////////////////////////////////////////////////////////////////
// Special case members
inline auto make_tuple(const VkAccelerationStructureBuildGeometryInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.type,
        obj.flags,
        obj.mode,
        obj.srcAccelerationStructure,
        obj.dstAccelerationStructure,
        obj.geometryCount,
        detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryKHR>{ (size_t)obj.geometryCount, obj.pGeometries },
        detail::PointerArrayTupleElementWrapper<VkAccelerationStructureGeometryKHR>{ (size_t)obj.geometryCount, obj.ppGeometries }
        // NOTE : We're ignoring scratchData for comparisons...this can be revisited if
        //  it becomes necessary to differentiate objects by scratchData...
        // obj.scratchData
    );
}

inline auto make_tuple(const VkAccelerationStructureTrianglesOpacityMicromapEXT& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.indexType,
        obj.indexBuffer,
        obj.indexStride,
        obj.baseTriangle,
        obj.usageCountsCount,
        detail::ArrayTupleElementWrapper<VkMicromapUsageEXT>{ (size_t)obj.usageCountsCount, obj.pUsageCounts },
        detail::PointerArrayTupleElementWrapper<VkMicromapUsageEXT>{ (size_t)obj.usageCountsCount, obj.ppUsageCounts },
        obj.micromap
    );
}

inline auto make_tuple(const VkAccelerationStructureVersionInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        // NOTE : pVersionData is expected to point to the header of a previously
        //  serialized acceleration structure, so this comparison just uses the
        //  address...this can be revisited if deep comparisons become necessary.
        obj.pVersionData
    );
}

inline auto make_tuple(const VkMicromapBuildInfoEXT& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.type,
        obj.flags,
        obj.mode,
        obj.dstMicromap,
        obj.usageCountsCount,
        detail::ArrayTupleElementWrapper<VkMicromapUsageEXT>{ (size_t)obj.usageCountsCount, obj.pUsageCounts },
        detail::PointerArrayTupleElementWrapper<VkMicromapUsageEXT>{ (size_t)obj.usageCountsCount, obj.ppUsageCounts },
        obj.data,
        obj.scratchData,
        obj.triangleArray,
        obj.triangleArrayStride
    );
}

inline auto make_tuple(const VkMicromapVersionInfoEXT& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        // NOTE : pVersionData is expected to point to the header of a previously
        //  serialized micromap, so this comparison just uses the address...this can be
        //  revisited if deep comparisons become necessary.
        obj.pVersionData
    );
}

inline auto make_tuple(const VkPipelineMultisampleStateCreateInfo& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.flags,
        obj.rasterizationSamples,
        obj.sampleShadingEnable,
        obj.minSampleShading,
        detail::ArrayTupleElementWrapper<VkSampleMask>{ ((size_t)obj.rasterizationSamples + 31) / 32, obj.pSampleMask },
        obj.alphaToCoverageEnable,
        obj.alphaToOneEnable
    );
}

inline auto make_tuple(const VkShaderModuleCreateInfo& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.flags,
        obj.codeSize,
        detail::ArrayTupleElementWrapper<uint32_t>{ obj.codeSize / sizeof(uint32_t), obj.pCode }
    );
}

inline auto make_tuple(const VkTransformMatrixKHR& obj)
{
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<float>{ 12, (const float*)obj.matrix }
    );
}

////////////////////////////////////////////////////////////////////////////////
// Unions
inline auto make_tuple(const VkAccelerationStructureGeometryDataKHR& obj)
{
    switch (((VkBaseInStructure&)obj).sType) {
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR: {
        return std::make_tuple(
            obj.triangles,
            VkAccelerationStructureGeometryAabbsDataKHR{ },
            VkAccelerationStructureGeometryInstancesDataKHR{ }
        );
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR: {
        return std::make_tuple(
            VkAccelerationStructureGeometryTrianglesDataKHR{ },
            obj.aabbs,
            VkAccelerationStructureGeometryInstancesDataKHR{ }
        );
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR: {
        return std::make_tuple(
            VkAccelerationStructureGeometryTrianglesDataKHR{ },
            VkAccelerationStructureGeometryAabbsDataKHR{ },
            obj.instances
        );
    } break;
    default: {
    } break;
    }
    return std::make_tuple(
        VkAccelerationStructureGeometryTrianglesDataKHR{ },
        VkAccelerationStructureGeometryAabbsDataKHR{ },
        VkAccelerationStructureGeometryInstancesDataKHR{ }
    );
}

inline auto make_tuple(const VkAccelerationStructureMotionInstanceDataNV& obj)
{
    return std::make_tuple(
        obj.srtMotionInstance
    );
}

inline auto make_tuple(const VkClearColorValue& obj)
{
    return std::make_tuple(
        obj.uint32[0],
        obj.uint32[1],
        obj.uint32[2],
        obj.uint32[3]
    );
}

inline auto make_tuple(const VkClearValue& obj)
{
    return std::make_tuple(
        obj.color
    );
}

inline auto make_tuple(const VkDeviceOrHostAddressConstKHR& obj)
{
    return std::make_tuple(
        obj.deviceAddress
    );
}

inline auto make_tuple(const VkDeviceOrHostAddressKHR& obj)
{
    return std::make_tuple(
        obj.deviceAddress
    );
}

inline auto make_tuple(const VkPerformanceCounterResultKHR& obj)
{
    return std::make_tuple(
        obj.uint64
    );
}

inline auto make_tuple(const VkPerformanceValueDataINTEL& obj)
{
    return std::make_tuple(
        obj.value64
    );
}

inline auto make_tuple(const VkPipelineExecutableStatisticValueKHR& obj)
{
    return std::make_tuple(
        obj.u64
    );
}

////////////////////////////////////////////////////////////////////////////////
// GvkCommandStructures
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureAllocateCommandBuffers)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureAllocateDescriptorSets)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureBuildAccelerationStructuresKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdBuildAccelerationStructuresIndirectKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdBuildAccelerationStructuresKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetBlendConstants)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetSampleMaskEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetFragmentShadingRateEnumNV)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCmdSetFragmentShadingRateKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureGetAccelerationStructureBuildSizesKHR)
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureCreateXlibSurfaceKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(GvkCommandStructureGetPhysicalDeviceXlibPresentationSupportKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

} // namespace gvk


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
#include "gvk-structures/detail/make-tuple-utilities.hpp"

#include <tuple>

namespace gvk {

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_MAKE_TUPLE_DEFINITION(VkXlibSurfaceCreateInfoKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
inline auto make_tuple(const VkExportFenceWin32HandleInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.pAttributes,
        obj.dwAccess,
        detail::WStringTupleElementWrapper { obj.name }
    );
}

inline auto make_tuple(const VkExportMemoryWin32HandleInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.pAttributes,
        obj.dwAccess,
        detail::WStringTupleElementWrapper { obj.name }
    );
}

inline auto make_tuple(const VkExportMemoryWin32HandleInfoNV& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.pAttributes,
        obj.dwAccess
    );
}

inline auto make_tuple(const VkExportSemaphoreWin32HandleInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.pAttributes,
        obj.dwAccess,
        detail::WStringTupleElementWrapper { obj.name }
    );
}

inline auto make_tuple(const VkImportFenceWin32HandleInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.fence,
        obj.flags,
        obj.handleType,
        obj.handle,
        detail::WStringTupleElementWrapper { obj.name }
    );
}

inline auto make_tuple(const VkImportMemoryWin32HandleInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.handleType,
        obj.handle,
        detail::WStringTupleElementWrapper { obj.name }
    );
}

inline auto make_tuple(const VkImportMemoryWin32HandleInfoNV& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.handleType,
        obj.handle
    );
}

inline auto make_tuple(const VkImportSemaphoreWin32HandleInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.semaphore,
        obj.flags,
        obj.handleType,
        obj.handle,
        detail::WStringTupleElementWrapper { obj.name }
    );
}
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoBeginCodingInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoCodingControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeUsageInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264FrameSizeKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264GopRemainingFrameInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264NaluSliceInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264QpKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264QualityLevelPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264RateControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264RateControlLayerInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersFeedbackInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersGetInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265FrameSizeKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265GopRemainingFrameInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265NaluSliceSegmentInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265QpKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265QualityLevelPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265RateControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265RateControlLayerInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersFeedbackInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersGetInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQualityLevelInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQualityLevelPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeSessionParametersFeedbackInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeSessionParametersGetInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeUsageInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEndCodingInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoFormatPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoInlineQueryInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoPictureResourceInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoProfileListInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoReferenceSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionParametersUpdateInfoKHR)

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
        detail::PointerArrayTupleElementWrapper<VkAccelerationStructureGeometryKHR>{ (size_t)obj.geometryCount, obj.ppGeometries },
        obj.scratchData
    );
}

inline auto make_tuple(const VkAccelerationStructureTrianglesDisplacementMicromapNV& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.displacementBiasAndScaleFormat,
        obj.displacementVectorFormat,
        obj.displacementBiasAndScaleBuffer,
        obj.displacementBiasAndScaleStride,
        obj.displacementVectorBuffer,
        obj.displacementVectorStride,
        obj.displacedMicromapPrimitiveFlags,
        obj.displacedMicromapPrimitiveFlagsStride,
        obj.indexType,
        obj.indexBuffer,
        obj.indexStride,
        obj.baseTriangle,
        obj.usageCountsCount,
        detail::ArrayTupleElementWrapper<VkMicromapUsageEXT> { (size_t)obj.usageCountsCount, obj.pUsageCounts },
        detail::PointerArrayTupleElementWrapper<VkMicromapUsageEXT> { (size_t)obj.usageCountsCount, obj.ppUsageCounts },
        obj.micromap
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

inline auto make_tuple(const VkPipelineCacheCreateInfo& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.flags,
        obj.initialDataSize,
        detail::ArrayTupleElementWrapper<uint8_t> { obj.initialDataSize, (const uint8_t*)obj.pInitialData }
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

inline auto make_tuple(const VkShaderCreateInfoEXT& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.flags,
        obj.stage,
        obj.nextStage,
        obj.codeType,
        obj.codeSize,
        detail::ArrayTupleElementWrapper<uint8_t> { obj.codeSize, (const uint8_t*)obj.pCode },
        detail::StringTupleElementWrapper { obj.pName },
        obj.setLayoutCount,
        detail::ArrayTupleElementWrapper<VkDescriptorSetLayout> { obj.setLayoutCount, obj.pSetLayouts },
        obj.pushConstantRangeCount,
        detail::ArrayTupleElementWrapper<VkPushConstantRange> { obj.pushConstantRangeCount, obj.pPushConstantRanges },
        detail::ArrayTupleElementWrapper<VkSpecializationInfo> { 1, obj.pSpecializationInfo }
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

inline auto make_tuple(const VkSpecializationInfo& obj)
{
    return std::make_tuple(
        obj.mapEntryCount,
        detail::ArrayTupleElementWrapper<VkSpecializationMapEntry> { obj.mapEntryCount, obj.pMapEntries },
        obj.dataSize,
        detail::ArrayTupleElementWrapper<uint8_t> { obj.dataSize, (const uint8_t*)obj.pData }
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

} // namespace gvk


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
inline auto make_tuple(const VkXlibSurfaceCreateInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        obj.flags,
        obj.dpy,
        obj.window
    );
}
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
GVK_STUB_MAKE_TUPLE_DEFINITION(VkBindVideoSessionMemoryInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoDecodeVP9FeaturesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoEncodeAV1FeaturesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoFormatInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoMaintenance1FeaturesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkPhysicalDeviceVideoMaintenance2FeaturesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkQueryPoolVideoEncodeFeedbackCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkQueueFamilyVideoPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoBeginCodingInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoCodingControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeAV1CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeAV1DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeAV1InlineSessionParametersInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeAV1PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeAV1ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeAV1SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264InlineSessionParametersInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265InlineSessionParametersInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeUsageInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeVP9CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeVP9PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeVP9ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1CapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1DpbSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1FrameSizeKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1GopRemainingFrameInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1PictureInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1ProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1QIndexKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1QualityLevelPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1QuantizationMapCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1RateControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1RateControlLayerInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1SessionCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeAV1SessionParametersCreateInfoKHR)
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
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264QuantizationMapCapabilitiesKHR)
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
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265QuantizationMapCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265RateControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265RateControlLayerInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersFeedbackInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersGetInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeIntraRefreshCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeIntraRefreshInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQualityLevelInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQualityLevelPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQuantizationMapCapabilitiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQuantizationMapInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeSessionIntraRefreshCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeSessionParametersFeedbackInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeSessionParametersGetInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeUsageInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEndCodingInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoFormatAV1QuantizationMapPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoFormatH265QuantizationMapPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoFormatPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoFormatQuantizationMapPropertiesKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoInlineQueryInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoPictureResourceInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoProfileInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoProfileListInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoReferenceIntraRefreshInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoReferenceSlotInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoSessionParametersUpdateInfoKHR)

////////////////////////////////////////////////////////////////////////////////
// Special case members
inline auto make_tuple(const VkAccelerationStructureVersionInfoKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        detail::ArrayTupleElementWrapper<uint8_t> { 2 * VK_UUID_SIZE, obj.pVersionData }
    );
}

inline auto make_tuple(const VkMicromapVersionInfoEXT& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper { obj.pNext },
        detail::ArrayTupleElementWrapper<uint8_t> { 2 * VK_UUID_SIZE, obj.pVersionData }
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

inline auto make_tuple(const VkPipelineExecutableInternalRepresentationKHR& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        detail::ArrayTupleElementWrapper<char> { VK_MAX_DESCRIPTION_SIZE, obj.name },
        detail::ArrayTupleElementWrapper<char> { VK_MAX_DESCRIPTION_SIZE, obj.description },
        obj.isText,
        obj.dataSize,
        detail::ArrayTupleElementWrapper<uint8_t> { obj.dataSize, (const uint8_t*)obj.pData }
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

inline auto make_tuple(const VkWriteDescriptorSet& obj)
{
    detail::ArrayTupleElementWrapper<VkDescriptorBufferInfo> bufferInfos;
    detail::ArrayTupleElementWrapper<VkDescriptorImageInfo> imageInfos;
    detail::ArrayTupleElementWrapper<VkBufferView> texelBufferViews;
    switch (obj.descriptorType) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
        bufferInfos.count = obj.descriptorCount;
        bufferInfos.ptr = obj.pBufferInfo;
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        imageInfos.count = obj.descriptorCount;
        imageInfos.ptr = obj.pImageInfo;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        texelBufferViews.count = obj.descriptorCount;
        texelBufferViews.ptr = obj.pTexelBufferView;
    } break;
    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
    case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
    case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
    case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
    default: {
        // NOOP :
    } break;
    }
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.dstSet,
        obj.dstBinding,
        obj.dstArrayElement,
        obj.descriptorCount,
        obj.descriptorType,
        bufferInfos,
        imageInfos,
        texelBufferViews
    );
}

////////////////////////////////////////////////////////////////////////////////
// Array of pointer members
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
        detail::PNextTupleElementWrapper{ obj.pNext },
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

inline auto make_tuple(const VkMicromapBuildInfoEXT& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
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

////////////////////////////////////////////////////////////////////////////////
// Unions
inline auto make_tuple(const VkAccelerationStructureGeometryDataKHR& obj)
{
    switch (((VkBaseInStructure&)obj).sType) {
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryTrianglesDataKHR> { 1, &obj.triangles },
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryAabbsDataKHR> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryInstancesDataKHR> { 0, nullptr }
        );
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryTrianglesDataKHR> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryAabbsDataKHR> { 1, &obj.aabbs },
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryInstancesDataKHR> { 0, nullptr }
        );
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryTrianglesDataKHR> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryAabbsDataKHR> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryInstancesDataKHR> { 1, &obj.instances }
        );
    } break;
    default: {
    } break;
    }
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryTrianglesDataKHR> { 0, nullptr },
        detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryAabbsDataKHR> { 0, nullptr },
        detail::ArrayTupleElementWrapper<VkAccelerationStructureGeometryInstancesDataKHR> { 0, nullptr }
    );
}

inline auto make_tuple(const VkAccelerationStructureMotionInstanceDataNV& obj)
{
    // POD union
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<uint8_t> { sizeof(obj), (const uint8_t*)&obj }
    );
}

inline auto make_tuple(const VkClearColorValue& obj)
{
    // POD union
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<uint8_t> { sizeof(obj), (const uint8_t*)&obj }
    );
}

inline auto make_tuple(const VkClearValue& obj)
{
    // POD union
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<uint8_t> { sizeof(obj), (const uint8_t*)&obj }
    );
}

inline auto make_tuple(const VkClusterAccelerationStructureOpInputNV& obj)
{
    // NOTE : Union of pointers to structures that all have sType; use sType to interpret.
    auto pObj = (const VkBaseInStructure*&)obj;
    auto sType = pObj ? pObj->sType : VkStructureType{ };
    switch (sType) {
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureClustersBottomLevelInputNV> { 1, obj.pClustersBottomLevel },
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureTriangleClusterInputNV> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureMoveObjectsInputNV> { 0, nullptr }
        );
    } break;
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureClustersBottomLevelInputNV> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureTriangleClusterInputNV> { 1, obj.pTriangleClusters },
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureMoveObjectsInputNV> { 0, nullptr }
        );
    } break;
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureClustersBottomLevelInputNV> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureTriangleClusterInputNV> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureMoveObjectsInputNV> { 1, obj.pMoveObjects }
        );
    } break;
    default: {
    } break;
    }
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureClustersBottomLevelInputNV> { 0, nullptr },
        detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureTriangleClusterInputNV> { 0, nullptr },
        detail::ArrayTupleElementWrapper<VkClusterAccelerationStructureMoveObjectsInputNV> { 0, nullptr }
    );
}

inline auto make_tuple(const VkDescriptorDataEXT& obj)
{
    assert(false && "VkDescriptorDataEXT cannot be directly converted to std::tuple<>; convert to std::tuple<> via VkDescriptorGetInfoEXT which specifies VkDescriptorType");
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<uint8_t> { sizeof(obj), (const uint8_t*)&obj }
    );
}

inline auto make_tuple(const VkDeviceOrHostAddressConstAMDX& obj)
{
    assert(sizeof(obj.deviceAddress) == sizeof(VkDeviceOrHostAddressConstAMDX));
    return std::make_tuple(
        obj.deviceAddress
    );
}

inline auto make_tuple(const VkDeviceOrHostAddressConstKHR& obj)
{
    assert(sizeof(obj.deviceAddress) == sizeof(VkDeviceOrHostAddressConstKHR));
    return std::make_tuple(
        obj.deviceAddress
    );
}

inline auto make_tuple(const VkDeviceOrHostAddressKHR& obj)
{
    assert(sizeof(obj.deviceAddress) == sizeof(VkDeviceOrHostAddressKHR));
    return std::make_tuple(
        obj.deviceAddress
    );
}

inline auto make_tuple(const VkIndirectCommandsTokenDataEXT& obj)
{
    assert(false && "VkIndirectCommandsTokenDataEXT cannot be directly converted to std::tuple<>; convert to std::tuple<> via VkIndirectCommandsLayoutTokenEXT which specifies VkIndirectCommandsTokenTypeEXT");
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<uint8_t> { sizeof(obj), (const uint8_t*)&obj }
    );
}

inline auto make_tuple(const VkIndirectExecutionSetInfoEXT& obj)
{
    // NOTE : Union of pointers to structures that all have sType; use sType to interpret.
    auto pObj = (const VkBaseInStructure*&)obj;
    auto sType = pObj ? pObj->sType : VkStructureType{ };
    switch (sType) {
    case VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkIndirectExecutionSetPipelineInfoEXT> { 1, obj.pPipelineInfo },
            detail::ArrayTupleElementWrapper<VkIndirectExecutionSetShaderInfoEXT> { 0, nullptr }
        );
    } break;
    case VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT: {
        return std::make_tuple(
            detail::ArrayTupleElementWrapper<VkIndirectExecutionSetPipelineInfoEXT> { 0, nullptr },
            detail::ArrayTupleElementWrapper<VkIndirectExecutionSetShaderInfoEXT> { 1, obj.pShaderInfo }
        );
    } break;
    default: {
    } break;
    }
    return std::make_tuple(
        detail::ArrayTupleElementWrapper<VkIndirectExecutionSetPipelineInfoEXT> { 0, nullptr },
        detail::ArrayTupleElementWrapper<VkIndirectExecutionSetShaderInfoEXT> { 0, nullptr }
    );
}

inline auto make_tuple(const VkPerformanceCounterResultKHR& obj)
{
    assert(sizeof(obj.uint64) == sizeof(VkPerformanceCounterResultKHR));
    return std::make_tuple(
        obj.uint64
    );
}

inline auto make_tuple(const VkPerformanceValueDataINTEL& obj)
{
    assert(sizeof(obj.value64) == sizeof(VkPerformanceValueDataINTEL));
    return std::make_tuple(
        obj.value64
    );
}

inline auto make_tuple(const VkPipelineExecutableStatisticValueKHR& obj)
{
    assert(sizeof(obj.u64) == sizeof(VkPipelineExecutableStatisticValueKHR));
    return std::make_tuple(
        obj.u64
    );
}

////////////////////////////////////////////////////////////////////////////////
// Union members
inline auto make_tuple(const VkDescriptorGetInfoEXT& obj)
{
    detail::ArrayTupleElementWrapper<VkSampler> sampler;
    detail::ArrayTupleElementWrapper<VkDescriptorImageInfo> descriptorImageInfo;
    detail::ArrayTupleElementWrapper<VkDescriptorAddressInfoEXT> descriptorAddressInfo;
    VkDeviceAddress deviceAddress = 0;
    switch (obj.type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER: {
        sampler.count = 1;
        sampler.ptr = obj.data.pSampler;
    } break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
        descriptorImageInfo.count = 1;
        descriptorImageInfo.ptr = obj.data.pCombinedImageSampler;
    } break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        descriptorImageInfo.count = 1;
        descriptorImageInfo.ptr = obj.data.pInputAttachmentImage;
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
        descriptorImageInfo.count = 1;
        descriptorImageInfo.ptr = obj.data.pSampledImage;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
        descriptorImageInfo.count = 1;
        descriptorImageInfo.ptr = obj.data.pStorageImage;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
        descriptorAddressInfo.count = 1;
        descriptorAddressInfo.ptr = obj.data.pUniformTexelBuffer;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        descriptorAddressInfo.count = 1;
        descriptorAddressInfo.ptr = obj.data.pStorageTexelBuffer;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
        descriptorAddressInfo.count = 1;
        descriptorAddressInfo.ptr = obj.data.pUniformBuffer;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
        descriptorAddressInfo.count = 1;
        descriptorAddressInfo.ptr = obj.data.pStorageBuffer;
    } break;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
        deviceAddress = obj.data.accelerationStructure;
    } break;
    default: {
        // NOOP :
    } break;
    }
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.type,
        sampler,
        descriptorImageInfo,
        descriptorAddressInfo,
        deviceAddress
    );
}

inline auto make_tuple(const VkIndirectCommandsLayoutTokenEXT& obj)
{
    detail::ArrayTupleElementWrapper<VkIndirectCommandsPushConstantTokenEXT> pushConstant;
    detail::ArrayTupleElementWrapper<VkIndirectCommandsVertexBufferTokenEXT> vertexBuffer;
    detail::ArrayTupleElementWrapper<VkIndirectCommandsIndexBufferTokenEXT> indexBuffer;
    detail::ArrayTupleElementWrapper<VkIndirectCommandsExecutionSetTokenEXT> executionSet;
    switch (obj.type) {
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT: {
        pushConstant.count = 1;
        pushConstant.ptr = obj.data.pPushConstant;
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT: {
        vertexBuffer.count = 1;
        vertexBuffer.ptr = obj.data.pVertexBuffer;
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT: {
        indexBuffer.count = 1;
        indexBuffer.ptr = obj.data.pIndexBuffer;
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT: {
        executionSet.count = 1;
        executionSet.ptr = obj.data.pExecutionSet;
    } break;
    default: {
        // NOOP :
    } break;
    }
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.type,
        pushConstant,
        vertexBuffer,
        indexBuffer,
        executionSet,
        obj.offset
    );
}

inline auto make_tuple(const VkPerformanceValueINTEL& obj)
{
    return std::make_tuple(
        obj.type,
        obj.data
    );
}

} // namespace gvk

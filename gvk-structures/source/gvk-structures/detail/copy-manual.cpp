
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

#include "gvk-structures/detail/copy-utilities.hpp"
#include "gvk-structures/generated/core-structure-create-copy.hpp"
#include "gvk-structures/generated/core-structure-destroy-copy.hpp"

namespace gvk {
namespace detail {

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
template <> VkXlibSurfaceCreateInfoKHR create_structure_copy<VkXlibSurfaceCreateInfoKHR>(const VkXlibSurfaceCreateInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkXlibSurfaceCreateInfoKHR>(const VkXlibSurfaceCreateInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
}
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(SECURITY_ATTRIBUTES)

template <> VkExportFenceWin32HandleInfoKHR create_structure_copy<VkExportFenceWin32HandleInfoKHR>(const VkExportFenceWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.pAttributes = create_dynamic_array_copy(1, result.pAttributes, pAllocator);
    result.name = create_dynamic_string_copy(obj.name, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkExportFenceWin32HandleInfoKHR>(const VkExportFenceWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(1, obj.pAttributes, pAllocator);
    destroy_dynamic_string_copy(obj.name, pAllocator);
}

template <> VkExportMemoryWin32HandleInfoKHR create_structure_copy<VkExportMemoryWin32HandleInfoKHR>(const VkExportMemoryWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.pAttributes = create_dynamic_array_copy(1, result.pAttributes, pAllocator);
    result.name = create_dynamic_string_copy(obj.name, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkExportMemoryWin32HandleInfoKHR>(const VkExportMemoryWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(1, obj.pAttributes, pAllocator);
    destroy_dynamic_string_copy(obj.name, pAllocator);
}

template <> VkExportMemoryWin32HandleInfoNV create_structure_copy<VkExportMemoryWin32HandleInfoNV>(const VkExportMemoryWin32HandleInfoNV& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.pAttributes = create_dynamic_array_copy(1, result.pAttributes, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkExportMemoryWin32HandleInfoNV>(const VkExportMemoryWin32HandleInfoNV& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(1, obj.pAttributes, pAllocator);
}

template <> VkExportSemaphoreWin32HandleInfoKHR create_structure_copy<VkExportSemaphoreWin32HandleInfoKHR>(const VkExportSemaphoreWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.pAttributes = create_dynamic_array_copy(1, result.pAttributes, pAllocator);
    result.name = create_dynamic_string_copy(obj.name, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkExportSemaphoreWin32HandleInfoKHR>(const VkExportSemaphoreWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(1, obj.pAttributes, pAllocator);
    destroy_dynamic_string_copy(obj.name, pAllocator);
}

template <> VkImportFenceWin32HandleInfoKHR create_structure_copy<VkImportFenceWin32HandleInfoKHR>(const VkImportFenceWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.name = create_dynamic_string_copy(obj.name, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkImportFenceWin32HandleInfoKHR>(const VkImportFenceWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_string_copy(obj.name, pAllocator);
}

template <> VkImportMemoryWin32HandleInfoKHR create_structure_copy<VkImportMemoryWin32HandleInfoKHR>(const VkImportMemoryWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.name = create_dynamic_string_copy(obj.name, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkImportMemoryWin32HandleInfoKHR>(const VkImportMemoryWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_string_copy(obj.name, pAllocator);
}

template <> VkImportMemoryWin32HandleInfoNV create_structure_copy<VkImportMemoryWin32HandleInfoNV>(const VkImportMemoryWin32HandleInfoNV& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkImportMemoryWin32HandleInfoNV>(const VkImportMemoryWin32HandleInfoNV& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
}

template <> VkImportSemaphoreWin32HandleInfoKHR create_structure_copy<VkImportSemaphoreWin32HandleInfoKHR>(const VkImportSemaphoreWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.name = create_dynamic_string_copy(obj.name, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkImportSemaphoreWin32HandleInfoKHR>(const VkImportSemaphoreWin32HandleInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_string_copy(obj.name, pAllocator);
}
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkBindVideoSessionMemoryInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoDecodeVP9FeaturesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoEncodeAV1FeaturesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoFormatInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoMaintenance1FeaturesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPhysicalDeviceVideoMaintenance2FeaturesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkQueryPoolVideoEncodeFeedbackCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkQueueFamilyVideoPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoBeginCodingInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoCodingControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeAV1CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeAV1DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeAV1InlineSessionParametersInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeAV1PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeAV1ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeAV1SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264InlineSessionParametersInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265InlineSessionParametersInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeUsageInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeVP9CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeVP9PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeVP9ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1FrameSizeKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1GopRemainingFrameInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1QIndexKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1QualityLevelPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1QuantizationMapCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1RateControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1RateControlLayerInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1SessionCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeAV1SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264FrameSizeKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264GopRemainingFrameInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264NaluSliceInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264QpKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264QualityLevelPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264QuantizationMapCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264RateControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264RateControlLayerInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersAddInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersFeedbackInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersGetInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265FrameSizeKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265GopRemainingFrameInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265NaluSliceSegmentInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265QpKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265QualityLevelPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265QuantizationMapCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265RateControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265RateControlLayerInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersAddInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersFeedbackInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersGetInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeIntraRefreshCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeIntraRefreshInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeQualityLevelInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeQualityLevelPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeQuantizationMapCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeQuantizationMapInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeSessionIntraRefreshCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeSessionParametersFeedbackInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeSessionParametersGetInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeUsageInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEndCodingInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoFormatAV1QuantizationMapPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoFormatH265QuantizationMapPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoFormatPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoFormatQuantizationMapPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoInlineQueryInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoPictureResourceInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoProfileListInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoReferenceIntraRefreshInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoReferenceSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionParametersUpdateInfoKHR)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <> VkAccelerationStructureVersionInfoKHR create_structure_copy<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pVersionData = create_dynamic_array_copy(2 * VK_UUID_SIZE, obj.pVersionData, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(2 * VK_UUID_SIZE, obj.pVersionData, pAllocator);
}

template <> VkMicromapVersionInfoEXT create_structure_copy<VkMicromapVersionInfoEXT>(const VkMicromapVersionInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pVersionData = create_dynamic_array_copy(2 * VK_UUID_SIZE, obj.pVersionData, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkMicromapVersionInfoEXT>(const VkMicromapVersionInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(2 * VK_UUID_SIZE, obj.pVersionData, pAllocator);
}

template <> VkPipelineCacheCreateInfo create_structure_copy<VkPipelineCacheCreateInfo>(const VkPipelineCacheCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pInitialData = (const void*)create_dynamic_array_copy(obj.initialDataSize, (const uint8_t*)obj.pInitialData, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkPipelineCacheCreateInfo>(const VkPipelineCacheCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.initialDataSize, (const uint8_t*)obj.pInitialData, pAllocator);
}

template <> VkPipelineExecutableInternalRepresentationKHR create_structure_copy<VkPipelineExecutableInternalRepresentationKHR>(const VkPipelineExecutableInternalRepresentationKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pData = (void*)create_dynamic_array_copy(obj.dataSize, (const uint8_t*)obj.pData, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkPipelineExecutableInternalRepresentationKHR>(const VkPipelineExecutableInternalRepresentationKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.dataSize, (const uint8_t*)obj.pData, pAllocator);
}

template <> VkPipelineMultisampleStateCreateInfo create_structure_copy<VkPipelineMultisampleStateCreateInfo>(const VkPipelineMultisampleStateCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pSampleMask = create_dynamic_array_copy((obj.rasterizationSamples + 31) / 32, obj.pSampleMask, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkPipelineMultisampleStateCreateInfo>(const VkPipelineMultisampleStateCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy((obj.rasterizationSamples + 31) / 32, obj.pSampleMask, pAllocator);
}

template <> VkShaderCreateInfoEXT create_structure_copy<VkShaderCreateInfoEXT>(const VkShaderCreateInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pCode = create_dynamic_array_copy(obj.codeSize, (uint8_t*)obj.pCode, pAllocator);
    result.pName = create_dynamic_string_copy(obj.pName, pAllocator);
    result.pSetLayouts = create_dynamic_array_copy(obj.setLayoutCount, obj.pSetLayouts, pAllocator);
    result.pPushConstantRanges = create_dynamic_array_copy(obj.pushConstantRangeCount, obj.pPushConstantRanges, pAllocator);
    result.pSpecializationInfo = create_dynamic_array_copy(1, obj.pSpecializationInfo, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkShaderCreateInfoEXT>(const VkShaderCreateInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.codeSize, (uint8_t*)obj.pCode, pAllocator);
    destroy_dynamic_string_copy(obj.pName, pAllocator);
    destroy_dynamic_array_copy(obj.setLayoutCount, obj.pSetLayouts, pAllocator);
    destroy_dynamic_array_copy(obj.pushConstantRangeCount, obj.pPushConstantRanges, pAllocator);
    destroy_dynamic_array_copy(1, obj.pSpecializationInfo, pAllocator);
}

template <> VkShaderModuleCreateInfo create_structure_copy<VkShaderModuleCreateInfo>(const VkShaderModuleCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pCode = create_dynamic_array_copy(obj.codeSize / sizeof(uint32_t), obj.pCode, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkShaderModuleCreateInfo>(const VkShaderModuleCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.codeSize / sizeof(uint32_t), obj.pCode, pAllocator);
}

template <> VkSpecializationInfo create_structure_copy<VkSpecializationInfo>(const VkSpecializationInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pMapEntries = create_dynamic_array_copy(obj.mapEntryCount, obj.pMapEntries, pAllocator);
    result.pData = (const void*)create_dynamic_array_copy(obj.dataSize, (const uint8_t*)obj.pData, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkSpecializationInfo>(const VkSpecializationInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_dynamic_array_copy(obj.mapEntryCount, obj.pMapEntries, pAllocator);
    destroy_dynamic_array_copy(obj.dataSize, (const uint8_t*)obj.pData, pAllocator);
}

template <> VkTransformMatrixKHR create_structure_copy<VkTransformMatrixKHR>(const VkTransformMatrixKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    return obj;
}

template <> void destroy_structure_copy<VkTransformMatrixKHR>(const VkTransformMatrixKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
}

template <> VkWriteDescriptorSet create_structure_copy<VkWriteDescriptorSet>(const VkWriteDescriptorSet& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pBufferInfo = nullptr;
    result.pImageInfo = nullptr;
    result.pTexelBufferView = nullptr;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    switch (obj.descriptorType) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
        result.pBufferInfo = create_dynamic_array_copy(obj.descriptorCount, obj.pBufferInfo, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        result.pImageInfo = create_dynamic_array_copy(obj.descriptorCount, obj.pImageInfo, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        result.pTexelBufferView = create_dynamic_array_copy(obj.descriptorCount, obj.pTexelBufferView, pAllocator);
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
    return result;
}

template <> void destroy_structure_copy<VkWriteDescriptorSet>(const VkWriteDescriptorSet& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    switch (obj.descriptorType) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
        destroy_dynamic_array_copy(obj.descriptorCount, obj.pBufferInfo, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        destroy_dynamic_array_copy(obj.descriptorCount, obj.pImageInfo, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        destroy_dynamic_array_copy(obj.descriptorCount, obj.pTexelBufferView, pAllocator);
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
}

////////////////////////////////////////////////////////////////////////////////
// Array of pointer members
template <> VkAccelerationStructureBuildGeometryInfoKHR create_structure_copy<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pGeometries = create_dynamic_array_copy(obj.geometryCount, obj.pGeometries, pAllocator);
    result.ppGeometries = create_dynamic_pointer_array_copy(obj.geometryCount, obj.ppGeometries, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.geometryCount, obj.pGeometries, pAllocator);
    destroy_dynamic_pointer_array_copy(obj.geometryCount, obj.ppGeometries, pAllocator);
}

template <> VkAccelerationStructureTrianglesDisplacementMicromapNV create_structure_copy<VkAccelerationStructureTrianglesDisplacementMicromapNV>(const VkAccelerationStructureTrianglesDisplacementMicromapNV& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.pUsageCounts = create_dynamic_array_copy(obj.usageCountsCount, obj.pUsageCounts, pAllocator);
    result.ppUsageCounts = create_dynamic_pointer_array_copy(obj.usageCountsCount, obj.ppUsageCounts, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureTrianglesDisplacementMicromapNV>(const VkAccelerationStructureTrianglesDisplacementMicromapNV& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.usageCountsCount, obj.pUsageCounts, pAllocator);
    destroy_dynamic_pointer_array_copy(obj.usageCountsCount, obj.ppUsageCounts, pAllocator);
}

template <> VkAccelerationStructureTrianglesOpacityMicromapEXT create_structure_copy<VkAccelerationStructureTrianglesOpacityMicromapEXT>(const VkAccelerationStructureTrianglesOpacityMicromapEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);
    result.pUsageCounts = create_dynamic_array_copy(obj.usageCountsCount, obj.pUsageCounts, pAllocator);
    result.ppUsageCounts = create_dynamic_pointer_array_copy(obj.usageCountsCount, obj.ppUsageCounts, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureTrianglesOpacityMicromapEXT>(const VkAccelerationStructureTrianglesOpacityMicromapEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.usageCountsCount, obj.pUsageCounts, pAllocator);
    destroy_dynamic_pointer_array_copy(obj.usageCountsCount, obj.ppUsageCounts, pAllocator);
}

template <> VkMicromapBuildInfoEXT create_structure_copy<VkMicromapBuildInfoEXT>(const VkMicromapBuildInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pUsageCounts = create_dynamic_array_copy(obj.usageCountsCount, obj.pUsageCounts, pAllocator);
    result.ppUsageCounts = create_dynamic_pointer_array_copy(obj.usageCountsCount, obj.ppUsageCounts, pAllocator);
    return result;
}

template <> void destroy_structure_copy<VkMicromapBuildInfoEXT>(const VkMicromapBuildInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.usageCountsCount, obj.pUsageCounts, pAllocator);
    destroy_dynamic_pointer_array_copy(obj.usageCountsCount, obj.ppUsageCounts, pAllocator);
}

////////////////////////////////////////////////////////////////////////////////
// Unions
template <> VkAccelerationStructureGeometryDataKHR create_structure_copy<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Union of structures that all have sType; use sType to interpret.
    auto result = obj;
    switch (((VkBaseInStructure&)obj).sType) {
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR: {
        result.triangles = create_structure_copy(result.triangles, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR: {
        result.aabbs = create_structure_copy(result.aabbs, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR: {
        result.instances = create_structure_copy(result.instances, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Union of structures that all have sType; use sType to interpret.
    switch (((VkBaseInStructure&)obj).sType) {
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR: {
        destroy_structure_copy(obj.triangles, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR: {
        destroy_structure_copy(obj.aabbs, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR: {
        destroy_structure_copy(obj.instances, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
}

template <> VkAccelerationStructureMotionInstanceDataNV create_structure_copy<VkAccelerationStructureMotionInstanceDataNV>(const VkAccelerationStructureMotionInstanceDataNV& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkAccelerationStructureMotionInstanceDataNV>(const VkAccelerationStructureMotionInstanceDataNV& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkClearColorValue create_structure_copy<VkClearColorValue>(const VkClearColorValue& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkClearColorValue>(const VkClearColorValue& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkClearValue create_structure_copy<VkClearValue>(const VkClearValue& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkClearValue>(const VkClearValue& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkClusterAccelerationStructureOpInputNV create_structure_copy<VkClusterAccelerationStructureOpInputNV>(const VkClusterAccelerationStructureOpInputNV& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Union of pointers to structures that all have sType; use sType to interpret.
    auto result = obj;
    auto pObj = (const VkBaseInStructure*&)obj;
    switch (pObj ? pObj->sType : VkStructureType{ }) {
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV: {
        result.pClustersBottomLevel = create_dynamic_array_copy(1, obj.pClustersBottomLevel, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV: {
        result.pTriangleClusters = create_dynamic_array_copy(1, obj.pTriangleClusters, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV: {
        result.pMoveObjects = create_dynamic_array_copy(1, obj.pMoveObjects, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
    return result;
}

template <> void destroy_structure_copy<VkClusterAccelerationStructureOpInputNV>(const VkClusterAccelerationStructureOpInputNV& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Union of pointers to structures that all have sType; use sType to interpret.
    auto pObj = (const VkBaseInStructure*&)obj;
    switch (pObj ? pObj->sType : VkStructureType{ }) {
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV: {
        destroy_dynamic_array_copy(1, obj.pClustersBottomLevel, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV: {
        destroy_dynamic_array_copy(1, obj.pTriangleClusters, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV: {
        destroy_dynamic_array_copy(1, obj.pMoveObjects, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
}

template <> VkDescriptorDataEXT create_structure_copy<VkDescriptorDataEXT>(const VkDescriptorDataEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    assert(false && "VkDescriptorDataEXT cannot be directly copied; copy via VkDescriptorGetInfoEXT which specifies VkDescriptorType");
    return obj;
}

template <> void destroy_structure_copy<VkDescriptorDataEXT>(const VkDescriptorDataEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    assert(false && "VkDescriptorDataEXT cannot be directly destroyed; destroy via VkDescriptorGetInfoEXT which specifies VkDescriptorType");
}

template <> VkDeviceOrHostAddressConstAMDX create_structure_copy<VkDeviceOrHostAddressConstAMDX>(const VkDeviceOrHostAddressConstAMDX& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkDeviceOrHostAddressConstAMDX>(const VkDeviceOrHostAddressConstAMDX& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkDeviceOrHostAddressConstKHR create_structure_copy<VkDeviceOrHostAddressConstKHR>(const VkDeviceOrHostAddressConstKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkDeviceOrHostAddressConstKHR>(const VkDeviceOrHostAddressConstKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkDeviceOrHostAddressKHR create_structure_copy<VkDeviceOrHostAddressKHR>(const VkDeviceOrHostAddressKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkDeviceOrHostAddressKHR>(const VkDeviceOrHostAddressKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkIndirectCommandsTokenDataEXT create_structure_copy<VkIndirectCommandsTokenDataEXT>(const VkIndirectCommandsTokenDataEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    assert(false && "VkIndirectCommandsTokenDataEXT cannot be directly copied; copy via VkIndirectCommandsLayoutTokenEXT which specifies VkIndirectCommandsTokenTypeEXT");
    return obj;
}

template <> void destroy_structure_copy<VkIndirectCommandsTokenDataEXT>(const VkIndirectCommandsTokenDataEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    assert(false && "VkIndirectCommandsTokenDataEXT cannot be directly destroyed; destroy via VkIndirectCommandsLayoutTokenEXT which specifies VkIndirectCommandsTokenTypeEXT");
}

template <> VkIndirectExecutionSetInfoEXT create_structure_copy<VkIndirectExecutionSetInfoEXT>(const VkIndirectExecutionSetInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Union of pointers to structures that all have sType; use sType to interpret.
    auto result = obj;
    auto pObj = (const VkBaseInStructure*&)obj;
    switch (pObj ? pObj->sType : VkStructureType{ }) {
    case VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT: {
        result.pPipelineInfo = create_dynamic_array_copy(1, obj.pPipelineInfo, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT: {
        result.pShaderInfo = create_dynamic_array_copy(1, obj.pShaderInfo, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
    return result;
}

template <> void destroy_structure_copy<VkIndirectExecutionSetInfoEXT>(const VkIndirectExecutionSetInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Union of pointers to structures that all have sType; use sType to interpret.
    auto pObj = (const VkBaseInStructure*&)obj;
    switch (pObj ? pObj->sType : VkStructureType{ }) {
    case VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT: {
        destroy_dynamic_array_copy(1, obj.pPipelineInfo, pAllocator);
    } break;
    case VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT: {
        destroy_dynamic_array_copy(1, obj.pShaderInfo, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
}

template <> VkPerformanceCounterResultKHR create_structure_copy<VkPerformanceCounterResultKHR>(const VkPerformanceCounterResultKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkPerformanceCounterResultKHR>(const VkPerformanceCounterResultKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

template <> VkPerformanceValueDataINTEL create_structure_copy<VkPerformanceValueDataINTEL>(const VkPerformanceValueDataINTEL& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    assert(false && "VkPerformanceValueDataINTEL cannot be directly copied; copy via VkPerformanceValueINTEL which specifies VkPerformanceValueTypeINTEL");
    return obj;
}

template <> void destroy_structure_copy<VkPerformanceValueDataINTEL>(const VkPerformanceValueDataINTEL& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    assert(false && "VkPerformanceValueDataINTEL cannot be directly destroyed; destroy via VkPerformanceValueINTEL which specifies VkPerformanceValueTypeINTEL");
}

template <> VkPipelineExecutableStatisticValueKHR create_structure_copy<VkPipelineExecutableStatisticValueKHR>(const VkPipelineExecutableStatisticValueKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)pAllocator;
    // NOTE : POD union
    return obj;
}

template <> void destroy_structure_copy<VkPipelineExecutableStatisticValueKHR>(const VkPipelineExecutableStatisticValueKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    (void)obj;
    (void)pAllocator;
    // NOOP : POD union
}

////////////////////////////////////////////////////////////////////////////////
// Union members
template <> VkDescriptorGetInfoEXT create_structure_copy<VkDescriptorGetInfoEXT>(const VkDescriptorGetInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);

    // NOTE : Use obj.type to interpret obj.data union
    switch (obj.type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER: {
        result.data.pSampler = create_dynamic_array_copy(1, obj.data.pSampler, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
        result.data.pCombinedImageSampler = create_dynamic_array_copy(1, obj.data.pCombinedImageSampler, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        result.data.pInputAttachmentImage = create_dynamic_array_copy(1, obj.data.pInputAttachmentImage, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
        result.data.pSampledImage = create_dynamic_array_copy(1, obj.data.pSampledImage, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
        result.data.pStorageImage = create_dynamic_array_copy(1, obj.data.pStorageImage, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
        result.data.pUniformTexelBuffer = create_dynamic_array_copy(1, obj.data.pUniformTexelBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        result.data.pStorageTexelBuffer = create_dynamic_array_copy(1, obj.data.pStorageTexelBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
        result.data.pUniformBuffer = create_dynamic_array_copy(1, obj.data.pUniformBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
        result.data.pStorageBuffer = create_dynamic_array_copy(1, obj.data.pStorageBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
        result.data.accelerationStructure = obj.data.accelerationStructure;
    } break;
    default: {
        // NOOP :
    } break;
    }
    return result;
}

template <> void destroy_structure_copy<VkDescriptorGetInfoEXT>(const VkDescriptorGetInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);

    // NOTE : Use obj.type to interpret obj.data union
    switch (obj.type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER: {
        destroy_dynamic_array_copy(1, obj.data.pSampler, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
        destroy_dynamic_array_copy(1, obj.data.pCombinedImageSampler, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
        destroy_dynamic_array_copy(1, obj.data.pInputAttachmentImage, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
        destroy_dynamic_array_copy(1, obj.data.pSampledImage, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
        destroy_dynamic_array_copy(1, obj.data.pStorageImage, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
        destroy_dynamic_array_copy(1, obj.data.pUniformTexelBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        destroy_dynamic_array_copy(1, obj.data.pStorageTexelBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
        destroy_dynamic_array_copy(1, obj.data.pUniformBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
        destroy_dynamic_array_copy(1, obj.data.pStorageBuffer, pAllocator);
    } break;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
        // NOOP : VkDeviceAddress
    } break;
    default: {
        // NOOP :
    } break;
    }
}

template <> VkIndirectCommandsLayoutTokenEXT create_structure_copy<VkIndirectCommandsLayoutTokenEXT>(const VkIndirectCommandsLayoutTokenEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = create_pnext_copy(obj.pNext, pAllocator);

    // NOTE : Use obj.type to interpret obj.data union
    switch (obj.type) {
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT: {
        result.data.pPushConstant = create_dynamic_array_copy(1, obj.data.pPushConstant, pAllocator);
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT: {
        result.data.pVertexBuffer = create_dynamic_array_copy(1, obj.data.pVertexBuffer, pAllocator);
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT: {
        result.data.pIndexBuffer = create_dynamic_array_copy(1, obj.data.pIndexBuffer, pAllocator);
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT: {
        result.data.pExecutionSet = create_dynamic_array_copy(1, obj.data.pExecutionSet, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
    return result;
}

template <> void destroy_structure_copy<VkIndirectCommandsLayoutTokenEXT>(const VkIndirectCommandsLayoutTokenEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);

    // NOTE : Use obj.type to interpret obj.data union
    switch (obj.type) {
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT: {
        destroy_dynamic_array_copy(1, obj.data.pPushConstant, pAllocator);
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT: {
        destroy_dynamic_array_copy(1, obj.data.pVertexBuffer, pAllocator);
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT: {
        destroy_dynamic_array_copy(1, obj.data.pIndexBuffer, pAllocator);
    } break;
    case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT: {
        destroy_dynamic_array_copy(1, obj.data.pExecutionSet, pAllocator);
    } break;
    default: {
        // NOOP :
    } break;
    }
}

template <> VkPerformanceValueINTEL create_structure_copy<VkPerformanceValueINTEL>(const VkPerformanceValueINTEL& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    // NOTE : Use obj.type to interpret obj.data union
    // NOTE : Only VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL needs explicit handling
    if (obj.type == VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL) {
        result.data.valueString = create_dynamic_string_copy(obj.data.valueString, pAllocator);
    }
    return result;
}

template <> void destroy_structure_copy<VkPerformanceValueINTEL>(const VkPerformanceValueINTEL& obj, const VkAllocationCallbacks* pAllocator)
{
    // NOTE : Use obj.type to interpret obj.data union
    // NOTE : Only VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL needs explicit handling
    if (obj.type == VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL) {
        destroy_dynamic_string_copy(obj.data.valueString, pAllocator);
    }
}

} // namespace detail
} // namespace gvk


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

#include "gvk-defines.hpp"
#include "gvk-structures/generated/core-structure-enumerate-handles.hpp"
#include "gvk-structures/detail/handle-enumeration-utilities.hpp"

namespace gvk {
namespace detail {

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkXlibSurfaceCreateInfoKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <>
void enumerate_structure_handles<VkExportFenceWin32HandleInfoKHR>(const VkExportFenceWin32HandleInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkExportMemoryWin32HandleInfoKHR>(const VkExportMemoryWin32HandleInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkExportMemoryWin32HandleInfoNV>(const VkExportMemoryWin32HandleInfoNV& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkExportSemaphoreWin32HandleInfoKHR>(const VkExportSemaphoreWin32HandleInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkImportFenceWin32HandleInfoKHR>(const VkImportFenceWin32HandleInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_handle(obj.fence, callback);
}

template <>
void enumerate_structure_handles<VkImportMemoryWin32HandleInfoKHR>(const VkImportMemoryWin32HandleInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkImportMemoryWin32HandleInfoNV>(const VkImportMemoryWin32HandleInfoNV& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkImportSemaphoreWin32HandleInfoKHR>(const VkImportSemaphoreWin32HandleInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_handle(obj.semaphore, callback);
}
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoBeginCodingInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoCapabilitiesKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoCodingControlInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoDecodeUsageInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeCapabilitiesKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264CapabilitiesEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264FrameSizeEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264NaluSliceInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264ProfileInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264QpEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264RateControlInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264RateControlLayerInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264SessionParametersCreateInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265CapabilitiesEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265FrameSizeEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265NaluSliceSegmentInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265ProfileInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265QpEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265RateControlInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265RateControlLayerInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265SessionParametersCreateInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeH265VclFrameInfoEXT)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEncodeUsageInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoEndCodingInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoFormatPropertiesKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoPictureResourceInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoProfileInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoProfileListInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoReferenceSlotInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoSessionCreateInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VkVideoSessionParametersUpdateInfoKHR)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <>
void enumerate_structure_handles<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_handle(obj.srcAccelerationStructure, callback);
    enumerate_handle(obj.dstAccelerationStructure, callback);
    enumerate_dynamic_structure_array_handles(obj.geometryCount, obj.pGeometries, callback);
    enumerate_dynamic_structure_array_handles(obj.geometryCount, obj.ppGeometries, callback);
}

template <>
void enumerate_structure_handles<VkAccelerationStructureTrianglesDisplacementMicromapNV>(const VkAccelerationStructureTrianglesDisplacementMicromapNV& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_handle(obj.micromap, callback);
}

template <>
void enumerate_structure_handles<VkAccelerationStructureTrianglesOpacityMicromapEXT>(const VkAccelerationStructureTrianglesOpacityMicromapEXT& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_handle(obj.micromap, callback);
}

template <>
void enumerate_structure_handles<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkMicromapBuildInfoEXT>(const VkMicromapBuildInfoEXT& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_handle(obj.dstMicromap, callback);
}

template <>
void enumerate_structure_handles<VkMicromapVersionInfoEXT>(const VkMicromapVersionInfoEXT& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <> void enumerate_structure_handles<VkPipelineCacheCreateInfo>(const VkPipelineCacheCreateInfo& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkPipelineMultisampleStateCreateInfo>(const VkPipelineMultisampleStateCreateInfo& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <> void enumerate_structure_handles<VkSpecializationInfo>(const VkSpecializationInfo& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
    // NOOP : No handles
}

template <>
void enumerate_structure_handles<VkShaderCreateInfoEXT>(const VkShaderCreateInfoEXT& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
    enumerate_dynamic_handle_array(obj.setLayoutCount, obj.pSetLayouts, callback);
}

template <>
void enumerate_structure_handles<VkShaderModuleCreateInfo>(const VkShaderModuleCreateInfo& obj, EnumerateHandlesCallback callback)
{
    enumerate_pnext_handles(obj.pNext, callback);
}

template <>
void enumerate_structure_handles<VkTransformMatrixKHR>(const VkTransformMatrixKHR& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
    // NOOP : No handles
}

////////////////////////////////////////////////////////////////////////////////
// Unions
template <>
void enumerate_structure_handles<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkAccelerationStructureMotionInstanceDataNV>(const VkAccelerationStructureMotionInstanceDataNV& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkClearColorValue>(const VkClearColorValue& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkClearValue>(const VkClearValue& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkDeviceOrHostAddressConstKHR>(const VkDeviceOrHostAddressConstKHR& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkDeviceOrHostAddressKHR>(const VkDeviceOrHostAddressKHR& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkPerformanceCounterResultKHR>(const VkPerformanceCounterResultKHR& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkPerformanceValueDataINTEL>(const VkPerformanceValueDataINTEL& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

template <>
void enumerate_structure_handles<VkPipelineExecutableStatisticValueKHR>(const VkPipelineExecutableStatisticValueKHR& obj, EnumerateHandlesCallback callback)
{
    (void)obj;
    (void)callback;
}

} // namespace detail
} // namespace gvk

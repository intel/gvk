
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
#include "gvk-structures/detail/cerealization-utilities.hpp"

#define GVK_STUB_CEREALIZATION_FUNCTIONS(VK_STRUCTURE_TYPE) \
template <typename ArchiveType> inline void save(ArchiveType&, const VK_STRUCTURE_TYPE&) { } \
template <typename ArchiveType> inline void load(ArchiveType&, VK_STRUCTURE_TYPE&) { }

namespace cereal {

////////////////////////////////////////////////////////////////////////////////
// Linux
#ifdef VK_USE_PLATFORM_XLIB_KHR
GVK_STUB_CEREALIZATION_FUNCTIONS(VkXlibSurfaceCreateInfoKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_CEREALIZATION_FUNCTIONS(SECURITY_ATTRIBUTES)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkExportFenceWin32HandleInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkExportMemoryWin32HandleInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkExportMemoryWin32HandleInfoNV)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkExportSemaphoreWin32HandleInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkImportFenceWin32HandleInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkImportMemoryWin32HandleInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkImportMemoryWin32HandleInfoNV)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkImportSemaphoreWin32HandleInfoKHR)

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkSurfaceFullScreenExclusiveWin32InfoEXT& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    gvk::detail::cerealize_handle(archive, obj.hmonitor);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkSurfaceFullScreenExclusiveWin32InfoEXT& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    obj.hmonitor = gvk::detail::decerealize_handle<HMONITOR>(archive);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkWin32SurfaceCreateInfoKHR& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    archive(obj.flags);
    gvk::detail::cerealize_handle(archive, obj.hinstance);
    gvk::detail::cerealize_handle(archive, obj.hwnd);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkWin32SurfaceCreateInfoKHR& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    archive(obj.flags);
    obj.hinstance = gvk::detail::decerealize_handle<HINSTANCE>(archive);
    obj.hwnd = gvk::detail::decerealize_handle<HWND>(archive);
}
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoBeginCodingInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoCapabilitiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoCodingControlInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeUsageInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeCapabilitiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264CapabilitiesEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264EmitPictureParametersInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264FrameSizeEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264NaluSliceInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264ProfileInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264QpEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264RateControlInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264RateControlLayerInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264ReferenceListsInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264SessionParametersCreateInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265CapabilitiesEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265EmitPictureParametersInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265FrameSizeEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265NaluSliceSegmentInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265ProfileInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265QpEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265RateControlInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265RateControlLayerInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265ReferenceListsInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265SessionParametersCreateInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265VclFrameInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeUsageInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEndCodingInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoFormatPropertiesKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoPictureResourceInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoProfileInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoProfileListInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoReferenceSlotInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoSessionCreateInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoSessionParametersUpdateInfoKHR)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureBuildGeometryInfoKHR& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    archive(obj.type);
    archive(obj.flags);
    archive(obj.mode);
    gvk::detail::cerealize_handle(archive, obj.srcAccelerationStructure);
    gvk::detail::cerealize_handle(archive, obj.dstAccelerationStructure);
    archive(obj.geometryCount);
    gvk::detail::cerealize_dynamic_array(archive, obj.geometryCount, obj.pGeometries);
    gvk::detail::cerealize_dynamic_pointer_array(archive, obj.geometryCount, obj.ppGeometries);
    // NOTE : Not serializing scratchData...this can be revisited if it becomes necessary
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureBuildGeometryInfoKHR& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    archive(obj.type);
    archive(obj.flags);
    archive(obj.mode);
    obj.srcAccelerationStructure = gvk::detail::decerealize_handle<VkAccelerationStructureKHR>(archive);
    obj.dstAccelerationStructure = gvk::detail::decerealize_handle<VkAccelerationStructureKHR>(archive);
    archive(obj.geometryCount);
    obj.pGeometries = gvk::detail::decerealize_dynamic_array<VkAccelerationStructureGeometryKHR>(archive);
    obj.ppGeometries = gvk::detail::decerealize_dynamic_pointer_array<VkAccelerationStructureGeometryKHR>(archive);
    // NOTE : Not serializing scratchData...this can be revisited if it becomes necessary
    obj.scratchData = { };
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureTrianglesOpacityMicromapEXT& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    archive(obj.indexType);
    archive(obj.indexBuffer);
    archive(obj.indexStride);
    archive(obj.baseTriangle);
    archive(obj.usageCountsCount);
    gvk::detail::cerealize_dynamic_array(archive, obj.usageCountsCount, obj.pUsageCounts);
    gvk::detail::cerealize_dynamic_pointer_array(archive, obj.usageCountsCount, obj.ppUsageCounts);
    gvk::detail::cerealize_handle(archive, obj.micromap);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureTrianglesOpacityMicromapEXT& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    archive(obj.indexType);
    archive(obj.indexBuffer);
    archive(obj.indexStride);
    archive(obj.baseTriangle);
    archive(obj.usageCountsCount);
    obj.pUsageCounts = gvk::detail::decerealize_dynamic_array<VkMicromapUsageEXT>(archive);
    obj.ppUsageCounts = gvk::detail::decerealize_dynamic_pointer_array<VkMicromapUsageEXT>(archive);
    obj.micromap = gvk::detail::decerealize_handle<VkMicromapEXT>(archive);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureVersionInfoKHR& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    // NOTE : Not serializing pVersionData...this can be revisited if it becomes necessary
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureVersionInfoKHR& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    // NOTE : Not serializing pVersionData...this can be revisited if it becomes necessary
    obj.pVersionData = nullptr;
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkMicromapBuildInfoEXT& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    archive(obj.type);
    archive(obj.flags);
    archive(obj.mode);
    gvk::detail::cerealize_handle(archive, obj.dstMicromap);
    archive(obj.usageCountsCount);
    gvk::detail::cerealize_dynamic_array(archive, obj.usageCountsCount, obj.pUsageCounts);
    gvk::detail::cerealize_dynamic_pointer_array(archive, obj.usageCountsCount, obj.ppUsageCounts);
    archive(obj.data);
    archive(obj.scratchData);
    archive(obj.triangleArray);
    archive(obj.triangleArrayStride);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkMicromapBuildInfoEXT& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    archive(obj.type);
    archive(obj.flags);
    archive(obj.mode);
    obj.dstMicromap = gvk::detail::decerealize_handle<VkMicromapEXT>(archive);
    archive(obj.usageCountsCount);
    obj.pUsageCounts = gvk::detail::decerealize_dynamic_array<VkMicromapUsageEXT>(archive);
    obj.ppUsageCounts = gvk::detail::decerealize_dynamic_pointer_array<VkMicromapUsageEXT>(archive);
    archive(obj.data);
    archive(obj.scratchData);
    archive(obj.triangleArray);
    archive(obj.triangleArrayStride);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkMicromapVersionInfoEXT& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    // NOTE : Not serializing pVersionData...this can be revisited if it becomes necessary
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkMicromapVersionInfoEXT& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    // NOTE : Not serializing pVersionData...this can be revisited if it becomes necessary
    obj.pVersionData = nullptr;
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkPipelineMultisampleStateCreateInfo& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    archive(obj.flags);
    archive(obj.rasterizationSamples);
    archive(obj.sampleShadingEnable);
    archive(obj.minSampleShading);
    gvk::detail::cerealize_dynamic_array(archive, (obj.rasterizationSamples + 31) / 32, obj.pSampleMask);
    archive(obj.alphaToCoverageEnable);
    archive(obj.alphaToOneEnable);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkPipelineMultisampleStateCreateInfo& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    archive(obj.flags);
    archive(obj.rasterizationSamples);
    archive(obj.sampleShadingEnable);
    archive(obj.minSampleShading);
    obj.pSampleMask = gvk::detail::decerealize_dynamic_array<VkSampleMask>(archive);
    archive(obj.alphaToCoverageEnable);
    archive(obj.alphaToOneEnable);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkShaderModuleCreateInfo& obj)
{
    archive(obj.sType);
    gvk::detail::cerealize_pnext(archive, obj.pNext);
    archive(obj.flags);
    archive(obj.codeSize);
    gvk::detail::cerealize_dynamic_array(archive, obj.codeSize / sizeof(uint32_t), obj.pCode);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkShaderModuleCreateInfo& obj)
{
    archive(obj.sType);
    obj.pNext = gvk::detail::decerealize_pnext(archive);
    archive(obj.flags);
    archive(obj.codeSize);
    obj.pCode = gvk::detail::decerealize_dynamic_array<uint32_t>(archive);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkTransformMatrixKHR& obj)
{
    gvk::detail::cerealize_static_array<12>(archive, obj.matrix);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkTransformMatrixKHR& obj)
{
    gvk::detail::decerealize_static_array<12>(archive, obj.matrix);
}

////////////////////////////////////////////////////////////////////////////////
// Unions
template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureGeometryDataKHR& obj)
{
    archive(((VkBaseInStructure&)obj).sType);
    switch (((VkBaseInStructure&)obj).sType) {
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR: {
        archive(obj.triangles);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR: {
        archive(obj.aabbs);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR: {
        archive(obj.instances);
    } break;
    default: {
    } break;
    }
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureGeometryDataKHR& obj)
{
    VkStructureType sType{ };
    archive(sType);
    switch (sType) {
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR: {
        archive(obj.triangles);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR: {
        archive(obj.aabbs);
    } break;
    case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR: {
        archive(obj.instances);
    } break;
    default: {
    } break;
    }
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureMotionInstanceDataNV& obj)
{
    archive(obj.srtMotionInstance);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureMotionInstanceDataNV& obj)
{
    archive(obj.srtMotionInstance);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkClearColorValue& obj)
{
    gvk::detail::cerealize_static_array<4>(archive, obj.uint32);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkClearColorValue& obj)
{
    gvk::detail::decerealize_static_array<4>(archive, obj.uint32);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkClearValue& obj)
{
    archive(obj.color);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkClearValue& obj)
{
    archive(obj.color);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkDeviceOrHostAddressConstKHR& obj)
{
    archive(obj.deviceAddress);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkDeviceOrHostAddressConstKHR& obj)
{
    archive(obj.deviceAddress);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkDeviceOrHostAddressKHR& obj)
{
    archive(obj.deviceAddress);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkDeviceOrHostAddressKHR& obj)
{
    archive(obj.deviceAddress);
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkPerformanceCounterResultKHR& obj)
{
    archive(obj.uint64);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkPerformanceCounterResultKHR& obj)
{
    archive(obj.uint64);
}

GVK_STUB_CEREALIZATION_FUNCTIONS(VkPerformanceValueDataINTEL)

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkPipelineExecutableStatisticValueKHR& obj)
{
    archive(obj.u64);
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkPipelineExecutableStatisticValueKHR& obj)
{
    archive(obj.u64);
}

////////////////////////////////////////////////////////////////////////////////
// Custom cerealization required
template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureInstanceKHR& obj)
{
    archive(cereal::binary_data(&obj, sizeof(VkAccelerationStructureInstanceKHR)));
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureInstanceKHR& obj)
{
    archive(cereal::binary_data(&obj, sizeof(VkAccelerationStructureInstanceKHR)));
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureMatrixMotionInstanceNV& obj)
{
    archive(cereal::binary_data(&obj, sizeof(VkAccelerationStructureMatrixMotionInstanceNV)));
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureMatrixMotionInstanceNV& obj)
{
    archive(cereal::binary_data(&obj, sizeof(VkAccelerationStructureMatrixMotionInstanceNV)));
}

template <typename ArchiveType>
inline void save(ArchiveType& archive, const VkAccelerationStructureSRTMotionInstanceNV& obj)
{
    archive(cereal::binary_data(&obj, sizeof(VkAccelerationStructureSRTMotionInstanceNV)));
}

template <typename ArchiveType>
inline void load(ArchiveType& archive, VkAccelerationStructureSRTMotionInstanceNV& obj)
{
    archive(cereal::binary_data(&obj, sizeof(VkAccelerationStructureSRTMotionInstanceNV)));
}

} // namespace cereal

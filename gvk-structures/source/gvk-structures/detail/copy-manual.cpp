
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
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkXlibSurfaceCreateInfoKHR)
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
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoBeginCodingInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoCodingControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264SessionParametersAddInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265CapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265DpbSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265PictureInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265ProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265SessionParametersAddInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265SessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeUsageInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeCapabilitiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264CapabilitiesEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264FrameSizeEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264NaluSliceInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264ProfileInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264QpEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264RateControlInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264RateControlLayerInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersCreateInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265CapabilitiesEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265FrameSizeEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265NaluSliceSegmentInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265ProfileInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265QpEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265RateControlInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265RateControlLayerInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersCreateInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265VclFrameInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeRateControlInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeRateControlLayerInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeUsageInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEndCodingInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoFormatPropertiesKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoPictureResourceInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoProfileInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoProfileListInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoReferenceSlotInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionMemoryRequirementsKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionParametersCreateInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoSessionParametersUpdateInfoKHR)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <> VkAccelerationStructureBuildGeometryInfoKHR create_structure_copy<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pGeometries = create_dynamic_array_copy(obj.geometryCount, obj.pGeometries, pAllocator);
    result.ppGeometries = create_dynamic_pointer_array_copy(obj.geometryCount, obj.ppGeometries, pAllocator);
    // NOTE : We're not copying obj.scratchData...this can be revisited if it
    //  becomes necessary.
    result.scratchData = { }; // get_default<VkDeviceOrHostAddressKHR>();
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.geometryCount, obj.pGeometries, pAllocator);
    destroy_dynamic_pointer_array_copy(obj.geometryCount, obj.ppGeometries, pAllocator);
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

template <> VkAccelerationStructureVersionInfoKHR create_structure_copy<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    // NOTE : pVersionData is expected to point to the header of a previously
    //  serialized acceleration structure, so we're just copying the address...this
    //  can be revisited if deep copies become necessary.
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
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

template <> VkMicromapVersionInfoEXT create_structure_copy<VkMicromapVersionInfoEXT>(const VkMicromapVersionInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    // NOTE : pVersionData is expected to point to the header of a previously
    //  serialized micromap, so this comparison just uses the address...this can be
    //  revisited if deep comparisons become necessary.
    return result;
}

template <> void destroy_structure_copy<VkMicromapVersionInfoEXT>(const VkMicromapVersionInfoEXT& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
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

GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkTransformMatrixKHR)

////////////////////////////////////////////////////////////////////////////////
// Unions
template <> VkAccelerationStructureGeometryDataKHR create_structure_copy<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, const VkAllocationCallbacks* pAllocator)
{
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
    } break;
    }
    return result;
}

template <> void destroy_structure_copy<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, const VkAllocationCallbacks* pAllocator)
{
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
    } break;
    }
}

GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkAccelerationStructureMotionInstanceDataNV)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkClearColorValue)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkClearValue)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkDeviceOrHostAddressConstKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkDeviceOrHostAddressKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPerformanceCounterResultKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPerformanceValueDataINTEL)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkPipelineExecutableStatisticValueKHR)

} // namespace detail
} // namespace gvk

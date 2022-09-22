
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

#include "gvk/detail/structure-copy-utilities.hpp"
#include "gvk/generated/create-structure-copy.hpp"
#include "gvk/generated/destroy-structure-copy.hpp"
#include "gvk/defaults.hpp"

namespace gvk {
namespace detail {

#define GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VK_STRUCTURE_TYPE) \
template <> VK_STRUCTURE_TYPE create_structure_copy<VK_STRUCTURE_TYPE>(const VK_STRUCTURE_TYPE& obj, const VkAllocationCallbacks*) { return obj; } \
template <> void destroy_structure_copy<VK_STRUCTURE_TYPE>(const VK_STRUCTURE_TYPE&, const VkAllocationCallbacks*) { }

////////////////////////////////////////////////////////////////////////////////
// Win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkExportFenceWin32HandleInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkExportMemoryWin32HandleInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkExportMemoryWin32HandleInfoNV)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkExportSemaphoreWin32HandleInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkImportFenceWin32HandleInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkImportMemoryWin32HandleInfoKHR)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkImportMemoryWin32HandleInfoNV)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkImportSemaphoreWin32HandleInfoKHR)
#endif // VK_USE_PLATFORM_WIN32_KHR

////////////////////////////////////////////////////////////////////////////////
// Video encode/decode
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264DpbSlotInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264MvcEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264PictureInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH264SessionParametersAddInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265SessionParametersAddInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265DpbSlotInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoDecodeH265PictureInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264NaluSliceEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264ReferenceListsEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265NaluSliceSegmentEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265ReferenceListsEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkVideoEncodeH265VclFrameInfoEXT)

////////////////////////////////////////////////////////////////////////////////
// Special case members
template <>
VkAccelerationStructureBuildGeometryInfoKHR create_structure_copy<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    if (obj.geometryCount) {
        if (obj.pGeometries) {
            result.pGeometries = create_dynamic_array_copy(obj.geometryCount, obj.pGeometries, pAllocator);
        } else if (obj.ppGeometries) {
            auto ppGeometries = create_dynamic_array_copy(obj.geometryCount, obj.ppGeometries, pAllocator);
            result.ppGeometries = ppGeometries;
            for (uint32_t i = 0; i < obj.geometryCount; ++i) {
                ppGeometries[i] = create_dynamic_array_copy(1, obj.ppGeometries[i], pAllocator);
            }
        }
    }
    // NOTE : We're not copying obj.scratchData...this can be revisited if it
    //  becomes necessary.
    result.scratchData = get_default<VkDeviceOrHostAddressKHR>();
    return result;
}

template <>
void destroy_structure_copy<VkAccelerationStructureBuildGeometryInfoKHR>(const VkAccelerationStructureBuildGeometryInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.geometryCount, obj.pGeometries, pAllocator);
    if (obj.geometryCount && obj.ppGeometries) {
        for (uint32_t i = 0; i < obj.geometryCount; ++i) {
            destroy_dynamic_array_copy(1, obj.ppGeometries[i], pAllocator);
        }
    }
    destroy_dynamic_array_copy(obj.geometryCount, obj.ppGeometries, pAllocator);
}

template <>
VkAccelerationStructureVersionInfoKHR create_structure_copy<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    // NOTE : pVersionData is expected to point to the header of a previously
    //  serialized acceleration structure, so we're just copying the address...this
    //  can be revisited if deep copies become necessary.
    return result;
}

template <>
void destroy_structure_copy<VkAccelerationStructureVersionInfoKHR>(const VkAccelerationStructureVersionInfoKHR& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
}

template <>
VkPipelineMultisampleStateCreateInfo create_structure_copy<VkPipelineMultisampleStateCreateInfo>(const VkPipelineMultisampleStateCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pSampleMask = create_dynamic_array_copy((obj.rasterizationSamples + 31) / 32, obj.pSampleMask, pAllocator);
    return result;
}

template <>
void destroy_structure_copy<VkPipelineMultisampleStateCreateInfo>(const VkPipelineMultisampleStateCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy((obj.rasterizationSamples + 31) / 32, obj.pSampleMask, pAllocator);
}

template <>
VkShaderModuleCreateInfo create_structure_copy<VkShaderModuleCreateInfo>(const VkShaderModuleCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pNext = (const void*)create_pnext_copy(obj.pNext, pAllocator);
    result.pCode = create_dynamic_array_copy(obj.codeSize / sizeof(uint32_t), obj.pCode, pAllocator);
    return result;
}

template <>
void destroy_structure_copy<VkShaderModuleCreateInfo>(const VkShaderModuleCreateInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pNext, pAllocator);
    destroy_dynamic_array_copy(obj.codeSize / sizeof(uint32_t), obj.pCode, pAllocator);
}

GVK_STUB_STRUCTURE_COPY_FUNCTIONS(VkTransformMatrixKHR)

////////////////////////////////////////////////////////////////////////////////
// Unions
template <>
VkAccelerationStructureGeometryDataKHR create_structure_copy<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, const VkAllocationCallbacks* pAllocator)
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

template <>
void destroy_structure_copy<VkAccelerationStructureGeometryDataKHR>(const VkAccelerationStructureGeometryDataKHR& obj, const VkAllocationCallbacks* pAllocator)
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


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

#include "gvk/detail/structure-copy-utilities.hpp"
#include "gvk/defines.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/types/common.hpp"

#include <iosfwd>

namespace gvk {
namespace detail {

extern thread_local const VkAllocationCallbacks* tlpDecerealizationAllocator;

template <typename ArchiveType>
void cerealize_pnext(ArchiveType& archive, const void* const& pNext);

template <typename ArchiveType>
void* decerealize_pnext(ArchiveType& archive);

template <typename ArchiveType, typename HandleType>
inline void cerealize_handle(ArchiveType& archive, HandleType handle)
{
    archive((uint64_t)handle);
}

template <typename ArchiveType, typename ObjectType>
inline void cerealize_dynamic_array(ArchiveType& archive, size_t count, const ObjectType* pObjs)
{
    if (count && pObjs) {
        archive(count);
        for (size_t i = 0; i < count; ++i) {
            archive(pObjs[i]);
        }
    } else {
        archive(size_t{ 0 });
    }
}

template <typename ArchiveType, typename HandleType>
inline void cerealize_dynamic_handle_array(ArchiveType& archive, size_t count, const HandleType* pHandles)
{
    if (count && pHandles) {
        archive(count);
        for (size_t i = 0; i < count; ++i) {
            cerealize_handle(archive, pHandles[i]);
        }
    } else {
        archive(size_t{ 0 });
    }
}

template <typename ArchiveType>
inline void cerealize_dynamic_string(ArchiveType& archive, const char* pStr)
{
    if (pStr) {
        size_t strLen = strlen(pStr);
        archive(strLen);
        archive(cereal::binary_data(pStr, strLen));
    } else {
        archive(size_t{ 0 });
    }
}

template <typename ArchiveType>
inline void cerealize_dynamic_string_array(ArchiveType& archive, size_t count, const char* const* ppStrs)
{
    if (count && ppStrs) {
        archive(count);
        for (size_t i = 0; i < count; ++i) {
            cerealize_dynamic_string(archive, ppStrs[i]);
        }
    } else {
        archive(size_t{ 0 });
    }
}

template <size_t Count, typename ArchiveType, typename ObjectType>
inline void cerealize_static_array(ArchiveType& archive, const ObjectType* pObjs)
{
    for (size_t i = 0; i < Count; ++i) {
        archive(pObjs[i]);
    }
}

template <size_t Count, typename ArchiveType, typename HandleType>
inline void cerealize_static_handle_array(ArchiveType& archive, const HandleType* pHandles)
{
    for (size_t i = 0; i < Count; ++i) {
        cerealize_handle(archive, pHandles[i]);
    }
}

template <typename HandleType, typename ArchiveType>
inline HandleType decerealize_handle(ArchiveType& archive)
{
    uint64_t decerealizedHandle = 0;
    archive(decerealizedHandle);
    return (HandleType)decerealizedHandle;
}

template <typename ObjectType, typename ArchiveType>
inline ObjectType* decerealize_dynamic_array(ArchiveType& archive)
{
    ObjectType* pObjs = nullptr;
    size_t count = 0;
    archive(count);
    if (count) {
        assert(detail::tlpDecerealizationAllocator);
        auto pAllocator = detail::tlpDecerealizationAllocator;
        pObjs = (ObjectType*)pAllocator->pfnAllocation(pAllocator->pUserData, count * sizeof(ObjectType), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (size_t i = 0; i < count; ++i) {
            archive(pObjs[i]);
        }
    }
    return pObjs;
}

template <typename HandleType, typename ArchiveType>
inline HandleType* decerealize_dynamic_handle_array(ArchiveType& archive)
{
    HandleType* pHandles = nullptr;
    size_t count = 0;
    archive(count);
    if (count) {
        assert(detail::tlpDecerealizationAllocator);
        auto pAllocator = detail::tlpDecerealizationAllocator;
        pHandles = (HandleType*)pAllocator->pfnAllocation(pAllocator->pUserData, count * sizeof(HandleType), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (size_t i = 0; i < count; ++i) {
            pHandles[i] = decerealize_handle<HandleType>(archive);
        }
    }
    return pHandles;
}

template <typename ArchiveType>
inline char* decerealize_dynamic_string(ArchiveType& archive)
{
    char* pStr = nullptr;
    size_t strLen = 0;
    archive(strLen);
    if (strLen) {
        assert(detail::tlpDecerealizationAllocator);
        auto pAllocator = detail::tlpDecerealizationAllocator;
        pStr = (char*)pAllocator->pfnAllocation(pAllocator->pUserData, strLen + 1, 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        archive(cereal::binary_data(pStr, strLen));
        pStr[strLen] = '\0';
    }
    return pStr;
}

template <typename ArchiveType>
inline char** decerealize_dynamic_string_array(ArchiveType& archive)
{
    char** ppStrs = nullptr;
    size_t count = 0;
    archive(count);
    if (count) {
        assert(detail::tlpDecerealizationAllocator);
        auto pAllocator = detail::tlpDecerealizationAllocator;
        ppStrs = (char**)pAllocator->pfnAllocation(pAllocator->pUserData, count * sizeof(char*), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (size_t i = 0; i < count; ++i) {
            ppStrs[i] = decerealize_dynamic_string(archive);
        }
    }
    return ppStrs;
}

template <size_t Count, typename ArchiveType, typename ObjectType>
inline void decerealize_static_array(ArchiveType& archive, ObjectType* pObjs)
{
    for (size_t i = 0; i < Count; ++i) {
        archive(pObjs[i]);
    }
}

template <size_t Count, typename ArchiveType, typename HandleType>
inline void decerealize_static_handle_array(ArchiveType& archive, HandleType* pHandles)
{
    for (size_t i = 0; i < Count; ++i) {
        pHandles[i] = decerealize_handle<HandleType>(archive);
    }
}

} // namespace detail
} // namespace gvk

#define GVK_STUB_CEREALIZATION_FUNCTIONS(VK_STRUCTURE_TYPE) \
template <typename ArchiveType> inline void save(ArchiveType&, const VK_STRUCTURE_TYPE&) { } \
template <typename ArchiveType> inline void load(ArchiveType&, VK_STRUCTURE_TYPE&) { }

namespace cereal {

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
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264DpbSlotInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264MvcEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264PictureInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH264SessionParametersAddInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265SessionParametersAddInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265DpbSlotInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoDecodeH265PictureInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264NaluSliceEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264ReferenceListsEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265NaluSliceSegmentEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265ReferenceListsEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_CEREALIZATION_FUNCTIONS(VkVideoEncodeH265VclFrameInfoEXT)

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
    if (obj.ppGeometries) {
        archive(true);
        for (uint32_t i = 0; i < obj.geometryCount; ++i) {
            gvk::detail::cerealize_dynamic_array(archive, 1, obj.ppGeometries[i]);
        }
    } else {
        archive(false);
    }
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
    bool serialized_ppGeometries = false;
    archive(serialized_ppGeometries);
    if (obj.geometryCount && serialized_ppGeometries) {
        assert(gvk::detail::tlpDecerealizationAllocator);
        auto pAllocator = gvk::detail::tlpDecerealizationAllocator;
        auto size = obj.geometryCount * sizeof(VkAccelerationStructureGeometryKHR*);
        auto ppGeometries = (VkAccelerationStructureGeometryKHR**)pAllocator->pfnAllocation(pAllocator->pUserData, size, 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        obj.ppGeometries = ppGeometries;
        for (uint32_t i = 0; i < obj.geometryCount; ++i) {
            ppGeometries[i] = gvk::detail::decerealize_dynamic_array<VkAccelerationStructureGeometryKHR>(archive);
        }
    }
    // NOTE : Not serializing scratchData...this can be revisited if it becomes necessary
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

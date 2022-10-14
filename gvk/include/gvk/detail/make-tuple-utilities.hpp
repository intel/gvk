
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

#include "gvk/defines.hpp"

#include <algorithm>
#include <tuple>

namespace gvk {
namespace detail {

template <typename T>
struct ArrayTupleElementWrapper final
{
    inline ArrayTupleElementWrapper(size_t countArg, const T* ptrArg)
        : count { countArg }
        , ptr { ptrArg }
    {
    }

    inline const T* begin() const
    {
        return count && ptr ? ptr : nullptr;
    }

    inline const T* end() const
    {
        return count && ptr ? ptr + count : nullptr;
    }

    size_t count { };
    const T* ptr { };
};

template <typename T>
inline bool operator==(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator!=(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator>(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const ArrayTupleElementWrapper<T>& lhs, const ArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs < rhs);
}

template <typename T>
struct PointerArrayTupleElementWrapper final
{
    inline PointerArrayTupleElementWrapper(size_t countArg, const T* ptrArg)
        : count { countArg }
        , ptr { ptrArg }
    {
    }
    
    inline const T* begin() const
    {
        return count && ptr ? ptr : nullptr;
    }

    inline const T* end() const
    {
        return count && ptr ? ptr + count : nullptr;
    }

    size_t count{ };
    const T* ptr{ };
};

template <typename T>
inline bool operator==(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return std::equal(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](auto lhsPtr, auto rhsPtr)
        {
            return ArrayTupleElementWrapper{ 1, lhsPtr } == ArrayTupleElementWrapper{ 1, rhsPtr };
        }
    );
}

template <typename T>
inline bool operator!=(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](auto lhsPtr, auto rhsPtr)
        {
            return ArrayTupleElementWrapper{ 1, lhsPtr } < ArrayTupleElementWrapper{ 1, rhsPtr };
        }
    );
}

template <typename T>
inline bool operator>(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const PointerArrayTupleElementWrapper<T>& lhs, const PointerArrayTupleElementWrapper<T>& rhs)
{
    return !(lhs < rhs);
}

struct PNextTupleElementWrapper final
{
    const void* pNext { nullptr };
};

bool operator==(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator!=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator<(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator>(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator<=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);
bool operator>=(const PNextTupleElementWrapper& lhs, const PNextTupleElementWrapper& rhs);

struct StringTupleElementWrapper final
{
    const char* pStr { nullptr };
};

bool operator==(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator!=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator<(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator>(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator<=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);
bool operator>=(const StringTupleElementWrapper& lhs, const StringTupleElementWrapper& rhs);

struct StringArrayTupleElementWrapper final
{
    size_t count { };
    const char* const* ppStrs { nullptr };
};

bool operator==(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator!=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator<(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator>(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator<=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);
bool operator>=(const StringArrayTupleElementWrapper& lhs, const StringArrayTupleElementWrapper& rhs);

} // namespace detail

#define GVK_STUB_MAKE_TUPLE_DEFINITION(VK_STRUCTURE_TYPE) \
inline auto make_tuple(const VK_STRUCTURE_TYPE&) { return std::make_tuple(0); }

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
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264DpbSlotInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264MvcEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264PictureInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH264SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265DpbSlotInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoDecodeH265PictureInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264DpbSlotInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264NaluSliceEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264ReferenceListsEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH264VclFrameInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265DpbSlotInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265NaluSliceSegmentEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265ReferenceListsEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265SessionParametersAddInfoEXT)
GVK_STUB_MAKE_TUPLE_DEFINITION(VkVideoEncodeH265VclFrameInfoEXT)

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
        detail::ArrayTupleElementWrapper((size_t)obj.geometryCount, obj.pGeometries),
        detail::PointerArrayTupleElementWrapper((size_t)obj.geometryCount, obj.ppGeometries)
        // NOTE : We're ignoring scratchData for comparisons...this can be revisited if
        //  it becomes necessary to differentiate objects by scratchData...
        // obj.scratchData
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

inline auto make_tuple(const VkPipelineMultisampleStateCreateInfo& obj)
{
    return std::make_tuple(
        obj.sType,
        detail::PNextTupleElementWrapper{ obj.pNext },
        obj.flags,
        obj.rasterizationSamples,
        obj.sampleShadingEnable,
        obj.minSampleShading,
        detail::ArrayTupleElementWrapper(((size_t)obj.rasterizationSamples + 31) / 32, obj.pSampleMask),
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
        detail::ArrayTupleElementWrapper(obj.codeSize / sizeof(uint32_t), obj.pCode)
    );
}

inline auto make_tuple(const VkTransformMatrixKHR& obj)
{
    return std::make_tuple(
        detail::ArrayTupleElementWrapper(12, (const float*)obj.matrix)
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
        obj.uint32[2]
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

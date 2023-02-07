
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
#include "gvk-structures/detail/copy-utilities.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/types/common.hpp"

#include <iosfwd>
#include <type_traits>

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

template <typename ArchiveType, typename ObjectType>
inline void cerealize_dynamic_pointer_array(ArchiveType& archive, size_t count, const ObjectType* const* ppObjs)
{
    if (count && ppObjs) {
        archive(count);
        for (size_t i = 0; i < count; ++i) {
            cerealize_dynamic_array(archive, 1, ppObjs[i]);
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
        assert(tlpDecerealizationAllocator);
        auto pAllocator = tlpDecerealizationAllocator;
        pObjs = (ObjectType*)pAllocator->pfnAllocation(pAllocator->pUserData, count * sizeof(ObjectType), 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (size_t i = 0; i < count; ++i) {
            archive(pObjs[i]);
        }
    }
    return pObjs;
}

template <typename ObjectType, typename ArchiveType>
inline ObjectType** decerealize_dynamic_pointer_array(ArchiveType& archive)
{
    ObjectType** ppObjs = nullptr;
    size_t count = 0;
    archive(count);
    if (count) {
        assert(tlpDecerealizationAllocator);
        auto pAllocator = tlpDecerealizationAllocator;
        auto size = count * sizeof(ObjectType*);
        ppObjs = (ObjectType**)pAllocator->pfnAllocation(pAllocator->pUserData, size, 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (uint32_t i = 0; i < count; ++i) {
            ppObjs[i] = decerealize_dynamic_array<ObjectType>(archive);
        }
    }
    return ppObjs;
}

template <typename HandleType, typename ArchiveType>
inline HandleType* decerealize_dynamic_handle_array(ArchiveType& archive)
{
    HandleType* pHandles = nullptr;
    size_t count = 0;
    archive(count);
    if (count) {
        assert(tlpDecerealizationAllocator);
        auto pAllocator = tlpDecerealizationAllocator;
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
        assert(tlpDecerealizationAllocator);
        auto pAllocator = tlpDecerealizationAllocator;
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
        assert(tlpDecerealizationAllocator);
        auto pAllocator = tlpDecerealizationAllocator;
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

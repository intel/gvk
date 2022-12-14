
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

#include <cassert>
#include <cstdlib>

namespace gvk {
namespace detail {

template <typename ObjectType>
inline ObjectType create_structure_copy(const ObjectType& obj, const VkAllocationCallbacks*)
{
    return obj;
}

template <typename ObjectType>
inline void destroy_structure_copy(const ObjectType&, const VkAllocationCallbacks*)
{
}

inline const VkAllocationCallbacks* validate_allocation_callbacks(const VkAllocationCallbacks* pAllocator)
{
    static const VkAllocationCallbacks sAllocator{
        nullptr,
        [](void*, size_t size, size_t, VkSystemAllocationScope) { return malloc(size); },
        nullptr,
        [](void*, void* pMemory) { free(pMemory); },
        nullptr,
        nullptr,
    };
    return (pAllocator && pAllocator->pfnAllocation && pAllocator->pfnFree) ? pAllocator : &sAllocator;
}

template <typename CountType, typename ObjectType>
inline ObjectType* create_dynamic_array_copy(CountType objCount, const ObjectType* pObjs, const VkAllocationCallbacks* pAllocator)
{
    ObjectType* pResult = nullptr;
    if (objCount && pObjs) {
        pAllocator = validate_allocation_callbacks(pAllocator);
        pResult = (ObjectType*)pAllocator->pfnAllocation(pAllocator->pUserData, sizeof(ObjectType) * objCount, 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (CountType i = 0; i < objCount; ++i) {
            pResult[i] = create_structure_copy(pObjs[i], pAllocator);
        }
    }
    return pResult;
}

inline char* create_dynamic_string_copy(const char* pStr, const VkAllocationCallbacks* pAllocator)
{
    char* pResult = nullptr;
    if (pStr) {
        pAllocator = validate_allocation_callbacks(pAllocator);
        auto size = strlen(pStr) + 1;
        pResult = (char*)pAllocator->pfnAllocation(pAllocator->pUserData, sizeof(char) * size, 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        memcpy(pResult, pStr, size);
    }
    return pResult;
}

template <typename CountType>
inline char** create_dynamic_string_array_copy(CountType strCount, const char* const* ppStrs, const VkAllocationCallbacks* pAllocator)
{
    char** ppResult = nullptr;
    if (strCount && ppStrs) {
        pAllocator = validate_allocation_callbacks(pAllocator);
        ppResult = (char**)pAllocator->pfnAllocation(pAllocator->pUserData, sizeof(char*) * strCount, 0, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        for (CountType i = 0; i < strCount; ++i) {
            ppResult[i] = create_dynamic_string_copy(ppStrs[i], pAllocator);
        }
    }
    return ppResult;
}

template <size_t Count, typename ObjectType>
inline void create_static_array_copy(ObjectType* pDst, const ObjectType* pSrc, const VkAllocationCallbacks* pAllocator)
{
    assert(Count);
    assert(pDst);
    assert(pSrc);
    for (size_t i = 0; i < Count; ++i) {
        pDst[i] = create_structure_copy(pSrc[i], pAllocator);
    }
}

template <size_t Size>
inline void create_static_string_copy(char* pDstStr, const char* pSrcStr)
{
    assert(Size);
    assert(pDstStr);
    assert(pSrcStr);
    for (size_t i = 0; i < Size && pSrcStr[i]; ++i) {
        pDstStr[i] = pSrcStr[i];
    }
}

template <typename CountType, typename ObjectType>
inline void destroy_dynamic_array_copy(CountType objCount, const ObjectType* pObjs, const VkAllocationCallbacks* pAllocator)
{
    if (objCount && pObjs) {
        pAllocator = validate_allocation_callbacks(pAllocator);
        for (CountType i = 0; i < objCount; ++i) {
            destroy_structure_copy(pObjs[i], pAllocator);
        }
        pAllocator->pfnFree(pAllocator->pUserData, (void*)pObjs);
    }
}

inline void destroy_dynamic_string_copy(const char* pStr, const VkAllocationCallbacks* pAllocator)
{
    if (pStr) {
        pAllocator = validate_allocation_callbacks(pAllocator);
        pAllocator->pfnFree(pAllocator->pUserData, (void*)pStr);
    }
}

template <typename CountType>
inline void destroy_dynamic_string_array_copy(CountType strCount, const char* const* ppStrs, const VkAllocationCallbacks* pAllocator)
{
    if (strCount && ppStrs) {
        pAllocator = validate_allocation_callbacks(pAllocator);
        for (CountType i = 0; i < strCount; ++i) {
            destroy_dynamic_string_copy(ppStrs[i], pAllocator);
        }
        pAllocator->pfnFree(pAllocator->pUserData, (void*)ppStrs);
    }
}

template <size_t Count, typename ObjectType>
inline void destroy_static_array_copy(const ObjectType* pObjs, const VkAllocationCallbacks* pAllocator)
{
    assert(Count);
    assert(pObjs);
    pAllocator = validate_allocation_callbacks(pAllocator);
    for (size_t i = 0; i < Count; ++i) {
        destroy_structure_copy(pObjs[i], pAllocator);
    }
}

} // namespace detail
} // namespace gvk


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
#include "gvk-structures/get-object-type.hpp"

#include <cassert>
#include <functional>

#define GVK_STUB_ENUMERATE_STRUCTURE_HANDLES_DEFINITION(VK_STRUCTURE_TYPE)                                                      \
template <> void enumerate_structure_handles<VK_STRUCTURE_TYPE>(const VK_STRUCTURE_TYPE&, EnumerateHandlesCallback)             \
{                                                                                                                               \
    assert(false && "gvk::detail::enumerate_structure_handles<" #VK_STRUCTURE_TYPE ">() unserviced; gvk maintenance required"); \
}

namespace gvk {
namespace detail {

using EnumerateHandlesCallback = std::function<void(VkObjectType, const uint64_t&)>;

template <typename StructureType>
inline void enumerate_structure_handles(const StructureType& structure, EnumerateHandlesCallback callback)
{
    (void)structure;
    (void)callback;
}

void enumerate_pnext_handles(const void* pNext, EnumerateHandlesCallback callback);

template <typename VkHandleType>
inline void enumerate_handle(const VkHandleType& handle, EnumerateHandlesCallback callback)
{
    auto& typelessHandle = (const uint64_t&)handle;
    callback(get_object_type<VkHandleType>(), typelessHandle);
}

template <typename CountType, typename VkHandleType>
inline void enumerate_dynamic_handle_array(CountType handleCount, const VkHandleType* pHandles, EnumerateHandlesCallback callback)
{
    if (handleCount && pHandles) {
        for (CountType i = 0; i < handleCount; ++i) {
            enumerate_handle(pHandles[i], callback);
        }
    }
}

template <size_t Count, typename VkHandleType>
inline void enumerate_static_handle_array(const VkHandleType* pHandles, EnumerateHandlesCallback callback)
{
    if (pHandles) {
        for (size_t i = 0; i < Count; ++i) {
            enumerate_handle(pHandles[i], callback);
        }
    }
}

template <typename CountType, typename ObjectType>
inline void enumerate_dynamic_structure_array_handles(CountType objCount, const ObjectType* pObjs, EnumerateHandlesCallback callback)
{
    if (objCount && pObjs) {
        for (CountType i = 0; i < objCount; ++i) {
            enumerate_structure_handles(const_cast<ObjectType*>(pObjs)[i], callback);
        }
    }
}

template <size_t Count, typename ObjectType>
inline void enumerate_static_structure_array_handles(const ObjectType* pObjs, EnumerateHandlesCallback callback)
{
    if (pObjs) {
        for (size_t i = 0; i < Count; ++i) {
            enumerate_structure_handles(pObjs[i], callback);
        }
    }
}

} // namespace detail
} // namespace gvk

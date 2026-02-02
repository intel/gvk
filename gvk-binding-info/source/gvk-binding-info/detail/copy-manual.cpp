
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

#include "gvk-binding-info/generated/binding-info-structure-create-copy.hpp"
#include "gvk-structures.hpp"
#include "gvk-structures/detail/get-count.hpp"

namespace gvk {
namespace detail {

template <> GvkResourceInfo create_structure_copy<GvkResourceInfo>(const GvkResourceInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    auto result = obj;
    result.pCreateInfo = (VkBaseOutStructure*)create_pnext_copy(obj.pCreateInfo, pAllocator);
    return result;
}

template <> void destroy_structure_copy<GvkResourceInfo>(const GvkResourceInfo& obj, const VkAllocationCallbacks* pAllocator)
{
    destroy_pnext_copy(obj.pCreateInfo, pAllocator);
}

} // namespace detail
} // namespace gvk

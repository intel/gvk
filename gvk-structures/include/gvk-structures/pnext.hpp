
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
#include "gvk-structures/get-stype.hpp"

#include <set>

namespace gvk {

template <typename PNextStructureType, typename BaseStructureType>
const PNextStructureType* get_pnext(const BaseStructureType& base)
{
    auto pNext = (const VkBaseInStructure*)base.pNext;
    while (pNext) {
        if (pNext->sType == get_stype<PNextStructureType>()) {
            return (const PNextStructureType*)pNext;
        }
        pNext = pNext->pNext;
    }
    return nullptr;
}

namespace detail {

template <typename PNextStructureType>
inline const PNextStructureType* get_pnext_structure(const void* pNext)
{
    auto pBaseInStructure = (const VkBaseInStructure*)pNext;
    while (pBaseInStructure) {
        if (pBaseInStructure->sType == get_stype<PNextStructureType>()) {
            return (const PNextStructureType*)pBaseInStructure;
        }
        pNext = pBaseInStructure->pNext;
    }
    return nullptr;
}

inline const void* remove_pnext_entries(VkBaseOutStructure* pNext, const std::set<VkStructureType>& structureTypes)
{
    while (pNext) {
        while (pNext->pNext && structureTypes.count(pNext->pNext->sType)) {
            auto pRemove = pNext->pNext;
            pNext->pNext = pRemove->pNext;
            pRemove->pNext = nullptr;
            detail::destroy_pnext_copy(pRemove, nullptr);
        }
        pNext = pNext->pNext;
    }
    return pNext;
}

inline const void* remove_pnext_entries(VkBaseOutStructure* pNext, VkStructureType structureType)
{
    return remove_pnext_entries(pNext, std::set<VkStructureType> { structureType });
}

} // namespace detail
} // namespace gvk

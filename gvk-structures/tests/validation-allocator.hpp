
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

#define _CRT_SECURE_NO_WARNINGS

#include "gvk-defines.hpp"

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#undef Bool
#endif
#include "gtest/gtest.h"

namespace gvk {
namespace validation {

class Allocator
{
public:
    inline Allocator()
    {
        mAllocationCallbacks.pUserData = this;
        mAllocationCallbacks.pfnAllocation = validate_allocation;
        mAllocationCallbacks.pfnFree = validate_free;
    }

    inline ~Allocator()
    {
        if (!mAllocations.empty()) {
            ADD_FAILURE();
        }
    }

    inline const VkAllocationCallbacks& get_allocation_callbacks() const
    {
        return mAllocationCallbacks;
    }

    inline static void* validate_allocation(void* pUserData, size_t size, size_t, VkSystemAllocationScope)
    {
        auto pMemory = malloc(size);
        ((Allocator*)pUserData)->mAllocations.insert(pMemory);
        return pMemory;
    }

    inline static void validate_free(void* pUserData, void* pMemory)
    {
        ((Allocator*)pUserData)->mAllocations.erase(pMemory);
        free(pMemory);
    }

private:
    VkAllocationCallbacks mAllocationCallbacks{ };
    std::set<void*> mAllocations;

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
};

} // namespace validation
} // namespace gvk

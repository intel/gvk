
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

#include "gvk-defines.hpp"

namespace gvk {

PFN_result_scope_callback gPfnGvkResultScopeCallback;
thread_local PFN_result_scope_callback tlPfnGvkResultScopeCallback;

const VkAllocationCallbacks* validate_allocator(const VkAllocationCallbacks& allocator)
{
    return (allocator.pfnAllocation && allocator.pfnFree) ? &allocator : nullptr;
}

namespace detail {

VkBool32 process_result_scope_failure(VkResult gvkResult, const char* pFileLine, const char* pGvkCall)
{
    if (gvk::tlPfnGvkResultScopeCallback) {
        return gvk::tlPfnGvkResultScopeCallback(gvkResult, pFileLine, pGvkCall);
    } else if (gvk::gPfnGvkResultScopeCallback) {
        return gvk::gPfnGvkResultScopeCallback(gvkResult, pFileLine, pGvkCall);
    }
    return VK_FALSE;
}

} // namespace detail
} // namespace gvk

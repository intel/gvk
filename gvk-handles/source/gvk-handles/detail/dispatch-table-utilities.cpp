
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

#include "gvk/detail/dispatch-table-utilities.hpp"

namespace gvk {
namespace detail {

static void* sVulkanRuntime;

VkResult load_runtime()
{
    #ifdef __linux__
    constexpr const char* const VulkanRuntimeLibraryName = "libvulkan.so.1";
    #endif
    #ifdef VK_USE_PLATFORM_WIN32_KHR
    constexpr const char* const VulkanRuntimeLibraryName = "vulkan-1.dll";
    #endif
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen(VulkanRuntimeLibraryName);
    }
    return sVulkanRuntime ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT;
}

void unload_runtime()
{
    if (sVulkanRuntime) {
        gvk_dlclose(sVulkanRuntime);
        sVulkanRuntime = NULL;
    }
}

PFN_vkGetInstanceProcAddr load_get_instance_proc_addr()
{
    return load_runtime() == VK_SUCCESS ? (PFN_vkGetInstanceProcAddr)gvk_dlsym(sVulkanRuntime, "vkGetInstanceProcAddr") : nullptr;
}

} // namespace detail
} // namespace gvk


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

#include "gvk-runtime.hpp"

namespace gvk {

static void* sVulkanRuntime;

VkResult load_vulkan_runtime()
{
#ifdef GVK_PLATFORM_LINUX
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen("libvulkan.so.1");
    }
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen("libvulkan.so");
    }
#endif
#ifdef GVK_PLATFORM_WINDOWS
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen("vulkan-1.dll");
    }
#endif
    return sVulkanRuntime ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT;
}

void unload_vulkan_runtime()
{
    if (sVulkanRuntime) {
        gvk_dlclose(sVulkanRuntime);
        sVulkanRuntime = NULL;
    }
}

PFN_vkGetInstanceProcAddr load_vkGetInstanceProcAddr()
{
    return (load_vulkan_runtime() == VK_SUCCESS) ? (PFN_vkGetInstanceProcAddr)gvk_dlsym(sVulkanRuntime, "vkGetInstanceProcAddr") : nullptr;
}

#ifdef GVK_PLATFORM_WINDOWS
BOOL get_this_module_handle(HMODULE* phModule)
{
    gvk_assert(phModule);
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&get_this_module_handle, phModule);
}

DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath)
{
    gvk_assert(pPath);
    const size_t CharBufferSize = 16384;
    std::vector<wchar_t> wcharBuffer(CharBufferSize);
    auto result = GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    if (result && wcharBuffer[0]) {
        *pPath = wcharBuffer.data();
    }
    return result;
}

DWORD get_this_module_path(std::filesystem::path* pPath)
{
    HMODULE hModule = NULL;
    return get_this_module_handle(&hModule) ? get_module_path(hModule, pPath) : 0;
}

std::string get_win32_error_str(DWORD errorCode)
{
    std::string errorStr = "Win32 [" + std::to_string(errorCode) + "]";
    LPSTR pErrorStr = NULL;
    auto dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    if (FormatMessage(dwFlags, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pErrorStr, 0, NULL)) {
        errorStr += " " + std::string(pErrorStr);
    }
    LocalFree(pErrorStr);
    return errorStr;
}
#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk

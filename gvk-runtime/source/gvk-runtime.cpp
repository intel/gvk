
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

#ifdef GVK_PLATFORM_WINDOWS
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif

#include <array>

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

const std::vector<std::string>& get_validation_layer_settings()
{
    // TODO : Settings should be parsed from layer JSON for all layers

    // FROM : https://vulkan.lunarg.com/doc/view/1.4.304.0/windows/khronos_validation_layer.html
    static const std::vector<std::string> scLayerSettings{
        /*
        Name                                                                  Type        Default Value
        */
        "VK_LAYER_FINE_GRAINED_LOCKING",                                   // BOOL      : true
        "VK_KHRONOS_VALIDATION_VALIDATE_CORE",                             // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_IMAGE_LAYOUT",                        // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_COMMAND_BUFFER",                      // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_OBJECT_IN_USE",                       // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_QUERY",                               // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_SHADERS",                             // BOOL      : true
        "VK_KHRONOS_VALIDATION_CHECK_SHADERS_CACHING",                     // BOOL      : true
        "VK_KHRONOS_VALIDATION_DEBUG_DISABLE_SPIRV_VAL",                   // BOOL      : false
        "VK_KHRONOS_VALIDATION_UNIQUE_HANDLES",                            // BOOL      : true
        "VK_KHRONOS_VALIDATION_OBJECT_LIFETIME",                           // BOOL      : true
        "VK_KHRONOS_VALIDATION_STATELESS_PARAM",                           // BOOL      : true
        "VK_KHRONOS_VALIDATION_THREAD_SAFETY",                             // BOOL      : true
        "VK_KHRONOS_VALIDATION_VALIDATE_SYNC",                             // BOOL      : false
        "VK_KHRONOS_VALIDATION_SYNCVAL_SUBMIT_TIME_VALIDATION",            // BOOL      : true
        "VK_KHRONOS_VALIDATION_SYNCVAL_SHADER_ACCESSES_HEURISTIC",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_SYNCVAL_MESSAGE_EXTRA_PROPERTIES",          // BOOL      : false
        "VK_KHRONOS_VALIDATION_PRINTF_ENABLE",                             // BOOL      : false
        "VK_KHRONOS_VALIDATION_PRINTF_TO_STDOUT",                          // BOOL      : true
        "VK_KHRONOS_VALIDATION_PRINTF_VERBOSE",                            // BOOL      : false
        // "VK_KHRONOS_VALIDATION_PRINTF_BUFFER_SIZE",                     // INT       : 1024
        "VK_KHRONOS_VALIDATION_GPUAV_ENABLE",                              // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_SHADER_INSTRUMENTATION",              // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_DESCRIPTOR_CHECKS",                   // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_WARN_ON_ROBUST_OOB",                  // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_BUFFER_ADDRESS_OOB",                  // BOOL      : true
        // "VK_KHRONOS_VALIDATION_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES",      // INT       : 10000
        "VK_KHRONOS_VALIDATION_GPUAV_VALIDATE_RAY_QUERY",                  // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING",    // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_SELECT_INSTRUMENTED_SHADERS",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_BUFFERS_VALIDATION",                  // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_INDIRECT_DRAWS_BUFFERS",              // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_INDIRECT_DISPATCHES_BUFFERS",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS",         // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_BUFFER_COPIES",                       // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_INDEX_BUFFERS",                       // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_RESERVE_BINDING_SLOT",                // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_VMA_LINEAR_OUTPUT",                   // BOOL      : true
        "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS", // BOOL      : false
        "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS",     // BOOL      : false
        // "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_MAX_INSTRUMENTATIONS_COUNT", // INT       : 0
        "VK_KHRONOS_VALIDATION_GPUAV_DEBUG_PRINT_INSTRUMENTATION_INFO",    // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES",                   // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_ARM",               // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_AMD",               // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_IMG",               // BOOL      : false
        "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_NVIDIA",            // BOOL      : false
        // "VK_KHRONOS_VALIDATION_DEBUG_ACTION",                           // FLAGS     : VK_DBG_LAYER_ACTION_LOG_MSG
        // "VK_KHRONOS_VALIDATION_LOG_FILENAME",                           // SAVE_FILE : stdout
        // "VK_KHRONOS_VALIDATION_REPORT_FLAGS",                           // FLAGS     : error
        "VK_KHRONOS_VALIDATION_ENABLE_MESSAGE_LIMIT",                      // BOOL      : true
        // "VK_LAYER_DUPLICATE_MESSAGE_LIMIT",                             // INT       : 10
        // "VK_LAYER_MESSAGE_ID_FILTER",                                   // LIST      :
        "VK_KHRONOS_VALIDATION_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME",   // BOOL      : false
    };
    return scLayerSettings;
}

uint64_t get_thread_id()
{
#if defined(_WIN32)
    return (uint64_t)GetCurrentThreadId();
#else
    return (uint64_t)gettid();
#endif
}

#ifdef GVK_PLATFORM_WINDOWS

BOOL is_console(HANDLE handle)
{
    DWORD mode{ };
    return GetFileType(handle) == FILE_TYPE_CHAR && GetConsoleMode(handle, &mode);
}

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

DWORD get_process_path(HANDLE hProcess, std::filesystem::path* pPath)
{
    gvk_assert(pPath);
    const size_t CharBufferSize = 16384;
    std::vector<wchar_t> wcharBuffer(CharBufferSize);
    auto result = GetProcessImageFileNameW(hProcess, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    if (result && wcharBuffer[0]) {
        *pPath = wcharBuffer.data();
    }
    return result;
}

DWORD get_this_process_path(std::filesystem::path* pPath)
{
    return get_process_path(GetCurrentProcess(), pPath);
}

DWORD get_process_name(HANDLE hProcess, std::filesystem::path* pPath)
{
    gvk_assert(pPath);
    auto result = get_process_path(hProcess, pPath);
    if (result && pPath->has_filename()) {
        *pPath = pPath->filename();
    }
    return result;
}

DWORD get_this_process_name(std::filesystem::path* pPath)
{
    return get_process_name(GetCurrentProcess(), pPath);
}

BOOL is_process_elevated(HANDLE hProcess)
{
    BOOL isElevated = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(hProcess ? hProcess : GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation{ };
        DWORD dwSize = 0;
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
            isElevated = elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return isElevated;
}

BOOL register_explicit_layer(const std::filesystem::path& layerJsonPath)
{
    // TODO : gvk_result_scope should be setup to work with arbitrary return codes so
    //  it can be used to simplify error handling in general and report out detailed
    //  information from Windows API calls (and any other API we interact with)
    try {
        auto normalizedPath = std::filesystem::weakly_canonical(layerJsonPath);

        // Validate layer JSON path
        auto status = normalizedPath.is_absolute() ? ERROR_SUCCESS : ERROR_PATH_NOT_FOUND;
        if (status == ERROR_SUCCESS) {
            status = std::filesystem::exists(normalizedPath) ? ERROR_SUCCESS : ERROR_FILE_NOT_FOUND;

            if (status == ERROR_SUCCESS) {
                // Open (or create) the Vulkan explicit layers registry key
                HKEY hKey = NULL;
                status = RegCreateKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Khronos\\Vulkan\\ExplicitLayers",
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_SET_VALUE,
                    NULL,
                    &hKey,
                    NULL
                );

                if (status == ERROR_SUCCESS) {
                    // Set the layer manifest path as a registry value with data = 0
                    DWORD data = 0;
                    status = RegSetValueExW(hKey, normalizedPath.wstring().c_str(), 0, REG_DWORD, (const BYTE*)&data, sizeof(data));
                    RegCloseKey(hKey);
                }
            }
        }
        return status == ERROR_SUCCESS;
    } catch (...) {
        return FALSE;
    }
}

void unregister_explicit_layer(const std::filesystem::path& layerName)
{
    // Open the Vulkan explicit layers registry key
    HKEY hKey = NULL;
    auto status = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Khronos\\Vulkan\\ExplicitLayers",
        0,
        KEY_SET_VALUE | KEY_QUERY_VALUE,
        &hKey
    );
    if (status == ERROR_SUCCESS) {

        // Get the length of the longest value under the key, sans null terminator
        DWORD maxValueNameLength = 0;
        status = RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &maxValueNameLength, NULL, NULL, NULL);
        ++maxValueNameLength; // Account for null terminator

        // Enumerate registry values and remove any matching the given layer name
        DWORD index = 0;
        while (status == ERROR_SUCCESS) {
            auto valueNameLength = maxValueNameLength;
            std::wstring valueName(valueNameLength, L'\0');
            status = RegEnumValueW(hKey, index, valueName.data(), &valueNameLength, NULL, NULL, NULL, NULL);
            valueName.resize(valueNameLength); // Remove excess null terminators
            if (status == ERROR_SUCCESS && std::filesystem::path(valueName).stem() == layerName.stem()) {
                status = RegDeleteValueW(hKey, valueName.c_str());
                // Don't increment index since the current item was removed
                continue;
            }
            ++index;
        }
        RegCloseKey(hKey);
    }
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

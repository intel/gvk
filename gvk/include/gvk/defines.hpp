
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

/**
NOTE : gvk_dl<open/sym/close> may be overriden to use a different system call (LoadLibraryEx(), for example)
NOTE : Best practices for loading dll/so libraries (ie. full paths instead of relative paths) should be exercised
*/

#ifdef __linux__
#ifndef VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <dlfcn.h>
#ifndef gvk_dlopen
#define gvk_dlopen(LIBRARY_NAME) dlopen(LIBRARY_NAME, RTLD_LAZY | RTLD_LOCAL)
#endif
#ifndef gvk_dlsym
#define gvk_dlsym(LIBRARY_HANDLE, SYMBOL_NAME) dlsym(LIBRARY_HANDLE, SYMBOL_NAME)
#endif
#ifndef gvk_dlclose
#define gvk_dlclose(LIBRARY_HANDLE) dlclose(LIBRARY_HANDLE)
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#ifndef gvk_dlopen
#define gvk_dlopen(LIBRARY_NAME) LoadLibraryA(LIBRARY_NAME)
#endif
#ifndef gvk_dlsym
#define gvk_dlsym(LIBRARY_HANDLE, SYMBOL_NAME) GetProcAddress((HMODULE)LIBRARY_HANDLE, SYMBOL_NAME)
#endif
#ifndef gvk_dlclose
#define gvk_dlclose(LIBRARY_HANDLE) FreeLibrary((HMODULE)LIBRARY_HANDLE)
#endif
#endif

#ifndef VK_ENABLE_BETA_EXTENSIONS
#define VK_ENABLE_BETA_EXTENSIONS
#endif
#include "vulkan/vulkan.h"

#if 0
#define VMA_DEBUG_LOG(format, ...) do { \
    printf(format, __VA_ARGS__); \
    printf("\n"); \
} while(false)
#endif

#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wnullability-completeness"
#endif
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#if defined(__clang__)
#define GVK_COMPILER_CLANG
#elif defined(__GNUC__)
#define GVK_COMPILER_GCC
#elif defined(_MSVC_LANG)
#define GVK_COMPILER_MSVC
#endif

#define gvk_stringify(STR) #STR
#define gvk_expand(STR) gvk_stringify(STR)
#define gvk_file_line (__FILE__ "(" gvk_expand(__LINE__) ")")

/**
@example
    VkResult example_function()
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            gvk_result(vkFunctionCall0(...));
            gvk_result(vkFunctionCall1(...));
            gvk_result(vkFunctionCall2(...));
        } gvk_result_scope_end;
        if (gvkResult == VK_ERROR_<...>) {
            // ...recover...
        }
        return gvkResult;
    }
*/
#define gvk_result_scope_begin(GVK_RESULT) VkResult gvkResult = GVK_RESULT; {
#define gvk_result_scope_end } GVK_FAIL:
#define gvk_result(GVK_CALL) \
gvkResult = (GVK_CALL); \
if (gvkResult != VK_SUCCESS) { \
    assert(gvkResult == VK_SUCCESS && #GVK_CALL); \
    goto GVK_FAIL; \
}

namespace gvk {

inline const VkAllocationCallbacks* validate_allocator(const VkAllocationCallbacks& allocator)
{
    return (allocator.pfnAllocation && allocator.pfnFree) ? &allocator : nullptr;
}

} // namespace gvk

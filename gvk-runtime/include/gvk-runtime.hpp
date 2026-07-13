
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
#include "gvk-runtime/child-process.hpp"
#include "gvk-runtime/io-pipe.hpp"
#include "gvk-runtime/io-thread.hpp"
#include "gvk-runtime/ipc-messenger.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace gvk {

VkResult load_vulkan_runtime();
void unload_vulkan_runtime();
PFN_vkGetInstanceProcAddr load_vkGetInstanceProcAddr();
const std::vector<std::string>& get_validation_layer_settings();

uint64_t get_thread_id();

#ifdef GVK_PLATFORM_WINDOWS
BOOL is_console(HANDLE handle);
BOOL get_this_module_handle(HMODULE* phModule);
DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath);
DWORD get_this_module_path(std::filesystem::path* pPath);
DWORD get_process_path(HANDLE hProcess, std::filesystem::path* pPath);
DWORD get_this_process_path(std::filesystem::path* pPath);
DWORD get_process_name(HANDLE hProcess, std::filesystem::path* pPath);
DWORD get_this_process_name(std::filesystem::path* pPath);
BOOL is_process_elevated(HANDLE hProcess = NULL);
BOOL register_explicit_layer(const std::filesystem::path& layerJsonPath);
void unregister_explicit_layer(const std::filesystem::path& layerName);
std::string get_win32_error_str(DWORD errorCode);
#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk

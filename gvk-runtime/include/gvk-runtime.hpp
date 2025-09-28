
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

#include <filesystem>
#include <string>

namespace gvk {

VkResult load_vulkan_runtime();
void unload_vulkan_runtime();
PFN_vkGetInstanceProcAddr load_vkGetInstanceProcAddr();

#ifdef GVK_PLATFORM_WINDOWS
BOOL get_this_module_handle(HMODULE* phModule);
DWORD get_module_path(HMODULE hModule, std::filesystem::path* pPath);
DWORD get_this_module_path(std::filesystem::path* pPath);
std::string get_win32_error_str(DWORD errorCode);
#endif // GVK_PLATFORM_WINDOWS

} // namespace gvk


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
#include "gvk-dispatch-table.hpp"
#include "gvk-handles.hpp"
#include "gvk-restore-point/generated/restore-info.h"
#include "gvk-structures.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.h"

#include <thread>
#include <filesystem>

namespace gvk {
namespace restore_point {

class CreateInfo final
{
public:
    std::filesystem::path path;
    std::function<void(std::thread::id)> threadInitializationCallback;
    std::function<void(VkDevice, VkDeviceMemory, VkMemoryAllocateInfo, void*)> processDeviceMemoryDataCallback;
};

class ApplyInfo final
{
public:
    std::filesystem::path path;
    std::function<void(std::thread::id)> threadInitializationCallback;
    std::function<void(VkObjectType, uint64_t, uint64_t)> processObjectCallback;
    std::function<void(VkDevice, VkDeviceMemory, VkMemoryAllocateInfo, void*)> processDeviceMemoryDataCallback;
    std::function<void(VkDevice, VkImage, VkImageCreateInfo, const VkImageLayout*)> processImageLayoutsCallback;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    std::function<void(VkWin32SurfaceCreateInfoKHR* pWin32SurfaceCreateInfo, uint32_t width, uint32_t height)> processWin32SurfaceCreateInfoCallback;
#endif
    DispatchTable dispatchTable;
    bool recording { false };
};

} // namespace restore_point
} // namespace gvk

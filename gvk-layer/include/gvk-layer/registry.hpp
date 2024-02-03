
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

#include "gvk-layer/generated/basic-layer.hpp"
#include "gvk-layer/generated/layer-hooks.hpp"
#include "gvk-defines.hpp"
#include "gvk-dispatch-table.hpp"

#include "vulkan/vk_layer.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace gvk {
namespace layer {

template <typename DispatchableVkHandleType>
inline void* get_dispatch_key(DispatchableVkHandleType dispatchableVkHandle)
{
    assert(dispatchableVkHandle);
    return *(void**)dispatchableVkHandle;
}

class Registry final
{
public:
    static Registry& get();

    template <typename DispatchableVkHandleType>
    inline DispatchTable& get_instance_dispatch_table(DispatchableVkHandleType dispatchableVkHandle)
    {
        static DispatchTable sDispatchTable;
        auto itr = VkInstanceDispatchTables.find(layer::get_dispatch_key(dispatchableVkHandle));
        assert(itr != VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        return itr != VkInstanceDispatchTables.end() ? itr->second : sDispatchTable;
    }

    template <typename DispatchableVkHandleType>
    inline DispatchTable& get_device_dispatch_table(DispatchableVkHandleType dispatchableVkHandle)
    {
        static DispatchTable sDispatchTable;
        auto itr = VkDeviceDispatchTables.find(layer::get_dispatch_key(dispatchableVkHandle));
        assert(itr != VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
        return itr != VkDeviceDispatchTables.end() ? itr->second : sDispatchTable;
    }

    std::mutex mutex;
    VkInstance instance{ };
    uint32_t apiVersion{ VK_API_VERSION_1_0 };
    std::vector<std::unique_ptr<BasicLayer>> layers;
    std::unordered_map<void*, DispatchTable> VkInstanceDispatchTables;
    std::unordered_map<void*, DispatchTable> VkDeviceDispatchTables;
    using ApplicationVkPhysicalDevice = VkPhysicalDevice;
    using LoaderVkPhysicalDevice = VkPhysicalDevice;
    std::unordered_map<ApplicationVkPhysicalDevice, LoaderVkPhysicalDevice> VkPhysicalDevices;

private:
    Registry() = default;
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
};

extern void on_load(Registry& registry);
VkLayerInstanceCreateInfo* get_instance_chain_info(const VkInstanceCreateInfo* pCreateInfo, VkLayerFunction layerFunction);
VkLayerDeviceCreateInfo* get_device_chain_info(const VkDeviceCreateInfo* pCreateInfo, VkLayerFunction layerFunction);
PFN_vkVoidFunction get_instance_proc_addr(VkInstance instance, const char* pName);
PFN_vkVoidFunction get_physical_device_proc_addr(VkInstance instance, const char* pName);
PFN_vkVoidFunction get_device_proc_addr(VkDevice device, const char* pName);

} // namespace layer
} // namespace gvk

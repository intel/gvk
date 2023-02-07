
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

#include "gvk-layer/generated/basic-layer.hpp"
#include "gvk-layer/generated/layer-hooks.hpp"
#include "gvk-layer/registry.hpp"

#include "vulkan/vk_layer.h"

#include <cassert>
#include <cstring>
#include <memory>

namespace gvk {
namespace layer {

Registry& Registry::get()
{
    static Registry sInstance;
    return sInstance;
}

VkLayerInstanceCreateInfo* get_instance_chain_info(const VkInstanceCreateInfo* pCreateInfo, VkLayerFunction layerFunction)
{
    assert(pCreateInfo);
    auto pChainInfo = (VkLayerInstanceCreateInfo*)pCreateInfo->pNext;
    while (pChainInfo && !(pChainInfo->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && pChainInfo->function == layerFunction)) {
        pChainInfo = (VkLayerInstanceCreateInfo*)pChainInfo->pNext;
    }
    assert(pChainInfo && "Failed to get VkLayerInstanceCreateInfo; are the Vulkan SDK and runtime configured correctly?");
    return pChainInfo;
}

VkLayerDeviceCreateInfo* get_device_chain_info(const VkDeviceCreateInfo* pCreateInfo, VkLayerFunction layerFunction)
{
    assert(pCreateInfo);
    auto pChainInfo = (VkLayerDeviceCreateInfo*)pCreateInfo->pNext;
    while (pChainInfo && !(pChainInfo->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && pChainInfo->function == layerFunction)) {
        pChainInfo = (VkLayerDeviceCreateInfo*)pChainInfo->pNext;
    }
    assert(pChainInfo && "Failed to get VkLayerDeviceCreateInfo; are the Vulkan SDK and runtime configured correctly?");
    return pChainInfo;
}

VkResult create_instance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    assert(pCreateInfo);
    assert(pInstance);
    auto vkResult = VK_ERROR_INITIALIZATION_FAILED;
    auto pLayerInstanceCreateInfo = get_instance_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    auto pfn_vkGetInstanceProcAddr = (pLayerInstanceCreateInfo && pLayerInstanceCreateInfo->u.pLayerInfo) ? pLayerInstanceCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr : nullptr;
    auto pfn_vkCreateInstance = pfn_vkGetInstanceProcAddr ? (PFN_vkCreateInstance)pfn_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance") : nullptr;
    if (pfn_vkCreateInstance) {
        pLayerInstanceCreateInfo->u.pLayerInfo = pLayerInstanceCreateInfo->u.pLayerInfo->pNext;
        auto& layers = Registry::get().layers;
        on_load(Registry::get());
        vkResult = VK_SUCCESS;
        for (auto layerItr = layers.begin(); layerItr != layers.end(); ++layerItr) {
            assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
            vkResult = (*layerItr)->pre_vkCreateInstance(pCreateInfo, pAllocator, pInstance, vkResult);
        }
        vkResult = pfn_vkCreateInstance(pCreateInfo, pAllocator, pInstance);
        if (vkResult == VK_SUCCESS) {
            DispatchTable instanceDispatchTable { };
            instanceDispatchTable.gvkGetInstanceProcAddr = pfn_vkGetInstanceProcAddr;
            DispatchTable::load_instance_entry_points(*pInstance, &instanceDispatchTable);
            std::lock_guard<std::mutex> lock(Registry::get().mutex);
            Registry::get().VkInstanceDispatchTables.insert({ get_dispatch_key(*pInstance), instanceDispatchTable });
        }
        for (auto layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr) {
            assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
            vkResult = (*layerItr)->post_vkCreateInstance(pCreateInfo, pAllocator, pInstance, vkResult);
        }
    }
    return vkResult;
}

void destroy_instance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    assert(instance);
    auto& layers = Registry::get().layers;
    for (auto layerItr = layers.begin(); layerItr != layers.end(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        (*layerItr)->pre_vkDestroyInstance(instance, pAllocator);
    }
    const auto& instanceDispatchTableItr = Registry::get().VkInstanceDispatchTables.find(get_dispatch_key(instance));
    assert(instanceDispatchTableItr != Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
    const auto& instanceDispatchTable = instanceDispatchTableItr->second;
    assert(instanceDispatchTable.gvkDestroyInstance && "gvk::layer::Registry VkInstance gvk::DispatchTable contains a null entry point; are the Vulkan SDK, runtime, and layers configured correctly?");
    instanceDispatchTable.gvkDestroyInstance(instance, pAllocator);
    {
        std::lock_guard<std::mutex> lock(Registry::get().mutex);
        Registry::get().VkInstanceDispatchTables.erase(get_dispatch_key(instance));
    }
    for (auto layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        (*layerItr)->post_vkDestroyInstance(instance, pAllocator);
    }
}

VkResult create_device(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    assert(physicalDevice);
    assert(pCreateInfo);
    assert(pDevice);
    auto vkResult = VK_ERROR_INITIALIZATION_FAILED;
    auto pLayerDeviceCreateInfo = get_device_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    auto pfn_vkGetDeviceProcAddr = (pLayerDeviceCreateInfo && pLayerDeviceCreateInfo->u.pLayerInfo) ? pLayerDeviceCreateInfo->u.pLayerInfo->pfnNextGetDeviceProcAddr : nullptr;
    const auto& instanceDispatchTableItr = Registry::get().VkInstanceDispatchTables.find(get_dispatch_key(physicalDevice));
    assert(instanceDispatchTableItr != Registry::get().VkInstanceDispatchTables.end() && "Failed to get gvk::layer::Registry VkInstance gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
    const auto& instanceDispatchTable = instanceDispatchTableItr->second;
    if (pfn_vkGetDeviceProcAddr && instanceDispatchTable.gvkCreateDevice) {
        pLayerDeviceCreateInfo->u.pLayerInfo = pLayerDeviceCreateInfo->u.pLayerInfo->pNext;
        auto& layers = Registry::get().layers;
        vkResult = VK_SUCCESS;
        for (auto layerItr = layers.begin(); layerItr != layers.end(); ++layerItr) {
            assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
            vkResult = (*layerItr)->pre_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, vkResult);
        }
        vkResult = instanceDispatchTable.gvkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
        if (vkResult == VK_SUCCESS) {
            DispatchTable deviceDispatchTable { };
            deviceDispatchTable.gvkGetDeviceProcAddr = pfn_vkGetDeviceProcAddr;
            DispatchTable::load_device_entry_points(*pDevice, &deviceDispatchTable);
            std::lock_guard<std::mutex> lock(Registry::get().mutex);
            Registry::get().VkDeviceDispatchTables.insert({ get_dispatch_key(*pDevice), deviceDispatchTable });
        }
        for (auto layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr) {
            assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
            vkResult = (*layerItr)->post_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, vkResult);
        }
    }
    return vkResult;
}

void destroy_device(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    assert(device);
    auto& layers = Registry::get().layers;
    for (auto layerItr = layers.begin(); layerItr != layers.end(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        (*layerItr)->pre_vkDestroyDevice(device, pAllocator);
    }
    const auto& deviceDispatchTableItr = Registry::get().VkDeviceDispatchTables.find(get_dispatch_key(device));
    assert(deviceDispatchTableItr != Registry::get().VkDeviceDispatchTables.end() && "Failed to get gvk::layer::Registry VkDevice gvk::DispatchTable; are the Vulkan SDK, runtime, and layers configured correctly?");
    const auto& deviceDispatchTable = deviceDispatchTableItr->second;
    assert(deviceDispatchTable.gvkDestroyDevice && "gvk::layer::Registry VkDevice gvk::DispatchTable contains a null entry point for vkDestroyDevice; are the Vulkan SDK, runtime, and layers configured correctly?");
    deviceDispatchTable.gvkDestroyDevice(device, pAllocator);
    {
        std::lock_guard<std::mutex> lock(Registry::get().mutex);
        Registry::get().VkDeviceDispatchTables.erase(get_dispatch_key(device));
    }
    for (auto layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        (*layerItr)->post_vkDestroyDevice(device, pAllocator);
    }
}

PFN_vkVoidFunction get_instance_proc_addr(VkInstance, const char* pName)
{
    assert(pName);
    if (!strcmp(pName, "vkCreateInstance")) {
        return (PFN_vkVoidFunction)gvk::layer::create_instance;
    } else if (!strcmp(pName, "vkDestroyInstance")) {
        return (PFN_vkVoidFunction)gvk::layer::destroy_instance;
    } else if (!strcmp(pName, "vkCreateDevice")) {
        return (PFN_vkVoidFunction)gvk::layer::create_device;
    } else if (!strcmp(pName, "vkDestroyDevice")) {
        return (PFN_vkVoidFunction)gvk::layer::destroy_device;
    }
    return gvk::layer::hooks::get(pName);
}

PFN_vkVoidFunction get_physical_device_proc_addr(VkInstance, const char* pName)
{
    return gvk::layer::hooks::get(pName);
}

PFN_vkVoidFunction get_device_proc_addr(VkDevice, const char* pName)
{
    return gvk::layer::hooks::get(pName);
}

} // namespace layer
} // namespace gvk

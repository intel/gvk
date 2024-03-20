
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
#include "gvk-structures.hpp"

#include "vulkan/vk_layer.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#endif // VK_USE_PLATFORM_WIN32_KHRS

#include <cassert>
#include <cstring>
#include <map>
#include <memory>

namespace gvk {
namespace layer {

Registry& Registry::get()
{
    static Registry* spRegistry{ new Registry };
    return *spRegistry;
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
    std::lock_guard<std::mutex> lock(Registry::get().mutex);
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
            Registry::get().instance = *pInstance;
            Registry::get().apiVersion = pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0;
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
    std::lock_guard<std::mutex> lock(Registry::get().mutex);
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
    Registry::get().VkInstanceDispatchTables.clear();
    Registry::get().VkDeviceDispatchTables.clear();
    Registry::get().VkPhysicalDevices.clear();
    for (auto layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr) {
        assert(*layerItr && "gvk::layer::Registry contains a null layer; are layers configured correctly and intialized via gvk::layer::on_load()?");
        (*layerItr)->post_vkDestroyInstance(instance, pAllocator);
    }
    layers.clear();
}

VkResult get_physical_device_infos(const DispatchTable& dispatchTable, VkInstance instance, std::map<VkPhysicalDeviceProperties, std::vector<VkPhysicalDevice>>& physicalDeviceInfos)
{
    assert(dispatchTable.gvkEnumeratePhysicalDevices);
    assert(dispatchTable.gvkGetPhysicalDeviceProperties);
    assert(instance);
    physicalDeviceInfos.clear();
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        uint32_t physicalDeviceCount = 0;
        gvk_result(dispatchTable.gvkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        gvk_result(dispatchTable.gvkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));
        for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
            VkPhysicalDeviceProperties physicalDeviceProperties{ };
            dispatchTable.gvkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
            physicalDeviceInfos[physicalDeviceProperties].push_back(physicalDevices[i]);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult create_physical_device_mappings(VkInstance instance)
{
    assert(instance);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Get VkPhysicalDevice and VkPhysicalDeviceProperties as seen by the application
        DispatchTable applicationDispatchTable{ };
        DispatchTable::load_global_entry_points(&applicationDispatchTable);
        DispatchTable::load_instance_entry_points(instance, &applicationDispatchTable);
        std::map<VkPhysicalDeviceProperties, std::vector<VkPhysicalDevice>> applicationPhysicalDeviceInfos;
        get_physical_device_infos(applicationDispatchTable, instance, applicationPhysicalDeviceInfos);

        // Get VkPhysicalDevice and VkPhysicalDeviceProperties as seen by the loader
        const auto& layerInstanceDispatchTableItr = Registry::get().VkInstanceDispatchTables.find(get_dispatch_key(instance));
        assert(layerInstanceDispatchTableItr != Registry::get().VkInstanceDispatchTables.end());
        std::map<VkPhysicalDeviceProperties, std::vector<VkPhysicalDevice>> loaderPhysicalDeviceInfos;
        get_physical_device_infos(layerInstanceDispatchTableItr->second, instance, loaderPhysicalDeviceInfos);

        // Map application VkPhysicalDevices to loader VkPhysicalDevices
        for (auto applicationPhysicalDeviceInfoItr : applicationPhysicalDeviceInfos) {
            const auto& physicalDeviceProperties = applicationPhysicalDeviceInfoItr.first;
            auto loaderPhysicalDeviceInfoItr = loaderPhysicalDeviceInfos.find(physicalDeviceProperties);
            gvk_result(loaderPhysicalDeviceInfoItr != loaderPhysicalDeviceInfos.end() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            const auto& applicationPhysicalDevices = applicationPhysicalDeviceInfoItr.second;
            const auto& loaderPhysicalDevices = loaderPhysicalDeviceInfoItr->second;
            gvk_result(applicationPhysicalDevices.size() == loaderPhysicalDevices.size() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            for (uint32_t i = 0; i < applicationPhysicalDevices.size(); ++i) {
                gvk_result(Registry::get().VkPhysicalDevices.insert({ applicationPhysicalDevices[i], loaderPhysicalDevices[i] }).second ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            }
            loaderPhysicalDeviceInfos.erase(physicalDeviceProperties);
        }

        // Ensure that every VkPhysicalDevice has been mapped 
        gvk_result(loaderPhysicalDeviceInfos.empty() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult create_device(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    assert(physicalDevice);
    assert(pCreateInfo);
    assert(pDevice);
    std::lock_guard<std::mutex> lock(Registry::get().mutex);
    auto vkResult = create_physical_device_mappings(Registry::get().instance);
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
    std::lock_guard<std::mutex> lock(Registry::get().mutex);
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
    Registry::get().VkDeviceDispatchTables.erase(get_dispatch_key(device));
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

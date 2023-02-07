
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

#include "gvk-handles/context.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-handles/utilities.hpp"
#include "gvk-handles/handles.hpp"
#include "gvk-structures/defaults.hpp"

#include <algorithm>
#include <vector>

namespace gvk {

VkResult Context::create(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext)
{
    assert(pCreateInfo);
    assert(pContext);
    pContext->reset();
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Create gvk::Instance
        auto instanceCreateInfo = pCreateInfo->pInstanceCreateInfo ? *pCreateInfo->pInstanceCreateInfo : get_default<VkInstanceCreateInfo>();
        std::vector<const char*> layers(instanceCreateInfo.ppEnabledLayerNames, instanceCreateInfo.ppEnabledLayerNames + instanceCreateInfo.enabledLayerCount);
        if (pCreateInfo->loadApiDumpLayer) {
            layers.push_back("VK_LAYER_LUNARG_api_dump");
        }
        if (pCreateInfo->loadValidationLayer) {
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        std::vector<const char*> instanceExtensions(instanceCreateInfo.ppEnabledExtensionNames, instanceCreateInfo.ppEnabledExtensionNames + instanceCreateInfo.enabledExtensionCount);
        if (pCreateInfo->pDebugUtilsMessengerCreateInfo) {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        if (pCreateInfo->loadWsiExtensions) {
            instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_XLIB_KHR
            instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        }
        instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
        instanceCreateInfo.ppEnabledLayerNames = !layers.empty() ? layers.data() : nullptr;
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = !instanceExtensions.empty() ? instanceExtensions.data() : nullptr;
        gvk_result(pContext->create_instance(&instanceCreateInfo, pAllocator));
        pContext->mPhysicalDevices = pContext->sort_physical_devices(pContext->mInstance.get<PhysicalDevices>());

        // Create gvk::DebugUtilsMessengerEXT
        if (pCreateInfo->pDebugUtilsMessengerCreateInfo) {
            gvk_result(pContext->create_debug_utils_messenger(pCreateInfo->pDebugUtilsMessengerCreateInfo, pAllocator));
        }

        // Create gvk::Device
        auto deviceCreateInfo = pCreateInfo->pDeviceCreateInfo ? *pCreateInfo->pDeviceCreateInfo : get_default<VkDeviceCreateInfo>();
        assert(!deviceCreateInfo.queueCreateInfoCount == !deviceCreateInfo.pQueueCreateInfos);
        if (!deviceCreateInfo.queueCreateInfoCount || !deviceCreateInfo.pQueueCreateInfos) {
            deviceCreateInfo.queueCreateInfoCount = 1;
            deviceCreateInfo.pQueueCreateInfos = &gvk::get_default<VkDeviceQueueCreateInfo>();
        }
        std::vector<const char*> deviceExtensions(deviceCreateInfo.ppEnabledExtensionNames, deviceCreateInfo.ppEnabledExtensionNames + deviceCreateInfo.enabledExtensionCount);
        if (pCreateInfo->loadWsiExtensions) {
            deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }
        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = !deviceExtensions.empty() ? deviceExtensions.data() : nullptr;
        gvk_result(pContext->create_devices(&deviceCreateInfo, pAllocator));

        // Allocate gvk::CommandBuffers
        gvk_result(pContext->allocate_command_buffers(pAllocator));
    } gvk_result_scope_end
    return gvkResult;
}

Context::~Context()
{
    reset();
}

Context::operator bool() const
{
    return mInstance && !mDevices.empty() && !mCommandBuffers.empty();
}

void Context::reset()
{
    mInstance.reset();
    mDebugUtilsMessenger.reset();
    mDevices.clear();
    mCommandBuffers.clear();
}

const Instance& Context::get_instance() const
{
    return mInstance;
}

const std::vector<PhysicalDevice>& Context::get_physical_devices() const
{
    return mPhysicalDevices;
}

const std::vector<Device>& Context::get_devices() const
{
    return mDevices;
}

const std::vector<CommandBuffer>& Context::get_command_buffers() const
{
    return mCommandBuffers;
}

VkResult Context::create_instance(const VkInstanceCreateInfo* pInstanceCreateInfo, const VkAllocationCallbacks* pAllocator)
{
    assert(pInstanceCreateInfo);
    return Instance::create(pInstanceCreateInfo, pAllocator, &mInstance);
}

VkResult Context::create_debug_utils_messenger(const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo, const VkAllocationCallbacks* pAllocator)
{
    assert(pDebugUtilsMessengerCreateInfo);
    return DebugUtilsMessengerEXT::create(mInstance, pDebugUtilsMessengerCreateInfo, pAllocator, &mDebugUtilsMessenger);
}

std::vector<PhysicalDevice> Context::sort_physical_devices(std::vector<PhysicalDevice> physicalDevices) const
{
    std::sort(physicalDevices.begin(), physicalDevices.end(),
        [this](const auto& lhs, const auto& rhs)
        {
            return get_physical_device_rating(lhs) > get_physical_device_rating(rhs);
        }
    );
    return physicalDevices;
}

uint32_t Context::get_physical_device_rating(const PhysicalDevice& physicalDevice) const
{
    VkPhysicalDeviceProperties physicalDeviceProperties { };
    const auto& dispatchTable = physicalDevice.get<DispatchTable>();
    assert(dispatchTable.gvkGetPhysicalDeviceProperties);
    dispatchTable.gvkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    uint32_t rating = 0;
#ifdef GVK_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
    switch (physicalDeviceProperties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:          ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:            ++rating;
    default: {
    } break;
    }
#ifdef GVK_COMPILER_GCC
#pragma GCC diagnostic pop
#endif
    return rating;
}

VkResult Context::create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks* pAllocator)
{
    assert(pDeviceCreateInfo);
    mDevices.resize(1);
    return Device::create(mPhysicalDevices[0], pDeviceCreateInfo, pAllocator, mDevices.data());
}

VkResult Context::allocate_command_buffers(const VkAllocationCallbacks* pAllocator)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = get_queue_family(mDevices[0], 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        CommandPool commandPool;
        gvk_result(CommandPool::create(mDevices[0], &commandPoolCreateInfo, pAllocator, &commandPool));
        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        mCommandBuffers.resize(commandBufferAllocateInfo.commandBufferCount);
        gvk_result(CommandBuffer::allocate(mDevices[0], &commandBufferAllocateInfo, mCommandBuffers.data()));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

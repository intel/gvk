
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#include "gvk/context.hpp"
#include "gvk/generated/dispatch-table.hpp"

#include <algorithm>
#include <cassert>

namespace gvk {

VkResult Context::create(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext)
{
    assert(pCreateInfo);
    assert(pContext);

#ifndef VK_NO_PROTOTYPES
    gvk::DispatchTable::load_static_entry_points(&gvk::gDispatchTable);
#endif

    pContext->reset();

    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Create gvk::Instance
        std::vector<const char*> layers;
        std::vector<const char*> instanceExtensions;
        if (pCreateInfo->pDebugUtilsMessengerCreateInfo) {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        if (pCreateInfo->pSysSurfaceCreateInfo) {
            instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            #ifdef VK_USE_PLATFORM_WIN32_KHR
            instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            #elif VK_USE_PLATFORM_XLIB_KHR
            enabledExtensionNames.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
            #endif
        }
        auto applicationInfo = pCreateInfo->pApplicationInfo ? *pCreateInfo->pApplicationInfo : get_default<VkApplicationInfo>();
        auto instanceCreateInfo = get_default<VkInstanceCreateInfo>();
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
        instanceCreateInfo.ppEnabledLayerNames = layers.data();
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
        gvk_result(pContext->create_instance(&instanceCreateInfo, pAllocator));

        // Create gvk::DebugUtilsMessengerEXT
        if (pCreateInfo->pDebugUtilsMessengerCreateInfo) {
            gvk_result(pContext->create_debug_utils_messenger(pCreateInfo->pDebugUtilsMessengerCreateInfo, pAllocator));
        }

        // Create gvk::Device
        std::vector<const char*> deviceExtensions;
        if (pCreateInfo->pSysSurfaceCreateInfo) {
            deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }
        float queuePriority = 0.0f;
        auto deviceQueueCreateInfo = get_default<VkDeviceQueueCreateInfo>();
        deviceQueueCreateInfo.queueFamilyIndex = 0;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        auto deviceCreateInfo = get_default<VkDeviceCreateInfo>();
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        gvk_result(pContext->create_devices(&deviceCreateInfo, pAllocator));

        // Allocate gvk::CommandBuffers
        gvk_result(pContext->allocate_command_buffers(pAllocator));

        // Create gvk::sys::Surface and gvk::WsiManager
        if (pCreateInfo->pSysSurfaceCreateInfo) {
            auto sysSurfaceCreateInfo = *pCreateInfo->pSysSurfaceCreateInfo;
            if (!sysSurfaceCreateInfo.pTitle && applicationInfo.pApplicationName) {
                sysSurfaceCreateInfo.pTitle = applicationInfo.pApplicationName;
            }
            gvk_result(pContext->create_sys_surface(&sysSurfaceCreateInfo));

            auto wsiManagerCreateInfo = get_default<WsiManager::CreateInfo>();
#ifdef VK_USE_PLATFORM_WIN32_KHR
            auto surfaceCreateInfo = get_default<VkWin32SurfaceCreateInfoKHR>();
            surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
            surfaceCreateInfo.hwnd = (HWND)pContext->mSysSurface.get_hwnd();
            wsiManagerCreateInfo.pWin32SurfaceCreateInfoKHR = &surfaceCreateInfo;
#endif // VK_USE_PLATFORM_WIN32_KHR
            wsiManagerCreateInfo.queueFamilyIndex = get_queue_family(pContext->mDevices[0], 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
            gvk_result(pContext->create_wsi_manager(&wsiManagerCreateInfo, pAllocator));
        }
    } gvk_result_scope_end
    return gvkResult;
}

Context::~Context()
{
    reset();
}

void Context::reset()
{
    mInstance.reset();
    mDebugUtilsMessenger.reset();
    mDevices.clear();
    mCommandBuffers.clear();
    mSysSurface.reset();
    mWsiManager.reset();
}

const Instance& Context::get_instance() const
{
    return mInstance;
}

std::vector<PhysicalDevice> Context::get_physical_devices() const
{
    return sort_physical_devices();
}

const std::vector<Device>& Context::get_devices() const
{
    return mDevices;
}

const std::vector<CommandBuffer>& Context::get_command_buffers() const
{
    return mCommandBuffers;
}

const sys::Surface& Context::get_sys_surface() const
{
    return mSysSurface;
}

const WsiManager& Context::get_wsi_manager() const
{
    return mWsiManager;
}

WsiManager& Context::get_wsi_manager()
{
    return mWsiManager;
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

std::vector<PhysicalDevice> Context::sort_physical_devices() const
{
    auto physicalDevices = mInstance.get<const std::vector<PhysicalDevice>&>();
    std::sort(physicalDevices.begin(), physicalDevices.end(),
        [this](const auto& lhs, const auto& rhs)
        {
            return get_physical_device_rating(lhs) < get_physical_device_rating(rhs);
        }
    );
    return physicalDevices;
}

uint32_t Context::get_physical_device_rating(const PhysicalDevice& physicalDevice) const
{
    VkPhysicalDeviceProperties physicalDeviceProperties { };
    assert(gDispatchTable.gvkGetPhysicalDeviceProperties);
    gDispatchTable.gvkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkPhysicalDeviceFeatures physicalDeviceFeatures { };
    gDispatchTable.gvkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
    uint32_t rating = 0;
    switch (physicalDeviceProperties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:          ++rating;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:            ++rating;
    default: {
    } break;
    }
    return rating;
}

VkResult Context::create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks* pAllocator)
{
    assert(pDeviceCreateInfo);
    mDevices.resize(1);
    return Device::create(get_physical_devices()[0], pDeviceCreateInfo, pAllocator, &mDevices[0]);
}

VkResult Context::allocate_command_buffers(const VkAllocationCallbacks* pAllocator)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto commandPoolCreateInfo = get_default<VkCommandPoolCreateInfo>();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        commandPoolCreateInfo.queueFamilyIndex = get_queue_family(mDevices[0], 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        CommandPool commandPool;
        gvk_result(CommandPool::create(mDevices[0], &commandPoolCreateInfo, pAllocator, &commandPool));
        auto commandBufferAllocateInfo = get_default<VkCommandBufferAllocateInfo>();
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        mCommandBuffers.resize(1);
        gvk_result(CommandBuffer::allocate(mDevices[0], &commandBufferAllocateInfo, &mCommandBuffers[0]));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Context::create_sys_surface(const sys::Surface::CreateInfo* pSysSurfaceCreateInfo)
{
    assert(pSysSurfaceCreateInfo);
    return sys::Surface::create(pSysSurfaceCreateInfo, &mSysSurface) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

VkResult Context::create_wsi_manager(const WsiManager::CreateInfo* pWsiManagerCreateInfo, const VkAllocationCallbacks* pAllocator)
{
    assert(pWsiManagerCreateInfo);
    return WsiManager::create(mDevices[0], pWsiManagerCreateInfo, pAllocator, &mWsiManager);
}

} // namespace gvk

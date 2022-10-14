
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
    DispatchTable::load_static_entry_points(&DispatchTable::get_global_dispatch_table());
#else
    auto& dispatchTable = DispatchTable::get_global_dispatch_table();
    dispatchTable.gvkGetInstanceProcAddr = detail::load_get_instance_proc_addr();
    assert(dispatchTable.gvkGetInstanceProcAddr);
    DispatchTable::load_instance_entry_points(VK_NULL_HANDLE, &dispatchTable);
    assert(dispatchTable.gvkCreateInstance);
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
#ifdef VK_USE_PLATFORM_XLIB_KHR
            instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
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
#ifdef VK_NO_PROTOTYPES
        DispatchTable::load_instance_entry_points(pContext->mInstance, &DispatchTable::get_global_dispatch_table());
#endif

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
#ifdef VK_NO_PROTOTYPES
        DispatchTable::load_device_entry_points(pContext->mDevices[0], &DispatchTable::get_global_dispatch_table());
#endif

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

#ifdef VK_USE_PLATFORM_XLIB_KHR
            auto xlibSurfaceCreateInfo = get_default<VkXlibSurfaceCreateInfoKHR>();
            xlibSurfaceCreateInfo.dpy = (Display*)pContext->mSysSurface.get_display();
            xlibSurfaceCreateInfo.window = (Window)pContext->mSysSurface.get_window();
            wsiManagerCreateInfo.pXlibSurfaceCreateInfoKHR = &xlibSurfaceCreateInfo;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            auto win32SurfaceCreateInfo = get_default<VkWin32SurfaceCreateInfoKHR>();
            win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
            win32SurfaceCreateInfo.hwnd = (HWND)pContext->mSysSurface.get_hwnd();
            wsiManagerCreateInfo.pWin32SurfaceCreateInfoKHR = &win32SurfaceCreateInfo;
#endif

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
    auto dispatchTable = DispatchTable::get_global_dispatch_table();
    assert(dispatchTable.gvkGetPhysicalDeviceProperties);
    dispatchTable.gvkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkPhysicalDeviceFeatures physicalDeviceFeatures { };
    dispatchTable.gvkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
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

namespace detail {

#ifdef VK_NO_PROTOTYPES
static void* sVulkanRuntime;

VkResult load_runtime()
{
    #ifdef __linux__
    constexpr const char* const VulkanRuntimeLibraryName = "libvulkan.so.1";
    #endif
    #ifdef VK_USE_PLATFORM_WIN32_KHR
    constexpr const char* const VulkanRuntimeLibraryName = "vulkan-1.dll";
    #endif
    if (!sVulkanRuntime) {
        sVulkanRuntime = gvk_dlopen(VulkanRuntimeLibraryName);
    }
    return sVulkanRuntime ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT;
}

void unload_runtime()
{
    if (sVulkanRuntime) {
        gvk_dlclose(sVulkanRuntime);
        sVulkanRuntime = NULL;
    }
}

PFN_vkGetInstanceProcAddr load_get_instance_proc_addr()
{
    load_runtime();
    return (PFN_vkGetInstanceProcAddr)gvk_dlsym(sVulkanRuntime, "vkGetInstanceProcAddr");
}
#endif

} // namespace detail
} // namespace gvk


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
#include "gvk-handles/handles.hpp"

namespace gvk {

/**
Provides high level control over Instance, Device(s)/Queue(s), and other utility objects
    @note Context may be extended to customize creation
*/
class Context
{
public:
    struct CreateInfo
    {
        /**
        Whether or not to load VK_LAYER_LUNARG_api_dump
        */
        VkBool32 loadApiDumpLayer{ VK_FALSE };

        /**
        Whether or not to load VK_LAYER_KHRONOS_validation
        */
        VkBool32 loadValidationLayer{ VK_FALSE };

        /**
        Whether or not to load VkSurfaceKHR and VkSwapchainKHR extensions
        */
        VkBool32 loadWsiExtensions{ VK_FALSE };

        /**
        Optional VkInstance creation parameters
        */
        const VkInstanceCreateInfo* pInstanceCreateInfo{ nullptr };

        /**
        Optional creation parameters for a DebugUtilsMessenger
            @note If provided, the debug utils extension to be loaded
        */
        const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo{ nullptr };

        /**
        Optional VkDevice creation parameters
        */
        const VkDeviceCreateInfo* pDeviceCreateInfo{ nullptr };
    };

    static VkResult create(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext);
    virtual ~Context();

    template <typename T>
    const T& get() const
    {
        assert(mReference && "Attempting to dereference nullref gvk::Context");
        if constexpr (std::is_same_v<T, Instance>) { return mReference->mInstance; }
        if constexpr (std::is_same_v<T, PhysicalDevices>) { return mReference->mPhysicalDevices; }
        if constexpr (std::is_same_v<T, DebugUtilsMessengerEXT>) { return mReference->mDebugUtilsMessenger; }
        if constexpr (std::is_same_v<T, Devices>) { return mReference->mDevices; }
        if constexpr (std::is_same_v<T, CommandBuffers>) { return mReference->mCommandBuffers; }
    }

protected:
    virtual VkResult create_instance(const VkInstanceCreateInfo* pInstanceCreateInfo, Instance* pInstance) const;
    virtual VkResult create_debug_utils_messenger(const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo, DebugUtilsMessengerEXT* pDebugUtilsMessenger) const;
    virtual uint32_t get_physical_device_rating(const PhysicalDevice& physicalDevice) const;
    virtual std::vector<PhysicalDevice> sort_physical_devices(std::vector<PhysicalDevice> physicalDevices) const;
    virtual VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, std::vector<Device>* pDevices) const;
    virtual VkResult allocate_command_buffers(std::vector<CommandBuffer>* pCommandBuffers) const;

private:
    class ControlBlock final
    {
    public:
        ControlBlock() = default;
        ~ControlBlock();
        Instance mInstance;
        std::vector<PhysicalDevice> mPhysicalDevices;
        DebugUtilsMessengerEXT mDebugUtilsMessenger;
        std::vector<Device> mDevices;
        std::vector<CommandBuffer> mCommandBuffers;
    private:
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock& operator=(const ControlBlock&) = delete;
    };

    gvk_reference_type(Context)
};

} // namespace gvk

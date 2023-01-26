
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
Provides high level control over Instance, Device(s)/Queue(s), WsiManager (Window System Integration) and several other utility objects
    @note Context may be extended to customize creation
*/
class Context
{
public:
    /**
    Creation parameters for Context
    */
    struct CreateInfo
    {
        /**
        Optional VkInstance creation parameters
        */
        const VkInstanceCreateInfo* pInstanceCreateInfo { nullptr };

        /**
        Whether or not to load VK_LAYER_LUNARG_api_dump
        */
        VkBool32 loadApiDumpLayer { VK_FALSE };

        /**
        Whether or not to load VK_LAYER_KHRONOS_validation
        */
        VkBool32 loadValidationLayer { VK_FALSE };

        /**
        Whether or not to load VkSurfaceKHR and VkSwapchainKHR extensions
        */
        VkBool32 loadWsiExtensions { VK_FALSE };

        /**
        Optional creation parameters for a DebugUtilsMessenger
            @note If provided, the debug utils extension to be loaded
        */
        const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo { nullptr };

        /**
        Optional VkDevice creation parameters
        */
        const VkDeviceCreateInfo* pDeviceCreateInfo { nullptr };
    };

    /**
    Constructs an instance of Context
    */
    Context() = default;

    /**
    Moves an instance of Context
    @param [in] other The Context to move from
    */
    Context(Context&& other) = default;

    /**
    Moves an instance of Context
    @param [in] other The Context to move from
    @return A reference to this Context
    */
    Context& operator=(Context&& other) = default;

    /**
    Creates an instance of Context
    @param [in] pCreateInfo A pointer to the Context creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @param [out] pContext A pointer to the Context to create
    @return The VkResult
    */
    static VkResult create(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext);

    /**
    Destroys this instance of Context
    */
    virtual ~Context();

    /**
    Gets a value indicating whether or not this Context is in a valid state
    @return Whether or not this Context is in a valid state
    */
    operator bool() const;

    /**
    Destroys this instance of Context
    */
    void reset();

    /**
    Gets this Context object's Instance
    @return This Context object's Instance object
    */
    const Instance& get_instance() const;

    /**
    Gets this Context object's PhysicalDevice objects
    @return This Context object's PhysicalDevice objects
        @note The PhysicalDevice objects returned by this method are the same as the PhysicalDevice objects provided by this Context object's Instance sorted by the rating provided by get_physical_device_rating()
    */
    const std::vector<PhysicalDevice>& get_physical_devices() const;

    /**
    Gets this Context object's Device objects
    @return This Context object's Device objects
    */
    const std::vector<Device>& get_devices() const;

    /**
    Gets this Context object's CommandBuffer objects
    @return This Context object's CommandBuffer objects
    */
    const std::vector<CommandBuffer>& get_command_buffers() const;

protected:
    /**
    Creates this Context object's Instance
    @param [in] pInstanceCreateInfo Instance creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return Instance creation result
        @note This method may be overriden to customize Instance creation
        @note If the base implementation is not called from this method, it must populate mInstance
    */
    virtual VkResult create_instance(const VkInstanceCreateInfo* pInstanceCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Creates this Context object's DebugUtilsMessenger
    @param [in] pDebugUtilsMessengerCreateInfo DebugUtilsMessenger creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return DebugUtilsMessenger creation result
        @note This method may be overriden to customize DebugUtilsMessenger creation
    */
    virtual VkResult create_debug_utils_messenger(const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Gets this Context object's PhysicalDevice objects sorted by the rating provided by get_physical_device_rating()
    @return This Context object's PhysicalDevice objects sorted by the rating provided by get_physical_device_rating()
        @note This method may be overriden to customize PhysicalDevice sorting
    */
    virtual std::vector<PhysicalDevice> sort_physical_devices(std::vector<PhysicalDevice> physicalDevices) const;

    /**
    Gets a given PhysicalDevice object's rating
    @param [in] physicalDevice The PhysicalDevice to get the rating for
    @return The given PhysicalDevice object's rating
        @note This method may be overriden to customize device rating
    */
    virtual uint32_t get_physical_device_rating(const PhysicalDevice& physicalDevice) const;

    /**
    Creates this Context object's Device objects
    @param [in] pDeviceCreateInfo Device creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return Device creation result
        @note This method may be overriden to customize Device creation
        @note The base implementation of this method uses the PhysicalDevice at index 0 of the collection returned from sort_physical_devices()
        @note If the base implementation is not called from this method, it must populate mDevices with at least 1 Device
    */
    virtual VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Allocates this Context object's CommandBuffer objects
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return CommandBuffer allocation result
        @note This method may be overriden to customize CommandBuffer allocation
    */
    virtual VkResult allocate_command_buffers(const VkAllocationCallbacks* pAllocator);

    Instance mInstance;
    DebugUtilsMessengerEXT mDebugUtilsMessenger;
    std::vector<PhysicalDevice> mPhysicalDevices;
    std::vector<Device> mDevices;
    std::vector<CommandBuffer> mCommandBuffers;

private:
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
};


} // namespace gvk

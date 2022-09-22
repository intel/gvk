
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

#include "gvk/generated/dispatch-table.hpp"
#include "gvk/system/surface.hpp"
#include "gvk/defines.hpp"
#include "gvk/handles.hpp"
#include "gvk/structures.hpp"
#include "gvk/wsi-manager.hpp"

namespace gvk {

/**
Provides high level control over Instance, Device(s)/Queue(s), WsiManager (Window System Integration) and several other utility objects
    @note Context may be extended to customize resource creation
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
        Optional VkApplicationInfo parameters
        */
        const VkApplicationInfo* pApplicationInfo{ nullptr };

        /**
        Optional creation parameters for a sys::Surface
            @note If provided, platform specific WSI extensions will be loaded
        */
        const sys::Surface::CreateInfo* pSysSurfaceCreateInfo{ nullptr };

        /**
        Optional creation parameters for a DebugUtilsMessenger
            @note If provided, the debug utils extension to be loaded
        */
        const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo{ nullptr };
    };

    /**
    Creates an instance of Context
    @param [in] pCreateInfo A pointer to the Context creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @param [out] pContext A pointer to the Context to create
    @return The VkResult
    */
    static VkResult create(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext);

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
    Destroys this instance of Context
    */
    virtual ~Context();

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
    */
    std::vector<PhysicalDevice> get_physical_devices() const;

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

    /**
    Gets this Context object's sys::Surface
    @return This Context object's sys::Surface object
    */
    const sys::Surface& get_sys_surface() const;

    /**
    Gets this Context object's WsiManager
    @return This Context object's WsiManager object
    */
    const WsiManager& get_wsi_manager() const;

    /**
    Gets this Context object's WsiManager
    @return This Context object's WsiManager object
    */
    WsiManager& get_wsi_manager();

protected:
    /**
    Creates this Context object's Instance
    @param [in] pInstanceCreateInfo Instance creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return Instance creation result
        @note This method may be overriden to customize Instance creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_instance(const VkInstanceCreateInfo* pInstanceCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Creates this Context object's DebugUtilsMessenger
    @param [in] pDebugUtilsMessengerCreateInfo :DebugUtilsMessenger creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return DebugUtilsMessenger creation result
        @note This method may be overriden to customize DebugUtilsMessenger creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_debug_utils_messenger(const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Gets this Context object's PhysicalDevice objects sorted by the rating provided by get_physical_device_rating()
    @return This Context object's PhysicalDevice objects sorted by the rating provided by get_physical_device_rating()
        @note This method may be overriden to customize PhysicalDevice sorting
    */
    virtual std::vector<PhysicalDevice> sort_physical_devices() const;

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
        @note If the base implementation is not called from this method, it must populate mDevices with at least 1 Device
        @note The Device at index 0 will be used as the parent Device for resources created by this Context
    */
    virtual VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Allocates this Context object's CommandBuffer objects
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return CommandBuffer allocation result
        @note This method may be overriden to customize CommandBuffer allocation
    */
    virtual VkResult allocate_command_buffers(const VkAllocationCallbacks* pAllocator);

    /**
    Creates this Context object's sys::Surface
    @param [in] pSysSurfaceCreateInfo sys::Surface creation parameters
    @return sys::Surface creation result
        @note This method may be overriden to customize sys::Surface creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_sys_surface(const sys::Surface::CreateInfo* pSysSurfaceCreateInfo);

    /**
    Creates this Context object's WsiManager
    @param [in] pWsiManagerCreateInfo WsiManager creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return WsiManager creation result
        @note This method may be overriden to customize WsiManager creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_wsi_manager(const WsiManager::CreateInfo* pWsiManagerCreateInfo, const VkAllocationCallbacks* pAllocator);

    Instance mInstance;
    DebugUtilsMessengerEXT mDebugUtilsMessenger;
    std::vector<Device> mDevices;
    std::vector<CommandBuffer> mCommandBuffers;
    sys::Surface mSysSurface;
    WsiManager mWsiManager;

private:
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
};

/**
Record and execute a VkCommandBuffer immediately
@typename <RecordCommandBufferFunctionType> The type of function to call to record the VkCommandBuffer
    @note The function type must accept a single VkCommandBuffer argument
@param [in] vkQueue The VkQueue to submit the recorded VkCommandBuffer to
@param [in] vkCommandBuffer The vkCommandBuffer to record and submit
@param [in] vkFence The VkFence to signal when the submited VkCommandBuffer completes execution
    @note If this argument is VK_NULL_HANDLE this call will block on vkQueueWaitIdle() after VkCommandBuffer submission
@return The VkResult
*/
template <typename RecordCommandBufferFunctionType>
inline VkResult execute_immediately(
    VkQueue vkQueue,
    VkCommandBuffer vkCommandBuffer,
    VkFence vkFence,
    RecordCommandBufferFunctionType recordCommandBuffer
)
{
    auto dispatchTable = DispatchTable::get_global_dispatch_table();
    gvk_result_scope_begin(VK_INCOMPLETE) {
        auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        assert(dispatchTable.gvkBeginCommandBuffer);
        gvk_result(dispatchTable.gvkBeginCommandBuffer(vkCommandBuffer, &commandBufferBeginInfo));
        recordCommandBuffer(vkCommandBuffer);
        assert(dispatchTable.gvkEndCommandBuffer);
        gvk_result(dispatchTable.gvkEndCommandBuffer(vkCommandBuffer));

        auto submitInfo = get_default<VkSubmitInfo>();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        assert(dispatchTable.gvkQueueSubmit);
        gvk_result(dispatchTable.gvkQueueSubmit(vkQueue, 1, &submitInfo, vkFence));

        if (!vkFence) {
            assert(dispatchTable.gvkQueueWaitIdle);
            gvk_result(dispatchTable.gvkQueueWaitIdle(vkQueue));
        }
    } gvk_result_scope_end
    return gvkResult;
}

namespace detail {

#ifdef VK_NO_PROTOTYPES
VkResult load_runtime();
void unload_runtime();
PFN_vkGetInstanceProcAddr load_get_instance_proc_addr();
#endif

} // namespace detail
} // namespace gvk

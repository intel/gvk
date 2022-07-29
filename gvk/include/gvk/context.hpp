
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

#pragma once

#include "gvk/generated/dispatch-table.hpp"
#include "gvk/system/surface.hpp"
#include "gvk/defines.hpp"
#include "gvk/handles.hpp"
#include "gvk/structures.hpp"
#include "gvk/wsi-manager.hpp"

namespace gvk {

/**
Provides high level control over gvk::Instance, gvk::Device(s)/gvk::Queue(s), gvk::WsiManager (Window System Integration) and several other utility objects
    @note gvk::Context may be extended to customize resource creation
*/
class Context
{
public:
    /**
    Creation parameters for gvk::Context
    */
    struct CreateInfo
    {
        /**
        Optional VkApplicationInfo parameters
        */
        const VkApplicationInfo* pApplicationInfo{ nullptr };

        /**
        Optional creation parameters for a gvk::sys::Surface
            @note If provided, platform specific WSI extensions will be loaded
        */
        const sys::Surface::CreateInfo* pSysSurfaceCreateInfo{ nullptr };

        /**
        Optional creation parameters for a gvk::DebugUtilsMessenger
            @note If provided, the debug utils extension to be loaded
        */
        const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo{ nullptr };
    };

    /**
    Creates an instance of gvk::Context
    @param [in] pCreateInfo A pointer to the gvk::Context creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @param [out] pContext A pointer to the gvk::Context to create
    @return The VkResult
    */
    static VkResult create(const CreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, Context* pContext);

    /**
    Constructs an instance of gvk::Context
    */
    Context() = default;

    /**
    Moves an instance of gvk::Context
    @param [in] other The gvk::Context to move from
    */
    Context(Context&& other) = default;

    /**
    Moves an instance of gvk::Context
    @param [in] other The gvk::Context to move from
    @return A reference to this gvk::Context
    */
    Context& operator=(Context&& other) = default;

    /**
    Destroys this instance of gvk::Context
    */
    virtual ~Context();

    /**
    Destroys this instance of gvk::Context
    */
    void reset();

    /**
    Gets this gvk::Context object's gvk::Instance
    @return This gvk::Context object's gvk::Instance object
    */
    const Instance& get_instance() const;

    /**
    Gets this gvk::Context object's gvk::PhysicalDevice objects
    @return This gvk::Context object's gvk::PhysicalDevice objects
    */
    std::vector<PhysicalDevice> get_physical_devices() const;

    /**
    Gets this gvk::Context object's gvk::Device objects
    @return This gvk::Context object's gvk::Device objects
    */
    const std::vector<Device>& get_devices() const;

    /**
    Gets this gvk::Context object's gvk::CommandBuffer objects
    @return This gvk::Context object's gvk::CommandBuffer objects
    */
    const std::vector<CommandBuffer>& get_command_buffers() const;

    /**
    Gets this gvk::Context object's gvk::sys::Surface
    @return This gvk::Context object's gvk::sys::Surface object
    */
    const sys::Surface& get_sys_surface() const;

    /**
    Gets this gvk::Context object's gvk::WsiManager
    @return This gvk::Context object's gvk::WsiManager object
    */
    const WsiManager& get_wsi_manager() const;

    /**
    Gets this gvk::Context object's gvk::WsiManager
    @return This gvk::Context object's gvk::WsiManager object
    */
    WsiManager& get_wsi_manager();

protected:
    /**
    Creates this gvk::Context object's gvk::Instance
    @param [in] pInstanceCreateInfo gvk::Instance creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return gvk::Instance creation result
        @note This method may be overriden to customize gvk::Instance creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_instance(const VkInstanceCreateInfo* pInstanceCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Creates this gvk::Context object's gvk::DebugUtilsMessenger
    @param [in] pDebugUtilsMessengerCreateInfo gvk::DebugUtilsMessenger creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return gvk::DebugUtilsMessenger creation result
        @note This method may be overriden to customize gvk::DebugUtilsMessenger creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_debug_utils_messenger(const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessengerCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Gets this gvk::Context object's gvk::PhysicalDevice objects sorted by the rating provided by get_physical_device_rating()
    @return This gvk::Context object's gvk::PhysicalDevice objects sorted by the rating provided by get_physical_device_rating()
        @note This method may be overriden to customize gvk::PhysicalDevice sorting
    */
    virtual std::vector<PhysicalDevice> sort_physical_devices() const;

    /**
    Gets a given gvk::PhysicalDevice object's rating
    @param [in] physicalDevice The gvk::PhysicalDevice to get the rating for
    @return The given gvk::PhysicalDevice object's rating
        @note This method may be overriden to customize device rating
    */
    virtual uint32_t get_physical_device_rating(const PhysicalDevice& physicalDevice) const;

    /**
    Creates this gvk::Context object's gvk::Device objects
    @param [in] pDeviceCreateInfo gvk::Device creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return gvk::Device creation result
        @note This method may be overriden to customize gvk::Device creation
        @note If the base implementation is not called from this method, it must populate mDevices with at least 1 gvk::Device
        @note The gvk::Device at index 0 will be used as the parent gvk::Device for resources created by this gvk::Context
    */
    virtual VkResult create_devices(const VkDeviceCreateInfo* pDeviceCreateInfo, const VkAllocationCallbacks* pAllocator);

    /**
    Allocates this gvk::Context object's gvk::CommandBuffer objects
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return gvk::CommandBuffer allocation result
        @note This method may be overriden to customize gvk::CommandBuffer allocation
    */
    virtual VkResult allocate_command_buffers(const VkAllocationCallbacks* pAllocator);

    /**
    Creates this gvk::Context object's gvk::sys::Surface
    @param [in] pSysSurfaceCreateInfo gvk::sys::Surface creation parameters
    @return gvk::sys::Surface creation result
        @note This method may be overriden to customize gvk::sys::Surface creation
        @note If this method is overriden, the base implementation must be called from the override
    */
    virtual VkResult create_sys_surface(const sys::Surface::CreateInfo* pSysSurfaceCreateInfo);

    /**
    Creates this gvk::Context object's gvk::WsiManager
    @param [in] pWsiManagerCreateInfo gvk::WsiManager creation parameters
    @param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
    @return gvk::WsiManager creation result
        @note This method may be overriden to customize gvk::WsiManager creation
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
    gvk_result_scope_begin(VK_INCOMPLETE) {
        auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        assert(gDispatchTable.gvkBeginCommandBuffer);
        gvk_result(gDispatchTable.gvkBeginCommandBuffer(vkCommandBuffer, &commandBufferBeginInfo));
        recordCommandBuffer(vkCommandBuffer);
        assert(gDispatchTable.gvkEndCommandBuffer);
        gvk_result(gDispatchTable.gvkEndCommandBuffer(vkCommandBuffer));

        auto submitInfo = get_default<VkSubmitInfo>();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        assert(gDispatchTable.gvkQueueSubmit);
        gvk_result(gDispatchTable.gvkQueueSubmit(vkQueue, 1, &submitInfo, vkFence));

        if (!vkFence) {
            assert(gDispatchTable.gvkQueueWaitIdle);
            gvk_result(gDispatchTable.gvkQueueWaitIdle(vkQueue));
        }
    } gvk_result_scope_end
    return gvkResult;
}

} // namespace gvk


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

#include "gvk-metadata-extractor-sample-utilities.hpp"
#include "gvk-layer/registry.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-containers/streambuf.hpp"
#include "gvk-environment.hpp"
#include "gvk-handles.hpp"
#include "gvk-structures.hpp"

#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker_metadata_extractor.hpp"

#include <Windows.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>

// NOTE : There's much better locking strategies than what's presented here.
//  Locks aren't even really guaranteed to encapsulate a particular frame...
//  that's kinda always true, but it's particularly true if queue submissions
//  are coming in on multiple threads...
//  BUT, beginning recording at vkQueuePresentKHR() and stopping at the next
//  vkQueuePresentKHR will result in accurate frame captures for the vast majority
//  of workloads.  To fully ensure frame accuracy, need to track which command
//  buffers contribute to the presented swapchain image.

class GvkMetadataExtractorSampleLayer final
    : public gvk::layer::BasicApiCallHandler
{
public:
    static constexpr gvk::Printer::Flags PrinterFlags { gvk::Printer::Default & ~gvk::Printer::EnumValue };

    VkResult pre_execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) override final
    {
        (void)pCreateInfo;
        (void)pAllocator;
        (void)pInstance;

        // Check for 'wait for debugger' option
        if (gvk::get_env_var_true("GVK_METADATA_EXTRACTOR_WAIT_FOR_DEBUGGER")) {
            MessageBox(0, "VK_LAYER_INTEL_gvk_metadata_extractor_sample", "Attach Debugger Now", 0);
        }

        gvk_result_scope_begin(VK_SUCCESS) {
            // Get IPC handles
            ipcReadHandle = (HANDLE)gvk::string::to_number<uint64_t>(gvk::get_env_var("GVK_METADATA_EXTRACTOR_SAMPLE_LAYER_IPC_READ_HANDLE"));
            gvk_result_assert(ipcReadHandle);
            ipcWriteHandle = (HANDLE)gvk::string::to_number<uint64_t>(gvk::get_env_var("GVK_METADATA_EXTRACTOR_SAMPLE_LAYER_IPC_WRITE_HANDLE"));
            gvk_result_assert(ipcWriteHandle);
        } gvk_result_scope_end;
        return gvkResult;
    }

    VkResult post_execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) override final
    {
        gvk_result_scope_begin(VK_SUCCESS) {

            // Load VK_LAYER_INTEL_gvk_state_tracker entry points
            gvk_result(gvk::state_tracker::load_layer_entry_points());

            // Create gvk::Instance
            // NOTE : create_unmanaged() creates a wrapper for a raw Vulkan handle.  Unlike
            //  managed gvk handles, this object's lifetime is not automatically managed via
            //  refcount.  The underlying Vulkan handle must remain valid for the lifetime of
            //  the unmanaged gvk handle, and the unmanaged gvk handle must be manually destroyed
            //  before the underlying Vulkan handle is destroyed.
            // NOTE : create_unmanaged() is only available for gvk::Instance and gvk::Device
            // NOTE : An unmanaged gvk::Instance may be used to create managed child objects
            gvk_result(gvk::Instance::create_unmanaged(pCreateInfo, pAllocator, &dispatchTable, *pInstance, &gvkInstance));

            // Send VkInstanceCreateInfo to GUI
            ipcMessenger.write(ipcWriteHandle, "VkInstanceCreateInfo", *pCreateInfo);

        } gvk_result_scope_end;
        return gvkResult;
    }

    void pre_execute_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) override final
    {
        (void)instance;
        (void)pAllocator;
        // Destroy unmanaged gvk::Instance
        gvkInstance = gvk::nullref;
    }

    VkResult post_execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) override final
    {
        std::lock_guard<std::mutex> lock(mutex);
        gvk_result_scope_begin(VK_SUCCESS) {

            // Create gvk::Device
            // NOTE : create_unmanaged() creates a wrapper for a raw Vulkan handle.  Unlike
            //  managed gvk handles, this object's lifetime is not automatically managed via
            //  refcount.  The underlying Vulkan handle must remain valid for the lifetime of
            //  the unmanaged gvk handle, and the unmanaged gvk handle must be manually destroyed
            //  before the underlying Vulkan handle is destroyed.
            // NOTE : create_unmanaged() is only available for gvk::Instance and gvk::Device
            // NOTE : An unmanaged gvk::Device may be used to create managed child objects
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, pAllocator, &dispatchTable, *pDevice, &gvkDevice));
            gvk_result_assert(gvkDevices.insert(gvkDevice).second);

            // Send VkDeviceCreateInfo to GUI
            ipcMessenger.write(ipcWriteHandle, "VkDeviceCreateInfo", *pCreateInfo);
        } gvk_result_scope_end;
        return gvkResult;
    }

    void pre_execute_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override final
    {
        (void)pAllocator;
        std::lock_guard<std::mutex> lock(mutex);
        // Destroy unmanaged gvk::Device
        gvkDevices.erase(device);
    }

    VkResult pre_execute_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override final
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (refreshCmds) {

            // Add queue submission to metadata extractor timeline
            auto command = gvk::get_default<GvkCommandStructureQueueSubmit>();
            command.queue = queue;
            command.submitCount = submitCount;
            command.pSubmits = pSubmits;
            command.fence = fence;
            metadataExtractor.add_command((const GvkCommandBaseStructure&)command);

            // Process submitted command buffers
            for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
                for (uint32_t commandBuffer_i = 0; commandBuffer_i < pSubmits[submit_i].commandBufferCount; ++commandBuffer_i) {
                    enumerate_command_buffer_cmds(pSubmits[submit_i].pCommandBuffers[commandBuffer_i]);
                }
            }
        }
        return VK_SUCCESS;
    }

    VkResult pre_execute_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override final
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (refreshCmds) {

            // Add queue submission to metadata extractor timeline
            auto command = gvk::get_default<GvkCommandStructureQueueSubmit2>();
            command.queue = queue;
            command.submitCount = submitCount;
            command.pSubmits = pSubmits;
            command.fence = fence;
            metadataExtractor.add_command((const GvkCommandBaseStructure&)command);

            // Process submitted command buffers
            for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
                for (uint32_t commandBuffer_i = 0; commandBuffer_i < pSubmits[submit_i].commandBufferInfoCount; ++commandBuffer_i) {
                    enumerate_command_buffer_cmds(pSubmits[submit_i].pCommandBufferInfos[commandBuffer_i].commandBuffer);
                }
            }
        }
        return VK_SUCCESS;
    }

    VkResult pre_execute_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override final
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (refreshCmds) {

            // Add queue submission to metadata extractor timeline
            auto command = gvk::get_default<GvkCommandStructureQueueSubmit2KHR>();
            command.queue = queue;
            command.submitCount = submitCount;
            command.pSubmits = pSubmits;
            command.fence = fence;
            metadataExtractor.add_command((const GvkCommandBaseStructure&)command);

            // Process submitted command buffers
            for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
                for (uint32_t commandBuffer_i = 0; commandBuffer_i < pSubmits[submit_i].commandBufferInfoCount; ++commandBuffer_i) {
                    enumerate_command_buffer_cmds(pSubmits[submit_i].pCommandBufferInfos[commandBuffer_i].commandBuffer);
                }
            }
        }
        return VK_SUCCESS;
    }

    void enumerate_command_buffer_cmds(VkCommandBuffer commandBuffer)
    {
        // Setup GvkStateTrackedObject
        auto stateTrackedCommandBuffer = gvk::get_default<GvkStateTrackedObject>();
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)commandBuffer;
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)commandBuffer;

        // Setup GvkStateTrackedObjectEnumerateInfo
        auto stateTrackedObjectEnumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
        stateTrackedObjectEnumerateInfo.pfnCallback = enumerate_command_buffer_cmds_callback;
        stateTrackedObjectEnumerateInfo.pUserData = this;

        // Enumerate recorded command buffer cmds
        gvkEnumerateStateTrackedCommandBufferCmds(&stateTrackedCommandBuffer, &stateTrackedObjectEnumerateInfo);
    }

    static void enumerate_command_buffer_cmds_callback(const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)
    {
        (void)pStateTrackedObject;
        assert(pInfo);
        assert(pUserData);
        auto pMetadataExtractorSampleLayer = (GvkMetadataExtractorSampleLayer*)pUserData;

        // Add recorded cmd to metadata extractor timeline
        pMetadataExtractorSampleLayer->metadataExtractor.add_command(*(GvkCommandBaseStructure*)pInfo);
    }

    VkResult pre_execute_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) override final
    {
        std::lock_guard<std::mutex> lock(mutex);
        gvk_result_scope_begin(VK_SUCCESS) {

            if (refreshCmds) {
                refreshCmds = false;

                // Add queue present to metadata extractor timeline
                auto command = gvk::get_default<GvkCommandStructureQueuePresentKHR>();
                command.queue = queue;
                command.pPresentInfo = pPresentInfo;
                metadataExtractor.add_command((const GvkCommandBaseStructure&)command);

                // Send metadata extractor timeline to the GUI
                gvk_result(VK_SUCCESS);
                const auto& commands = metadataExtractor.get_commands();
                auto commandCollection = gvk::get_default<GvkCommandCollection>();
                commandCollection.commandCount = (uint32_t)commands.size();
                commandCollection.ppCommands = commands.data();
                ipcMessenger.write(ipcWriteHandle, "GvkCommandCollection", commandCollection);
            }

            // Check for messages from the GUI
            for (const auto& message : ipcMessenger.read(ipcReadHandle)) {

                // On "Refresh Cmds" clear metadata extractor to start tracking the next frame
                if (message.text == "Refresh Cmds") {
                    refreshCmds = true;
                    metadataExtractor.reset();

                // On "Refresh Binding Info" send the binding state at the requested point on the
                //  metadata extractor timeline to the GUI
                } else if (gvk::string::starts_with(message.text, "Refresh Binding Info")) {
                    auto selectedCmd = gvk::string::to_number<uint64_t>(gvk::string::remove(message.text, "Refresh Binding Info "));
                    ipcMessenger.write(ipcWriteHandle, "GvkBindingInfo", metadataExtractor.get_binding_info(selectedCmd));
                }
            }
        } gvk_result_scope_end;
        return gvkResult;
    }

    gvk::Instance gvkInstance;
    std::set<gvk::Device> gvkDevices;
    gvk::MetadataExtractor metadataExtractor;
    HANDLE ipcReadHandle{ };
    HANDLE ipcWriteHandle{ };
    GvkMetadataExtractorSampleIpcMessenger ipcMessenger;
    bool refreshCmds{ };
    std::mutex mutex;
};

namespace gvk {
namespace layer {

void on_load(const VkInstanceCreateInfo* pInstanceCreateInfo, Registry& registry)
{
    (void)pInstanceCreateInfo;
    registry.apiCallHandler = std::make_unique<GvkMetadataExtractorSampleLayer>();
}

} // namespace layer
} // namespace gvk

#ifdef __cplusplus
extern "C" {
#endif

VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

#ifdef __cplusplus
}
#endif

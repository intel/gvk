
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
#include "gvk-restore-point/generated/basic-applier.hpp"
#include "gvk-restore-point/copy-engine.hpp"
#include "gvk-layer/log.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace gvk {
namespace restore_point {

class Applier final
    : public BasicApplier
{
public:
    VkResult apply_restore_point(const ApplyInfo& applyInfo) override final;
    VkResult register_restored_object(const GvkStateTrackedObject& capturedObject, const GvkStateTrackedObject& restoredObject) override final;
    void register_restored_object_destruction(const GvkStateTrackedObject& restoredObject) override final;
    GvkStateTrackedObject get_restored_object(const GvkStateTrackedObject& restorePointObject) override final;

protected:
    class AccelerationStructureSerializationResources final
    {
    public:
        AccelerationStructureSerializationResources() = default;
        ~AccelerationStructureSerializationResources();
        VkResult validate(const DispatchTable& dispatchTable, VkDevice vkDevice, const GvkAccelerationStructureSerilizationInfoKHR& serializationInfo);
        VkBuffer get_buffer() const;
        VkDeviceMemory get_device_memory() const;
        void reset();

    private:
        DispatchTable mDispatchTable;
        VkDevice mVkDevice{ };
        VkBuffer mVkBuffer{ };
        VkDeviceMemory mVkDeviceMemory{ };

        AccelerationStructureSerializationResources(const AccelerationStructureSerializationResources&) = delete;
        AccelerationStructureSerializationResources& operator=(const AccelerationStructureSerializationResources&) = delete;
    };

    VkResult restore_object(const GvkStateTrackedObject& restorePointObject) override final;
    VkResult restore_object_state(const GvkStateTrackedObject& restorePointObject) override final;
    VkResult restore_object_name(const GvkStateTrackedObject& restoredObject, uint32_t dependencyCount, const GvkStateTrackedObject* pDependencies, const char* pName) override final;
    VkResult restore_object_data(const GvkStateTrackedObject& capturedObject);
    void destroy_object(const GvkStateTrackedObject& restorePointObject) override final;

    // VkInstance
    VkResult restore_VkInstance(const GvkStateTrackedObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkInstance_state(const GvkStateTrackedObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo) override final;

    // VkPhysicalDevice
    VkResult restore_VkPhysicalDevice(const GvkStateTrackedObject& restorePointObject, const GvkPhysicalDeviceRestoreInfo& restoreInfo) override final;

    // VkDevice
    VkResult restore_VkDevice(const GvkStateTrackedObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkDevice_state(const GvkStateTrackedObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo) override final;

    // VkCommandBuffer
#if 0
    VkResult restore_VkCommandBuffer(const GvkStateTrackedObject& restorePointObject, const GvkCommandBufferRestoreInfo& restoreInfo) override final;
    void destroy_VkCommandBuffer(const GvkStateTrackedObject& restorePointObject) override final;
#endif
    VkResult restore_VkCommandBuffer_cmds(const GvkStateTrackedObject& restorePointObject);

    // VkDescriptorSet
    VkResult restore_VkDescriptorSet(const GvkStateTrackedObject& restorePointObject, const GvkDescriptorSetRestoreInfo& restoreInfo) override final;
    void destroy_VkDescriptorSet(const GvkStateTrackedObject& restorePointObject) override final;
    VkResult restore_VkDescriptorSet_bindings(const GvkStateTrackedObject& restorePointObject);

    // VkEvent
    VkResult restore_VkEvent_state(const GvkStateTrackedObject& restorePointObject, const GvkEventRestoreInfo& restoreInfo) override final;

    // VkFence
    VkResult restore_VkFence_state(const GvkStateTrackedObject& restorePointObject, const GvkFenceRestoreInfo& restoreInfo) override final;

    // VkSemaphore
    VkResult restore_VkSemaphore_state(const GvkStateTrackedObject& restorePointObject, const GvkSemaphoreRestoreInfo& restoreInfo) override final;

    // VkSwapchainKHR
    VkResult restore_VkSwapchainKHR(const GvkStateTrackedObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo) override final;
    VkResult restore_VkSwapchainKHR_state(const GvkStateTrackedObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo) override final;
    void destroy_VkSwapchainKHR(const GvkStateTrackedObject& restorePointObject) override final;

    // VkDeviceMemory
    VkResult restore_VkDeviceMemory(const GvkStateTrackedObject& restorePointObject, const GvkDeviceMemoryRestoreInfo& restoreInfo) override final;
    VkResult restore_VkDeviceMemory_data(const GvkStateTrackedObject& restorePointObject);
    static void process_VkDeviceMemory_data_upload(const CopyEngine::UploadDeviceMemoryInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult restore_VkDeviceMemory_mapping(const GvkStateTrackedObject& restorePointObject);

    // VkAccelerationStructureKHR
    VkResult restore_VkAccelerationStructureKHR_data();
    VkResult restore_VkAccelerationStructureKHR_data(const GvkStateTrackedObject& restorePointObject, AccelerationStructureSerializationResources& accelerationStructureSerializationResources);
    static void process_VkAccelerationStructureKHR_data_upload(const CopyEngine::UploadAccelerationStructureInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult process_VkAccelerationStructureKHR_builds(const GvkStateTrackedObject& restorePointObject);
    void apply_VkAccelerationStructure_restore_point(const std::vector<GvkStateTrackedObject>& capturedAccelerationStructures);

    // VkBuffer
    VkResult restore_VkBuffer(const GvkStateTrackedObject& restorePointObject, const GvkBufferRestoreInfo& restoreInfo) override final;
    VkResult restore_VkBuffer_data(const GvkStateTrackedObject& restorePointObject);
    static void process_VkBuffer_data_upload(const CopyEngine::UploadBufferInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);

    // VkImage
    VkResult restore_VkImage(const GvkStateTrackedObject& restorePointObject, const GvkImageRestoreInfo& restoreInfo) override final;
    VkResult restore_VkImage_layouts(const GvkStateTrackedObject& restorePointObject);
    VkResult restore_VkImage_data(const GvkStateTrackedObject& restorePointObject);
    static void process_VkImage_data_upload(const CopyEngine::UploadImageInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult process_VkImage_layouts(const GvkStateTrackedObject& restorePointObject);

    VkResult process_GvkCommandStructureAllocateCommandBuffers(const GvkStateTrackedObject& restorePointObject, const GvkCommandBufferRestoreInfo& restoreInfo, GvkCommandStructureAllocateCommandBuffers& commandStructure) override final;
    VkResult process_GvkCommandStructureCreateComputePipelines(const GvkStateTrackedObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateComputePipelines& commandStructure) override final;
    VkResult process_GvkCommandStructureCreateGraphicsPipelines(const GvkStateTrackedObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateGraphicsPipelines& commandStructure) override final;
    VkResult process_GvkCommandStructureCreateRayTracingPipelinesKHR(const GvkStateTrackedObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateRayTracingPipelinesKHR& commandStructure) override final;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult process_GvkCommandStructureCreateWin32SurfaceKHR(const GvkStateTrackedObject& restorePointObject, const GvkSurfaceRestoreInfoKHR& restoreInfo, GvkCommandStructureCreateWin32SurfaceKHR& commandStructure) override final;
#endif // VK_USE_PLATFORM_WIN32_KHR

private:
    Instance mInstance;
    std::set<Device> mDevices;
    VkDeviceMemory mAccelerationStructureSerializationMemory{ };
    DispatchTable mApplicationDispatchTable{ };
    std::map<VkPhysicalDeviceProperties, std::vector<VkPhysicalDevice>> mUnrestoredPhysicalDevices;
    std::unordered_map<VkDevice, CopyEngine> mCopyEngines;
    std::map<VkDevice, CommandPool> mCommandPools;
    std::map<VkDevice, VkCommandBuffer> mVkCommandBuffers;
    std::map<VkDevice, Fence> mFences;
    std::map<VkDevice, Auto<GvkDeviceRestoreInfo>> mDeviceRestoreInfos;
    layer::Log mLog;
};

} // namespace state_tracker
} // namespace gvk


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
#include "gvk-restore-point/logger.hpp"

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
    void apply_VkAccelerationStructure_restore_point(const std::vector<GvkRestorePointObject>& capturedAccelerationStructures);
    VkResult register_restored_object_ex(const GvkRestorePointObject& capturedObject, const GvkRestorePointObject& restoredObject) override final;

protected:
    VkResult restore_object(const GvkRestorePointObject& restorePointObject) override final;
    VkResult restore_object_state(const GvkRestorePointObject& restorePointObject) override final;
    VkResult restore_object_name(const GvkRestorePointObject& restoredObject, uint32_t dependencyCount, const GvkRestorePointObject* pDependencies, const char* pName) override final;
    VkResult restore_VkInstance(const GvkRestorePointObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkInstance_state(const GvkRestorePointObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkDevice(const GvkRestorePointObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkDevice_state(const GvkRestorePointObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkDescriptorSet(const GvkRestorePointObject& restorePointObject, const GvkDescriptorSetRestoreInfo& restoreInfo) override final;
    void destroy_VkDescriptorSet(const GvkRestorePointObject& restorePointObject) override final;
    VkResult restore_VkEvent_state(const GvkRestorePointObject& restorePointObject, const GvkEventRestoreInfo& restoreInfo) override final;
    VkResult restore_VkFence_state(const GvkRestorePointObject& restorePointObject, const GvkFenceRestoreInfo& restoreInfo) override final;
    VkResult restore_VkSemaphore_state(const GvkRestorePointObject& restorePointObject, const GvkSemaphoreRestoreInfo& restoreInfo) override final;
    VkResult restore_VkSwapchainKHR(const GvkRestorePointObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo) override final;
    VkResult restore_VkSwapchainKHR_state(const GvkRestorePointObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo) override final;
    void destroy_VkSwapchainKHR(const GvkRestorePointObject& restorePointObject) override final;
    VkResult process_VkInstance(const GvkRestorePointObject& restorePointObject, const GvkInstanceRestoreInfo& restoreInfo) override final;
    VkResult process_VkPhysicalDevice(const GvkRestorePointObject& restorePointObject, const GvkPhysicalDeviceRestoreInfo& restoreInfo) override final;
    VkResult process_VkDevice(const GvkRestorePointObject& restorePointObject, const GvkDeviceRestoreInfo& restoreInfo) override final;
    VkResult process_VkDeviceMemory(const GvkRestorePointObject& restorePointObject, const GvkDeviceMemoryRestoreInfo& restoreInfo) override final;
    VkResult process_VkAccelerationStructureKHR(const GvkRestorePointObject& restorePointObject, const GvkAccelerationStructureRestoreInfoKHR& restoreInfo) override final;
    VkResult process_VkBuffer(const GvkRestorePointObject& restorePointObject, const GvkBufferRestoreInfo& restoreInfo) override final;
    VkResult process_VkImage(const GvkRestorePointObject& restorePointObject, const GvkImageRestoreInfo& restoreInfo) override final;
    VkResult process_VkDescriptorSet(const GvkRestorePointObject& restorePointObject, const GvkDescriptorSetRestoreInfo& restoreInfo) override final;
    VkResult process_VkEvent(const GvkRestorePointObject& restorePointObject, const GvkEventRestoreInfo& restoreInfo) override final;
    VkResult process_VkFence(const GvkRestorePointObject& restorePointObject, const GvkFenceRestoreInfo& restoreInfo) override final;
    VkResult process_VkSemaphore(const GvkRestorePointObject& restorePointObject, const GvkSemaphoreRestoreInfo& restoreInfo) override final;
    VkResult process_VkSwapchainKHR(const GvkRestorePointObject& restorePointObject, const GvkSwapchainRestoreInfoKHR& restoreInfo) override final;
    VkResult process_GvkCommandStructureAllocateCommandBuffers(const GvkRestorePointObject& restorePointObject, const GvkCommandBufferRestoreInfo& restoreInfo, GvkCommandStructureAllocateCommandBuffers& commandStructure) override final;
    VkResult process_GvkCommandStructureCreateComputePipelines(const GvkRestorePointObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateComputePipelines& commandStructure) override final;
    VkResult process_GvkCommandStructureCreateGraphicsPipelines(const GvkRestorePointObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateGraphicsPipelines& commandStructure) override final;
    VkResult process_GvkCommandStructureCreateRayTracingPipelinesKHR(const GvkRestorePointObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateRayTracingPipelinesKHR& commandStructure) override final;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult process_GvkCommandStructureCreateWin32SurfaceKHR(const GvkRestorePointObject& restorePointObject, const GvkSurfaceRestoreInfoKHR& restoreInfo, GvkCommandStructureCreateWin32SurfaceKHR& commandStructure) override final;
#endif // VK_USE_PLATFORM_WIN32_KHR
    VkResult restore_VkImage_layouts(const GvkRestorePointObject& restorePointObject);
    VkResult process_VkDeviceMemory_data(const GvkRestorePointObject& restorePointObject);
    static void process_VkDeviceMemory_data_upload(const CopyEngine::UploadDeviceMemoryInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult process_VkAccelerationStructureKHR_data(const GvkRestorePointObject& restorePointObject);
    static void process_VkAccelerationStructureKHR_data_upload(const CopyEngine::UploadAccelerationStructureInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult process_VkBuffer_data(const GvkRestorePointObject& restorePointObject);
    static void process_VkBuffer_data_upload(const CopyEngine::UploadBufferInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult process_VkImage_data(const GvkRestorePointObject& restorePointObject);
    static void process_VkImage_data_upload(const CopyEngine::UploadImageInfo& uploadInfo, const VkBindBufferMemoryInfo& bindBufferMemoryInfo, uint8_t* pData);
    VkResult process_VkImage_layouts(const GvkRestorePointObject& restorePointObject);
    VkResult process_VkAccelerationStructureKHR_builds(const GvkRestorePointObject& restorePointObject);
    VkResult process_VkDeviceMemory_mapping(const GvkRestorePointObject& restorePointObject);
    VkResult process_VkDescriptorSet_bindings(const GvkRestorePointObject& restorePointObject);
    VkResult process_VkCommandBuffer_cmds(const GvkRestorePointObject& restorePointObject);
    VkResult process_transient_objects();

private:
    Instance mInstance;
    std::set<Device> mDevices;
    VkBuffer mAccelerationStructureSerializationBuffer{ };
    VkDeviceMemory mAccelerationStructureSerializationMemory{ };
    DispatchTable mApplicationDispatchTable{ };
    std::map<VkPhysicalDeviceProperties, std::vector<VkPhysicalDevice>> mUnrestoredPhysicalDevices;
    std::unordered_map<VkDevice, CopyEngine> mCopyEngines;
    std::map<VkDevice, CommandPool> mCommandPools;
    std::map<VkDevice, VkCommandBuffer> mVkCommandBuffers;
    std::map<VkDevice, Fence> mFences;
    Log mLog;
};

} // namespace state_tracker
} // namespace gvk


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

#include "gvk-state-tracker/device-address-tracker.hpp"
#include "gvk-state-tracker/generated/basic-state-tracker.hpp"
#define VK_LAYER_INTEL_gvk_state_tracker_hpp_OMIT_ENTRY_POINT_DECLARATIONS
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include <unordered_map>

namespace gvk {
namespace state_tracker {

class StateTracker final
    : public BasicStateTracker
{
public:
    enum class PhysicalDeviceEnumerationMode
    {
        Application,
        Loader,
    };

    void set_state_tracked_object_name(const GvkStateTrackedObject* pStateTrackedObject, const char* pName);
    VkResult post_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/acceleration-structure.cpp
    VkResult pre_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult) override final;
    VkResult post_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult) override final;
    VkResult pre_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, VkResult gvkResult) override final;
    VkResult post_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, VkResult gvkResult) override final;
    VkResult process_build_acceleration_structures(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/buffer.cpp
    VkResult pre_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult gvkResult) override final;
    VkResult post_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult gvkResult) override final;
    VkDeviceAddress post_vkGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress gvkResult) override final;
    VkDeviceAddress post_vkGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress gvkResult) override final;
    VkDeviceAddress post_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress gvkResult) override final;
    void post_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/command-buffer.cpp
    VkResult post_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult) override final;
    VkResult post_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult gvkResult) override final;
    void post_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) override final;
    VkResult post_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo, VkResult gvkResult) override final;
    VkResult post_vkEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult gvkResult) override final;
    VkResult post_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/descriptor-set.cpp
    VkResult post_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout, VkResult gvkResult) override final;
    VkResult post_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult gvkResult) override final;
    void post_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) override final;
    VkResult post_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, VkResult gvkResult) override final;
    VkResult post_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, VkResult gvkResult) override final;
    void post_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) override final;
    void post_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) override final;
    void post_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) override final;
    void write_descriptor_sets(VkDevice vkDevice, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites);
    void copy_descriptor_sets(VkDevice vkDevice, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/device.cpp
    VkResult pre_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult) override final;
    VkResult post_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/device-memory.cpp
    VkResult pre_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult) override final;
    VkResult post_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult) override final;
    void post_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) override final;
    VkResult post_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData, VkResult gvkResult) override final;
    void post_vkUnmapMemory(VkDevice device, VkDeviceMemory memory) override final;
    VkResult post_vkBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos, VkResult gvkResult) override final;
    VkResult post_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult gvkResult) override final;
    VkResult post_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult gvkResult) override final;
    VkResult post_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult gvkResult) override final;
    VkResult post_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult gvkResult) override final;
    VkResult post_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult gvkResult) override final;
    VkResult post_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult gvkResult) override final;
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VkResult post_vkBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos, VkResult gvkResult) override final;
#endif // VK_ENABLE_BETA_EXTENSIONS

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/framebuffer.cpp
    VkResult post_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/image.cpp
    VkResult pre_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult gvkResult) override final;
    VkResult post_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult gvkResult) override final;
    void post_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/instance.cpp
    VkResult post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/pipeline.cpp
    VkResult post_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, VkResult gvkResult) override final;
    VkResult post_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/queue.cpp
    VkResult post_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence, VkResult gvkResult) override final;
    VkResult post_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult gvkResult) override final;
    VkResult post_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult) override final;
    VkResult post_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult) override final;
    VkResult post_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/query-pool.cpp
#if 0
    VkResult post_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool, VkResult gvkResult) override final;
    void post_vkResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) override final;
    void post_vkResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) override final;
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/semaphore.cpp
    VkResult post_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult gvkResult) override final;
    VkResult post_vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo, VkResult gvkResult) override final;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult post_vkImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, VkResult gvkResult) override final;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_FUCHSIA
    VkResult post_vkImportSemaphoreZirconHandleFUCHSIA(VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo, VkResult gvkResult) override final;
#endif // VK_USE_PLATFORM_FUCHSIA
    VkResult post_vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult gvkResult) override final;
    VkResult post_vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult gvkResult) override final;
    VkResult post_vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult gvkResult) override final;
    VkResult post_vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/shader.cpp
    VkResult post_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/swapchain.cpp
    VkResult pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult) override final;
    VkResult post_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult) override final;
    VkResult post_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult gvkResult) override final;
    VkResult post_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult gvkResult) override final;
    VkResult post_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout, VkResult gvkResult) override final;
    VkResult post_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo, VkResult gvkResult) override final;
    void post_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Defined in /source/gvk-state-tracker/validation-cache.cpp
    VkResult post_vkCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache, VkResult gvkResult) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // Exported entry points
    static PhysicalDeviceEnumerationMode get_physical_device_enumeration_mode();
    static void set_physical_device_enumeration_mode(PhysicalDeviceEnumerationMode physicalDeviceRetrievalMode);
    static VkPhysicalDevice get_loader_physical_device_handle(VkPhysicalDevice applicationVkPhyicalDevice);
    static void get_state_tracker_physical_device(VkInstance instance, VkPhysicalDevice physicalDevice, VkPhysicalDevice* pStateTrackerPhysicalDevice);
    static void enumerate_state_tracked_objects(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
    static void enumerate_state_tracked_object_dependencies(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
    static void enumerate_state_tracked_object_bindings(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
    static void enumerate_state_tracked_command_buffer_cmds(const GvkStateTrackedObject* pStateTrackedObject, const GvkStateTrackedObjectEnumerateInfo* pEnumerateInfo);
    static void get_state_tracked_object_info(const GvkStateTrackedObject* pStateTrackedObject, GvkStateTrackedObjectInfo* pStateTrackedObjectInfo);
    static void get_state_tracked_object_create_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo);
    static void get_state_tracked_object_allocate_info(const GvkStateTrackedObject* pStateTrackedObject, VkStructureType* pAllocateInfoType, VkBaseOutStructure* pAllocateInfo);
    static void get_state_tracked_image_layouts(const GvkStateTrackedObject* pStateTrackedImage, const VkImageSubresourceRange* pImageSubresourceRange, VkImageLayout* pImageLayouts);
    static void get_state_tracked_mapped_memory(const GvkStateTrackedObject* pStateTrackedDeviceMemory, VkDeviceSize* pOffset, VkDeviceSize* pSize, VkMemoryMapFlags* pFlags, void** ppData);
    static void gvk_state_tracked_accleration_structure_build_info(const GvkStateTrackedObject* pStateTrackedAcclerationStructure, VkAccelerationStructureBuildGeometryInfoKHR* pBuildGeometryInfo, VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos);

private:
    static PhysicalDeviceEnumerationMode smPhysicalDeviceEnumerationMode;
};

} // namespace state_tracker
} // namespace gvk

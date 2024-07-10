
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
#include "gvk-restore-point/generated/basic-layer.hpp"
#include "VK_LAYER_INTEL_gvk_restore_point.h"

#include <mutex>
#include <set>
#include <unordered_map>

namespace gvk {
namespace restore_point {

class Layer final
    : public BasicLayer
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/instance.cpp
    VkResult pre_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult) override final;
    VkResult post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/device.cpp
    VkResult pre_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult) override final;
    VkResult post_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/device-memory.cpp
    VkResult pre_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult gvkResult) override final;
    VkResult pre_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData, VkResult gvkResult) override final;
    VkResult post_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData, VkResult gvkResult) override final;
    VkResult pre_vkMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData, VkResult gvkResult) override final;
    VkResult post_vkMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData, VkResult gvkResult) override final;
    void pre_vkUnmapMemory(VkDevice device, VkDeviceMemory memory) override final;
    void post_vkUnmapMemory(VkDevice device, VkDeviceMemory memory) override final;
    VkResult pre_vkUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo, VkResult gvkResult) override final;
    VkResult post_vkUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/acceleration-structure.cpp
    VkResult pre_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/buffer.cpp
    VkResult pre_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/image.cpp
    VkResult pre_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/command-buffer.cpp
    void pre_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) override final;
    void post_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) override final;
    VkResult pre_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult) override final;
    VkResult post_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult gvkResult) override final;
    VkResult pre_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult) override final;
    VkResult post_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult gvkResult) override final;
    VkResult pre_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult gvkResult) override final;
    VkResult post_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult gvkResult) override final;
    void pre_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) override final;
    void post_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/descriptor-set.cpp
    void pre_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) override final;
    void post_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) override final;
    VkResult pre_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult gvkResult) override final;
    VkResult post_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult gvkResult) override final;
    VkResult pre_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, VkResult gvkResult) override final;
    VkResult post_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, VkResult gvkResult) override final;
    VkResult pre_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, VkResult gvkResult) override final;
    VkResult post_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/pipeline.cpp
    VkResult pre_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VkResult pre_vkCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
#endif // VK_ENABLE_BETA_EXTENSIONS
    VkResult pre_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult pre_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult pre_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;
    VkResult post_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/shader.cpp
    VkResult pre_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, VkResult gvkResult) override final;
    VkResult post_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/swapchain.cpp
    VkResult pre_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult gvkResult) override final;
    VkResult pre_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult gvkResult) override final;
    VkResult post_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // NOTE : Defined in gvk/gvk-restore-point/source/gvk-restore-point/handles/queue.cpp
    VkResult process_queue_submission(VkCommandBuffer vkCommandBuffer);
    VkResult pre_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult gvkResult) override final;
    VkResult pre_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult) override final;
    VkResult pre_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, VkResult gvkResult) override final;

    ///////////////////////////////////////////////////////////////////////////////
    // Exported entry points
    static VkResult create_restore_point(VkInstance instance, const GvkRestorePointCreateInfo* pCreateInfo, GvkRestorePoint* pRestorePoint);
    static VkResult apply_restore_point(VkInstance instance, const GvkRestorePointApplyInfo* pApplyInfo, GvkRestorePoint restorePoint);
    static VkResult get_restore_point_manifest(VkInstance instance, GvkRestorePoint restorePoint, GvkRestorePointManifest* pManifest);
    static void destroy_restore_point(VkInstance instance, GvkRestorePoint restorePoint);

private:
    std::unordered_map<uint64_t, size_t> mDeviceAddressCreationCallIndices;
    std::mutex mLiveObjectMutex;
    std::set<GvkStateTrackedObject> mLiveObjects;
};

} // namespace state_tracker
} // namespace gvk

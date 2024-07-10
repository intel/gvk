
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

#include "gvk-restore-point/applier.hpp"
#include "gvk-restore-point/creator.hpp"
#include "gvk-restore-point/layer.hpp"

namespace gvk {
namespace restore_point {

VkResult Layer::pre_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)device;
    (void)pipelineCache;
    (void)createInfoCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pPipelines;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)pipelineCache;
    (void)pCreateInfos;
    (void)pAllocator;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        assert(pPipelines);
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            auto stateTrackedPipeline = get_default<GvkStateTrackedObject>();
            stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
            stateTrackedPipeline.handle = (uint64_t)pPipelines[i];
            stateTrackedPipeline.dispatchableHandle = (uint64_t)device;
            gvkRestorePoint->createdObjects.insert(stateTrackedPipeline);
        }
    }
    return gvkResult;
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
VkResult Layer::pre_vkCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)device;
    (void)pipelineCache;
    (void)createInfoCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pPipelines;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)pipelineCache;
    (void)pCreateInfos;
    (void)pAllocator;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        assert(pPipelines);
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            auto stateTrackedPipeline = get_default<GvkStateTrackedObject>();
            stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
            stateTrackedPipeline.handle = (uint64_t)pPipelines[i];
            stateTrackedPipeline.dispatchableHandle = (uint64_t)device;
            gvkRestorePoint->createdObjects.insert(stateTrackedPipeline);
        }
    }
    return gvkResult;
}
#endif // VK_ENABLE_BETA_EXTENSIONS

VkResult Layer::pre_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)device;
    (void)pipelineCache;
    (void)createInfoCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pPipelines;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)pipelineCache;
    (void)pCreateInfos;
    (void)pAllocator;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        assert(pPipelines);
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            auto stateTrackedPipeline = get_default<GvkStateTrackedObject>();
            stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
            stateTrackedPipeline.handle = (uint64_t)pPipelines[i];
            stateTrackedPipeline.dispatchableHandle = (uint64_t)device;
            gvkRestorePoint->createdObjects.insert(stateTrackedPipeline);
        }
    }
    return gvkResult;
}

VkResult Layer::pre_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)device;
    (void)deferredOperation;
    (void)pipelineCache;
    (void)createInfoCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pPipelines;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)deferredOperation;
    (void)pipelineCache;
    (void)pCreateInfos;
    (void)pAllocator;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        assert(pPipelines);
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            auto stateTrackedPipeline = get_default<GvkStateTrackedObject>();
            stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
            stateTrackedPipeline.handle = (uint64_t)pPipelines[i];
            stateTrackedPipeline.dispatchableHandle = (uint64_t)device;
            gvkRestorePoint->createdObjects.insert(stateTrackedPipeline);
        }
    }
    return gvkResult;
}

VkResult Layer::pre_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)device;
    (void)pipelineCache;
    (void)createInfoCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pPipelines;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    (void)pipelineCache;
    (void)pCreateInfos;
    (void)pAllocator;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        assert(pPipelines);
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            auto stateTrackedPipeline = get_default<GvkStateTrackedObject>();
            stateTrackedPipeline.type = VK_OBJECT_TYPE_PIPELINE;
            stateTrackedPipeline.handle = (uint64_t)pPipelines[i];
            stateTrackedPipeline.dispatchableHandle = (uint64_t)device;
            gvkRestorePoint->createdObjects.insert(stateTrackedPipeline);
        }
    }
    return gvkResult;
}

VkResult Applier::process_GvkCommandStructureCreateComputePipelines(const GvkStateTrackedObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateComputePipelines& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    commandStructure.pipelineCache = VK_NULL_HANDLE;
    commandStructure.createInfoCount = 1;
    return VK_SUCCESS;
}

VkResult Applier::process_GvkCommandStructureCreateGraphicsPipelines(const GvkStateTrackedObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateGraphicsPipelines& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    commandStructure.pipelineCache = VK_NULL_HANDLE;
    commandStructure.createInfoCount = 1;
    return VK_SUCCESS;
}

VkResult Applier::process_GvkCommandStructureCreateRayTracingPipelinesKHR(const GvkStateTrackedObject& restorePointObject, const GvkPipelineRestoreInfo& restoreInfo, GvkCommandStructureCreateRayTracingPipelinesKHR& commandStructure)
{
    // TODO : Reset after processing
    (void)restorePointObject;
    (void)restoreInfo;
    commandStructure.pipelineCache = VK_NULL_HANDLE;
    commandStructure.createInfoCount = 1;
    return VK_SUCCESS;
}

} // namespace restore_point
} // namespace gvk

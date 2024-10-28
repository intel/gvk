
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
#include "gvk-layer/registry.hpp"
#include "gvk-restore-point/creator.hpp"
#include "gvk-restore-point/layer.hpp"
#include "gvk-structures/detail/cerealization-utilities.hpp"

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
    (void)deferredOperation;
    (void)pipelineCache;
    (void)pAllocator;
    (void)pPipelines;
    if (gvkResult == VK_SUCCESS) {
        assert(pCreateInfos);

        // Get VkPhysicalDevice from state tracker
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        auto stateTrackedDevice = get_default<GvkStateTrackedObject>();
        stateTrackedDevice.type = VK_OBJECT_TYPE_DEVICE;
        stateTrackedDevice.handle = (uint64_t)device;
        stateTrackedDevice.dispatchableHandle = (uint64_t)device;
        auto enumerateInfo = get_default<GvkStateTrackedObjectEnumerateInfo>();
        enumerateInfo.pUserData = &physicalDevice;
        enumerateInfo.pfnCallback = [](GvkStateTrackedObject const* pStateTrackedObject, VkBaseInStructure const*, void* pUserData)
        {
            assert(pStateTrackedObject);
            assert(pUserData);
            if (pStateTrackedObject->type == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
                *(VkPhysicalDevice*)pUserData = (VkPhysicalDevice)pStateTrackedObject->handle;
            }
        };
        gvkEnumerateStateTrackedObjectDependencies(&stateTrackedDevice, &enumerateInfo);
        assert(physicalDevice);
        physicalDevice = layer::Registry::get().VkPhysicalDevices[physicalDevice];
        assert(physicalDevice);

        // NOTE : Only enable on Intel discrete graphics.  See the note in
        //  gvk/gvk-restore-point/source/gvk-restore-point/handles/device.cpp
        //  Layer::pre_vkCreateDevice() for more info.
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        const auto& layerDispatchTable = layer::Registry::get().get_physical_device_dispatch_table(physicalDevice);
        layerDispatchTable.gvkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        if (physicalDeviceProperties.vendorID == 0x8086 && physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                const_cast<VkRayTracingPipelineCreateInfoKHR*>(&pCreateInfos[i])->flags |= VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
            }
        }
    }
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

VkResult Creator::process_VkPipeline(GvkPipelineRestoreInfo& restoreInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        std::vector<uint8_t> data;
        if (restoreInfo.pRayTracingPipelineCreateInfoKHR) {
            Device gvkDevice = get_dependency<VkDevice>(restoreInfo.dependencyCount, restoreInfo.pDependencies);
            auto gvkPhysicalDevice = gvkDevice.get<PhysicalDevice>();

            // NOTE : Only enable on Intel discrete graphics.  See the note in
            //  gvk/gvk-restore-point/source/gvk-restore-point/handles/device.cpp
            //  Layer::pre_vkCreateDevice() for more info.
            VkPhysicalDeviceProperties physicalDeviceProperties{};
            const auto& layerDispatchTable = layer::Registry::get().get_physical_device_dispatch_table(gvkPhysicalDevice.get<VkPhysicalDevice>());
            layerDispatchTable.gvkGetPhysicalDeviceProperties(gvkPhysicalDevice, &physicalDeviceProperties);
            if (physicalDeviceProperties.vendorID == 0x8086 && physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {

                // Get VkPhysicalDeviceRayTracingPipelinePropertiesKHR
                auto pApplicationInfo = gvkDevice.get<Instance>().get<VkInstanceCreateInfo>().pApplicationInfo;
                auto apiVersion = pApplicationInfo ? pApplicationInfo->apiVersion : VK_API_VERSION_1_0;
                auto pfnGetPhysicalDeviceProperties2 = apiVersion < VK_API_VERSION_1_2 ? gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceProperties2KHR : gvkPhysicalDevice.get<DispatchTable>().gvkGetPhysicalDeviceProperties2;
                auto physicalDeviceRayTracingPipelineProperties = get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
                auto physicalDeviceProperties2 = get_default<VkPhysicalDeviceProperties2>();
                physicalDeviceProperties2.pNext = &physicalDeviceRayTracingPipelineProperties;
                pfnGetPhysicalDeviceProperties2(gvkPhysicalDevice, &physicalDeviceProperties2);

                // NOTE : This must be set before serializing VkRayTracingPipelineCreateInfoKHR
                //  because it contains the size info for shader group capture/replay handles.
                //  It would be much nicer to not need to set any kind of global/thread_local
                /// state to serialize things, but this seems to be the best option for now.
                detail::tlPhysicalDeviceRayTracingPipelineProperties = physicalDeviceRayTracingPipelineProperties;
                detail::tlPhysicalDeviceRayTracingPipelineProperties.pNext = nullptr;

                // Get shader group capture/replay handles
                data.resize(restoreInfo.pRayTracingPipelineCreateInfoKHR->groupCount * physicalDeviceRayTracingPipelineProperties.shaderGroupHandleCaptureReplaySize);
                gvk_result(gvkDevice.get<DispatchTable>().gvkGetRayTracingCaptureReplayShaderGroupHandlesKHR(
                    gvkDevice,
                    restoreInfo.handle,
                    0,
                    restoreInfo.pRayTracingPipelineCreateInfoKHR->groupCount,
                    data.size(),
                    data.data()
                ));

                // Populate GvkPipelineRestoreInfo with shader group capture/replay handles
                auto pHandle = data.data();
                for (uint32_t i = 0; i < restoreInfo.pRayTracingPipelineCreateInfoKHR->groupCount; ++i) {
                    const_cast<VkRayTracingShaderGroupCreateInfoKHR&>(restoreInfo.pRayTracingPipelineCreateInfoKHR->pGroups[i]).pShaderGroupCaptureReplayHandle = pHandle;
                    pHandle += physicalDeviceRayTracingPipelineProperties.shaderGroupHandleCaptureReplaySize;
                }
            }
        }
        gvk_result(BasicCreator::process_VkPipeline(restoreInfo));
    } gvk_result_scope_end;
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

VkResult Layer::pre_vkCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineBinaryHandlesInfoKHR* pBinaries, VkResult gvkResult)
{
    (void)device;
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pBinaries;
    (void)gvkResult;
    return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult Layer::post_vkCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineBinaryHandlesInfoKHR* pBinaries, VkResult gvkResult)
{
    (void)device;
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pBinaries;
    (void)gvkResult;
    return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult Applier::restore_VkPipelineBinaryKHR(const GvkStateTrackedObject& restorePointObject, const GvkPipelineBinaryRestoreInfoKHR& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult Applier::restore_VkPipelineBinaryKHR_state(const GvkStateTrackedObject& restorePointObject, const GvkPipelineBinaryRestoreInfoKHR& restoreInfo)
{
    (void)restorePointObject;
    (void)restoreInfo;
    return VK_ERROR_INITIALIZATION_FAILED;
}

void Applier::destroy_VkPipelineBinaryKHR(const GvkStateTrackedObject& restorePointObject)
{
    (void)restorePointObject;
}

} // namespace restore_point
} // namespace gvk

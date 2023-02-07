
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

#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-layer/registry.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[pipeline_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[pipeline_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, pCreateInfos[pipeline_i].layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mComputePipelineCreateInfo = pCreateInfos[pipeline_i];
            controlBlock.mShaderModules.push_back(ShaderModule({ device, pCreateInfos[pipeline_i].stage.module }));
            assert(controlBlock.mShaderModules.back());
            gvkDevice.mReference.get_obj().mPipelineTracker.insert(gvkPipeline);
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[pipeline_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[pipeline_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, pCreateInfos[pipeline_i].layout });
            controlBlock.mRenderPass = RenderPass({ device, pCreateInfos[pipeline_i].renderPass });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mGraphicsPipelineCreateInfo = pCreateInfos[pipeline_i];
            for (uint32_t stage_i = 0; stage_i < pCreateInfos[pipeline_i].stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, pCreateInfos[pipeline_i].pStages[stage_i].module }));
                assert(controlBlock.mShaderModules.back());
            }
            gvkDevice.mReference.get_obj().mPipelineTracker.insert(gvkPipeline);
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[pipeline_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[pipeline_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mDeferredOperationKHR = DeferredOperationKHR({ device, deferredOperation });
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, pCreateInfos[pipeline_i].layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mRayTracingPipelineCreateInfoKHR = pCreateInfos[pipeline_i];
            for (uint32_t stage_i = 0; stage_i < pCreateInfos[pipeline_i].stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, pCreateInfos[pipeline_i].pStages[stage_i].module }));
                assert(controlBlock.mShaderModules.back());
            }
            gvkDevice.mReference.get_obj().mPipelineTracker.insert(gvkPipeline);
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[pipeline_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[pipeline_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, pCreateInfos[pipeline_i].layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mRayTracingPipelineCreateInfoNV = pCreateInfos[pipeline_i];
            for (uint32_t stage_i = 0; stage_i < pCreateInfos[pipeline_i].stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, pCreateInfos[pipeline_i].pStages[stage_i].module }));
                assert(controlBlock.mShaderModules.back());
            }
            gvkDevice.mReference.get_obj().mPipelineTracker.insert(gvkPipeline);
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

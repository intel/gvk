
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

VkResult StateTracker::post_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        assert(device);
        assert(pCreateInfo);
        assert(pPipelineLayout);
        gvkResult = BasicStateTracker::post_vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, gvkResult);
        assert(gvkResult == VK_SUCCESS);
        PipelineLayout gvkPipelineLayout({ device, *pPipelineLayout });
        assert(gvkPipelineLayout);
        auto& controlBlock = gvkPipelineLayout.mReference.get_obj();
        controlBlock.mDescriptorSetLayouts.reserve(pCreateInfo->setLayoutCount);
        for (uint32_t setLayout_i = 0; setLayout_i < pCreateInfo->setLayoutCount; ++setLayout_i) {
            controlBlock.mDescriptorSetLayouts.push_back(DescriptorSetLayout({ device, pCreateInfo->pSetLayouts[setLayout_i] }));
            assert(controlBlock.mDescriptorSetLayouts.back());
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        assert(createInfoCount);
        assert(pCreateInfos);
        assert(pPipelines);
        for (uint32_t createInfo_i = 0; createInfo_i < createInfoCount; ++createInfo_i) {
            const auto& createInfo = pCreateInfos[createInfo_i];
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[createInfo_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[createInfo_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, createInfo.layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mComputePipelineCreateInfo = createInfo;
            controlBlock.mShaderModules.push_back(ShaderModule({ device, createInfo.stage.module }));
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
        assert(createInfoCount);
        assert(pCreateInfos);
        assert(pPipelines);
        for (uint32_t createInfo_i = 0; createInfo_i < createInfoCount; ++createInfo_i) {
            const auto& createInfo = pCreateInfos[createInfo_i];
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[createInfo_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[createInfo_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, createInfo.layout });
            controlBlock.mRenderPass = RenderPass({ device, createInfo.renderPass });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mGraphicsPipelineCreateInfo = createInfo;
            controlBlock.mShaderModules.reserve(createInfo.stageCount);
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, createInfo.pStages[stage_i].module }));
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
        assert(createInfoCount);
        assert(pCreateInfos);
        assert(pPipelines);
        for (uint32_t createInfo_i = 0; createInfo_i < createInfoCount; ++createInfo_i) {
            const auto& createInfo = pCreateInfos[createInfo_i];
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[createInfo_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[createInfo_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mDeferredOperationKHR = DeferredOperationKHR({ device, deferredOperation });
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, createInfo.layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mRayTracingPipelineCreateInfoKHR = createInfo;
            controlBlock.mShaderModules.reserve(createInfo.stageCount);
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, createInfo.pStages[stage_i].module }));
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
        assert(createInfoCount);
        assert(pCreateInfos);
        assert(pPipelines);
        for (uint32_t createInfo_i = 0; createInfo_i < createInfoCount; ++createInfo_i) {
            const auto& createInfo = pCreateInfos[createInfo_i];
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[createInfo_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[createInfo_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, createInfo.layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mRayTracingPipelineCreateInfoNV = createInfo;
            controlBlock.mShaderModules.reserve(createInfo.stageCount);
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, createInfo.pStages[stage_i].module }));
                assert(controlBlock.mShaderModules.back());
            }
            gvkDevice.mReference.get_obj().mPipelineTracker.insert(gvkPipeline);
        }
    }
    return gvkResult;
}

VkResult StateTracker::post_vkCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        assert(createInfoCount);
        assert(pCreateInfos);
        assert(pPipelines);
        for (uint32_t createInfo_i = 0; createInfo_i < createInfoCount; ++createInfo_i) {
            const auto& createInfo = pCreateInfos[createInfo_i];
            Pipeline gvkPipeline;
            gvkPipeline.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkPipeline>(device, pPipelines[createInfo_i]));
            auto& controlBlock = gvkPipeline.mReference.get_obj();
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mVkPipeline = pPipelines[createInfo_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mPipelineCache = PipelineCache({ device, pipelineCache });
            controlBlock.mPipelineLayout = PipelineLayout({ device, createInfo.layout });
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mExecutionGraphPipelineCreateInfoAMDX = createInfo;
            controlBlock.mShaderModules.reserve(createInfo.stageCount);
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                controlBlock.mShaderModules.push_back(ShaderModule({ device, createInfo.pStages[stage_i].module }));
                assert(controlBlock.mShaderModules.back());
            }
            gvkDevice.mReference.get_obj().mPipelineTracker.insert(gvkPipeline);
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

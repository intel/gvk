
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

#include "gvk-pipeline-explorer/pipeline-explorer.hpp"

namespace gvk {

VkResult PipelineExplorer::execute_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool));
        pipeline_explorer::CommandPoolInfo commandPoolInfo(gvk::newref, { device, *pCommandPool });
        commandPoolInfo->deviceInfo = device;
        commandPoolInfo->vkHandle = *pCommandPool;
        commandPoolInfo->commandPoolCreateInfo = *pCreateInfo;
        auto inserted = commandPoolInfos.insert({ { device, *pCommandPool }, commandPoolInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    pipeline_explorer::CommandPoolInfo commandPoolInfo({ device, commandPool });
    if (commandPoolInfo) {
        commandPoolInfos.erase({ device, commandPool });
        // TODO : Double check if these explicit clears are necessary...
        //  Do children hold a reference to the parent that won't clear?
        //  I _think_ the container dtor is enough, but need to double check.
        for (const auto& commandBufferInfo : commandPoolInfo->commandBufferInfos) {
            commandBufferInfos.erase(commandBufferInfo->vkHandle);
        }
    } else {
        // TODO : Error message to GUI
    }
    BasicApiCallHandler::execute_vkDestroyCommandPool(device, commandPool, pAllocator);
}

VkResult PipelineExplorer::execute_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkResetCommandPool(device, commandPool, flags));
        pipeline_explorer::CommandPoolInfo commandPoolInfo({ device, commandPool });
        gvk_result(commandPoolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        for (auto commandBufferInfo : commandPoolInfo->commandBufferInfos) {
            gvk_result(commandBufferInfo->reset((VkCommandBufferResetFlags)flags));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // TODO : Documentation
        auto commandBufferCount = pAllocateInfo->commandBufferCount;
        thread_local std::vector<VkCommandBuffer> tlCommandBuffers;
        tlCommandBuffers.resize(commandBufferCount * 2);
        const_cast<VkCommandBufferAllocateInfo*>(pAllocateInfo)->commandBufferCount = (uint32_t)tlCommandBuffers.size();

        // TODO : Documentation
        gvk_result(BasicApiCallHandler::execute_vkAllocateCommandBuffers(device, pAllocateInfo, tlCommandBuffers.data()));

        // TODO : Documentation
        const_cast<VkCommandBufferAllocateInfo*>(pAllocateInfo)->commandBufferCount = commandBufferCount;
        memcpy(pCommandBuffers, tlCommandBuffers.data(), commandBufferCount * sizeof(VkCommandBuffer));

        // TODO : Documentation
        pipeline_explorer::CommandPoolInfo commandPoolInfo({ device, pAllocateInfo->commandPool });
        gvk_result(commandPoolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        if (commandPoolInfo) {
            for (uint32_t commandBuffer_i = 0; commandBuffer_i < pAllocateInfo->commandBufferCount; ++commandBuffer_i) {
                pipeline_explorer::CommandBufferInfo commandBufferInfo(gvk::newref, tlCommandBuffers[commandBuffer_i]);
                commandBufferInfo->deviceInfo = device;
                commandBufferInfo->commandPoolInfo = commandPoolInfo;
                commandBufferInfo->vkHandle = tlCommandBuffers[commandBuffer_i];
                commandBufferInfo->commandBufferAllocateInfo = *pAllocateInfo;
                commandBufferInfo->experimentCommandBuffer = tlCommandBuffers[commandBuffer_i + commandBufferCount];
                commandBufferInfo->experimentEnabled = VK_TRUE;
                auto inserted = commandBufferInfos.insert({ tlCommandBuffers[commandBuffer_i], commandBufferInfo }).second;
                gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                inserted = commandPoolInfo->commandBufferInfos.insert(commandBufferInfo).second;
                gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                if (vkLayer) {
                    *(void**)commandBufferInfo->experimentCommandBuffer = *(void**)device;
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    // TODO : Documentation
    thread_local std::vector<VkCommandBuffer> tlFreeCommandBuffers;
    tlFreeCommandBuffers.clear();
    tlFreeCommandBuffers.reserve(commandBufferCount * 2);
    tlFreeCommandBuffers.insert(tlFreeCommandBuffers.end(), pCommandBuffers, pCommandBuffers + commandBufferCount);
    for (uint32_t commandBuffer_i = 0; commandBuffer_i < commandBufferCount; ++commandBuffer_i) {
        pipeline_explorer::CommandBufferInfo commandBufferInfo(pCommandBuffers[commandBuffer_i]);
        if (commandBufferInfo) {
            tlFreeCommandBuffers.push_back(commandBufferInfo->experimentCommandBuffer);
            commandBufferInfo->commandPoolInfo->commandBufferInfos.erase(commandBufferInfo);
            commandBufferInfo->commandPoolInfo = gvk::nullref;
            commandBufferInfos.erase(pCommandBuffers[commandBuffer_i]);
        }
    }

    // TODO : Documentation
    BasicApiCallHandler::execute_vkFreeCommandBuffers(device, commandPool, (uint32_t)tlFreeCommandBuffers.size(), !tlFreeCommandBuffers.empty() ? tlFreeCommandBuffers.data() : nullptr);
}

VkResult PipelineExplorer::execute_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkResetCommandBuffer(commandBuffer, flags));
        pipeline_explorer::CommandBufferInfo commandBufferInfo(commandBuffer);
        if (commandBufferInfo) {
            gvk_result(commandBufferInfo->reset(flags));
            if (commandBufferInfo->experimentCommandBuffer) {
                gvk_result(BasicApiCallHandler::execute_vkResetCommandBuffer(commandBufferInfo->experimentCommandBuffer, flags));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkBeginCommandBuffer(commandBuffer, pBeginInfo));
        pipeline_explorer::CommandBufferInfo commandBufferInfo(commandBuffer);
        if (commandBufferInfo) {
            gvk_result(commandBufferInfo->begin(pBeginInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkEndCommandBuffer(commandBuffer));
        pipeline_explorer::CommandBufferInfo commandBufferInfo(commandBuffer);
        if (commandBufferInfo) {
            gvk_result(commandBufferInfo->end());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult end_collection_range(std::vector<GvkPipelineExplorerCollectionRange>& collectionRanges)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(!collectionRanges.empty() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        if (collectionRanges.back().device || collectionRanges.back().pipeline || collectionRanges.back().begin || collectionRanges.back().end) {
            gvk_result(collectionRanges.back().device ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(collectionRanges.back().pipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(collectionRanges.back().begin && collectionRanges.back().end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(collectionRanges.back().begin <= collectionRanges.back().end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            collectionRanges.push_back(gvk::get_default<GvkPipelineExplorerCollectionRange>());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult add_cmd_to_collection_range(VkDevice device, VkPipeline pipeline, VkPipelineBindPoint bindPoint, uint32_t cmdIndex, std::vector<GvkPipelineExplorerCollectionRange>& collectionRanges)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(!collectionRanges.empty() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        if (collectionRanges.back().pipeline != pipeline) {
            gvk_result(end_collection_range(collectionRanges));
            collectionRanges.back().device = device;
            collectionRanges.back().pipeline = pipeline;
            collectionRanges.back().bindPoint = bindPoint;
            collectionRanges.back().begin = cmdIndex;
            collectionRanges.back().end = cmdIndex;
        }
        gvk_result(collectionRanges.back().device == device ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(collectionRanges.back().pipeline == pipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(collectionRanges.back().bindPoint == bindPoint ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(collectionRanges.back().begin && collectionRanges.back().end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(collectionRanges.back().begin <= collectionRanges.back().end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        collectionRanges.back().end = cmdIndex;
    } gvk_result_scope_end;
    return gvkResult;
}

static bool sample_pipeline_metrics(VkDevice device, VkPipeline pipeline, const GvkPipelineExplorerRequestInfo& requestInfo)
{
    return (requestInfo.device == device && requestInfo.sampleMetricsPipeline == pipeline) || requestInfo.refreshActivePipelines;
}

VkResult PipelineExplorer::inspect_command_buffer(pipeline_explorer::QueueInfo queueInfo, pipeline_explorer::CommandBufferInfo commandBufferInfo, std::vector<const GvkCommandBaseStructure*>& cmds, std::vector<GvkPipelineExplorerCollectionRange>& collectionRanges)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // TODO : Documentation
        gvk_result(commandBufferInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        const auto& cmdTrackerCmds = commandBufferInfo->cmdTracker.get_commands();
        gvk_result(1 < cmdTrackerCmds.size() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(cmdTrackerCmds.front() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(cmdTrackerCmds.front()->sType == gvk::get_stype<GvkCommandStructureBeginCommandBuffer>() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        if (commandBufferInfo->commandBufferAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
            cmds.push_back(cmdTrackerCmds.front());
        }

        // TODO : Documentation
        VkPipeline computePipeline = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkPipeline rayTracingPipeline = VK_NULL_HANDLE;
        uint32_t* pComputePipelineExecutionCount = nullptr;
        uint32_t* pGraphicsPipelineExecutionCount = nullptr;
        uint32_t* pRayTracingPipelineExecutionCount = nullptr;
        VkDevice device = commandBufferInfo->deviceInfo->vkHandle;

        // TODO : Documentation
        for (uint32_t command_i = 1; command_i < cmdTrackerCmds.size() - 1; ++command_i) {
            switch (cmdTrackerCmds[command_i]->sType) {
            ////////////////////////////////////////////////////////////////////////////////
            // vkCmdBindPipeline
            case gvk::get_stype<GvkCommandStructureCmdBindPipeline>(): {
                auto pCmdBindPipeline = (GvkCommandStructureCmdBindPipeline*)cmdTrackerCmds[command_i];
                switch (pCmdBindPipeline->pipelineBindPoint) {
                case VK_PIPELINE_BIND_POINT_COMPUTE: {
                    computePipeline = pCmdBindPipeline->pipeline;
                    pComputePipelineExecutionCount = &pipelineExecutionCounts[{ device, computePipeline }];
                } break;
                case VK_PIPELINE_BIND_POINT_GRAPHICS: {
                    graphicsPipeline = pCmdBindPipeline->pipeline;
                    pGraphicsPipelineExecutionCount = &pipelineExecutionCounts[{ device, graphicsPipeline }];
                } break;
                case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
                    rayTracingPipeline = pCmdBindPipeline->pipeline;
                    pRayTracingPipelineExecutionCount = &pipelineExecutionCounts[{ device, rayTracingPipeline }];
                    pipeline_explorer::PipelineInfo raytracingPipelineInfo({ device, rayTracingPipeline });
                    gvk_result(raytracingPipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    if (raytracingPipelineInfo->experimentEnabled) {

#if 0
                        // TODO : Documentation
                        if (!queueInfo->fence) {
                            auto fenceCreateInfo = gvk::get_default<VkFenceCreateInfo>();
                            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                            gvk_result(gvk::Fence::create(gvkDevice, &fenceCreateInfo, nullptr, &queueInfo->fence));
                        }

                        // TODO : Documentation
                        gvk_result(gvkDevice.WaitForFences(1, &queueInfo->fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
#endif

                    }
                } break;
                default: {
                } break;
                }
            } break;
            ////////////////////////////////////////////////////////////////////////////////
            //------------------------------------------------------------------------------
            // vkCmdDispatch
            case gvk::get_stype<GvkCommandStructureCmdDispatch>():
            case gvk::get_stype<GvkCommandStructureCmdDispatchBase>():
            case gvk::get_stype<GvkCommandStructureCmdDispatchBaseKHR>():
            case gvk::get_stype<GvkCommandStructureCmdDispatchGraphAMDX>():
            case gvk::get_stype<GvkCommandStructureCmdDispatchGraphIndirectAMDX>():
            case gvk::get_stype<GvkCommandStructureCmdDispatchGraphIndirectCountAMDX>():
            case gvk::get_stype<GvkCommandStructureCmdDispatchIndirect>(): {
                gvk_result(pComputePipelineExecutionCount ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                *pComputePipelineExecutionCount += 1;
                if (sample_pipeline_metrics(device, computePipeline, requestInfo)) {
                    gvk_result(add_cmd_to_collection_range(device, computePipeline, VK_PIPELINE_BIND_POINT_COMPUTE, (uint32_t)cmds.size(), collectionRanges));
                }
            } break;
            ////////////////////////////////////////////////////////////////////////////////
            //------------------------------------------------------------------------------
            // vkCmdDraw
            case gvk::get_stype<GvkCommandStructureCmdDraw>():
            case gvk::get_stype<GvkCommandStructureCmdDrawClusterHUAWEI>():
            case gvk::get_stype<GvkCommandStructureCmdDrawClusterIndirectHUAWEI>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndexed>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirect>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirectCount>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirectCountAMD>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirectCountKHR>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndirect>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndirectByteCountEXT>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndirectCount>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndirectCountAMD>():
            case gvk::get_stype<GvkCommandStructureCmdDrawIndirectCountKHR>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMeshTasksEXT>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMeshTasksIndirectCountEXT>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMeshTasksIndirectCountNV>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMeshTasksIndirectEXT>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMeshTasksIndirectNV>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMeshTasksNV>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMultiEXT>():
            case gvk::get_stype<GvkCommandStructureCmdDrawMultiIndexedEXT>(): {
                gvk_result(pGraphicsPipelineExecutionCount ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                *pGraphicsPipelineExecutionCount += 1;
                if (sample_pipeline_metrics(device, graphicsPipeline, requestInfo)) {
                    gvk_result(add_cmd_to_collection_range(device, graphicsPipeline, VK_PIPELINE_BIND_POINT_GRAPHICS, (uint32_t)cmds.size(), collectionRanges));
                }
            } break;
            ////////////////////////////////////////////////////////////////////////////////
            //------------------------------------------------------------------------------
            // vkCmdTraceRays
            case gvk::get_stype<GvkCommandStructureCmdTraceRaysIndirect2KHR>():
            case gvk::get_stype<GvkCommandStructureCmdTraceRaysIndirectKHR>():
            case gvk::get_stype<GvkCommandStructureCmdTraceRaysKHR>():
            case gvk::get_stype<GvkCommandStructureCmdTraceRaysNV>(): {
                gvk_result(pRayTracingPipelineExecutionCount ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                *pRayTracingPipelineExecutionCount += 1;
                if (sample_pipeline_metrics(device, rayTracingPipeline, requestInfo)) {
                    gvk_result(add_cmd_to_collection_range(device, rayTracingPipeline, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, (uint32_t)cmds.size(), collectionRanges));
                }
            } break;
            default: {
                gvk_result(end_collection_range(collectionRanges));
            } break;
            }
            ////////////////////////////////////////////////////////////////////////////////
            //------------------------------------------------------------------------------
            // vkCmdExecuteCommands
            if (cmdTrackerCmds[command_i]->sType == gvk::get_stype<GvkCommandStructureCmdExecuteCommands>()) {
                auto pCmdExecuteCommands = (GvkCommandStructureCmdExecuteCommands*)cmdTrackerCmds[command_i];
                for (uint32_t commandBuffer_i = 0; commandBuffer_i < pCmdExecuteCommands->commandBufferCount; ++commandBuffer_i) {
                    pipeline_explorer::CommandBufferInfo secondaryCommandBufferInfo(pCmdExecuteCommands->pCommandBuffers[commandBuffer_i]);
                    gvk_result(secondaryCommandBufferInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    gvk_result(inspect_command_buffer(queueInfo, secondaryCommandBufferInfo, cmds, collectionRanges));
                }
            }
            else {
                // TODO : Documentation
#if 0
                gvk_result(!collectionRangeBegin == !collectionRangeEnd ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                if (sampleMetricsCmd) {
                    if (!collectionRangeBegin) {
                        collectionRangeBegin = command_i;
                    }
                    collectionRangeEnd = command_i;
                }
                else if (collectionRangeBegin && collectionRangeEnd) {
                    collectionRanges.push_back({ collectionRangeBegin, collectionRangeEnd });
                    collectionRangeBegin = 0;
                    collectionRangeEnd = 0;
                }
#endif
                cmds.push_back(cmdTrackerCmds[command_i]);
            }
        }

#if 0
        // TODO : Documentation
        gvk_result(!collectionRange.begin == !collectionRange.end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        // TODO : Double check this logic...does it allow for one big collection range starting at zero and never ended (no)?  Is that a real use-case?
        if (collectionRange.begin && collectionRange.end) {
            collectionRanges.push_back(collectionRange);
        }
#endif

        // TODO : Documentation
        gvk_result(cmdTrackerCmds.back() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(cmdTrackerCmds.back()->sType == gvk::get_stype<GvkCommandStructureEndCommandBuffer>() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        if (commandBufferInfo->commandBufferAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
            cmds.push_back(cmdTrackerCmds.back());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::tool_command_buffer(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // TODO : Documetation
        // TODO : Wrangle QueryManager
        bool timestampQueryPoolReset = false;
        bool pipelineStatisticsQueryPoolReset = false;

        // TODO : Documentation
        gvk::Device gvkDevice = toolCommandBufferInfo.device;
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);

        // TODO : Documentation
        pipeline_explorer::QueueInfo queueInfo = toolCommandBufferInfo.queue;
        gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);

        // TODO : Documentation
        gvk_result(handle_pre_process_command_buffer_callback(toolCommandBufferInfo));

        // Data that needs to be cached to restore after modifying/inserting cmds
        VkPushConstantsInfo pushConstantsInfo{ };
        std::vector<uint8_t> pushConstantsData;
        pipeline_explorer::PipelineInfo computePipelineInfo;
        pipeline_explorer::PipelineInfo graphicsPipelineInfo;
        pipeline_explorer::PipelineInfo raytracingPipelineInfo;

        // TODO : Documentation
        toolCommandBufferInfo.collectionRangeIndex = 0;
        pipeline_explorer::CommandBufferInfo commandBufferInfo;
        for (toolCommandBufferInfo.cmdIndex = 0; toolCommandBufferInfo.cmdIndex < toolCommandBufferInfo.cmdCount; ++toolCommandBufferInfo.cmdIndex) {
            auto pCmd = toolCommandBufferInfo.ppCmds[toolCommandBufferInfo.cmdIndex];
            if (pCmd->sType == gvk::get_stype<GvkCommandStructureBeginCommandBuffer>()) {
                commandBufferInfo = pCmd->commandBuffer;
            }
            gvk_result(commandBufferInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);

            // NOTE : Currently, both commandBufferInfo->experimentCommandBuffer 
            //  and commandBufferInfo->experimentEnabled should always be true.
            // TODO : Optionally disabling these requires a bit of a refactor, but it is
            //  desirable to eventually omit rerecording command buffers that have no
            //  acive experiment.
            if (commandBufferInfo->experimentCommandBuffer && commandBufferInfo->experimentEnabled) {
                ((GvkCommandCmdBaseStructure*)pCmd)->commandBuffer = commandBufferInfo->experimentCommandBuffer;

                // Cache pipeline to restore it after highlighting/experiment replacement
                VkPipeline pipeline = VK_NULL_HANDLE;

                // GvkCommandStructureCmdTraceRaysKHR copy for VKRT experiment modification
                // NOTE : Using gvk::Auto<> here so that the VkStridedDeviceAddressRegionKHRs
                //  for the shader binding tables are deep copied when doing VKRT experiments.
                // TODO : Route this to a scratchpad allocator
                gvk::Auto<GvkCommandStructureCmdTraceRaysKHR> cmdTraceRays{ };

                switch (pCmd->sType) {
                case gvk::get_stype<GvkCommandStructureCmdBindPipeline>(): {

                    // Get the PipelineInfo
                    auto pCmdBindPipeline = (GvkCommandStructureCmdBindPipeline*)pCmd;
                    pipeline_explorer::PipelineInfo pipelineInfo({ toolCommandBufferInfo.device, pCmdBindPipeline->pipeline });
                    gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    pipeline = pCmdBindPipeline->pipeline;

                    // Cache the pipeline being bound
                    switch (pCmdBindPipeline->pipelineBindPoint) {
                    case VK_PIPELINE_BIND_POINT_COMPUTE: {
                        computePipelineInfo = pipelineInfo;
                    } break;
                    case VK_PIPELINE_BIND_POINT_GRAPHICS: {
                        graphicsPipelineInfo = pipelineInfo;
                    } break;
                    case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
                        raytracingPipelineInfo = pipelineInfo;
                    } break;
                    default: {
                    } break;
                    }

                    // If there's no active metrics request and highlighting is enabled for the
                    //  pipeline, replace the pipeline with the highlighting pipeline, otherwise
                    //  replace the pipeline with the experiment pipeline if enabled.
                    // NOTE : The way this works out, a user can have highlighting enabled and
                    //  an experiment enabled...during a metrics request, the experiment pipeline
                    //  takes precedence, otherwise the highlighting pipeline takes precendence.
                    if (requestInfo->sType != gvk::get_stype<GvkPipelineExplorerRequestInfo>() && pipelineInfo->highlightEnabled && pipelineInfo->highlightPipeline) {
                        pCmdBindPipeline->pipeline = pipelineInfo->highlightPipeline;
                    } else if (pipelineInfo->experimentEnabled && pipelineInfo->experimentPipeline) {
                        pCmdBindPipeline->pipeline = pipelineInfo->experimentPipeline;
                    }
                } break;
                case gvk::get_stype<GvkCommandStructureCmdPushConstants>(): {
                    // Cache push constant data so that it can be restored
                    // NOTE : This is only actually necessary if an experiment uses push constants,
                    //  ie. for creating replacement shader binding tables.  If it ever becomes an
                    //  issue some tracking can be added to determine if any experiments are active
                    //  that require this caching and disable it otherwise.
                    // TODO : Route this to a scratchpad allocator
                    auto pCmdPushConstants = (const GvkCommandStructureCmdPushConstants*)pCmd;
                    pushConstantsInfo.sType = gvk::get_stype<VkPushConstantsInfo>();
                    pushConstantsInfo.layout = pCmdPushConstants->layout;
                    pushConstantsInfo.stageFlags = pCmdPushConstants->stageFlags;
                    if (pushConstantsData.size() < pCmdPushConstants->offset + pCmdPushConstants->size) {
                        pushConstantsData.resize(pCmdPushConstants->offset + pCmdPushConstants->size);
                    }
                    memcpy(pushConstantsData.data() + pCmdPushConstants->offset, pCmdPushConstants->pValues, pCmdPushConstants->size);
                    pushConstantsInfo.size = (uint32_t)pushConstantsData.size();
                    pushConstantsInfo.pValues = !pushConstantsData.empty() ? pushConstantsData.data() : nullptr;
                } break;
                case gvk::get_stype<GvkCommandStructureCmdPushConstants2>(): {
                    // Cache push constant data so that it can be restored
                    // NOTE : See comment above
                    // TODO : Route this to a scratchpad allocator
                    auto pCmdPushConstants2 = (const GvkCommandStructureCmdPushConstants2*)pCmd;
                    gvk_result(pCmdPushConstants2->pPushConstantsInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    gvk_result(pCmdPushConstants2->pPushConstantsInfo->pNext ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT);
                    pushConstantsInfo.sType = gvk::get_stype<VkPushConstantsInfo>();
                    pushConstantsInfo.layout = pCmdPushConstants2->pPushConstantsInfo->layout;
                    pushConstantsInfo.stageFlags = pCmdPushConstants2->pPushConstantsInfo->stageFlags;
                    if (pushConstantsData.size() < pCmdPushConstants2->pPushConstantsInfo->offset + pCmdPushConstants2->pPushConstantsInfo->size) {
                        pushConstantsData.resize(pCmdPushConstants2->pPushConstantsInfo->offset + pCmdPushConstants2->pPushConstantsInfo->size);
                    }
                    memcpy(pushConstantsData.data() + pCmdPushConstants2->pPushConstantsInfo->offset, pCmdPushConstants2->pPushConstantsInfo->pValues, pCmdPushConstants2->pPushConstantsInfo->size);
                    pushConstantsInfo.size = (uint32_t)pushConstantsData.size();
                    pushConstantsInfo.pValues = !pushConstantsData.empty() ? pushConstantsData.data() : nullptr;
                } break;
                case gvk::get_stype<GvkCommandStructureCmdTraceRaysKHR>(): {
                    gvk_result(raytracingPipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                    if (raytracingPipelineInfo->experimentEnabled && raytracingPipelineInfo->experimentPipeline) {
                        cmdTraceRays = *(GvkCommandStructureCmdTraceRaysKHR*)pCmd;
                        gvk_result(create_replacement_shader_binding_tables(queueInfo, raytracingPipelineInfo, &const_cast<GvkCommandStructureCmdTraceRaysKHR&>(*cmdTraceRays)));

                        // create_replacement_shader_binding_tables() utilizes compute pipelines, so
                        //  if there was a previously bound compute pipeline restore the binding
                        if (computePipelineInfo) {
                            dispatchTable.gvkCmdBindPipeline(pCmd->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineInfo->vkHandle);
                        }

                        // create_replacement_shader_binding_tables() utilizes push constants, so if
                        //  there was any previous push constant data pushed restore it
                        // NOTE : The approach used here is to copy all push constant data, then push it
                        //  with the VkPipelineLayout and VkShaderStageFlags used by the application's
                        //  last push.  There's only one push constant buffer per command buffer, so
                        //  there's a good chance this approach will always work, but if there's ever an
                        //  issue with VkPipelineLayout compatibility, it may be necessary to map push
                        //  constant ranges to particular VkPipelineLayouts.
                        if (pushConstantsInfo.sType == gvk::get_stype<VkPushConstantsInfo>()) {
                            dispatchTable.gvkCmdPushConstants(pCmd->commandBuffer, pushConstantsInfo.layout, pushConstantsInfo.stageFlags, 0, pushConstantsInfo.size, pushConstantsInfo.pValues);
                        }
                    }
                } break;
                case gvk::get_stype<GvkCommandStructureCmdTraceRaysIndirectKHR>(): {
                    gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
                } break;
                default: {
                } break;
                }

                ////////////////////////////////////////////////////////////////////////////////
                // TODO : Wrangle QueryManager
                //------------------------------------------------------------------------------
                // TODO : Documentation
                if (toolCommandBufferInfo.collectionRangeIndex < toolCommandBufferInfo.collectionRangeCount && toolCommandBufferInfo.cmdIndex == 1 &&
                    requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->refreshActivePipelines && !timestampQueryPoolReset) {
                    gvk_result(reset_timestamp_query_pool(queueInfo, pCmd->commandBuffer, toolCommandBufferInfo.collectionRangeCount * 2));
                    timestampQueryPoolReset = true;
                }
                // TODO : Documentation
                if (toolCommandBufferInfo.collectionRangeIndex < toolCommandBufferInfo.collectionRangeCount &&
                    toolCommandBufferInfo.cmdIndex == toolCommandBufferInfo.pCollectionRanges[toolCommandBufferInfo.collectionRangeIndex].begin) {
                    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->refreshActivePipelines) {
                        write_timestamp(queueInfo, pCmd->commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                    }
                }
                //------------------------------------------------------------------------------
                // TODO : Documentation
                if (toolCommandBufferInfo.collectionRangeIndex < toolCommandBufferInfo.collectionRangeCount && toolCommandBufferInfo.cmdIndex == 1 &&
                    toolCommandBufferInfo.metricsRequestIdCount == 1 && toolCommandBufferInfo.pMetricsRequestIds[0].x == GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY &&
                    requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && !pipelineStatisticsQueryPoolReset) {
                    gvk_result(reset_pipeline_statistics_query_pool(queueInfo, pCmd->commandBuffer, requestInfo->sampleMetricsPipeline, toolCommandBufferInfo.collectionRangeCount));
                    pipelineStatisticsQueryPoolReset = true;
                }
                if (toolCommandBufferInfo.collectionRangeIndex < toolCommandBufferInfo.collectionRangeCount &&
                    toolCommandBufferInfo.cmdIndex == toolCommandBufferInfo.pCollectionRanges[toolCommandBufferInfo.collectionRangeIndex].begin &&
                    toolCommandBufferInfo.metricsRequestIdCount == 1 && toolCommandBufferInfo.pMetricsRequestIds[0].x == GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY) {
                    begin_pipeline_statistics_query(queueInfo, pCmd->commandBuffer);
                }
                ////////////////////////////////////////////////////////////////////////////////

                // TODO : Documentation
                gvk_result(handle_pre_process_cmd_callback(toolCommandBufferInfo));

                // TODO : Documentation
                if (cmdTraceRays->sType == gvk::get_stype<GvkCommandStructureCmdTraceRaysKHR>()) {
                    gvk::detail::execute_command_structure(dispatchTable, cmdTraceRays);
                } else {
                    gvk::detail::execute_command_structure(dispatchTable, *pCmd);
                }

                // TODO : Documentation
                gvk_result(handle_post_process_cmd_callback(toolCommandBufferInfo));

                ////////////////////////////////////////////////////////////////////////////////
                // TODO : Wrangle QueryManager
                //------------------------------------------------------------------------------
                if (toolCommandBufferInfo.collectionRangeIndex < toolCommandBufferInfo.collectionRangeCount &&
                    toolCommandBufferInfo.cmdIndex == toolCommandBufferInfo.pCollectionRanges[toolCommandBufferInfo.collectionRangeIndex].end &&
                    toolCommandBufferInfo.metricsRequestIdCount == 1 && toolCommandBufferInfo.pMetricsRequestIds[0].x == GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY) {
                    end_pipeline_statistics_query(queueInfo, pCmd->commandBuffer);
                    ++toolCommandBufferInfo.collectionRangeIndex;
                }
                //------------------------------------------------------------------------------
                // TODO : Documentation
                if (toolCommandBufferInfo.collectionRangeIndex < toolCommandBufferInfo.collectionRangeCount &&
                    toolCommandBufferInfo.cmdIndex == toolCommandBufferInfo.pCollectionRanges[toolCommandBufferInfo.collectionRangeIndex].end) {
                    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->refreshActivePipelines) {
                        write_timestamp(queueInfo, pCmd->commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
                    }
                    ++toolCommandBufferInfo.collectionRangeIndex;
                }
                ////////////////////////////////////////////////////////////////////////////////

                switch (pCmd->sType) {
                case gvk::get_stype<GvkCommandStructureCmdBindPipeline>(): {
                    if (pipeline) {
                        // Revert the pipeline
                        ((GvkCommandStructureCmdBindPipeline*)pCmd)->pipeline = pipeline;
                    }
                } break;
                default: {
                } break;
                }

                // Revert the command buffer
                ((GvkCommandCmdBaseStructure*)pCmd)->commandBuffer = commandBufferInfo->vkHandle;
            }
        }

        // TODO : Documentation
        gvk_result(handle_post_process_command_buffer_callback(toolCommandBufferInfo));

    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

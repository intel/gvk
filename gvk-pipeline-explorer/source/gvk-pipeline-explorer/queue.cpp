
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

VkResult PipelineExplorer::execute_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
#if 1
    std::lock_guard<std::mutex> lock(queueSubmissionMutex);
#endif

    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {

        // TODO : Documentation
        pipeline_explorer::QueueInfo queueInfo = queue;
        gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk::Device gvkDevice = queueInfo->deviceInfo->vkHandle;
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);

        #if 0
        if (!queueInfo->fence) {
            auto fenceCreateInfo = gvk::get_default<VkFenceCreateInfo>();
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            gvk_result(gvk::Fence::create(queueInfo->deviceInfo->vkHandle, &fenceCreateInfo, nullptr, &queueInfo->fence));
        }
        gvk_result(gvkDevice.WaitForFences(1, &queueInfo->fence.get<VkFence>(), VK_TRUE, UINT64_MAX));
        #endif

        #if 0
        // TODO : Documentation
        for (auto& inUseShaderBindingTableBuffer : queueInfo->inUseShaderBindingTableBuffers) {
            gvk_result(inUseShaderBindingTableBuffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            auto size = inUseShaderBindingTableBuffer.get<VkBufferCreateInfo>().size;
            auto inserted = queueInfo->availableShaderBindingTableBuffers[size].insert(inUseShaderBindingTableBuffer).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        }
        queueInfo->inUseShaderBindingTableBuffers.clear();
        #else
        queueInfo->shaderBindingTableReplacementResources.reset_available_resources();
        #endif

        // TODO : Documentation
        thread_local std::vector<VkSubmitInfo> tlSubmits;
        thread_local std::vector<std::vector<VkCommandBuffer>> tlCommandBuffers;
        thread_local std::vector<const GvkCommandBaseStructure*> tlCommands;
        thread_local std::vector<GvkPipelineExplorerCollectionRange> tlCollectionRanges;
        thread_local std::vector<GvkPipelineExplorerMetricId> tlMetricRequestIds;
        tlSubmits.resize(submitCount);
        tlCommandBuffers.resize(submitCount);
        tlCommands.clear();
        tlCollectionRanges.clear();
        tlCollectionRanges.push_back(gvk::get_default<GvkPipelineExplorerCollectionRange>());
        tlMetricRequestIds.clear();
        if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->sampleMetricIdCount == 1 && requestInfo->pSampleMetricIds) {
            tlMetricRequestIds.push_back({ requestInfo->pSampleMetricIds[0].x, 0, 0, 0 });
        }

        // TODO : Documentation
        for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
            tlSubmits[submit_i] = pSubmits[submit_i];

            // TODO : Documentation
            auto pNext = (VkBaseInStructure const*)tlSubmits[submit_i].pNext;
            switch (pNext ? pNext->sType : 0) {
            case VK_STRUCTURE_TYPE_APPLICATION_INFO:
            case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO: {
                // NOOP :
            } break;
            default: {
                gvk_result(VK_ERROR_EXTENSION_NOT_PRESENT);
            } break;
            }

            // TODO : Documentation
            tlCommandBuffers[submit_i].resize(tlSubmits[submit_i].commandBufferCount);
            for (uint32_t commandBuffer_i = 0; commandBuffer_i < tlSubmits[submit_i].commandBufferCount; ++commandBuffer_i) {
                pipeline_explorer::CommandBufferInfo commandBufferInfo(tlSubmits[submit_i].pCommandBuffers[commandBuffer_i]);
                gvk_result(commandBufferInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                if (commandBufferInfo->experimentCommandBuffer && commandBufferInfo->experimentEnabled) {
                    tlCommandBuffers[submit_i][commandBuffer_i] = commandBufferInfo->experimentCommandBuffer;
                } else {
                    tlCommandBuffers[submit_i][commandBuffer_i] = tlSubmits[submit_i].pCommandBuffers[commandBuffer_i];
                }
                gvk_result(inspect_command_buffer(queueInfo, commandBufferInfo, tlCommands, tlCollectionRanges));
            }
            tlSubmits[submit_i].pCommandBuffers = !tlCommandBuffers[submit_i].empty() ? tlCommandBuffers[submit_i].data() : nullptr;
        }

#if 0
        auto vkResult = BasicPipelineExplorer::execute_vkQueueSubmit(queue, submitCount, pSubmits, fence);
        if (vkResult != VK_ERROR_DEVICE_LOST) {
            return vkResult;
        }
#endif

        // TODO : Documentation
        if (!tlCollectionRanges.back().device || !tlCollectionRanges.back().pipeline || !tlCollectionRanges.back().begin || !tlCollectionRanges.back().end) {
            gvk_result(!tlCollectionRanges.back().device ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(!tlCollectionRanges.back().pipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(!tlCollectionRanges.back().begin ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(!tlCollectionRanges.back().end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            tlCollectionRanges.pop_back();
        }

        // TODO : Documentation
        auto toolCommandBufferInfo = gvk::get_default<GvkPipelineExplorerToolCommandBufferInfo>();
        toolCommandBufferInfo.device = queueInfo->deviceInfo->vkHandle;
        toolCommandBufferInfo.queue = queue;
        toolCommandBufferInfo.cmdCount = (uint32_t)tlCommands.size();
        toolCommandBufferInfo.ppCmds = !tlCommands.empty() ? (GvkCommandCmdBaseStructure**)tlCommands.data() : nullptr;
        toolCommandBufferInfo.collectionRangeCount = (uint32_t)tlCollectionRanges.size();
        toolCommandBufferInfo.pCollectionRanges = !tlCollectionRanges.empty() ? tlCollectionRanges.data() : nullptr;
        toolCommandBufferInfo.metricsRequestIdCount = (uint32_t)tlMetricRequestIds.size();
        toolCommandBufferInfo.pMetricsRequestIds = !tlMetricRequestIds.empty() ? tlMetricRequestIds.data() : nullptr;

        // TODO : Documentation
        gvk_result(tool_command_buffer(toolCommandBufferInfo));

        // TODO : Documentation
        auto commandStructureQueueSubmit = gvk::get_default<GvkCommandStructureQueueSubmit>();
        commandStructureQueueSubmit.queue = queue;
        commandStructureQueueSubmit.submitCount = (uint32_t)tlSubmits.size();
        commandStructureQueueSubmit.pSubmits = !tlSubmits.empty() ? tlSubmits.data() : nullptr;

        // TODO : Documentation
        auto toolQueueInfo = gvk::get_default<GvkPipelineExplorerToolQueueInfo>();
        toolQueueInfo.device = queueInfo->deviceInfo->vkHandle;
        toolQueueInfo.queue = queue;
        toolQueueInfo.pCommand = (GvkCommandBaseStructure*)&commandStructureQueueSubmit;
        toolQueueInfo.collectionRangeCount = (uint32_t)tlCollectionRanges.size();
        toolQueueInfo.pCollectionRanges = !tlCollectionRanges.empty() ? tlCollectionRanges.data() : nullptr;
        toolQueueInfo.metricsRequestIdCount = (uint32_t)tlMetricRequestIds.size();
        toolQueueInfo.pMetricsRequestIds = !tlMetricRequestIds.empty() ? tlMetricRequestIds.data() : nullptr;

        // TODO : Documentation
        gvk_result(handle_pre_process_queue_submission_callback(toolQueueInfo));

        // TODO : Documentation
        gvk_result(BasicApiCallHandler::execute_vkQueueSubmit(queue, (uint32_t)tlSubmits.size(), !tlSubmits.empty() ? tlSubmits.data() : nullptr, fence));

        // TODO : Documentation
        gvk_result(handle_post_process_queue_submission_callback(toolQueueInfo));

        // TODO : Documentation
        #if 0
        gvk_result(BasicApiCallHandler::execute_vkResetFences(queueInfo->deviceInfo->vkHandle, 1, &queueInfo->fence.get<VkFence>()));
        gvk_result(BasicApiCallHandler::execute_vkQueueSubmit(queue, 0, nullptr, fence));
        #else
        gvk_result(BasicApiCallHandler::execute_vkQueueWaitIdle(queue));
        #endif

        #if 1
        queueInfo->shaderBindingTableReplacementResources.inspect_in_use_resources();
        #endif

        ////////////////////////////////////////////////////////////////////////////////
        // TODO : Wrangle QueryManager
        //------------------------------------------------------------------------------
        // TODO : Documentation
        if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->refreshActivePipelines && !tlCollectionRanges.empty()) {
            gvk_result(BasicApiCallHandler::execute_vkQueueWaitIdle(queue));
            thread_local std::vector<uint64_t> tlTimestampQueryResults;
            gvk_result(get_timestamp_results(queueInfo, tlTimestampQueryResults));
            gvk_result(tlTimestampQueryResults.size() == tlCollectionRanges.size() * 2 ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            uint32_t timestampQueryResultIndex = 0;
            for (uint32_t collectionRange_i = 0; collectionRange_i < tlCollectionRanges.size(); ++collectionRange_i) {
                const auto& collectionRange = tlCollectionRanges[collectionRange_i];
                auto begin = tlTimestampQueryResults[timestampQueryResultIndex++];
                auto end = tlTimestampQueryResults[timestampQueryResultIndex++];
                auto duration = (end - begin) * (double)queueInfo->deviceInfo->physicalDeviceInfo->physicalDeviceProperties->limits.timestampPeriod;
                pipelineTimestampQueryResults[{ collectionRange.device, collectionRange.pipeline }] += duration;
            }
        }
        //------------------------------------------------------------------------------
        if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && !tlCollectionRanges.empty() &&
            tlMetricRequestIds.size() == 1 && tlMetricRequestIds[0].x == GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY) {
            gvk_result(BasicApiCallHandler::execute_vkQueueWaitIdle(queue));
            thread_local std::vector<uint64_t> tlPipelineStatisticsQueryResults;
            gvk_result(get_pipeline_statistics_results(queueInfo, tlPipelineStatisticsQueryResults));
            gvk_result(tlPipelineStatisticsQueryResults.size() == queueInfo->get_enabled_pipieline_statistics_count() * tlCollectionRanges.size() ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            auto pipelineStatistics = queueInfo->get_enabled_pipeline_statistics();
            uint32_t pipelineStatisticsQueryResultIndex = 0;
            for (uint32_t collectionRange_i = 0; collectionRange_i < tlCollectionRanges.size(); ++collectionRange_i) {
                const auto& collectionRange = tlCollectionRanges[collectionRange_i];
                GvkPipelineExplorerMetricId metricId{ GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, 0, 0, 0 };
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT) {
                    metricId.y = VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;
                    pipelineStatisticsQueryResults[{ collectionRange.device, collectionRange.pipeline }][metricId] += tlPipelineStatisticsQueryResults[pipelineStatisticsQueryResultIndex++];
                }
            }
        }
        ////////////////////////////////////////////////////////////////////////////////
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(BasicApiCallHandler::execute_vkQueueSubmit2(queue, submitCount, pSubmits, fence));
        gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(BasicApiCallHandler::execute_vkQueueSubmit2KHR(queue, submitCount, pSubmits, fence));
        gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    auto vkResult = BasicApiCallHandler::execute_vkQueuePresentKHR(queue, pPresentInfo);
    process_end_of_frame_and_outgoing_messages();
    process_beginning_of_frame_and_incoming_messages();
    return vkResult;
}

VkResult PipelineExplorer::reset_timestamp_query_pool(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, uint32_t queryCount)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (queryCount && (!queueInfo->timestampQueryPool || queueInfo->timestampQueryPool.get<VkQueryPoolCreateInfo>().queryCount < queryCount)) {
            auto queryPoolCreateInfo = gvk::get_default<VkQueryPoolCreateInfo>();
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolCreateInfo.queryCount = queryCount;
            gvk_result(gvk::QueryPool::create(queueInfo->deviceInfo->vkHandle, &queryPoolCreateInfo, nullptr, &queueInfo->timestampQueryPool));
        }
        dispatchTable.gvkCmdResetQueryPool(commandBuffer, queueInfo->timestampQueryPool, 0, queryCount);
        queueInfo->timestampQueryIndex = 0;
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::write_timestamp(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage)
{
    dispatchTable.gvkCmdWriteTimestamp(commandBuffer, pipelineStage, queueInfo->timestampQueryPool, queueInfo->timestampQueryIndex++);
}

VkResult PipelineExplorer::get_timestamp_results(const pipeline_explorer::QueueInfo& queueInfo, std::vector<uint64_t>& results)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        results.clear();
        results.resize(queueInfo->timestampQueryIndex);
        gvk_result(dispatchTable.gvkGetQueryPoolResults(
            queueInfo->deviceInfo->vkHandle,
            queueInfo->timestampQueryPool,
            0,
            queueInfo->timestampQueryIndex,
            sizeof(uint64_t) * results.size(),
            results.data(),
            sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::reset_pipeline_statistics_query_pool(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, VkPipeline pipeline, uint32_t queryCount)
{
    (void)pipeline;
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        VkQueryPipelineStatisticFlags pipelineStatistics = 0;
        pipeline_explorer::PipelineInfo pipelineInfo({ requestInfo->device, requestInfo->sampleMetricsPipeline });
        gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        for (const auto& shaderModuleInfoItr : pipelineInfo->shaderModuleInfos) {
            switch (shaderModuleInfoItr.first) {
            case VK_SHADER_STAGE_VERTEX_BIT: {
                pipelineStatistics |=
                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
            } break;
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
                pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;
            } break;
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
                pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
            } break;
            case VK_SHADER_STAGE_GEOMETRY_BIT: {
                pipelineStatistics |=
                    VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT;
            } break;
            case VK_SHADER_STAGE_FRAGMENT_BIT: {
                pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
            } break;
            case VK_SHADER_STAGE_COMPUTE_BIT: {
                pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
            } break;
            case VK_SHADER_STAGE_TASK_BIT_EXT: {
                pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT;
            } break;
            case VK_SHADER_STAGE_MESH_BIT_EXT: {
                pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;
            } break;
            default: {
            } break;
            }
        }
        if (pipelineStatistics) {
            // TODO : Map of query pools based on statistics?
            auto queryPoolCreateInfo = queueInfo->pipelineStatisticsQueryPool ? queueInfo->pipelineStatisticsQueryPool.get<VkQueryPoolCreateInfo>() : gvk::get_default<VkQueryPoolCreateInfo>();
            if (queryCount && (!queueInfo->pipelineStatisticsQueryPool || queryPoolCreateInfo.flags != pipelineStatistics || queryPoolCreateInfo.queryCount < queryCount)) {
                queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
                queryPoolCreateInfo.queryCount = queryCount;
                queryPoolCreateInfo.pipelineStatistics = pipelineStatistics;
                gvk_result(gvk::QueryPool::create(queueInfo->deviceInfo->vkHandle, &queryPoolCreateInfo, nullptr, &queueInfo->pipelineStatisticsQueryPool));
            }
            dispatchTable.gvkCmdResetQueryPool(commandBuffer, queueInfo->pipelineStatisticsQueryPool, 0, queryCount);
        } else {
            queueInfo->pipelineStatisticsQueryPool.reset();
            messages.push_back("INFO : Pipeline " + pipeline_explorer::uuid_to_string(pipelineInfo->uuid, 18) + " at bind point " + gvk::to_string(pipelineInfo->bindPoint, pipeline_explorer::PrinterFlags) + " has no shader stages that provide statistics counters");
            // messages.push_back("INFO : Pipeline statistics unavailable for the requested pipeline");
        }
        queueInfo->pipelineStatisticsQueryIndex = 0;
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::begin_pipeline_statistics_query(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer)
{
    if (queueInfo->pipelineStatisticsQueryPool) {
        dispatchTable.gvkCmdBeginQuery(commandBuffer, queueInfo->pipelineStatisticsQueryPool, queueInfo->pipelineStatisticsQueryIndex, 0);
    }
}

void PipelineExplorer::end_pipeline_statistics_query(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer)
{
    if (queueInfo->pipelineStatisticsQueryPool) {
        dispatchTable.gvkCmdEndQuery(commandBuffer, queueInfo->pipelineStatisticsQueryPool, queueInfo->pipelineStatisticsQueryIndex++);
    }
}

VkResult PipelineExplorer::get_pipeline_statistics_results(const pipeline_explorer::QueueInfo& queueInfo, std::vector<uint64_t>& results)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        results.clear();
        gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        if (queueInfo->pipelineStatisticsQueryPool) {
            results.resize(queueInfo->get_enabled_pipieline_statistics_count() * queueInfo->pipelineStatisticsQueryIndex);
            gvk_result(dispatchTable.gvkGetQueryPoolResults(
                queueInfo->deviceInfo->vkHandle,
                queueInfo->pipelineStatisticsQueryPool,
                0,
                queueInfo->pipelineStatisticsQueryIndex,
                sizeof(uint64_t) * results.size(),
                results.data(),
                queueInfo->get_enabled_pipeline_statistics_result_size(),
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
            ));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

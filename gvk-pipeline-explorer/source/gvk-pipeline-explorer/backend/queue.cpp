
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

#include "gvk-pipeline-explorer/backend/pipeline-explorer.hpp"

namespace gvk {

VkResult PipelineExplorer::execute_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    #if 1
    // TODO : Per queue resources so that this lock can be removed
    std::lock_guard<std::mutex> lock(queueSubmissionMutex);
    #endif

    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {

        // Get queue info and device
        pipeline_explorer::QueueInfo queueInfo = queue;
        gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk::Device gvkDevice = queueInfo->deviceInfo->vkHandle;
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);

        // Reset shader binding table replacement resources
        queueInfo->shaderBindingTableReplacementResources.reset_available_resources();

        // Prepare collections for submission tracking and modification
        thread_local std::vector<VkSubmitInfo> tlSubmits;
        thread_local std::vector<std::vector<VkCommandBuffer>> tlCommandBuffers;
        thread_local std::vector<const GvkCommandBaseStructure*> tlCommands;
        thread_local std::vector<GvkPipelineExplorerCollectionRange> tlCollectionRanges;
        thread_local std::vector<GvkPipelineExplorerMetricId> tlMetricRequestIds;
        thread_local std::vector<VkPerformanceCounterKHR> tlPerformanceCounters;
        tlSubmits.resize(submitCount);
        tlCommandBuffers.resize(submitCount);
        tlCommands.clear();
        tlCollectionRanges.clear();
        tlCollectionRanges.push_back(gvk::get_default<GvkPipelineExplorerCollectionRange>());
        tlMetricRequestIds.clear();
        tlPerformanceCounters.clear();
        if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->sampleMetricIdCount == 1 && requestInfo->pSampleMetricIds) {
            tlMetricRequestIds.push_back({ requestInfo->pSampleMetricIds[0].x, 0, 0, 0 });
            tlPerformanceCounters.push_back(gvk::get_default<VkPerformanceCounterKHR>());
            memcpy(tlPerformanceCounters.back().uuid, &requestInfo->pSampleMetricIds[0].x, sizeof(requestInfo->pSampleMetricIds[0].x));
        } else if (performanceQueryManager.get_request().sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>()) {
            const auto& request = performanceQueryManager.get_request();
            tlPerformanceCounters.insert(tlPerformanceCounters.end(), request.pCounters, request.pCounters + request.counterCount);
        }

        // Prepare GvkCommandStructureQueueSubmit
        auto commandStructureQueueSubmit = gvk::get_default<GvkCommandStructureQueueSubmit>();
        commandStructureQueueSubmit.queue = queue;
        commandStructureQueueSubmit.submitCount = submitCount;
        commandStructureQueueSubmit.pSubmits = pSubmits;
        commandStructureQueueSubmit.fence = fence;
        if (TODO_shouldBeControlledByRequestInfo_getGpuCalls) {
            auto pCommandBaseStructure = (const GvkCommandBaseStructure*)&commandStructureQueueSubmit;
            mGpuCalls.add_command(*pCommandBaseStructure);
        }

        // Process submissions
        for (uint32_t submit_i = 0; submit_i < submitCount; ++submit_i) {
            tlSubmits[submit_i] = pSubmits[submit_i];

            // Check for pNext
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

            // Process command buffers
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

        // If the last collection range is empty, it's unused, remove it
        // NOTE : Checking for !begin()/!end() assumes that the command collection has
        //  vkBeginCommandBuffer() and vkEndCommandBuffer() as its first/last commands.
        if (!tlCollectionRanges.back().begin || !tlCollectionRanges.back().end) {
            gvk_result(!tlCollectionRanges.back().device ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(!tlCollectionRanges.back().pipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(!tlCollectionRanges.back().begin ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(!tlCollectionRanges.back().end ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            tlCollectionRanges.pop_back();
        }

        // Setup GvkPipelineExplorerToolCommandBufferInfoEx
        auto toolCommandBufferInfo = gvk::get_default<GvkPipelineExplorerToolCommandBufferInfoEx>();
        toolCommandBufferInfo.physicalDevice = queueInfo->deviceInfo->physicalDeviceInfo->vkHandle;
        toolCommandBufferInfo.device = queueInfo->deviceInfo->vkHandle;
        toolCommandBufferInfo.queue = queue;
        toolCommandBufferInfo.queueFamilyIndex = queueInfo->deviceQueueCreateInfo->queueFamilyIndex;
        toolCommandBufferInfo.cmdCount = (uint32_t)tlCommands.size();
        toolCommandBufferInfo.ppCmds = !tlCommands.empty() ? (GvkCommandCmdBaseStructure**)tlCommands.data() : nullptr;
        toolCommandBufferInfo.collectionRangeCount = (uint32_t)tlCollectionRanges.size();
        toolCommandBufferInfo.pCollectionRanges = !tlCollectionRanges.empty() ? tlCollectionRanges.data() : nullptr;
        toolCommandBufferInfo.counterCount = (uint32_t)tlPerformanceCounters.size();
        toolCommandBufferInfo.pCounters = !tlPerformanceCounters.empty() ? tlPerformanceCounters.data() : nullptr;

        // Record modified command buffers
        gvk_result(tool_command_buffers(toolCommandBufferInfo));

        // Replace application submissions with modified submissions
        commandStructureQueueSubmit.submitCount = (uint32_t)tlSubmits.size();
        commandStructureQueueSubmit.pSubmits = !tlSubmits.empty() ? tlSubmits.data() : nullptr;

        // Setup GvkPipelineExplorerToolQueueInfoEx
        auto toolQueueInfo = gvk::get_default<GvkPipelineExplorerToolQueueInfoEx>();
        toolQueueInfo.physicalDevice = queueInfo->deviceInfo->physicalDeviceInfo->vkHandle;
        toolQueueInfo.device = queueInfo->deviceInfo->vkHandle;
        toolQueueInfo.queue = queue;
        toolQueueInfo.queueFamilyIndex = queueInfo->deviceQueueCreateInfo->queueFamilyIndex;
        toolQueueInfo.pCommand = (GvkCommandBaseStructure*)&commandStructureQueueSubmit;
        toolQueueInfo.cmdCount = (uint32_t)tlCommands.size();
        toolQueueInfo.ppCmds = !tlCommands.empty() ? (GvkCommandCmdBaseStructure**)tlCommands.data() : nullptr;
        toolQueueInfo.collectionRangeCount = (uint32_t)tlCollectionRanges.size();
        toolQueueInfo.pCollectionRanges = !tlCollectionRanges.empty() ? tlCollectionRanges.data() : nullptr;
        toolQueueInfo.counterCount = (uint32_t)tlPerformanceCounters.size();
        toolQueueInfo.pCounters = !tlPerformanceCounters.empty() ? tlPerformanceCounters.data() : nullptr;

        // Fire callback
        gvk_result(handle_pre_process_queue_submission_callback_ex(toolQueueInfo));

        // Execute vkQueueSubmit() with modified command buffers
        gvk_result(BasicPipelineExplorer::execute_vkQueueSubmit(queue, (uint32_t)tlSubmits.size(), !tlSubmits.empty() ? tlSubmits.data() : nullptr, fence));

        // Fire callback
        gvk_result(handle_post_process_queue_submission_callback_ex(toolQueueInfo));

        // TODO : Request managers should execute vkQueueWaitIdle() only if necessary
        gvk_result(BasicPipelineExplorer::execute_vkQueueWaitIdle(queue));

        #if 0
        // DEBUGGING :
        queueInfo->shaderBindingTableReplacementResources.inspect_in_use_resources();
        #endif

        ////////////////////////////////////////////////////////////////////////////////
        // TODO : Wrangle QueryManager
        if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->refreshActivePipelines && !tlCollectionRanges.empty()) {
            gvk_result(BasicPipelineExplorer::execute_vkQueueWaitIdle(queue));
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
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // HACK : Need to wrangle auto query vs request query

        if (autoQuery) {

            std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::vector<std::vector<gvk::pipeline_explorer::CmdSequence>>> timestampResults;
            timestampQueryManager.extract_results(timestampResults);

            gvk::pipeline_explorer::DeviceInfo deviceInfo = toolQueueInfo.device;
            gvk_result(deviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            auto timestampPeriod = (double)deviceInfo->physicalDeviceInfo->physicalDeviceProperties->limits.timestampPeriod;

            std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, GvkPipelineExplorerMetricResultInfo> pipelineResults;
            auto pCmdDurations = gvk::detail::create_dynamic_array<double>(toolQueueInfo.cmdCount, nullptr);
            memset(pCmdDurations, 0, sizeof(double) * toolQueueInfo.cmdCount);
            for (const auto& timestampResultItr : timestampResults) {
                gvk_result(timestampResultItr.second.size() == 1 ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                for (const auto& cmdSequence : timestampResultItr.second[0]) {
                    gvk_result(cmdSequence.cmdTypes.size() == 1 ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                    gvk_result(cmdSequence.firstCmdIndex < toolQueueInfo.cmdCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                    auto tickCount = cmdSequence.endTimestamp - cmdSequence.beginTimestamp;
                    pCmdDurations[cmdSequence.firstCmdIndex] = (double)tickCount * timestampPeriod;
                    if (timestampResultItr.first.get_handle()) {
                        auto& pipelineResult = pipelineResults[timestampResultItr.first];
                        ++pipelineResult.sampleCount;
                        pipelineResult.total += pCmdDurations[cmdSequence.firstCmdIndex];
                    }
                }
            }

            size_t pipelineResult_i = 0;
            static std::unordered_set<gvk::HandleId<VkDevice, VkPipeline>> sDecompiledPipelines;
            auto pPipelineResults = gvk::detail::create_dynamic_array<GvkPipelineExplorerPipelineResultInfo>(pipelineResults.size(), nullptr);
            for (auto& pipelineResultItr : pipelineResults) {
                pipeline_explorer::PipelineInfo pipelineInfo = pipelineResultItr.first;

                if (sDecompiledPipelines.insert(pipelineResultItr.first).second) {
                    decompile_pipeline(pipelineResultItr.first.get_dispatchable_handle(), pipelineResultItr.first.get_handle());
                    write_pipeline_info(pipelineResultItr.first.get_dispatchable_handle(), pipelineResultItr.first.get_handle(), pipelineInfo->path);
                }

                gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                gvk_result(pipelineResult_i < pipelineResults.size() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                auto& pipelineResult = pPipelineResults[pipelineResult_i++];
                pipelineResult.pipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
                boost::multiprecision::export_bits(pipelineInfo->uuid, pipelineResult.pipelineInfo.uuid, 8);
                boost::multiprecision::export_bits(pipelineInfo->driverUUID, pipelineResult.pipelineInfo.driverUUID, 8);
                pipelineResult.pipelineInfo.pName = "VkPipeline"; // TODO : Get name from pipelineInfo
                pipelineResult.pipelineInfo.device = pipelineInfo->deviceInfo->vkHandle;
                pipelineResult.pipelineInfo.pipeline = pipelineInfo->vkHandle;
                pipelineResult.pipelineInfo.bindPoint = pipelineInfo->bindPoint;
                pipelineResult.pipelineInfo.labelCount = 0; // TODO : Get labels from pipelineInfo
                pipelineResult.pipelineInfo.pLabels = nullptr; // TODO : Get labels from pipelineInfo
                pipelineResult.pipelineInfo.experimentEnabled = pipelineInfo->experimentEnabled;
                // TODO : pipelineResult.pipelineInfo.experimentUUID;
                pipelineResult.pipelineInfo.highlightEnabled = pipelineInfo->highlightEnabled;
                memcpy(pipelineResult.pipelineInfo.highlightColor, pipelineInfo->highlightColor, sizeof(pipelineInfo->highlightColor));
                auto& metricResult = pipelineResultItr.second;
                metricResult.average = metricResult.total;
                if (metricResult.sampleCount) {
                    metricResult.average /= (double)metricResult.sampleCount;
                }
                pipelineResult.metricResultCount = 2;
                auto pMetricResults = gvk::detail::create_dynamic_array<GvkPipelineExplorerMetricResultInfo>(2, nullptr);
                pipelineResult.pMetricResults = pMetricResults;
                pMetricResults[0] = metricResult;
                pMetricResults[0].metricInfo.id.x = GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY;
                pMetricResults[1] = gvk::get_default<GvkPipelineExplorerMetricResultInfo>();
                pMetricResults[1].sampleCount = metricResult.sampleCount;
                pMetricResults[1].total = metricResult.sampleCount;
                pMetricResults[1].average = metricResult.sampleCount;
                pMetricResults[1].metricInfo.id.x = GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT;
            }

            auto autoQueryResultInfo = gvk::get_default<GvkPipelineExplorerAutoQueryResultInfo>();
            autoQueryResultInfo.pipelineResultCount = (uint32_t)pipelineResults.size();
            autoQueryResultInfo.pPipelineResults = pPipelineResults;
            autoQueryResultInfo.commands.commandCount = toolQueueInfo.cmdCount;
            autoQueryResultInfo.commands.ppCommands = (const GvkCommandBaseStructure**)toolQueueInfo.ppCmds;
            autoQueryResultInfo.commandCount = toolQueueInfo.cmdCount;
            autoQueryResultInfo.pCmdDurations = pCmdDurations;
            gvk::write_serialized_structure(workspacePath / ".data", autoQueryResultInfo, true);

            gvk::detail::destroy_dynamic_array_copy(autoQueryResultInfo.pipelineResultCount, autoQueryResultInfo.pPipelineResults, nullptr);
            gvk::detail::destroy_dynamic_array_copy(autoQueryResultInfo.commandCount, autoQueryResultInfo.pCmdDurations, nullptr);
        }

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(BasicPipelineExplorer::execute_vkQueueSubmit2(queue, submitCount, pSubmits, fence));
        gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(BasicPipelineExplorer::execute_vkQueueSubmit2KHR(queue, submitCount, pSubmits, fence));
        gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    auto vkResult = BasicPipelineExplorer::execute_vkQueuePresentKHR(queue, pPresentInfo);
    if (TODO_shouldBeControlledByRequestInfo_getGpuCalls) {
        auto command = gvk::get_default<GvkCommandStructureQueuePresentKHR>();
        command.queue = queue;
        command.pPresentInfo = pPresentInfo;
        command.result = vkResult;
        auto pCommandBaseStructure = (const GvkCommandBaseStructure*)&command;
        mGpuCalls.add_command(*pCommandBaseStructure);
    }
    handle_post_process_range_callback_ex();

    // HACK :
    auto pluginStatus = pluginManager.get_plugin_status();
    if (pipelineStatisticsQueryManager.get_request().sType != gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>() &&
        performanceQueryManager.get_request().sType != gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() &&
        timestampQueryManager.get_request().sType != gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() &&
        commandCollectionRequestManager.get_request().sType != gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>() &&
        pluginStatus == VK_SUCCESS &&
        !autoQuery) {
        toolCallbackInfo = { };
    }

    process_end_of_frame_and_outgoing_messages();
    process_beginning_of_frame_and_incoming_messages();
    handle_pre_process_range_callback_ex();
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

} // namespace gvk

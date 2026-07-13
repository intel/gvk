
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

#include "gvk-pipeline-explorer/backend/query-managers/timestamp-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"
#include "gvk-system.hpp"

#include <algorithm>

namespace gvk {
namespace pipeline_explorer {

uint64_t TimestampQueryManager::get_type_id() const
{
    return Tool::get_type_id<TimestampQueryManager>();
}

const GvkPipelineExplorerPerformanceQueryRequestInfo& TimestampQueryManager::get_request() const
{
    return mRequest;
}

VkResult TimestampQueryManager::initialize_auto_query()
{
    mAutoQuery = true;
    return VK_SUCCESS;
}

VkResult TimestampQueryManager::submit_request(const std::filesystem::path& workspace, gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo>&& request)
{
    mWorkspace = workspace;
    if (mRequest->sType != gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>()) {
        mEnabled = true;
        mRequest = std::move(request);
        mQueryIndex = 0;
        resultIndex = 0;
        warmupRangeCount = mRequest->warmupRangeCount;
        queryRangeCount = mRequest->queryRangeCount;
        return VK_SUCCESS;
    }
    return VK_NOT_READY;
}

bool TimestampQueryManager::collect_metrics(VkDevice device, VkPipeline pipeline) const
{
    (void)device;
    (void)pipeline;
    return mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() || mAutoQuery;
}

void TimestampQueryManager::extract_results(std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::vector<std::vector<CmdSequence>>>& extractResults)
{
    extractResults = std::move(results);
    results.clear();
}

bool TimestampQueryManager::tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const
{
    return Tool::tool_command(pCommand, vkDevice, vkQueue, vkPipeline);
}

uint32_t TimestampQueryManager::get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    return toolInfo.collectionRangeCount * 2;
}

VkResult TimestampQueryManager::validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (!mQueryPool || mQueryPool.get<VkQueryPoolCreateInfo>().queryCount < get_query_count(toolInfo)) {
            auto queryPoolCreateInfo = gvk::get_default<VkQueryPoolCreateInfo>();
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolCreateInfo.queryCount = get_query_count(toolInfo);
            gvk_result(gvk::QueryPool::create(toolInfo.device, &queryPoolCreateInfo, nullptr, &mQueryPool));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::pre_process_range()
{
    return QueryManager::pre_process_range();
}

VkResult TimestampQueryManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if ((mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() || mAutoQuery) && get_query_count(toolInfo)) {
            gvk_result(validate_query_resources(toolInfo));
            gvk_result(reset_query_resources(toolInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() || mAutoQuery) {
            if (toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
                toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].begin) {
                gvk_result(write_timestamp(toolInfo, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() || mAutoQuery) {
            if (toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
                toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].end) {
                gvk_result(write_timestamp(toolInfo, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::post_process_command_buffers(toolInfo);
}

VkResult TimestampQueryManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::pre_process_queue_submission(toolInfo);
}

VkResult TimestampQueryManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if ((mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() || mAutoQuery) && toolInfo.collectionRangeCount) {
            gvk::pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo = toolInfo.physicalDevice;
            gvk_result_assert(physicalDeviceInfo);
            gvk::Device gvkDevice = toolInfo.device;
            gvk_result_assert(gvkDevice);
            gvk::Queue gvkQueue = toolInfo.queue;
            gvk_result_assert(gvkQueue);
            gvk_result_assert(mQueryPool);

            // Wait for submission to complete before extracting results
            // NOTE : VK_QUERY_RESULT_WAIT_BIT should make a call to vkQueueWaitIdle()
            //  unnecessary but it's not totally reliable depending on implementation
            gvk_result(gvkQueue.QueueWaitIdle());

            // Get results
            std::vector<uint64_t> resultData(mQueryIndex);
            gvk_result(gvkDevice.GetQueryPoolResults(
                mQueryPool,
                0,
                mQueryIndex,
                sizeof(uint64_t) * resultData.size(),
                resultData.data(),
                sizeof(uint64_t),
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
            ));

            // Process results
            uint32_t resultsData_i = 0;
            gvk_result_assert(toolInfo.pCollectionRanges);
            for (uint32_t collectionRange_i = 0; collectionRange_i < toolInfo.collectionRangeCount; ++collectionRange_i) {
                const auto& collectionRange = toolInfo.pCollectionRanges[collectionRange_i];
                CmdSequence cmdSequence{ };
                gvk_result_assert(resultsData_i < resultData.size());
                cmdSequence.beginTimestamp = resultData[resultsData_i++];
                gvk_result_assert(resultsData_i < resultData.size());
                cmdSequence.endTimestamp = resultData[resultsData_i++];
                cmdSequence.firstCmdIndex = collectionRange.begin;
                cmdSequence.cmdTypes.resize(collectionRange.end - collectionRange.begin + 1);
                gvk_result_assert(toolInfo.ppCmds);

                // Process cmd types
                for (uint32_t cmd_i = 0; cmd_i < cmdSequence.cmdTypes.size(); ++cmd_i) {
                    gvk_result_assert(cmdSequence.firstCmdIndex + cmd_i < toolInfo.cmdCount);
                    cmdSequence.cmdTypes[cmd_i] = toolInfo.ppCmds[cmdSequence.firstCmdIndex + cmd_i]->sType;
                }

                // Prepare next results collection
                auto& rangeResults = results[{ collectionRange.device, collectionRange.pipeline }];
                rangeResults.reserve(warmupRangeCount + queryRangeCount);
                if (rangeResults.size() < resultIndex + 1) {
                    rangeResults.resize(resultIndex + 1);
                }
                rangeResults[resultIndex].push_back(cmdSequence);
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::pre_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::pre_process_queue_present(toolInfo);
}

VkResult TimestampQueryManager::post_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::post_process_queue_present(toolInfo);
}

VkResult TimestampQueryManager::post_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (!mWorkspace.empty()) {
            ++resultIndex;
            if (warmupRangeCount) {
                --warmupRangeCount;
            } else if (queryRangeCount) {
                --queryRangeCount;
            }
            if (!queryRangeCount) {
                gvk_result(publish_result());
                mRequest.reset();
                mWorkspace.clear();
                mQueryPool.reset();
                mQueryIndex = 0;
                warmupRangeCount = 0;
                queryRangeCount = 0;
                results.clear();
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::write_timestamp(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo, VkPipelineStageFlagBits pipelineStage)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() || mAutoQuery);
        gvk::Queue gvkQueue = toolInfo.queue;
        gvk_result_assert(gvkQueue);
        gvk_result_assert(mQueryPool);
        gvk_result_assert(toolInfo.cmdIndex < toolInfo.cmdCount);
        gvk_result_assert(toolInfo.ppCmds);
        auto commandBuffer = toolInfo.ppCmds[toolInfo.cmdIndex]->commandBuffer;
        gvk_result_assert(commandBuffer);
        gvkQueue.get<gvk::DispatchTable>().gvkCmdWriteTimestamp(commandBuffer, pipelineStage, mQueryPool, mQueryIndex++);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimestampQueryManager::publish_result() const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>());
        gvk_result_assert(!results.empty());
        gvk::pipeline_explorer::DeviceInfo deviceInfo = results.begin()->first.get_dispatchable_handle(); // mRequest->device;
        gvk_result_assert(deviceInfo);

        // Get date and time strings
        auto dateTime = gvk::system::DateTime::now();
        auto dateStr = dateTime.get_date_str();
        auto timeStr = dateTime.get_time_str();

        // Setup GvkPipelineExplorerTimestampQueryResultInfo
        auto timestampQueryResultInfo = gvk::get_default<GvkPipelineExplorerTimestampQueryResultInfo>();
        timestampQueryResultInfo.pName = nullptr;
        timestampQueryResultInfo.pDate = dateStr.c_str();
        timestampQueryResultInfo.pTime = timeStr.c_str();
        timestampQueryResultInfo.pNote = "ticks to nanoseconds : cmdSequence.totalCmdDuration = (cmdSequence.endTimestamp - cmdSequence.beginTimestamp) * timestampPeriod";
        timestampQueryResultInfo.timestampPeriod = deviceInfo->physicalDeviceInfo->physicalDeviceProperties->limits.timestampPeriod;
        timestampQueryResultInfo.pipelineTimestampQueryResultCount = (uint32_t)results.size();
        auto pPipelineTimestampQueryResults = gvk::detail::create_dynamic_array<GvkPipelineExplorerPipelineTimestampQueryResultInfo>(timestampQueryResultInfo.pipelineTimestampQueryResultCount, nullptr);
        timestampQueryResultInfo.pPipelineTimestampQueryResults = pPipelineTimestampQueryResults;

        // Prepare results report
        auto pipeline_i = 0;
        for (const auto& resultItr : results) {
            gvk::pipeline_explorer::PipelineInfo pipelineInfo(resultItr.first);
            gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            auto& pipelineTimestampQueryResults = pPipelineTimestampQueryResults[pipeline_i++];
            pipelineTimestampQueryResults = gvk::get_default<GvkPipelineExplorerPipelineTimestampQueryResultInfo>();
            pipelineTimestampQueryResults.pipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
            boost::multiprecision::export_bits(pipelineInfo->uuid, pipelineTimestampQueryResults.pipelineInfo.uuid, 8);
            boost::multiprecision::export_bits(pipelineInfo->driverUUID, pipelineTimestampQueryResults.pipelineInfo.driverUUID, 8);
            pipelineTimestampQueryResults.pipelineInfo.pName = "VkPipeline"; // TODO : Get name from pipelineInfo
            pipelineTimestampQueryResults.pipelineInfo.device = pipelineInfo->deviceInfo->vkHandle;
            pipelineTimestampQueryResults.pipelineInfo.pipeline = pipelineInfo->vkHandle;
            pipelineTimestampQueryResults.pipelineInfo.bindPoint = pipelineInfo->bindPoint;
            pipelineTimestampQueryResults.pipelineInfo.labelCount = 0; // TODO : Get labels from pipelineInfo
            pipelineTimestampQueryResults.pipelineInfo.pLabels = nullptr; // TODO : Get labels from pipelineInfo
            pipelineTimestampQueryResults.pipelineInfo.experimentEnabled = pipelineInfo->experimentEnabled;
            // TODO : pipelineTimestampQueryResults.pipelineInfo.experimentUUID;
            pipelineTimestampQueryResults.pipelineInfo.highlightEnabled = pipelineInfo->highlightEnabled;
            memcpy(pipelineTimestampQueryResults.pipelineInfo.highlightColor, pipelineInfo->highlightColor, sizeof(pipelineInfo->highlightColor));

            // Setup GvkPipelineExplorerTimestampQueryCmdRangeResultInfo array
            gvk_result(resultItr.second.size() == mRequest->warmupRangeCount + mRequest->queryRangeCount ? VK_SUCCESS : VK_INCOMPLETE);
            pipelineTimestampQueryResults.cmdRangeResultCount = mRequest->queryRangeCount;
            auto pCmdRangeResults = gvk::detail::create_dynamic_array<GvkPipelineExplorerTimestampQueryCmdRangeResultInfo>(pipelineTimestampQueryResults.cmdRangeResultCount, nullptr);
            pipelineTimestampQueryResults.pCmdRangeResults = pCmdRangeResults;

            // Populate GvkPipelineExplorerTimestampQueryCmdRangeResultInfo array
            uint64_t totalCmdRangeTicks = 0;
            for (uint32_t cmdRange_i = 0; cmdRange_i < pipelineTimestampQueryResults.cmdRangeResultCount; ++cmdRange_i) {
                auto& cmdRangeResult = pCmdRangeResults[cmdRange_i];
                cmdRangeResult = gvk::get_default<GvkPipelineExplorerTimestampQueryCmdRangeResultInfo>();
                cmdRangeResult.cmdSequenceCount = (uint32_t)resultItr.second[mRequest->warmupRangeCount + cmdRange_i].size();
                auto pCmdSequences = gvk::detail::create_dynamic_array<GvkPipelineExplorerTimestampQueryCmdSequenceResultInfo>(cmdRangeResult.cmdSequenceCount, nullptr);
                cmdRangeResult.pCmdSequences = pCmdSequences;

                // Process cmd sequence
                uint64_t totalCmdSequenceTicks = 0;
                for (uint32_t cmdSequence_i = 0; cmdSequence_i < cmdRangeResult.cmdSequenceCount; ++cmdSequence_i) {
                    auto& cmdSequence = pCmdSequences[cmdSequence_i];
                    cmdSequence = gvk::get_default<GvkPipelineExplorerTimestampQueryCmdSequenceResultInfo>();
                    cmdSequence.beginTimestamp = resultItr.second[mRequest->warmupRangeCount + cmdRange_i][cmdSequence_i].beginTimestamp;
                    cmdSequence.endTimestamp = resultItr.second[mRequest->warmupRangeCount + cmdRange_i][cmdSequence_i].endTimestamp;
                    auto tickCount = cmdSequence.endTimestamp - cmdSequence.beginTimestamp;
                    cmdSequence.totalCmdDuration = (double)tickCount * (double)timestampQueryResultInfo.timestampPeriod;
                    cmdSequence.firstCmdIndex = resultItr.second[mRequest->warmupRangeCount + cmdRange_i][cmdSequence_i].firstCmdIndex;
                    cmdSequence.cmdCount = (uint32_t)resultItr.second[mRequest->warmupRangeCount + cmdRange_i][cmdSequence_i].cmdTypes.size();
                    cmdSequence.averageCmdDuration = cmdSequence.totalCmdDuration / (double)cmdSequence.cmdCount;
                    auto pCmdTypes = gvk::detail::create_dynamic_array<GvkCommandStructureType>(cmdSequence.cmdCount, nullptr);
                    cmdSequence.pCmdTypes = pCmdTypes;

                    // Get cmd type
                    for (uint32_t cmd_i = 0; cmd_i < cmdSequence.cmdCount; ++cmd_i) {
                        pCmdTypes[cmd_i] = resultItr.second[mRequest->warmupRangeCount + cmdRange_i][cmdSequence_i].cmdTypes[cmd_i];
                    }

                    // Add ticks and counts
                    totalCmdRangeTicks += tickCount;
                    totalCmdSequenceTicks += tickCount;
                    cmdRangeResult.totalCmdSequenceCmdCount += cmdSequence.cmdCount;
                    pipelineTimestampQueryResults.totalCmdRangeCmdCount += cmdSequence.cmdCount;
                }

                // Calculate totals and averages
                cmdRangeResult.totalCmdSequenceDuration = (double)totalCmdSequenceTicks * (double)timestampQueryResultInfo.timestampPeriod;
                cmdRangeResult.averageCmdSequenceDuration = cmdRangeResult.totalCmdSequenceDuration / (double)cmdRangeResult.totalCmdSequenceCmdCount;
                cmdRangeResult.averageCmdSequenceCmdCount = (double)cmdRangeResult.totalCmdSequenceCmdCount / (double)cmdRangeResult.cmdSequenceCount;
            }

            // Calculate totals and averages
            pipelineTimestampQueryResults.totalCmdRangeDuration = (double)totalCmdRangeTicks * (double)timestampQueryResultInfo.timestampPeriod;
            pipelineTimestampQueryResults.averageCmdRangeDuration = pipelineTimestampQueryResults.totalCmdRangeDuration / (double)pipelineTimestampQueryResults.totalCmdRangeCmdCount;
            pipelineTimestampQueryResults.averageCmdRangeCmdCount = (double)pipelineTimestampQueryResults.totalCmdRangeCmdCount / (double)pipelineTimestampQueryResults.cmdRangeResultCount;
        }

        // Write report
        if (mRequest->pReportPath) {
            std::filesystem::path reportPath = mRequest->pReportPath;
            std::filesystem::create_directories(reportPath);
            auto dateTimeStr = gvk::string::replace(dateStr, "/", "-") + "_" + gvk::string::replace(timeStr, ":", "-");
            reportPath /= dateTimeStr + ".GvkPipelineExplorerTimestampQueryResultInfo.json";
            std::ofstream file(reportPath);
            file << gvk::to_string(timestampQueryResultInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // Send message to frontend
        if (!mWorkspace.empty()) {
            gvk::write_serialized_structure(mWorkspace / ".data", timestampQueryResultInfo);
        }

        // Clear pointers to memory not owned by result structure
        timestampQueryResultInfo.pName = nullptr;
        timestampQueryResultInfo.pDate = nullptr;
        timestampQueryResultInfo.pTime = nullptr;
        timestampQueryResultInfo.pNote = nullptr;
        gvk::detail::destroy_structure_copy(timestampQueryResultInfo, nullptr);
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk

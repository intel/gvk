
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

#include "gvk-pipeline-explorer/backend/query-managers/pipeline-statistics-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"
#include "gvk-system.hpp"

#include <bitset>
#include <filesystem>
#include <unordered_map>

namespace gvk {
namespace pipeline_explorer {

uint64_t PipelineStatisticsQueryManager::get_type_id() const
{
    return Tool::get_type_id<PipelineStatisticsQueryManager>();
}

void PipelineStatisticsQueryManager::reset()
{
    QueryManager::reset();
    mWorkspace.clear();
    mRequest.reset();
    mResults.clear();
}

const GvkPipelineExplorerPipelineStatisticsQueryRequestInfo& PipelineStatisticsQueryManager::get_request() const
{
    return mRequest;
}

VkResult PipelineStatisticsQueryManager::submit_request(const std::filesystem::path& workspace, gvk::Auto<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>&& request)
{
    mWorkspace = workspace;
    if (mRequest->sType != gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>()) {
        mRequest = std::move(request);
        mResults.push_back({ });
        mEnabled = true;
    }
    return VK_SUCCESS;
}

bool PipelineStatisticsQueryManager::collect_metrics(VkDevice device, VkPipeline pipeline) const
{
    // TODO : Move to RequestManager
    return
        mRequest->sType == gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>() &&
        mRequest->device == device &&
        mRequest->pipeline == pipeline;
}

bool PipelineStatisticsQueryManager::tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const
{
    (void)pCommand;
    (void)vkQueue;
    return
        mRequest->sType == gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>() &&
        mRequest->device == vkDevice &&
        mRequest->pipeline == vkPipeline;
}

uint32_t PipelineStatisticsQueryManager::get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    return QueryManager::get_query_count(toolInfo);
}

uint32_t PipelineStatisticsQueryManager::get_query_count(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const
{
    return QueryManager::get_query_count(toolInfo);
}

VkResult PipelineStatisticsQueryManager::validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // Use pipeline shader stages to determine which pipeline statistics to enable
        VkQueryPipelineStatisticFlags pipelineStatistics = 0;
        for (uint32_t collectionRange_i = 0; collectionRange_i < toolInfo.collectionRangeCount; ++collectionRange_i) {
            const auto& collectionRange = toolInfo.pCollectionRanges[collectionRange_i];
            pipeline_explorer::PipelineInfo pipelineInfo({ collectionRange.device, collectionRange.pipeline });
            gvk_result_assert(pipelineInfo);
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
                    pipelineStatistics |=
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
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
                case VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI: {
                    pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI;
                } break;
                default: {
                } break;
                }
            }
        }

        // If any pipeline statistics are available for query create query pool
        if (pipelineStatistics) {
            auto queryPoolCreateInfo = gvk::get_default<VkQueryPoolCreateInfo>();
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
            queryPoolCreateInfo.queryCount = toolInfo.collectionRangeCount;
            queryPoolCreateInfo.pipelineStatistics = pipelineStatistics;
            gvk_result(gvk::QueryPool::create(toolInfo.device, &queryPoolCreateInfo, nullptr, &mQueryPool));
        } else {
            mQueryPool.reset();
            // TODO : Message to GUI...
            //  messages.push_back("INFO : Pipeline " + pipeline_explorer::uuid_to_string(pipelineInfo->uuid, 18) + " at bind point " + gvk::to_string(pipelineInfo->bindPoint, pipeline_explorer::PrinterFlags) + " has no shader stages that provide statistics counters");
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineStatisticsQueryManager::reset_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::reset_query_resources(toolInfo);
}

VkResult PipelineStatisticsQueryManager::pre_process_range()
{
    return QueryManager::pre_process_range();
}

VkResult PipelineStatisticsQueryManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::pre_process_command_buffers(toolInfo);
}

VkResult PipelineStatisticsQueryManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>() && mQueryPool) {
            if (toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
                toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].begin) {
                gvk::Queue gvkQueue = toolInfo.queue;
                gvk_result_assert(gvkQueue);
                gvk_result_assert(mQueryPool);
                gvk_result_assert(toolInfo.cmdIndex < toolInfo.cmdCount);
                gvk_result_assert(toolInfo.ppCmds);
                auto commandBuffer = toolInfo.ppCmds[toolInfo.cmdIndex]->commandBuffer;
                gvk_result_assert(commandBuffer);
                gvkQueue.get<gvk::DispatchTable>().gvkCmdBeginQuery(commandBuffer, mQueryPool, mQueryIndex, 0);
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineStatisticsQueryManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>() && mQueryPool) {
            if (toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
                toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].end) {
                gvk::Queue gvkQueue = toolInfo.queue;
                gvk_result_assert(gvkQueue);
                gvk_result_assert(mQueryPool);
                gvk_result_assert(toolInfo.cmdIndex < toolInfo.cmdCount);
                gvk_result_assert(toolInfo.ppCmds);
                auto commandBuffer = toolInfo.ppCmds[toolInfo.cmdIndex]->commandBuffer;
                gvk_result_assert(commandBuffer);
                gvkQueue.get<gvk::DispatchTable>().gvkCmdEndQuery(commandBuffer, mQueryPool, mQueryIndex++);
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineStatisticsQueryManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::post_process_command_buffers(toolInfo);
}

VkResult PipelineStatisticsQueryManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::pre_process_queue_submission(toolInfo);
}

VkResult PipelineStatisticsQueryManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (get_query_count(toolInfo) && mQueryPool) {
            gvk::Device gvkDevice = toolInfo.device;
            gvk_result_assert(gvkDevice);
            gvk::Queue gvkQueue = toolInfo.queue;
            gvk_result_assert(gvkQueue);

            // In theory passing VK_QUERY_RESULT_WAIT_BIT to vkGetQueryPoolResults() should
            //  make it unnecessary to call vkQueueWaitIdle(), but omitting it yields some
            //  clearly incorrect 0 results.  Worth looking into, but simply waiting here
            //  should provide consistent behavior across different implementations/drivers.
            gvk_result(gvkQueue.QueueWaitIdle());

            // Get query pool results
            auto pipelineStatistics = mQueryPool.get<VkQueryPoolCreateInfo>().pipelineStatistics;
            auto pipelineStatisticsCount = std::bitset<32>(pipelineStatistics).count();
            std::vector<uint64_t> resultsData(pipelineStatisticsCount * mQueryIndex);
            gvk_result(gvkDevice.GetQueryPoolResults(
                mQueryPool,
                0,
                mQueryIndex,
                sizeof(uint64_t) * resultsData.size(),
                resultsData.data(),
                pipelineStatisticsCount * sizeof(uint64_t),
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
            ));

            // Add results for each enabled pipeline statistic
            uint32_t result_i = 0;
            for (uint32_t query_i = 0; query_i < mQueryIndex; ++query_i) {
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT] += resultsData[result_i++];
                }
                if (pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI) {
                    mResults.back()[VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI] += resultsData[result_i++];
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineStatisticsQueryManager::post_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>()) {
            gvk_result_assert(mResults.size() <= mRequest->warmupRangeCount + mRequest->queryRangeCount);
            if (mResults.size() == mRequest->warmupRangeCount + mRequest->queryRangeCount) {
                gvk_result(generate_report());
                reset();
            } else {
                mResults.push_back({ });
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineStatisticsQueryManager::generate_report()
{
    // TODO : Publishing to workspace/GUI should be handled by RequestManager

    gvk_result_scope_begin(VK_SUCCESS) {
        gvk::pipeline_explorer::PipelineInfo pipelineInfo({ mRequest->device, mRequest->pipeline });
        gvk_result_assert(pipelineInfo);

        // Get date and time strings
        auto dateTime = gvk::system::DateTime::now();
        auto dateStr = dateTime.get_date_str();
        auto timeStr = dateTime.get_time_str();

        // mResults is a std::vector<> containing a std::map<> of results for each of
        //  warmupRangeCount + sampleRangeCount values; so create a std::map<> of
        //  std::vector<> to organize values for the report
        std::map<VkQueryPipelineStatisticFlagBits, std::vector<double>> results;
        for (const auto& result : mResults) {
            for (const auto& statisticItr : result) {
                results[statisticItr.first].push_back(statisticItr.second);
            }
        }

        // If a particular result had no values for some or all of the range, its
        //  std::vector<> will be shorter so ensure every result collection is at
        //  least warmupRangeCount + queryRangeCount, even if its all zeros
        for (const auto& result : mResults) {
            for (const auto& statisticItr : result) {
                gvk_result(results[statisticItr.first].size() <= mRequest->warmupRangeCount + mRequest->queryRangeCount ? VK_SUCCESS : VK_ERROR_UNKNOWN);
                if (results[statisticItr.first].size() < mRequest->warmupRangeCount + mRequest->queryRangeCount) {
                    results[statisticItr.first].resize(mRequest->warmupRangeCount + mRequest->queryRangeCount);
                }
            }
        }

        // Prepare report
        auto pipelineStatisticsQueryResultInfo = gvk::get_default<GvkPipelineExplorerPipelineStatisticsQueryResultInfo>();
        pipelineStatisticsQueryResultInfo.pName = nullptr;
        pipelineStatisticsQueryResultInfo.pDate = dateStr.c_str();
        pipelineStatisticsQueryResultInfo.pTime = timeStr.c_str();
        pipelineStatisticsQueryResultInfo.pNote = nullptr;
        pipelineStatisticsQueryResultInfo.pipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
        boost::multiprecision::export_bits(pipelineInfo->uuid, pipelineStatisticsQueryResultInfo.pipelineInfo.uuid, 8);
        boost::multiprecision::export_bits(pipelineInfo->driverUUID, pipelineStatisticsQueryResultInfo.pipelineInfo.driverUUID, 8);
        pipelineStatisticsQueryResultInfo.pipelineInfo.pName = "VkPipeline"; // TODO : Get name from pipelineInfo
        pipelineStatisticsQueryResultInfo.pipelineInfo.device = pipelineInfo->deviceInfo->vkHandle;
        pipelineStatisticsQueryResultInfo.pipelineInfo.pipeline = pipelineInfo->vkHandle;
        pipelineStatisticsQueryResultInfo.pipelineInfo.bindPoint = pipelineInfo->bindPoint;
        pipelineStatisticsQueryResultInfo.pipelineInfo.labelCount = 0; // TODO : Get labels from pipelineInfo
        pipelineStatisticsQueryResultInfo.pipelineInfo.pLabels = nullptr; // TODO : Get labels from pipelineInfo
        pipelineStatisticsQueryResultInfo.pipelineInfo.experimentEnabled = pipelineInfo->experimentEnabled;
        // TODO : pipelineStatisticsQueryResultInfo.pipelineInfo.experimentUUID;
        pipelineStatisticsQueryResultInfo.pipelineInfo.highlightEnabled = pipelineInfo->highlightEnabled;
        memcpy(pipelineStatisticsQueryResultInfo.pipelineInfo.highlightColor, pipelineInfo->highlightColor, sizeof(pipelineInfo->highlightColor));
        pipelineStatisticsQueryResultInfo.counterResultCount = (uint32_t)results.size();
        auto pCounterResults = gvk::detail::create_dynamic_array<GvkPipelineExplorerPerformanceCounterResultInfo>(pipelineStatisticsQueryResultInfo.counterResultCount, nullptr);
        pipelineStatisticsQueryResultInfo.pCounterResults = pCounterResults;

        // Process results
        auto pCounterResult = pCounterResults;
        for (const auto& resultsItr : results) {

            // Setup GvkPipelineExplorerPerformanceCounterResultInfo
            *pCounterResult = gvk::get_default<GvkPipelineExplorerPerformanceCounterResultInfo>();
            pCounterResult->counter = gvk::get_default<VkPerformanceCounterKHR>();
            pCounterResult->counter.unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
            pCounterResult->counter.scope = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR;
            pCounterResult->counter.storage = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR;
            gvk_result_assert(sizeof(resultsItr.first) <= sizeof(pCounterResult->counter.uuid));
            memcpy(pCounterResult->counter.uuid, &resultsItr.first, sizeof(resultsItr.first));
            pCounterResult->description = gvk::get_default<VkPerformanceCounterDescriptionKHR>();

            // TODO : DRY
            //  pipeline-explorer.cpp
            //  pipeline-statistics-query-manager.cpp
            auto pipelineStatisticFlagStr = gvk::to_string(resultsItr.first, gvk::Printer::Default ^ gvk::Printer::EnumValue);
            pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "VK_QUERY_PIPELINE_STATISTIC_");
            pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "_BIT");
            pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "\"");
            std::string pipelineStatisticNameStr;
            for (auto token : gvk::string::split_snake_case(pipelineStatisticFlagStr)) {
                gvk_result_assert(!token.empty());
                token = gvk::string::to_lower(token);
                token[0] = gvk::string::to_upper(token[0]);
                if (!pipelineStatisticNameStr.empty()) {
                    pipelineStatisticNameStr += " ";
                }
                pipelineStatisticNameStr += token;
            }
#ifdef GVK_PLATFORM_WINDOWS
            strcpy_s(pCounterResult->description.name, sizeof(pCounterResult->description.name) - 1, pipelineStatisticNameStr.c_str());
#else
            // TODO :
#endif // GVK_PLATFORM_WINDOWS

            // Allocate values array
            pCounterResult->valueCount = mRequest->queryRangeCount;
            auto pValues = gvk::detail::create_dynamic_array<double>(pCounterResult->valueCount, nullptr);
            pCounterResult->pValues = pValues;

            // Populate values array and add each value to total
            gvk_result_assert(resultsItr.second.size() == mRequest->warmupRangeCount + mRequest->queryRangeCount);
            for (uint32_t value_i = 0; value_i < pCounterResult->valueCount; ++value_i) {
                pValues[value_i] = resultsItr.second[mRequest->warmupRangeCount + value_i];
                pCounterResult->total += pValues[value_i];
            }

            // Calculate average
            pCounterResult->average = pCounterResult->total / (double)pCounterResult->valueCount;

            // Iterate
            ++pCounterResult;
        }

        // Write report
        if (mRequest->pReportPath) {
            std::filesystem::path reportPath = mRequest->pReportPath;
            std::filesystem::create_directories(reportPath);
            auto dateTimeStr = gvk::string::replace(dateStr, "/", "-") + "_" + gvk::string::replace(timeStr, ":", "-");
            reportPath /= dateTimeStr + ".GvkPipelineExplorerPipelineStatisticsQueryResultInfo.json";
            std::ofstream file(reportPath);
            file << gvk::to_string(pipelineStatisticsQueryResultInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // Send message to frontend
        if (!mWorkspace.empty()) {
            gvk::write_serialized_structure(mWorkspace / ".data", pipelineStatisticsQueryResultInfo);
        }

        // Clear pointers to memory not owned by result structure
        pipelineStatisticsQueryResultInfo.pName = nullptr;
        pipelineStatisticsQueryResultInfo.pDate = nullptr;
        pipelineStatisticsQueryResultInfo.pTime = nullptr;
        pipelineStatisticsQueryResultInfo.pNote = nullptr;
        pipelineStatisticsQueryResultInfo.pipelineInfo.pName = nullptr;
        gvk::detail::destroy_structure_copy(pipelineStatisticsQueryResultInfo, nullptr);
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk

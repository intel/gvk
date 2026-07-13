
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

#include "gvk-pipeline-explorer/backend/query-managers/performance-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"
#include "gvk-system.hpp"

#include <algorithm>
#include <array>

namespace gvk {
namespace pipeline_explorer {

static std::array<uint8_t, VK_UUID_SIZE> get_uuid(const VkPerformanceCounterKHR& counter)
{
    std::array<uint8_t, VK_UUID_SIZE> uuid{ };
    assert(sizeof(uuid) == sizeof(counter.uuid));
    memcpy(uuid.data(), counter.uuid, sizeof(uuid));
    return uuid;
}

uint64_t PerformanceQueryManager::get_type_id() const
{
    return Tool::get_type_id<PerformanceQueryManager>();
}

void PerformanceQueryManager::reset()
{
    QueryManager::reset();
    mWorkspace.clear();
    mRequest.reset();
    mPendingRequests.clear();
    mCurrentRequests.clear();
    mCompleteRequests.clear();
}

const GvkPipelineExplorerPerformanceQueryRequestInfo& PerformanceQueryManager::get_request() const
{
    return mRequest;
}

VkResult PerformanceQueryManager::submit_request(const std::filesystem::path& workspace, gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo>&& request)
{
    // TODO : mWorkspace should be handled by RequestManager
    gvk_result_scope_begin(VK_SUCCESS) {
        mWorkspace = workspace;
        if (mRequest->sType != gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>()) {
            mRequest = std::move(request);
            gvk::pipeline_explorer::DeviceInfo deviceInfo = mRequest->device;
            gvk_result_assert(deviceInfo);

            // Loop over requested counters and add each to the mPendingRequests collection.
            //  Each iteration of the range-of-interest, some subset of the counters that
            //  can be queried simultaneously (possibly all) will be selected and processed.
            //  Each iteration, requests will be returned to mPendingRequests until they've
            //  been processed warmupRangeCount + sampleRangeCount times.
            for (uint32_t counter_i = 0; counter_i < mRequest->counterCount; ++counter_i) {
                auto uuid = get_uuid(mRequest->pCounters[counter_i]);
                RequestManager resultManager{ };
                gvk_result(deviceInfo->get_performance_counter(uuid, &resultManager.counter, &resultManager.description));
                resultManager.results.push_back({ });
                mPendingRequests[uuid] = resultManager;
            }

            // AcquireProfilingLockKHR().  The profiling lock will be held until every
            //  requested counter has warmupRangeCount + sampleRangeCount results.
            // TODO : Need to setup a timeout so that if another active tool is holding the
            //  system-wide profiling lock, Pipeline Explorer can reject the request and
            //  move on.
            auto acquireProfilingLockInfo = gvk::get_default<VkAcquireProfilingLockInfoKHR>();
            acquireProfilingLockInfo.timeout = UINT64_MAX;
            gvk_result(gvk::Device(mRequest->device).AcquireProfilingLockKHR(&acquireProfilingLockInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

bool PerformanceQueryManager::collect_metrics(VkDevice device, VkPipeline pipeline) const
{
    // TODO : Move to RequestManager
    return
        mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() &&
        mRequest->device == device &&
        mRequest->pipeline == pipeline;
}

bool PerformanceQueryManager::tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const
{
    (void)pCommand;
    (void)vkQueue;
    return
        mRequest->sType == gvk::get_stype<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>() &&
        mRequest->device == vkDevice &&
        mRequest->pipeline == vkPipeline;
}

uint32_t PerformanceQueryManager::get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    return QueryManager::get_query_count(toolInfo);
}

uint32_t PerformanceQueryManager::get_query_count(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const
{
    return QueryManager::get_query_count(toolInfo);
}

VkResult PerformanceQueryManager::validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk::PhysicalDevice gvkPhysicalDevice = toolInfo.physicalDevice;
        gvk_result_assert(gvkPhysicalDevice);
        gvk::Device gvkDevice = toolInfo.device;
        gvk_result_assert(gvkDevice);
        gvk::pipeline_explorer::DeviceInfo deviceInfo = toolInfo.device;
        gvk_result_assert(deviceInfo);
        const auto& queueFamilyInfoItr = deviceInfo->queueFamilyInfos.find(toolInfo.queueFamilyIndex);
        gvk_result_assert(queueFamilyInfoItr != deviceInfo->queueFamilyInfos.end());
        const auto& queueFamilyInfo = queueFamilyInfoItr->second;

        // Run through pending requests and find the indices for each requested counter
        //  based on the queue family this submission is occurring on.  Note that it is
        //  possible for the same counter to be available to multiple queue families,
        //  but have a different index on each.
        std::vector<uint32_t> counterIndices;
        counterIndices.reserve(mRequest->counterCount);
        for (const auto& pendingRequestItr : mPendingRequests) {
            const auto& counterIndexItr = queueFamilyInfo.performanceCounterIndices.find(pendingRequestItr.first);
            gvk_result_assert(counterIndexItr != queueFamilyInfo.performanceCounterIndices.end());
            auto itr = std::lower_bound(counterIndices.begin(), counterIndices.end(), counterIndexItr->second);
            counterIndices.insert(itr, counterIndexItr->second);
        }

        // Prepare a VkQueryPoolPerformanceCreateInfoKHR with all of the requested
        //  counters pending processing.
        auto performanceQueryCreateInfo = gvk::get_default<VkQueryPoolPerformanceCreateInfoKHR>();
        performanceQueryCreateInfo.queueFamilyIndex = toolInfo.queueFamilyIndex;
        performanceQueryCreateInfo.counterIndexCount = (uint32_t)counterIndices.size();
        performanceQueryCreateInfo.pCounterIndices = counterIndices.data();

        // Check if all of the counters can be queried in a single pass, if not remove
        //  the last counter, loop and check again.  Continue removing and looping til
        //  it's a collection that can be queried in a single pass, possibly resulting
        //  in just a single counter.
        // NOTE : This logic assumes that counters grouped together contiguously tend
        //  to be able to be queried at the same time...it's not always true, but it's
        //  true often enough that this is a fine approach for 10-20 counters or so.
        //  If queries of many more counters (50-100+) become necessary, it may be time
        //  to find a better strategy.
        uint32_t passCount = 0;
        while (passCount != 1 && performanceQueryCreateInfo.counterIndexCount) {
            gvkPhysicalDevice.GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(&performanceQueryCreateInfo, &passCount);
            if (passCount != 1 && performanceQueryCreateInfo.counterIndexCount) {
                --performanceQueryCreateInfo.counterIndexCount;
            }
        }

        // Sanity check the results of the last operation
        gvk_result_assert(passCount == 1);
        gvk_result_assert(performanceQueryCreateInfo.counterIndexCount);

        // Extract the requests for the prepared VkQueryPoolPerformanceCreateInfoKHR
        //  from the mPendingRequests collection and add them to the mCurrentRequests
        //  collection.
        for (uint32_t counter_i = 0; counter_i < performanceQueryCreateInfo.counterIndexCount; ++counter_i) {
            gvk_result_assert(counter_i < queueFamilyInfo.performanceCounters.size());
            const auto& counter = queueFamilyInfo.performanceCounters[performanceQueryCreateInfo.pCounterIndices[counter_i]];
            auto node = mPendingRequests.extract(get_uuid(counter));
            gvk_result_assert(node);
            mCurrentRequests.push_back(std::move(node.mapped()));
        }

        // Prepare a VkQueryPoolCreateInfo and set it's queryCount to match the current
        //  VkQueryPoolCreateInfo.  If the VkQueryPoolCreateInfos aren't equivalent it
        //  indicates that the QueryPool hasn't yet been created, or there's a mismatch
        //  between the VkQueryPoolPerformanceCreateInfoKHR pNext members.  If no
        //  QueryPool exists, the requested counters don't match, or the queryCount is
        //  less than get_query_count(), create a new query pool.
        const auto& existingQueryPoolCreateInfo = mQueryPool ? mQueryPool.get<VkQueryPoolCreateInfo>() : VkQueryPoolCreateInfo{ };
        auto queryPoolCreateInfo = gvk::get_default<VkQueryPoolCreateInfo>();
        queryPoolCreateInfo.pNext = &performanceQueryCreateInfo;
        queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
        queryPoolCreateInfo.queryCount = existingQueryPoolCreateInfo.queryCount;
        if (!mQueryPool || queryPoolCreateInfo != existingQueryPoolCreateInfo || queryPoolCreateInfo.queryCount < get_query_count(toolInfo)) {
            queryPoolCreateInfo.queryCount = get_query_count(toolInfo);
            gvk_result(gvk::QueryPool::create(gvkDevice, &queryPoolCreateInfo, nullptr, &mQueryPool));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PerformanceQueryManager::reset_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::reset_query_resources(toolInfo);
}

VkResult PerformanceQueryManager::pre_process_range()
{
    return QueryManager::pre_process_range();
}

VkResult PerformanceQueryManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::pre_process_command_buffers(toolInfo);
}

VkResult PerformanceQueryManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() &&
            toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
            toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].begin
        ) {
            gvk::Queue gvkQueue = toolInfo.queue;
            gvk_result_assert(gvkQueue);
            auto commandBuffer = toolInfo.ppCmds[toolInfo.cmdIndex]->commandBuffer;
            gvkQueue.get<gvk::DispatchTable>().gvkCmdBeginQuery(commandBuffer, mQueryPool, mQueryIndex, 0);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PerformanceQueryManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>() &&
            toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
            toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].end
        ) {
            gvk::Queue gvkQueue = toolInfo.queue;
            gvk_result_assert(gvkQueue);
            auto commandBuffer = toolInfo.ppCmds[toolInfo.cmdIndex]->commandBuffer;
            // TODO : May be a good idea to track resources and insert barriers here
            gvkQueue.get<gvk::DispatchTable>().gvkCmdEndQuery(commandBuffer, mQueryPool, mQueryIndex++);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PerformanceQueryManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::post_process_command_buffers(toolInfo);
}

VkResult PerformanceQueryManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::pre_process_queue_submission(toolInfo);
}

VkResult PerformanceQueryManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (get_query_count(toolInfo)) {
            gvk::Device gvkDevice = toolInfo.device;
            gvk_result_assert(gvkDevice);
            gvk::pipeline_explorer::DeviceInfo deviceInfo = toolInfo.device;
            gvk_result_assert(deviceInfo);
            gvk::Queue gvkQueue = toolInfo.queue;
            gvk_result_assert(gvkQueue);
            const auto& queueFamilyInfoItr = deviceInfo->queueFamilyInfos.find(toolInfo.queueFamilyIndex);
            gvk_result_assert(queueFamilyInfoItr != deviceInfo->queueFamilyInfos.end());
            const auto& queueFamilyInfo = queueFamilyInfoItr->second;
            gvk_result_assert(mQueryPool);

            // In theory passing VK_QUERY_RESULT_WAIT_BIT to vkGetQueryPoolResults() should
            //  make it unnecessary to call vkQueueWaitIdle(), but omitting it yields some
            //  clearly incorrect 0 results.  Worth looking into, but simply waiting here
            //  should provide consistent behavior across different implementations/drivers.
            gvk_result(gvkQueue.QueueWaitIdle());

            // Get query results.  There will be one result for each counter for each query.
            std::vector<VkPerformanceCounterResultKHR> resultsData(mCurrentRequests.size() * mQueryIndex);
            gvk_result(gvkDevice.GetQueryPoolResults(
                mQueryPool,
                0,
                mQueryIndex,
                sizeof(VkPerformanceCounterResultKHR) * resultsData.size(),
                resultsData.data(),
                sizeof(VkPerformanceCounterResultKHR) * mCurrentRequests.size(),
                VK_QUERY_RESULT_WAIT_BIT
            ));

            // Sanity check that mCurrentRequests matches the QueryPool as expected.
            const auto& queryPoolCreatetInfo = mQueryPool.get<VkQueryPoolCreateInfo>();
            const auto& pQueryPoolPerformanceCreateInfo = (const VkQueryPoolPerformanceCreateInfoKHR*)queryPoolCreatetInfo.pNext;
            gvk_result_assert(pQueryPoolPerformanceCreateInfo);
            gvk_result_assert(pQueryPoolPerformanceCreateInfo->sType == gvk::get_stype<VkQueryPoolPerformanceCreateInfoKHR>());
            gvk_result_assert(pQueryPoolPerformanceCreateInfo->counterIndexCount == mCurrentRequests.size());

            // Loop over query results and add them to the appropriate counter results.
            uint32_t resultsData_i = 0;
            for (uint32_t query_i = 0; query_i < mQueryIndex; ++query_i) {
                for (uint32_t counter_i = 0; counter_i < mCurrentRequests.size(); ++counter_i) {
                    auto& currentRequest = mCurrentRequests[counter_i];
                    const auto& counter = queueFamilyInfo.performanceCounters[pQueryPoolPerformanceCreateInfo->pCounterIndices[counter_i]];

                    // Sanity check
                    gvk_result_assert(currentRequest.counter == counter);
                    gvk_result_assert(resultsData_i < resultsData.size());
                    gvk_result_assert(!currentRequest.results.empty());

                    // Update currentRequest.results.  The back() of the results collection is the
                    //  current value.  Each time the range-of-interest completes a new empty entry
                    //  is added for the upcoming query range, once a counter has processed
                    //  warmupRangeCount + sampleRangeCount results it's considered complete.
                    switch (currentRequest.counter.storage) {
                    case VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR: { currentRequest.results.back().int32 += resultsData[resultsData_i++].int32; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_INT64_KHR: { currentRequest.results.back().int64 += resultsData[resultsData_i++].int64; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_UINT32_KHR: { currentRequest.results.back().uint32 += resultsData[resultsData_i++].uint32; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR: { currentRequest.results.back().uint64 += resultsData[resultsData_i++].uint64; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_FLOAT32_KHR: { currentRequest.results.back().float32 += resultsData[resultsData_i++].float32; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR: { currentRequest.results.back().float64 += resultsData[resultsData_i++].float64; } break;
                    default: { gvk_result(VK_ERROR_FEATURE_NOT_PRESENT); } break;
                    }
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PerformanceQueryManager::post_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerPerformanceQueryRequestInfo>()) {

            // Loop over mCurrentRequests and check how many results each has.  If a
            //  counter has warmupRangeCount + sampleRangeCount results move it to the
            //  mCompleteRequests collection, otherwise add a new empty result for the next
            //  time the counter is processed and move it back to mPendingRequests.  Then
            //  clear mCurrentRequests.
            for (auto& currentRequest : mCurrentRequests) {
                if (currentRequest.results.size() == mRequest->warmupRangeCount + mRequest->queryRangeCount) {
                    auto inserted = mCompleteRequests.insert(std::move(currentRequest)).second;
                    gvk_result_assert(inserted);
                } else {
                    currentRequest.results.push_back({ });
                    auto inserted = mPendingRequests.insert({ get_uuid(currentRequest.counter), std::move(currentRequest) }).second;
                    gvk_result_assert(inserted);
                }
            }
            mCurrentRequests.clear();

            // TODO : Timeout if no new results are extracted over N iterations

            // Once there are no more pending requests, ReleaseProfilingLockKHR(),
            //  generate_report(), and reset().
            if (mPendingRequests.empty()) {
                gvk_result_assert(mCompleteRequests.size() == mRequest->counterCount);
                gvk::Device(mRequest->device).ReleaseProfilingLockKHR();
                gvk_result(generate_report());
                reset();
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PerformanceQueryManager::generate_report()
{
    // TODO : Publishing to workspace/GUI should be handled by RequestManager

    gvk_result_scope_begin(VK_SUCCESS) {
        gvk::pipeline_explorer::PipelineInfo pipelineInfo({ mRequest->device, mRequest->pipeline });
        gvk_result_assert(pipelineInfo);

        // Get date and time strings
        auto dateTime = gvk::system::DateTime::now();
        auto dateStr = dateTime.get_date_str();
        auto timeStr = dateTime.get_time_str();

        // Prepare report
        auto performanceQueryResultInfo = gvk::get_default<GvkPipelineExplorerPerformanceQueryResultInfo>();
        performanceQueryResultInfo.pName = nullptr;
        performanceQueryResultInfo.pDate = dateStr.c_str();
        performanceQueryResultInfo.pTime = timeStr.c_str();
        performanceQueryResultInfo.pNote = nullptr;
        performanceQueryResultInfo.pipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
        boost::multiprecision::export_bits(pipelineInfo->uuid, performanceQueryResultInfo.pipelineInfo.uuid, 8);
        boost::multiprecision::export_bits(pipelineInfo->driverUUID, performanceQueryResultInfo.pipelineInfo.driverUUID, 8);
        performanceQueryResultInfo.pipelineInfo.pName = "VkPipeline"; // TODO : Get name from pipelineInfo
        performanceQueryResultInfo.pipelineInfo.device = pipelineInfo->deviceInfo->vkHandle;
        performanceQueryResultInfo.pipelineInfo.pipeline = pipelineInfo->vkHandle;
        performanceQueryResultInfo.pipelineInfo.bindPoint = pipelineInfo->bindPoint;
        performanceQueryResultInfo.pipelineInfo.labelCount = 0; // TODO : Get labels from pipelineInfo
        performanceQueryResultInfo.pipelineInfo.pLabels = nullptr; // TODO : Get labels from pipelineInfo
        performanceQueryResultInfo.pipelineInfo.experimentEnabled = pipelineInfo->experimentEnabled;
        // TODO : performanceQueryResultInfo.pipelineInfo.experimentUUID;
        performanceQueryResultInfo.pipelineInfo.highlightEnabled = pipelineInfo->highlightEnabled;
        memcpy(performanceQueryResultInfo.pipelineInfo.highlightColor, pipelineInfo->highlightColor, sizeof(pipelineInfo->highlightColor));
        performanceQueryResultInfo.groupResultCount = 1;
        auto pPerformanceQueryGroupResultInfos = gvk::detail::create_dynamic_array<GvkPipelineExplorerPerformanceQueryGroupResultInfo>(performanceQueryResultInfo.groupResultCount, nullptr);
        performanceQueryResultInfo.pGroupResults = pPerformanceQueryGroupResultInfos;

        // Process group results
        for (uint32_t group_i = 0; group_i < performanceQueryResultInfo.groupResultCount; ++group_i) {
            gvk_result_assert((uint32_t)mCompleteRequests.size() == mRequest->counterCount);

            // Setup GvkPipelineExplorerPerformanceQueryGroupResultInfo
            auto& performanceQueryGroupResultInfo = pPerformanceQueryGroupResultInfos[group_i];
            performanceQueryGroupResultInfo = gvk::get_default<GvkPipelineExplorerPerformanceQueryGroupResultInfo>();
            performanceQueryGroupResultInfo.counterResultCount = (uint32_t)mCompleteRequests.size();
            auto pPerformanceCounterResultInfos = gvk::detail::create_dynamic_array<GvkPipelineExplorerPerformanceCounterResultInfo>(performanceQueryGroupResultInfo.counterResultCount, nullptr);
            performanceQueryGroupResultInfo.pCounterResults = pPerformanceCounterResultInfos;

            // Process counter results
            for (uint32_t counter_i = 0; counter_i < performanceQueryGroupResultInfo.counterResultCount; ++counter_i) {
                const auto& completedRequest = mCompleteRequests[counter_i];

                // Setup GvkPipelineExplorerPerformanceCounterResultInfo
                auto& performanceCounterResultInfo = pPerformanceCounterResultInfos[counter_i];
                performanceCounterResultInfo = gvk::get_default<GvkPipelineExplorerPerformanceCounterResultInfo>();
                performanceCounterResultInfo.counter = completedRequest.counter;
                performanceCounterResultInfo.description = completedRequest.description;
                performanceCounterResultInfo.valueCount = mRequest->queryRangeCount;
                auto pValues = gvk::detail::create_dynamic_array<double>(performanceCounterResultInfo.valueCount, nullptr);
                performanceCounterResultInfo.pValues = pValues;

                // Process result values
                for (uint32_t value_i = 0; value_i < performanceCounterResultInfo.valueCount; ++value_i) {
                    const auto& result = completedRequest.results[mRequest->warmupRangeCount + value_i];
                    switch (performanceCounterResultInfo.counter.storage) {
                    case VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR: { pValues[value_i] = (double)result.int32; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_INT64_KHR: { pValues[value_i] = (double)result.int64; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_UINT32_KHR: { pValues[value_i] = (double)result.uint32; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR: { pValues[value_i] = (double)result.uint64; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_FLOAT32_KHR: { pValues[value_i] = (double)result.float32; } break;
                    case VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR: { pValues[value_i] = (double)result.float64; } break;
                    default: { gvk_result(VK_ERROR_FEATURE_NOT_PRESENT); } break;
                    }
                    performanceCounterResultInfo.total += pValues[value_i];
                }
                performanceCounterResultInfo.average = performanceCounterResultInfo.total / (double)performanceCounterResultInfo.valueCount;
            }
        }

        // Write report
        if (mRequest->pReportPath) {
            std::filesystem::path reportPath = mRequest->pReportPath;
            std::filesystem::create_directories(reportPath);
            auto dateTimeStr = gvk::string::replace(dateStr, "/", "-") + "_" + gvk::string::replace(timeStr, ":", "-");
            reportPath /= dateTimeStr + ".GvkPipelineExplorerPerformanceQueryResultInfo.json";
            std::ofstream file(reportPath);
            file << gvk::to_string(performanceQueryResultInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // Send message to frontend
        if (!mWorkspace.empty()) {
            gvk::write_serialized_structure(mWorkspace / ".data", performanceQueryResultInfo);
        }

        // Clear pointers to memory not owned by result structure
        performanceQueryResultInfo.pName = nullptr;
        performanceQueryResultInfo.pDate = nullptr;
        performanceQueryResultInfo.pTime = nullptr;
        performanceQueryResultInfo.pNote = nullptr;
        gvk::detail::destroy_structure_copy(performanceQueryResultInfo, nullptr);
    } gvk_result_scope_end;
    return gvkResult;
}

bool PerformanceQueryManager::RequestManager::operator==(const RequestManager& other) const
{
    return get_uuid(counter) == get_uuid(other.counter);
}

bool PerformanceQueryManager::RequestManager::operator!=(const RequestManager& other) const
{
    return !(get_uuid(counter) == get_uuid(other.counter));
}

bool PerformanceQueryManager::RequestManager::operator<(const RequestManager& other) const
{
    return get_uuid(counter) < get_uuid(other.counter);
}

} // namespace pipeline_explorer
} // namespace gvk

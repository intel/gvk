
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

#include "gvk-pipeline-explorer/backend/query-managers/timeline-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"
#include "gvk-system.hpp"

#include <array>
#include <iostream>

namespace gvk {
namespace pipeline_explorer {

// FROM : unified-telemetry/common/include/uci_time.h
static inline uint64_t get_ns_since_epoch()
{
#ifdef GVK_PLATFORM_WINDOWS
    // Windows: Use GetSystemTimePreciseAsFileTime for high-resolution time
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);

    // We need to multiply by 100 because Windows gives us 100ns intervals
    uint64_t ns_since_1601 = ((static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime) * 100;

    // This is the number of nanoseconds from Jan 1 1601 to Jan 1 1970
    constexpr uint64_t kNsFromWindowsToUnixEpoch = 11644473600000000000ULL;
    return ns_since_1601 - kNsFromWindowsToUnixEpoch;
#else
#if 0
    // Linux: Use clock_gettime with CLOCK_REALTIME for nanosecond precision
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // Convert seconds and nanoseconds to total nanoseconds
    return static_cast<uint64_t>(ts.tv_sec) * kNsPerSec + static_cast<uint64_t>(ts.tv_nsec);
#else
    return 0;
#endif
#endif
}

TimelineQueryManager::~TimelineQueryManager()
{
}

uint64_t TimelineQueryManager::get_type_id() const
{
    return Tool::get_type_id<TimelineQueryManager>();
}

VkResult TimelineQueryManager::enable(const std::filesystem::path& reportPath, gvk::pipeline_explorer::IpcMessenger* pIpcMessenger)
{
    mEnabled = true;
    mpIpcMessenger = pIpcMessenger;
    if (!reportPath.empty()) {
    }
    return VK_SUCCESS;
}

VkResult TimelineQueryManager::disable()
{
    mEnabled = false;
    return { };
}

bool TimelineQueryManager::collect_metrics(VkDevice device, VkPipeline pipeline) const
{
    (void)device;
    (void)pipeline;
    return mEnabled;
}

bool TimelineQueryManager::tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const
{
    return Tool::tool_command(pCommand, vkDevice, vkQueue, vkPipeline);
}

uint32_t TimelineQueryManager::get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    // Account for begin/end timestamp
    return toolInfo.collectionRangeCount * 2;
}

VkResult TimelineQueryManager::validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mEnabled);
        if (!mQueryPool || mQueryPool.get<VkQueryPoolCreateInfo>().queryCount < get_query_count(toolInfo)) {
            auto queryPoolCreateInfo = gvk::get_default<VkQueryPoolCreateInfo>();
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolCreateInfo.queryCount = get_query_count(toolInfo);
            gvk_result(gvk::QueryPool::create(toolInfo.device, &queryPoolCreateInfo, nullptr, &mQueryPool));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimelineQueryManager::pre_process_range()
{
    return QueryManager::pre_process_range();
}

VkResult TimelineQueryManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mEnabled);
        if (get_query_count(toolInfo)) {
            gvk_result(validate_query_resources(toolInfo));
            gvk_result(reset_query_resources(toolInfo));

            // Calibrate timestamps if VK_KHR_calibrated_timestamps is available
            gvk::pipeline_explorer::DeviceInfo deviceInfo = toolInfo.device;
            gvk_result_assert(deviceInfo);
            if (deviceInfo->VK_KHR_calibrated_timestamps_enabled) {
                gvk::Device gvkDevice = deviceInfo->vkHandle;
                gvk_result_assert(gvkDevice);
                gvk::PhysicalDevice gvkPhysicalDevice = gvkDevice.get<gvk::PhysicalDevice>();
                gvk_result_assert(gvkPhysicalDevice);

                // Get time domains supported by the physical device
                // TODO : Move this to enable()
                uint32_t timeDomainCount = 0;
                gvk_result(gvkPhysicalDevice.GetPhysicalDeviceCalibrateableTimeDomainsKHR(&timeDomainCount, nullptr));
                std::vector<VkTimeDomainKHR> timeDomains(timeDomainCount);
                gvk_result(gvkPhysicalDevice.GetPhysicalDeviceCalibrateableTimeDomainsKHR(&timeDomainCount, timeDomains.data()));

                // Capture calibration timestamps (GPU device time and CPU QPC time)
                std::array<VkCalibratedTimestampInfoKHR, 2> timestampInfos{ };
                timestampInfos[0] = gvk::get_default<VkCalibratedTimestampInfoKHR>();
                timestampInfos[0].timeDomain = VK_TIME_DOMAIN_DEVICE_KHR;
                timestampInfos[1] = gvk::get_default<VkCalibratedTimestampInfoKHR>();
                timestampInfos[1].timeDomain = VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_KHR;
                uint64_t maxDeviation = 0;
                gvk_result(gvkDevice.GetCalibratedTimestampsKHR((uint32_t)mCalibrationTimestamps.size(), timestampInfos.data(), mCalibrationTimestamps.data(), &maxDeviation));

#ifdef GVK_PLATFORM_WINDOWS
                LARGE_INTEGER qpcFrequency{ };
                QueryPerformanceFrequency(&qpcFrequency);
                double qpcToNsRatio = 1e9 / (double)qpcFrequency.QuadPart;

                // The calibrated QPC timestamp (already synchronized with GPU)
                double calibratedQpcNs = (double)mCalibrationTimestamps[1] * qpcToNsRatio;

                // Sample current QPC and Unix epoch as close together as possible
                LARGE_INTEGER currentQpc;
                QueryPerformanceCounter(&currentQpc);
                uint64_t currentUnixEpochNs = get_ns_since_epoch();
                double currentQpcNs = (double)currentQpc.QuadPart * qpcToNsRatio;

                // Calculate the offset from QPC timeline to Unix epoch timeline
                double qpcToUnixEpochOffset = currentUnixEpochNs - currentQpcNs;

                // Convert the calibrated QPC timestamp to Unix epoch
                mCalibrationUnixEpochNs = (uint64_t)std::round(calibratedQpcNs + qpcToUnixEpochOffset);
#endif
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimelineQueryManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mEnabled);
        if (get_query_count(toolInfo)) {
            if (toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
                toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].begin) {
                gvk_result(write_timestamp(toolInfo, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimelineQueryManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mEnabled);
        if (get_query_count(toolInfo)) {
            if (toolInfo.collectionRangeIndex < toolInfo.collectionRangeCount && toolInfo.pCollectionRanges &&
                toolInfo.cmdIndex == toolInfo.pCollectionRanges[toolInfo.collectionRangeIndex].end) {
                gvk_result(write_timestamp(toolInfo, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimelineQueryManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    return QueryManager::post_process_command_buffers(toolInfo);
}

VkResult TimelineQueryManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    // Prepare queue submit for timeline query results
    mQueueSubmit = gvk::get_default<GvkCommandStructureQueueSubmit>();
    if (toolInfo.pCommand && toolInfo.pCommand->sType == gvk::get_stype<GvkCommandStructureQueueSubmit>()) {
        mQueueSubmit = *(const GvkCommandStructureQueueSubmit*)toolInfo.pCommand;
    }

    // Prepare queue submit info for timeline query results
    mQueueSubmitInfo = gvk::get_default<GvkPipelineExplorerTimelineCommandInfo>();
    mQueueSubmitInfo.beginNs = get_ns_since_epoch();
    mQueueSubmitInfo.endNs = mQueueSubmitInfo.beginNs;
#ifdef GVK_PLATFORM_WINDOWS
    mQueueSubmitInfo.threadId = GetCurrentThreadId();
#endif
    mQueueSubmitInfo.queue = toolInfo.queue;
    mQueueSubmitInfo.pipeline = VK_NULL_HANDLE;

    return QueryManager::pre_process_queue_submission(toolInfo);
}

VkResult TimelineQueryManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mEnabled);
        if (toolInfo.collectionRangeCount) {

            // Get timestamp for queue submit completion
            mQueueSubmitInfo.endNs = get_ns_since_epoch();

            // Validate toolInfo resources
            gvk::pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo = toolInfo.physicalDevice;
            gvk_result(physicalDeviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk::Device gvkDevice = toolInfo.device;
            gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk::pipeline_explorer::DeviceInfo deviceInfo = toolInfo.device;
            gvk_result(deviceInfo ? VK_SUCCESS : VK_INCOMPLETE);
            gvk::Queue gvkQueue = toolInfo.queue;
            gvk_result(gvkQueue ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(mQueryPool ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

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

            // Convert calibration GPU timestamp to nanoseconds
            double timestampPeriod = (double)deviceInfo->physicalDeviceInfo->physicalDeviceProperties->limits.timestampPeriod;
            // GPU timestamp at calibration time, converted to nanoseconds
            double calibrationGpuNs = (double)mCalibrationTimestamps[0] * timestampPeriod;

            // Calculate GPU to Unix epoch offset
            double gpuToUnixEpochOffset = mCalibrationUnixEpochNs - calibrationGpuNs;

            // Prepare command ptr and info collections for processing
            uint32_t resultsData_i = 0;
            (void)resultsData_i;
            std::vector<const GvkCommandBaseStructure*> commandPtrs;
            commandPtrs.push_back((const GvkCommandBaseStructure*)&mQueueSubmit);
            std::vector<GvkPipelineExplorerTimelineCommandInfo> timelineCommandInfos;
            timelineCommandInfos.push_back(mQueueSubmitInfo);
            gvk_result(toolInfo.pCollectionRanges ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // Process collection ranges
            for (uint32_t collectionRange_i = 0; collectionRange_i < toolInfo.collectionRangeCount; ++collectionRange_i) {

                const auto& collectionRange = toolInfo.pCollectionRanges[collectionRange_i];
                gvk_result_assert(collectionRange.begin == collectionRange.end);

                // Get command
                gvk_result(toolInfo.ppCmds ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                commandPtrs.push_back((const GvkCommandBaseStructure*)toolInfo.ppCmds[collectionRange.begin]);

                // Calculate begin timestamp for command
                gvk_result_assert(resultsData_i < resultData.size());
                auto beginGpuNs = (double)resultData[resultsData_i++] * timestampPeriod;
                auto beginNs = (uint64_t)std::round(gpuToUnixEpochOffset + beginGpuNs);

                // Calculate end timestamp for command
                gvk_result_assert(resultsData_i < resultData.size());
                auto endGpuNs = (double)resultData[resultsData_i++] * timestampPeriod;
                auto endNs = (uint64_t)std::round(gpuToUnixEpochOffset + endGpuNs);

                // Setup GvkPipelineExplorerTimelineCommandInfo
                auto timelineCommandInfo = gvk::get_default<GvkPipelineExplorerTimelineCommandInfo>();
                timelineCommandInfo.beginNs = beginNs;
                timelineCommandInfo.endNs = endNs;
                // Thread id for vkQueueSubmit gets the CPU thread the call was made on, but thread
                //  id for vkCmds gets the VkQueue handle as a stable and unique id for referencing
                //  execution context and synchronization between commands on different queues.
                timelineCommandInfo.threadId = (uint64_t)toolInfo.queue;
                timelineCommandInfo.queue = toolInfo.queue;
                timelineCommandInfo.pipeline = collectionRange.pipeline;
                timelineCommandInfos.push_back(timelineCommandInfo);
            }

            // Report timeline query results via IPC
            if (mpIpcMessenger) {

                // TODO : Make date/time string optional
                #if 0
                // Get date and time strings
                auto dateTime = gvk::system::DateTime::now();
                auto dateStr = dateTime.get_date_str();
                auto timeStr = dateTime.get_time_str();
                #endif

                // Setup and write GvkPipelineExplorerTimelineQueryResultInfo
                gvk_result_assert(commandPtrs.size() == timelineCommandInfos.size());
                auto timelineQueryResultInfo = gvk::get_default<GvkPipelineExplorerTimelineQueryResultInfo>();
                timelineQueryResultInfo.pName = nullptr;
                #if 0
                timelineQueryResultInfo.pDate = dateStr.c_str();
                timelineQueryResultInfo.pTime = timeStr.c_str();
                #endif
                timelineQueryResultInfo.pNote = nullptr;
                timelineQueryResultInfo.physicalDevice = toolInfo.physicalDevice;
                timelineQueryResultInfo.device = toolInfo.device;
                timelineQueryResultInfo.timestampPeriod = (float)timestampPeriod;
                timelineQueryResultInfo.beginNs = mQueueSubmitInfo.beginNs;
                timelineQueryResultInfo.endNs = get_ns_since_epoch();
                timelineQueryResultInfo.commandInfoCount = (uint32_t)timelineCommandInfos.size();
                timelineQueryResultInfo.pCommandInfos = timelineCommandInfos.data();
                timelineQueryResultInfo.commands.commandCount = (uint32_t)commandPtrs.size();
                timelineQueryResultInfo.commands.ppCommands = commandPtrs.data();
                mpIpcMessenger->write("GvkPipelineExplorerTimelineQueryResultInfo", timelineQueryResultInfo);
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult TimelineQueryManager::pre_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::pre_process_queue_present(toolInfo);
}

VkResult TimelineQueryManager::post_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    return QueryManager::post_process_queue_present(toolInfo);
}

VkResult TimelineQueryManager::post_process_range()
{
    return QueryManager::post_process_range();
}

VkResult TimelineQueryManager::write_timestamp(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo, VkPipelineStageFlagBits pipelineStage)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(mEnabled && get_query_count(toolInfo));
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

} // namespace pipeline_explorer
} // namespace gvk

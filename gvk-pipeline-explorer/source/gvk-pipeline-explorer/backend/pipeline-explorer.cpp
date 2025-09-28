
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

#include "gvk-pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/backend/pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/backend/utilities.hpp"
#include "gvk-system/time.hpp"

#ifdef GVK_PLATFORM_WINDOWS
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace gvk {

VkResult PipelineExplorer::launch_gui(const std::filesystem::path& layerPath)
{
    (void)layerPath;
#ifdef GVK_PLATFORM_WINDOWS

    // Get current environment
    gvk::Environment env;
    uint32_t envCharCount = 0;
    env.get_env(&envCharCount, nullptr);
    std::vector<char> envData(envCharCount);
    env.get_env(&envCharCount, envData.data());

    // Launch GUI with the app's environment
    STARTUPINFO startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    auto guiPath = layerPath.parent_path() / "gvk-pipeline-explorer-gui.exe";
    auto cmdLine = "\"" + guiPath.string() + "\"";
    cmdLine += " -w \"" + workspacePath.string() + "\"";
    cmdLine += " -a \"" + applicationName + "\"";
    if (CreateProcess(
        guiPath.string().c_str(),
        cmdLine.data(),
        NULL,
        NULL,
        TRUE,
        0,
        !envData.empty() ? envData.data() : NULL,
        NULL,
        &startupInfo,
        &guiProcessInformation
    )) {
        return VK_SUCCESS;
    }

#endif // GVK_PLATFORM_WINDOWS
    return VK_ERROR_INITIALIZATION_FAILED;
}

static void setup_pipeline_statistic_counter(
    VkQueryPipelineStatisticFlagBits pipelineStatistic,
    std::vector<VkPerformanceCounterKHR>& counters,
    std::vector<VkPerformanceCounterDescriptionKHR>& descriptions,
    const std::string& description
)
{
    // Setup VkPerformanceCounterKHR
    counters.push_back(gvk::get_default<VkPerformanceCounterKHR>());
    counters.back().unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
    counters.back().scope = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR;
    counters.back().storage = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR;
    static_assert(sizeof(pipelineStatistic) <= sizeof(counters.back().uuid));
    memcpy(counters.back().uuid, &pipelineStatistic, sizeof(pipelineStatistic));

    // Get name
    // TODO : DRY
    //  pipeline-explorer.cpp
    //  pipeline-statistics-query-manager.cpp
    std::string pipelineStatisticNameStr;
    auto pipelineStatisticFlagStr = gvk::to_string(pipelineStatistic, gvk::Printer::Default ^ gvk::Printer::EnumValue);
    pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "VK_QUERY_PIPELINE_STATISTIC_");
    pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "_BIT");
    pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "\"");
    for (auto token : gvk::string::split_snake_case(pipelineStatisticFlagStr)) {
        assert(!token.empty());
        token = gvk::string::to_lower(token);
        token[0] = gvk::string::to_upper(token[0]);
        if (!pipelineStatisticNameStr.empty()) {
            pipelineStatisticNameStr += " ";
        }
        pipelineStatisticNameStr += token;
    }

#ifdef GVK_PLATFORM_WINDOWS
    // Setup VkPerformanceCounterDescriptionKHR
    descriptions.push_back(gvk::get_default<VkPerformanceCounterDescriptionKHR>());
    strcpy_s(descriptions.back().name, sizeof(descriptions.back().name) - 1, pipelineStatisticNameStr.c_str());
    strcpy_s(descriptions.back().category, sizeof(descriptions.back().category) - 1, "Pipeline Statistics");
    assert(description.size() - 1 < VK_MAX_DESCRIPTION_SIZE);
    strcpy_s(descriptions.back().description, sizeof(descriptions.back().description) - 1, description.c_str());
#else
    // TODO :
    (void)descriptions;
    (void)description;
#endif // GVK_PLATFORM_WINDOWS
}

void PipelineExplorer::process_end_of_frame_and_outgoing_messages()
{
    // Report available metrics to frontend
    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->refreshAvailableMetrics) {
        const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).refreshAvailableMetrics = false;
        std::vector<GvkPipelineExplorerMetricInfo> pipelineExplorerMetricInfo;
        for (const auto& metricsGroupItr : availableMetrics) {
            pipelineExplorerMetricInfo.push_back(metricsGroupItr.second);
        }
        auto pipelineExplorerAvailableMetricsInfo = gvk::get_default<GvkPipelineExplorerAvailableMetricsInfo>();
        pipelineExplorerAvailableMetricsInfo.metricInfoCount = (uint32_t)pipelineExplorerMetricInfo.size();
        pipelineExplorerAvailableMetricsInfo.pMetricInfos = !pipelineExplorerMetricInfo.empty() ? pipelineExplorerMetricInfo.data() : nullptr;
        gvk::write_serialized_structure(workspacePath / ".data", pipelineExplorerAvailableMetricsInfo);

        // Report available performance counters to frontend
        // TODO : Expose counters based on queue...
        std::set<VkPerformanceCounterKHR> uniquePerformanceCounters;
        std::vector<VkPerformanceCounterKHR> performanceCounters;
        std::vector<VkPerformanceCounterDescriptionKHR> performanceCounterDescriptions;
        deviceInfos.enumerate(
            [&](const auto& deviceInfoItr)
            {
                for (const auto& queueFamilyInfo : deviceInfoItr.second->queueFamiyInfos) {
                    for (size_t i = 0; i < queueFamilyInfo.second.performanceCounters.size() && i < queueFamilyInfo.second.performanceCounterDescriptions.size(); ++i) {
                        if (uniquePerformanceCounters.insert(queueFamilyInfo.second.performanceCounters[i]).second) {
                            performanceCounters.push_back(queueFamilyInfo.second.performanceCounters[i]);
                            performanceCounterDescriptions.push_back(queueFamilyInfo.second.performanceCounterDescriptions[i]);
                        }
                    }
                }
                return true;
            }
        );
        if (!performanceCounters.empty()) {
            auto pipelineExplorerPerformanceCounterCollection = gvk::get_default<GvkPipelineExplorerPerformanceCounterCollection>();
            pipelineExplorerPerformanceCounterCollection.count = (uint32_t)performanceCounters.size();
            pipelineExplorerPerformanceCounterCollection.pCounters = performanceCounters.data();
            pipelineExplorerPerformanceCounterCollection.pDescriptions = performanceCounterDescriptions.data();
            gvk::write_serialized_structure(workspacePath / ".data", pipelineExplorerPerformanceCounterCollection);

            #if 0
            std::ofstream counterJson(workspacePath / "GvkPipelineExplorerPerformanceCounterCollection.json");
            for (uint32_t counter_i = 0; counter_i < pipelineExplorerPerformanceCounterCollection.count; ++counter_i) {
                counterJson << gvk::to_string(pipelineExplorerPerformanceCounterCollection.pCounters[counter_i], gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
                counterJson << gvk::to_string(pipelineExplorerPerformanceCounterCollection.pDescriptions[counter_i], gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
            }
            #endif
        }

        // Report pipeline statistics counters to frontend
        std::vector<VkPerformanceCounterKHR> pipelineStatisticCounters;
        std::vector<VkPerformanceCounterDescriptionKHR> pipelineStatisticCounterDescriptions;
        deviceInfos.enumerate(
            [&](const auto& deviceInfoItr)
            {
                const auto& deviceInfo = deviceInfoItr.second;
                if (deviceInfo->pipelineStatisticsQuery_enabled) {
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of vertices processed by the input assembly stage. Vertices corresponding to incomplete primitives may contribute to the count."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of primitives processed by the input assembly stage. If primitive restart is enabled, restarting the primitive topology has no effect on the count. Incomplete primitives may be counted."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of vertex shader invocations."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of geometry shader invocations. In the case of instanced geometry shaders, the geometry shader invocations count is incremented for each separate instanced invocation."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of primitives generated by geometry shader invocations. Restarting primitive topology using SPIR-V instructions OpEndPrimitive or OpEndStreamPrimitive has no effect on primitive count."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of primitives processed by the primitive clipping stage of the pipeline."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of primitives output by the primitive clipping stage of the pipeline. The actual number of primitives output for a particular input primitive is implementation-dependent."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of fragment shader invocations."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of patches processed by the tessellation control shader."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of tessellation evaluation shader invocations."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of compute shader invocations. Implementations may execute more or less compute shader invocations than reported as long as the results remain unchanged."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of task shader invocations."
                    );
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of mesh shader invocations."
                    );
                    #if 0
                    setup_pipeline_statistic_counter(
                        VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI, pipelineStatisticCounters, pipelineStatisticCounterDescriptions,
                        "The number of cluster culling shader invocations."
                    );
                    #endif
                    return false;
                }
                return true;
            }
        );
        if (!pipelineStatisticCounters.empty()) {
            auto pipelineExplorerPerformanceCounterCollection = gvk::get_default<GvkPipelineExplorerPerformanceCounterCollection>();
            pipelineExplorerPerformanceCounterCollection.count = (uint32_t)pipelineStatisticCounters.size();
            pipelineExplorerPerformanceCounterCollection.pCounters = pipelineStatisticCounters.data();
            pipelineExplorerPerformanceCounterCollection.pDescriptions = pipelineStatisticCounterDescriptions.data();
#ifdef WIN32
            gvk::write_serialized_structure(workspacePath / ".data", "PipelineStatisticsCounterCollection", pipelineExplorerPerformanceCounterCollection);
#else
            (void)pipelineExplorerPerformanceCounterCollection;
            // TODO : Why doesn't this work on Linux?
            // TODO : Gotta rework structure utility includes anyway
#endif // WIN32
        }

        // Report plugin counters to frontend
        auto pluginCounterInfo = gvk::get_default<GvkPipelineExplorerPluginCounterInfo>();
        pluginManager.get_plugin_counter_info(VK_NULL_HANDLE, &pluginCounterInfo);
        if (pluginCounterInfo.groupCount) {
            gvk::write_serialized_structure(workspacePath / ".data", pluginCounterInfo);
            #if 0
            std::ofstream counterJson(workspacePath / "GvkPipelineExplorerPluginCounterInfo.json");
            counterJson << gvk::to_string(pluginCounterInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
            #endif
        }
    }

    // TODO : Rework API call recording and reporting
    if (TODO_shouldBeControlledByRequestInfo_getApiCalls) {
        TODO_shouldBeControlledByRequestInfo_getApiCalls = false;
        // TODO : Unify command recorder and GvkCommandCollection
        auto commandCollection = gvk::get_default<GvkCommandCollection>();
        commandCollection.commandCount = (uint32_t)mCommandRecorder.get_commands().size();
        commandCollection.ppCommands = mCommandRecorder.get_commands().data();
#ifdef WIN32
        (void)gvk::write_serialized_structure(workspacePath / ".data", "GvkApiCommandCollection", commandCollection);
#else
        (void)commandCollection;
        // TODO : Why doesn't this work on Linux?
        // TODO : Gotta rework structure utility includes anyway
#endif // WIN32
        mCommandRecorder.reset();
    }

    // TODO : Rework API call recording and reporting
    if (TODO_shouldBeControlledByRequestInfo_getGpuCalls) {
        TODO_shouldBeControlledByRequestInfo_getGpuCalls = false;
        // TODO : Unify command recorder and GvkCommandCollection
        auto commandCollection = gvk::get_default<GvkCommandCollection>();
        commandCollection.commandCount = (uint32_t)mGpuCalls.get_commands().size();
        commandCollection.ppCommands = mGpuCalls.get_commands().data();
#ifdef WIN32
        (void)gvk::write_serialized_structure(workspacePath / ".data", "GvkGpuCommandCollection", commandCollection);
#else
        (void)commandCollection;
        // TODO : Why doesn't this work on Linux?
        // TODO : Gotta rework structure utility includes anyway
#endif // WIN32
        mGpuCalls.reset();
    }

    // TODO : Rework all query request/result logic
    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->warmupRangeCount) {
        --const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).warmupRangeCount;
    } else if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->queryRangeCount) {
        for (const auto& pipelineExecutionCountItr : pipelineExecutionCounts) {
            auto device = pipelineExecutionCountItr.first.get_dispatchable_handle();
            auto pipeline = pipelineExecutionCountItr.first.get_handle();
            auto value = (double)pipelineExecutionCountItr.second;
            add_metric_result_to_report(device, pipeline, { GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT, 0, 0, 0 }, value);
        }
        for (const auto& pipelineTimestampQueryResultItr : pipelineTimestampQueryResults) {
            auto device = pipelineTimestampQueryResultItr.first.get_dispatchable_handle();
            auto pipeline = pipelineTimestampQueryResultItr.first.get_handle();
            auto value = pipelineTimestampQueryResultItr.second;
            add_metric_result_to_report(device, pipeline, { GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY, 0, 0, 0 }, value);
        }
        --const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).queryRangeCount;
    }
    if (!requestInfo->queryRangeCount) {
        if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && !metricsReport.empty()) {
            publish_metrics_report();
        }
        metricsReport.clear();
        requestInfo.reset();

        // TODO : Wrangle messages
        // TODO : Messages _really_ need to be wrangled to outgoing only
        // TODO : Result manager to deal with creating unique messages for a sample?
        if (!messages.empty()) {
            std::vector<const char*> messagePtrs;
            messagePtrs.reserve(messages.size());
            std::set<std::string> uniqueMessages;
            for (const auto& message : messages) {
                if (uniqueMessages.insert(message).second) {
                    messagePtrs.push_back(message.c_str());
                }
            }
            auto resultInfo = gvk::get_default<GvkPipelineExplorerResultInfo>();
            resultInfo.messageCount = (uint32_t)messagePtrs.size();
            resultInfo.ppMessages = !messagePtrs.empty() ? messagePtrs.data() : nullptr;
            gvk::write_serialized_structure(workspacePath / ".data", resultInfo);
            messages.clear();
        }
    }

    // TODO : Rework all query request/result logic
    pipelineExecutionCounts.clear();
    pipelineTimestampQueryResults.clear();
}

void PipelineExplorer::process_beginning_of_frame_and_incoming_messages()
{
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> performanceQueryRequestInfo;
    switch (gvk::read_serialized_structure(workspacePath / ".data", performanceQueryRequestInfo)) {
    case VK_SUCCESS: {
        switch (performanceQueryManager.submit_request(workspacePath, std::move(performanceQueryRequestInfo))) {
        case VK_SUCCESS: {
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &performanceQueryManager;
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

#ifdef WIN32
    // TODO : Rework all query request/result logic
    performanceQueryRequestInfo.reset();
    switch (gvk::read_serialized_structure(workspacePath / ".data", "MDAPI_REQUEST", performanceQueryRequestInfo)) {
    case VK_SUCCESS: {
        switch (pluginManager.submit_request(workspacePath, std::move(performanceQueryRequestInfo))) {
        case VK_SUCCESS: {
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::PluginManager::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::PluginManager::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::PluginManager::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::PluginManager::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::PluginManager::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::PluginManager::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::PluginManager::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::PluginManager::post_process_range;
            toolCallbackInfo.pUserData = &pluginManager;
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }
#else
    // TODO : Why doesn't this work on Linux?
    // TODO : Gotta rework structure utility includes anyway
#endif // WIN32

    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo> pipelineStatisticsQueryRequestInfo;
    switch (gvk::read_serialized_structure(workspacePath / ".data", pipelineStatisticsQueryRequestInfo)) {
    case VK_SUCCESS: {
        switch (pipelineStatisticsQueryManager.submit_request(workspacePath, std::move(pipelineStatisticsQueryRequestInfo))) {
        case VK_SUCCESS: {
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &pipelineStatisticsQueryManager;
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

    // TODO : Rework all query request/result logic
    commandCollectionRequestManager.process_incoming_requests(workspacePath);
    if (commandCollectionRequestManager.get_request().sType == gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>()) {
        toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
        toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
        toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
        toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
        toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
        toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
        toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
        toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
        toolCallbackInfo.pUserData = &commandCollectionRequestManager;
    }

#ifdef WIN32
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> timestampQueryRequest;
    switch (gvk::read_serialized_structure(workspacePath / ".data", "TimestampQueryRequest", timestampQueryRequest)) {
    case VK_SUCCESS: {
        switch (timestampQueryManager.submit_request(workspacePath, std::move(timestampQueryRequest))) {
        case VK_SUCCESS: {
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &timestampQueryManager;
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }
#else
    // TODO : Why doesn't this work on Linux?
    // TODO : Gotta rework structure utility includes anyway
#endif // WIN32

#ifdef WIN32
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> timestampHACKRequest;
    switch (gvk::read_serialized_structure(workspacePath / ".data", "TimestampHACKRequest", timestampHACKRequest)) {
    case VK_SUCCESS: {
        switch (timestampQueryManager.submit_request(workspacePath, std::move(timestampHACKRequest))) {
        case VK_SUCCESS: {
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &timestampQueryManager;
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }
#else
    // TODO : Why doesn't this work on Linux?
    // TODO : Gotta rework structure utility includes anyway
#endif // WIN32

    // TODO : Rework all query request/result logic
    if (requestInfo->sType != gvk::get_stype<GvkPipelineExplorerRequestInfo>()) {
        switch (gvk::read_serialized_structure(workspacePath / ".data", requestInfo)) {
        case VK_SUCCESS: {

            // TODO : Rework all query request/result logic
            if (requestInfo->refreshActivePipelines) {
                if (!requestInfo->queryRangeCount) {
                    const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).queryRangeCount = 1;
                }
            }

            // TODO : Rework all query request/result logic
            if (requestInfo->refreshAvailableMetrics) {
                requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
                const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).refreshAvailableMetrics = true;
            }

            // TODO : Rework all query request/result logic
            if (requestInfo->getApiCalls) {
                requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
                // TODO : Setup collection for sample range
                TODO_shouldBeControlledByRequestInfo_getApiCalls = true;
            }

            // TODO : Rework all query request/result logic
            if (requestInfo->getGpuCalls) {
                requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
                // TODO : Setup collection for sample range
                TODO_shouldBeControlledByRequestInfo_getGpuCalls = true;
            }

            // TODO : Rework all query request/result logic
            if (requestInfo->device && (requestInfo->decompilePipeline || requestInfo->recompilePipeline || requestInfo->experimentPipeline || requestInfo->highlightPipeline)) {

                // TODO : Rework all query request/result logic
                if (requestInfo->decompilePipeline) {
                    decompile_pipeline(requestInfo->device, requestInfo->decompilePipeline);
                    if (requestInfo->pDecompilePipelinePath) {
                        write_pipeline_info(requestInfo->device, requestInfo->decompilePipeline, requestInfo->pDecompilePipelinePath);
                    }
                }

                // TODO : Rework all query request/result logic
                if (requestInfo->recompilePipeline && requestInfo->pRecompilePipelinePath) {
                    create_experiment_pipeline(requestInfo->device, requestInfo->recompilePipeline, requestInfo->pRecompilePipelinePath);
                }

                // TODO : Rework all query request/result logic
                if (requestInfo->experimentPipeline && requestInfo->pExperimentPipelinePath) {
                    enable_experiment_pipeline(requestInfo->device, requestInfo->experimentPipeline, requestInfo->pExperimentPipelinePath, requestInfo->experimentEnabled);
                }

                // TODO : Rework all query request/result logic
                if (requestInfo->highlightPipeline && requestInfo->pHighlightPipelinePath) {
                    enable_highlight_pipeline(requestInfo->device, requestInfo->highlightPipeline, requestInfo->pHighlightPipelinePath, requestInfo->highlightEnabled, requestInfo->highlightColor);
                }

                // TODO : Wrangle messages
                // TODO : Messages _really_ need to be wrangled to outgoing only
                std::vector<const char*> messagePtrs;
                messagePtrs.reserve(messages.size());
                for (const auto& message : messages) {
                    messagePtrs.push_back(message.c_str());
                }
                auto resultInfo = gvk::get_default<GvkPipelineExplorerResultInfo>();
                resultInfo.messageCount = (uint32_t)messagePtrs.size();
                resultInfo.ppMessages = !messagePtrs.empty() ? messagePtrs.data() : nullptr;
                gvk::write_serialized_structure(workspacePath / ".data", resultInfo);
                messages.clear();

                // TODO : Wrangle requestInfo
                requestInfo.reset();
            }
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    }
}

std::vector<std::string> PipelineExplorer::add_metric_result_to_report(VkDevice device, VkPipeline pipeline, GvkPipelineExplorerMetricId metricId, double value)
{
    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && !requestInfo->warmupRangeCount && requestInfo->queryRangeCount) {
        metricsReport[{ device, pipeline }][metricId].push_back(value);
    }
    return { };
}

std::vector<std::string> PipelineExplorer::publish_metrics_report()
{
    // TODO : Rework all query request/result logic
    auto resultInfo = gvk::get_default<GvkPipelineExplorerResultInfo>();
    std::vector<GvkPipelineExplorerPipelineResultInfo> pipelineResultInfos;
    pipelineResultInfos.reserve(metricsReport.size());
    std::vector<std::vector<GvkPipelineExplorerMetricResultInfo>> metricResultInfos;
    metricResultInfos.reserve(metricsReport.size());
    for (const auto& pipelineItr : metricsReport) {
        pipeline_explorer::PipelineInfo pipelineInfo(pipelineItr.first);
        pipelineResultInfos.push_back(gvk::get_default<GvkPipelineExplorerPipelineResultInfo>());
        pipelineResultInfos.back().pipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
        boost::multiprecision::export_bits(pipelineInfo->uuid, pipelineResultInfos.back().pipelineInfo.uuid, 8);
        boost::multiprecision::export_bits(pipelineInfo->driverUUID, pipelineResultInfos.back().pipelineInfo.driverUUID, 8);
        pipelineResultInfos.back().pipelineInfo.pName = "VkPipeline"; // TODO : Get name from pipelineInfo
        pipelineResultInfos.back().pipelineInfo.device = pipelineInfo->deviceInfo->vkHandle;
        pipelineResultInfos.back().pipelineInfo.pipeline = pipelineInfo->vkHandle;
        pipelineResultInfos.back().pipelineInfo.bindPoint = pipelineInfo->bindPoint;
        pipelineResultInfos.back().pipelineInfo.labelCount = 0; // TODO : Get labels from pipelineInfo
        pipelineResultInfos.back().pipelineInfo.pLabels = nullptr; // TODO : Get labels from pipelineInfo
        pipelineResultInfos.back().pipelineInfo.experimentEnabled = pipelineInfo->experimentEnabled;
        // TODO : pipelineExplorerPipelineResultInfos.back().pipelineInfo.experimentUUID;
        pipelineResultInfos.back().pipelineInfo.highlightEnabled = pipelineInfo->highlightEnabled;
        memcpy(pipelineResultInfos.back().pipelineInfo.highlightColor, pipelineInfo->highlightColor, sizeof(pipelineInfo->highlightColor));
        metricResultInfos.push_back({ });
        metricResultInfos.back().reserve(pipelineItr.second.size());
        for (const auto& metricItr : pipelineItr.second) {
            metricResultInfos.back().push_back(gvk::get_default<GvkPipelineExplorerMetricResultInfo>());
            if (metricItr.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                metricResultInfos.back().back().metricInfo = gvk::get_default<GvkPipelineExplorerMetricInfo>();
                metricResultInfos.back().back().metricInfo.id = metricItr.first;
                metricResultInfos.back().back().metricInfo.type = GVK_PIPELINE_EXPLORER_METRIC_TYPE_COUNT;
                metricResultInfos.back().back().metricInfo.pUri = nullptr;
                metricResultInfos.back().back().metricInfo.pName = "Execution Count";
                metricResultInfos.back().back().metricInfo.pDescription = nullptr;
                metricResultInfos.back().back().metricInfo.pUnits = nullptr;
                metricResultInfos.back().back().metricInfo.pSymbolicName = nullptr;
                metricResultInfos.back().back().metricInfo.pGroupName = nullptr;
            } else if (metricItr.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                metricResultInfos.back().back().metricInfo = gvk::get_default<GvkPipelineExplorerMetricInfo>();
                metricResultInfos.back().back().metricInfo.id = metricItr.first;
                metricResultInfos.back().back().metricInfo.type = GVK_PIPELINE_EXPLORER_METRIC_TYPE_TIME;
                metricResultInfos.back().back().metricInfo.pUri = nullptr;
                metricResultInfos.back().back().metricInfo.pName = "Execution Time";
                metricResultInfos.back().back().metricInfo.pDescription = nullptr;
                metricResultInfos.back().back().metricInfo.pUnits = nullptr;
                metricResultInfos.back().back().metricInfo.pSymbolicName = nullptr;
                metricResultInfos.back().back().metricInfo.pGroupName = nullptr;
            } else {
                const auto& metricInfoItr = availableMetrics.find(metricItr.first);
                auto metricInfo = metricInfoItr != availableMetrics.end() ? *metricInfoItr->second : gvk::get_default<GvkPipelineExplorerMetricInfo>();
                metricResultInfos.back().back().metricInfo = metricInfo;
            }
            metricResultInfos.back().back().sampleCount = (uint32_t)metricItr.second.size();
            metricResultInfos.back().back().pValues = !metricItr.second.empty() ? metricItr.second.data() : nullptr;
            for (uint32_t sample_i = 0; sample_i < metricResultInfos.back().back().sampleCount; ++sample_i) {
                metricResultInfos.back().back().total += metricResultInfos.back().back().pValues[sample_i];
            }
            if (metricResultInfos.back().back().sampleCount) {
                metricResultInfos.back().back().average = metricResultInfos.back().back().total / metricResultInfos.back().back().sampleCount;
            }
        }
        pipelineResultInfos.back().metricResultCount = (uint32_t)metricResultInfos.back().size();
        pipelineResultInfos.back().pMetricResults = !metricResultInfos.back().empty() ? metricResultInfos.back().data() : nullptr;
    }

    // TODO : Rework all query request/result logic
    std::vector<std::string> messageStrs;

    // TODO : Rework all query request/result logic
    std::filesystem::path reportPath;
    if (requestInfo->pReportPath) {
        reportPath = requestInfo->pReportPath;
        std::filesystem::create_directories(reportPath);
        auto dateTime = gvk::system::DateTime::now();
        auto dateTimeStr = std::to_string(dateTime.dayOfTheMonth) + "-";
        dateTimeStr += std::to_string((int)dateTime.month) + "-";
        dateTimeStr += std::to_string(dateTime.year) + "_";
        dateTimeStr += std::to_string(dateTime.hour) + "-";
        dateTimeStr += std::to_string(dateTime.minute) + "-";
        dateTimeStr += std::to_string(dateTime.second);
        reportPath /= dateTimeStr + ".metrics";
        messageStrs.push_back("INFO : Metrics report written to \"" + reportPath.string() + "\"");
    }

    // TODO : Rework all query request/result logic
    std::vector<const char*> messagePtrs;
    messagePtrs.reserve(messageStrs.size());
    for (const auto& messageStr : messageStrs) {
        messagePtrs.push_back(messageStr.c_str());
    }

    // TODO : Date and time
    // TODO : Metadata
    resultInfo.messageCount = (uint32_t)messagePtrs.size();
    resultInfo.ppMessages = !messagePtrs.empty() ? messagePtrs.data() : nullptr;
    resultInfo.pipelineResultCount = (uint32_t)pipelineResultInfos.size();
    resultInfo.pPipelineResults = !pipelineResultInfos.empty() ? pipelineResultInfos.data() : nullptr;

    // TODO : Rework all query request/result logic
    gvk::write_serialized_structure(workspacePath / ".data", resultInfo);

    // TODO : Rework all query request/result logic
    if (!reportPath.empty()) {
        std::ofstream file(reportPath);
        file << gvk::to_string(resultInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
    }

    return { };
}

////////////////////////////////////////////////////////////////////////////////

VkResult PipelineExplorer::handle_pre_process_command_buffers_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPreProcessCommandBuffer) {
            toolCommandBufferCallbackInfo.pfnPreProcessCommandBuffer(&toolCommandBufferInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_pre_process_cmd_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPreProcessCmd) {
            toolCommandBufferCallbackInfo.pfnPreProcessCmd(&toolCommandBufferInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_cmd_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPostProcessCmd) {
            toolCommandBufferCallbackInfo.pfnPostProcessCmd(&toolCommandBufferInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_command_buffers_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPostProcessCommandBuffer) {
            toolCommandBufferCallbackInfo.pfnPostProcessCommandBuffer(&toolCommandBufferInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_pre_process_queue_submission_callback(GvkPipelineExplorerToolQueueInfo toolQueueInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPreProcessQueueSubmission) {
            toolCommandBufferCallbackInfo.pfnPreProcessQueueSubmission(&toolQueueInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_queue_submission_callback(GvkPipelineExplorerToolQueueInfo toolQueueInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPostProcessQueueSubmission) {
            toolCommandBufferCallbackInfo.pfnPostProcessQueueSubmission(&toolQueueInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

////////////////////////////////////////////////////////////////////////////////

VkResult PipelineExplorer::handle_pre_process_range_callback_ex()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPreProcessRange) {
            gvk_result(toolCallbackInfo.pfnPreProcessRange(toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_pre_process_command_buffers_callback_ex(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPreProcessCommandBuffers) {
            gvk_result(toolCallbackInfo.pfnPreProcessCommandBuffers(&toolInfo, toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_pre_process_cmd_callback_ex(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPreProcessCmd) {
            gvk_result(toolCallbackInfo.pfnPreProcessCmd(&toolInfo, toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_cmd_callback_ex(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPostProcessCmd) {
            gvk_result(toolCallbackInfo.pfnPostProcessCmd(&toolInfo, toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_command_buffers_callback_ex(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPostProcessCommandBuffers) {
            gvk_result(toolCallbackInfo.pfnPostProcessCommandBuffers(&toolInfo, toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_pre_process_queue_submission_callback_ex(GvkPipelineExplorerToolQueueInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPreProcessQueueSubmission) {
            gvk_result(toolCallbackInfo.pfnPreProcessQueueSubmission(&toolInfo, toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_queue_submission_callback_ex(GvkPipelineExplorerToolQueueInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPostProcessQueueSubmission) {
            gvk_result(toolCallbackInfo.pfnPostProcessQueueSubmission(&toolInfo, toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_range_callback_ex()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (toolCallbackInfo.pfnPostProcessRange) {
            gvk_result(toolCallbackInfo.pfnPostProcessRange(toolCallbackInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

////////////////////////////////////////////////////////////////////////////////

void PipelineExplorer::reset()
{
}

} // namespace gvk

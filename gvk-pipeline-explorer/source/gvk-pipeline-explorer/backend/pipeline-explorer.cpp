
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

VkResult PipelineExplorer::post_execute_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(BasicPipelineExplorer::post_execute_vkSetDebugUtilsObjectNameEXT(device, pNameInfo));
        if (pNameInfo && pNameInfo->pObjectName) {
            switch (pNameInfo->objectType) {
            case VK_OBJECT_TYPE_INSTANCE: {
                gvk::pipeline_explorer::InstanceInfo objectInfo((VkInstance)pNameInfo->objectHandle);
                if (objectInfo) {
                    objectInfo->name = pNameInfo->pObjectName;
                }
            } break;
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE: {
                gvk::pipeline_explorer::PhysicalDeviceInfo objectInfo((VkPhysicalDevice)pNameInfo->objectHandle);
                if (objectInfo) {
                    objectInfo->name = pNameInfo->pObjectName;
                }
            } break;
            case VK_OBJECT_TYPE_DEVICE: {
                gvk::pipeline_explorer::DeviceInfo objectInfo((VkDevice)pNameInfo->objectHandle);
                if (objectInfo) {
                    objectInfo->name = pNameInfo->pObjectName;
                }
            } break;
            case VK_OBJECT_TYPE_QUEUE: {
                gvk::pipeline_explorer::QueueInfo objectInfo((VkQueue)pNameInfo->objectHandle);
                if (objectInfo) {
                    objectInfo->name = pNameInfo->pObjectName;
                }
            } break;
            case VK_OBJECT_TYPE_COMMAND_BUFFER: {
                gvk::pipeline_explorer::CommandBufferInfo objectInfo((VkCommandBuffer)pNameInfo->objectHandle);
                if (objectInfo) {
                    objectInfo->name = pNameInfo->pObjectName;
                }
            } break;
            case VK_OBJECT_TYPE_PIPELINE: {
                gvk::pipeline_explorer::PipelineInfo objectInfo({ device, (VkPipeline)pNameInfo->objectHandle });
                if (objectInfo) {
                    objectInfo->name = pNameInfo->pObjectName;
                }
            } break;
            default: {
                // NOOP :
            } break;
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

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

void PipelineExplorer::start_ipc_thread()
{
    if (!mIpcThread.joinable()) {
        mIpcContext.restart();
        mupIpcWorkGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(asio::make_work_guard(mIpcContext));
        mupIpcTimer = std::make_unique<asio::steady_timer>(mIpcContext);
        mIpcThread = std::thread(
            [this]()
            {
                mIpcContext.run();
            }
        );
        process_incoming_messages();
        mIpcMessenger.write("gvk::pipeline_explorer::IpcMessenger started");
    }
}

void PipelineExplorer::stop_ipc_thread()
{
    mIpcMessenger.write("gvk::pipeline_explorer::IpcMessenger stopped");
    mupIpcWorkGuard.reset();
    mIpcContext.stop();
    if (mIpcThread.joinable()) {
        mIpcThread.join();
    }
}

void PipelineExplorer::enable_timeline_query()
{
#ifdef GVK_PLATFORM_WINDOWS
    auto pIpcMessenger = mIpcMessenger.get_read_pipe() && mIpcMessenger.get_write_pipe() ? &mIpcMessenger : nullptr;
#if 0
    if (timelineQueryManager.enable(mReportPath, pIpcMessenger) == VK_SUCCESS) {
        toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
        toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
        toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
        toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
        toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
        toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
        toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
        toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
        toolCallbackInfo.pUserData = &timelineQueryManager;
    }
#else
    mTimelineQueryManager.enable(mReportPath, pIpcMessenger);
#endif
#endif
}

void PipelineExplorer::disable_timeline_query()
{
    // NOOP : Currently automatically disabled at the end of the frame after reporting results
    // TODO : Rework API to allow manual disabling and reporting of timeline query results
}

void PipelineExplorer::process_incoming_messages()
{
#ifdef GVK_PLATFORM_WINDOWS
    mIpcContext.post(
        [this]()
        {
            // Process incoming messages
            for (const auto& message : mIpcMessenger.read()) {

                // Process incoming pipeline requests
                // TODO : Update frontend to send this message on the new message codepath
                if (message.text == "GvkPipelineExplorerPipelineRequestInfo") {
                    std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
                    gvk::Auto<GvkPipelineExplorerPipelineRequestInfo> pipelineRequestInfo;
                    gvk::deserialize(istrm, nullptr, pipelineRequestInfo);
                    std::lock_guard<std::mutex> lock(queueSubmissionMutex);
                    if (pipelineRequestInfo->decompilePipeline) {
                        decompile_pipeline(pipelineRequestInfo->device, pipelineRequestInfo->decompilePipeline);
                        if (pipelineRequestInfo->pDecompilePipelinePath) {
                            write_pipeline_info(pipelineRequestInfo->device, pipelineRequestInfo->decompilePipeline, pipelineRequestInfo->pDecompilePipelinePath);
                        }
                    }
                    if (pipelineRequestInfo->recompilePipeline && pipelineRequestInfo->pRecompilePipelinePath) {
                        create_experiment_pipeline(pipelineRequestInfo->device, pipelineRequestInfo->recompilePipeline, pipelineRequestInfo->pRecompilePipelinePath);
                    }
                    if (pipelineRequestInfo->experimentPipeline && pipelineRequestInfo->pExperimentPipelinePath) {
                        enable_experiment_pipeline(pipelineRequestInfo->device, pipelineRequestInfo->experimentPipeline, pipelineRequestInfo->pExperimentPipelinePath, pipelineRequestInfo->experimentEnabled);
                    }
                }

                // TODO : Documentation
                else if (message.text == "GvkPipelineExplorerQueryRequestInfo") {
                    std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
                    gvk::Auto<GvkPipelineExplorerQueryRequestInfo> queryRequestInfo;
                    gvk::deserialize(istrm, nullptr, queryRequestInfo);
                    std::lock_guard<std::mutex> lock(queueSubmissionMutex);
                    mToolDispatchManager.submit_request(*queryRequestInfo, mIpcMessenger);
                }

                // TODO : Documentation
                else if (message.text == "GvkPipelineExplorerTimelineQueryRequestInfo") {
                    std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
                    gvk::Auto<GvkPipelineExplorerTimelineQueryRequestInfo> timelineQueryRequestInfo;
                    gvk::deserialize(istrm, nullptr, timelineQueryRequestInfo);
                    // TODO : Pass timelineQueryRequestInfo into enable_timeline_query()
                    enable_timeline_query();
                    // timelineQueryManager.disable();
                }
            }

            // Process outgoing messages
            for (const auto& message : messages) {
                mIpcMessenger.write("message", message);
            }
            messages.clear();

            // Schedule next poll after short delay
            if (mupIpcTimer) {
                mupIpcTimer->expires_after(std::chrono::milliseconds(16));
                mupIpcTimer->async_wait(
                    [this](const asio::error_code& error)
                    {
                        if (!error) {
                            process_incoming_messages();
                        }
                    }
                );
            }
        }
    );
#endif
}

void PipelineExplorer::process_end_of_frame_and_outgoing_messages()
{
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
#if 0
        for (const auto& pipelineTimestampQueryResultItr : pipelineTimestampQueryResults) {
            auto device = pipelineTimestampQueryResultItr.first.get_dispatchable_handle();
            auto pipeline = pipelineTimestampQueryResultItr.first.get_handle();
            auto value = pipelineTimestampQueryResultItr.second;
            add_metric_result_to_report(device, pipeline, { GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY, 0, 0, 0 }, value);
        }
#endif
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
#if 0
    pipelineTimestampQueryResults.clear();
#endif
}

void PipelineExplorer::process_beginning_of_frame_and_incoming_messages()
{
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> performanceQueryRequestInfo;
    switch (gvk::read_serialized_structure(workspacePath / ".data", performanceQueryRequestInfo)) {
    case VK_SUCCESS: {
        switch (mPerformanceQueryManager.submit_request(workspacePath, std::move(performanceQueryRequestInfo))) {
        case VK_SUCCESS: {
#if 0
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &performanceQueryManager;
#else
            // performanceQueryManager.submit_request(workspacePath, std::move(performanceQueryRequestInfo));
#endif
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
#if 0
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::PluginManager::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::PluginManager::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::PluginManager::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::PluginManager::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::PluginManager::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::PluginManager::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::PluginManager::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::PluginManager::post_process_range;
            toolCallbackInfo.pUserData = &pluginManager;
#else
#endif
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
        switch (mPipelineStatisticsQueryManager.submit_request(workspacePath, std::move(pipelineStatisticsQueryRequestInfo))) {
        case VK_SUCCESS: {
#if 0
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &pipelineStatisticsQueryManager;
#else
#endif
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
    mCommandCollectionRequestManager.process_incoming_requests(workspacePath);
    if (mCommandCollectionRequestManager.get_request().sType == gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>()) {
#if 0
        toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
        toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
        toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
        toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
        toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
        toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
        toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
        toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
        toolCallbackInfo.pUserData = &commandCollectionRequestManager;
#else
#endif
    }

#ifdef WIN32
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> timestampQueryRequest;
    switch (gvk::read_serialized_structure(workspacePath / ".data", "TimestampQueryRequest", timestampQueryRequest)) {
    case VK_SUCCESS: {
        switch (mTimestampQueryManager.submit_request(workspacePath, std::move(timestampQueryRequest))) {
        case VK_SUCCESS: {
#if 0
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &timestampQueryManager;
#else
#endif
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
        switch (mTimestampQueryManager.submit_request(workspacePath, std::move(timestampHACKRequest))) {
        case VK_SUCCESS: {
#if 0
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &timestampQueryManager;
#else
#endif
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

VkResult PipelineExplorer::pre_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.pre_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::pre_process_command_buffers(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.pre_process_command_buffers(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::pre_process_cmd(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.pre_process_cmd(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_process_cmd(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.post_process_cmd(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_process_command_buffers(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.post_process_command_buffers(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::pre_process_queue_submission(GvkPipelineExplorerToolQueueInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.pre_process_queue_submission(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_process_queue_submission(GvkPipelineExplorerToolQueueInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.post_process_queue_submission(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::pre_process_queue_present(GvkPipelineExplorerToolQueueInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.pre_process_queue_present(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_process_queue_present(GvkPipelineExplorerToolQueueInfoEx toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.post_process_queue_present(toolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mToolDispatchManager.post_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

////////////////////////////////////////////////////////////////////////////////

void PipelineExplorer::reset()
{
    stop_ipc_thread();
}

} // namespace gvk

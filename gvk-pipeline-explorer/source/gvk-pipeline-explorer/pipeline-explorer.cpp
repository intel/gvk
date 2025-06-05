
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
#include "gvk-pipeline-explorer/pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/utilities.hpp"
#include "gvk-system/time.hpp"

namespace gvk {

void PipelineExplorer::process_end_of_frame_and_outgoing_messages()
{
    // TODO : Documentation
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
    }

    // TODO : Documentation
    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->warmupFrameCount) {
        --const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).warmupFrameCount;
    } else if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && requestInfo->sampleFrameCount) {
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
        for (const auto& pipelineStatisticsQueryResultsItr : pipelineStatisticsQueryResults) {
            auto device = pipelineStatisticsQueryResultsItr.first.get_dispatchable_handle();
            auto pipeline = pipelineStatisticsQueryResultsItr.first.get_handle();
            for (const auto& pipelineStatisticsQueryResultItr : pipelineStatisticsQueryResultsItr.second) {
                add_metric_result_to_report(device, pipeline, pipelineStatisticsQueryResultItr.first, (double)pipelineStatisticsQueryResultItr.second);
            }
        }
        --const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).sampleFrameCount;
    }
    if (!requestInfo->sampleFrameCount) {
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

    // TODO : Documentation
    pipelineExecutionCounts.clear();
    pipelineTimestampQueryResults.clear();
    pipelineStatisticsQueryResults.clear();
}

void PipelineExplorer::process_beginning_of_frame_and_incoming_messages()
{
    if (requestInfo->sType != gvk::get_stype<GvkPipelineExplorerRequestInfo>()) {
        switch (gvk::read_serialized_structure(workspacePath / ".data", requestInfo)) {
        case VK_SUCCESS: {
#if 0
            if (requestInfo->refreshActivePipelines || requestInfo->refreshAvailableMetrics) {
                const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).refreshActivePipelines = true;
                const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).refreshAvailableMetrics = true;
                if (!requestInfo->sampleFrameCount) {
                    const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).sampleFrameCount = 1;
                }
                // TODO : Turn on timestamp query
            }
#else
            // TODO : Documentation
            if (requestInfo->refreshActivePipelines) {
                if (!requestInfo->sampleFrameCount) {
                    const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).sampleFrameCount = 1;
                }
            }

            // TODO : Documentation
            if (requestInfo->refreshAvailableMetrics) {
                requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
                const_cast<GvkPipelineExplorerRequestInfo&>(*requestInfo).refreshAvailableMetrics = true;
            }

            // TODO : Documentation
            if (requestInfo->device && (requestInfo->decompilePipeline || requestInfo->recompilePipeline || requestInfo->experimentPipeline || requestInfo->highlightPipeline)) {

                // TODO : Documentation
                if (requestInfo->decompilePipeline) {
                    decompile_pipeline(requestInfo->device, requestInfo->decompilePipeline);
                    if (requestInfo->pDecompilePipelinePath) {
                        write_pipeline_info(requestInfo->device, requestInfo->decompilePipeline, requestInfo->pDecompilePipelinePath);
                    }
                }

                // TODO : Documentation
                if (requestInfo->recompilePipeline && requestInfo->pRecompilePipelinePath) {
                    create_experiment_pipeline(requestInfo->device, requestInfo->recompilePipeline, requestInfo->pRecompilePipelinePath);
                }

                // TODO : Documentation
                if (requestInfo->experimentPipeline && requestInfo->pExperimentPipelinePath) {
                    enable_experiment_pipeline(requestInfo->device, requestInfo->experimentPipeline, requestInfo->pExperimentPipelinePath, requestInfo->experimentEnabled);
                }

                // TODO : Documentation
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
    }
}

#if 0
std::string PipelineExplorer::get_pipeline_report_name(VkDevice device, VkPipeline pipeline)
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    (void)pipelineInfo; // TODO : Get report name from PipelineInfo
    return "VkPipeline-" + gvk::to_hex_string(pipeline);
}
#endif

std::vector<std::string> PipelineExplorer::add_metric_result_to_report(VkDevice device, VkPipeline pipeline, GvkPipelineExplorerMetricId metricId, double value)
{
    if (requestInfo->sType == gvk::get_stype<GvkPipelineExplorerRequestInfo>() && !requestInfo->warmupFrameCount && requestInfo->sampleFrameCount) {
        metricsReport[{ device, pipeline }][metricId].push_back(value);
    }
    return { };
}

std::vector<std::string> PipelineExplorer::publish_metrics_report()
{
    // TODO : Documentation
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

    // TODO : Documentation
    std::vector<std::string> messageStrs;

    // TODO : Documentation
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

    // TODO : Documentation
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

    // TODO : Documentation
    gvk::write_serialized_structure(workspacePath / ".data", resultInfo);

    // TODO : Documentation
    if (!reportPath.empty()) {
        std::ofstream file(reportPath);
        file << gvk::to_string(resultInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
    }

    return { };
}

VkResult PipelineExplorer::handle_pre_process_command_buffer_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
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
    (void)toolCommandBufferInfo;
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
    (void)toolCommandBufferInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPostProcessCmd) {
            toolCommandBufferCallbackInfo.pfnPostProcessCmd(&toolCommandBufferInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::handle_post_process_command_buffer_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo)
{
    (void)toolCommandBufferInfo;
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
    (void)toolQueueInfo;
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
    (void)toolQueueInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        if (toolCommandBufferCallbackInfo.pfnPostProcessQueueSubmission) {
            toolCommandBufferCallbackInfo.pfnPostProcessQueueSubmission(&toolQueueInfo, toolCommandBufferCallbackInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::reset()
{
}

} // namespace gvk

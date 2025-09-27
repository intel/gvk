
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

#include "gvk-pipeline-explorer/gui/pipelines-window.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

PipelinesWindow::PipelinesWindow(Window::Manager& windowManager)
    : Window(windowManager, "Pipelines")
{
}

void PipelinesWindow::on_gui(GuiInfo& guiInfo)
{
    // TODO : Unify timestamp query behavior
    ImGui::BeginDisabled(!guiInfo.applicationInfo.running && !guiInfo.cliProvidedWorkspace);
    {
        guiInfo.requestInfo.refreshActivePipelines = ImGui::Button("Refresh Active Pipelines");

        #if 0
        // DEBUGGING :
        static std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::pair<double, double>> sTimestamps;
        if (guiInfo.timestampInfo.requestResult.pending()) {
            if (guiInfo.timestampInfo.requestResult.check_result(guiInfo.workspaceInfo.workspace) == VK_SUCCESS) {
                guiInfo.timestampInfo.available = guiInfo.timestampInfo.requestResult.get_result();
                for (uint32_t i = 0; i < guiInfo.timestampInfo.available->pipelineTimestampQueryResultCount; ++i) {
                    const auto& pipelineTimestampQueryResult = guiInfo.timestampInfo.available->pPipelineTimestampQueryResults[i];

                    sTimestamps[{
                        pipelineTimestampQueryResult.pipelineInfo.device,
                        pipelineTimestampQueryResult.pipelineInfo.pipeline
                    }] = {
                        pipelineTimestampQueryResult.averageCmdRangeCmdCount,
                        pipelineTimestampQueryResult.averageCmdRangeDuration
                    };

                    for (uint32_t j = 0; j < pipelineTimestampQueryResult.cmdRangeResultCount; ++j) {
                        const auto& cmdRangeResult = pipelineTimestampQueryResult.pCmdRangeResults[j];
                        for (uint32_t k = 0; k < cmdRangeResult.cmdSequenceCount; ++k) {
                            const auto& cmdSequenceResult = cmdRangeResult.pCmdSequences[k];
                            for (uint32_t l = 0; l < cmdSequenceResult.cmdCount; ++l) {
                                auto cmdType = cmdSequenceResult.pCmdTypes[l];
                                (void)cmdType;
                            }
                        }
                    }
                }
            }
        }

        // Generate timestamp query report
        ImGui::SameLine();
        ImGui::BeginDisabled(guiInfo.timestampInfo.requestResult.pending());
        if (ImGui::Button("Generate Timestamp Query Report")) {
            auto request = gvk::get_default<GvkPipelineExplorerPerformanceQueryRequestInfo>();
            std::string reportPath = guiInfo.reportEnabled ? (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string() : std::string();
            request.pReportPath = !reportPath.empty() ? reportPath.c_str() : nullptr;
            request.warmupRangeCount = guiInfo.requestInfo.warmupRangeCount;
            request.queryRangeCount = guiInfo.requestInfo.queryRangeCount;
            guiInfo.timestampInfo.requestResult.submit_request(guiInfo.workspaceInfo.workspace, request, "TimestampQueryRequest");
        }
        ImGui::EndDisabled();
        #endif
    }
    ImGui::EndDisabled();

    // Draw pipelines table
    if (ImGui::BeginChild("##Pipelines Table")) {
        auto tableFlags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
            ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
        if (ImGui::BeginTable("Pipelines-Table", 10, tableFlags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("UUID",                  ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder);
            ImGui::TableSetupColumn("Driver UUID",           ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableSetupColumn("Handle",                ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableSetupColumn("Name",                  ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableSetupColumn("Bind Point",            ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableSetupColumn("Avg. Executions/Frame");
            ImGui::TableSetupColumn("Avg. Time/Frame (ns)");
            ImGui::TableSetupColumn("Highlight");
            ImGui::TableSetupColumn("Experiment",            ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableSetupColumn("Metrics",               ImGuiTableColumnFlags_DefaultHide);
            
            #if 0
            // DEBUGGING :
            ImGui::TableSetupColumn("AvgExecutionsEx", 0, 0, 10);
            ImGui::TableSetupColumn("AvgTimeEx",       0, 0, 11);
            #endif

            ImGui::TableHeadersRow();

            auto pTableSortSpecs = ImGui::TableGetSortSpecs();
            if (pTableSortSpecs && pTableSortSpecs->SpecsDirty) {
                pTableSortSpecs->SpecsDirty = false;
                guiInfo.pipelineSortSpecs.clear();
                guiInfo.pipelineSortSpecs.insert(guiInfo.pipelineSortSpecs.end(), pTableSortSpecs->Specs, pTableSortSpecs->Specs + pTableSortSpecs->SpecsCount);
                sort_pipelines(guiInfo);
            }

            ImGuiListClipper clipper;
            clipper.Begin((int)guiInfo.sortedPipelines.size());
            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n) {
                    auto pipeline = guiInfo.sortedPipelines[row_n];
                    auto& pipelineInfo = guiInfo.pipelineInfos[pipeline];
                    if (!guiInfo.selectedPipeline.get_handle()) {
                        guiInfo.selectedPipeline = pipelineInfo.pipeline;
                    }
                    ImGui::PushID(pipeline.get_handle());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.uuidStr.c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.driverUUIDStr.c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.handleStr.c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.name.c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.bindPointStr.c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }

                    // Execution count
                    ImGui::TableNextColumn();
                    for (const auto& metrics : pipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                            if (ImGui::Selectable(std::to_string(metrics.second->average).c_str(), pipelineInfo.sampleMetrics)) {
                                guiInfo.selectedPipeline = pipeline;
                            }
                            break;
                        }
                    }

                    // Duration
                    ImGui::TableNextColumn();
                    for (const auto& metrics : pipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                            if (ImGui::Selectable(std::to_string(metrics.second->average).c_str(), pipelineInfo.sampleMetrics)) {
                                guiInfo.selectedPipeline = pipeline;
                            }
                            break;
                        }
                    }

                    // Highlight
                    ImGui::TableNextColumn();
                    ImGui::BeginDisabled(!(pipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS || pipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR));
                    bool highlightStateChanged = false;
                    if (ImGui::ColorEdit4("Highlight Color", (float*)&pipelineInfo.highlightColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                        pipelineInfo.highlightEnabled = false;
                        highlightStateChanged = true;
                    }
                    ImGui::SameLine();
                    highlightStateChanged |= ImGui::Checkbox("##Highlight Enabled", &pipelineInfo.highlightEnabled);
                    if (highlightStateChanged) {
                        guiInfo.requestInfo.highlightPipeline = pipelineInfo.pipeline.get_handle();
                        guiInfo.requestInfo.highlightEnabled = pipelineInfo.highlightEnabled;
                        memcpy(guiInfo.requestInfo.highlightColor, &pipelineInfo.highlightColor, sizeof(pipelineInfo.highlightColor));
                    }
                    ImGui::EndDisabled();

                    // Experiment
                    ImGui::TableNextColumn();
                    if (ImGui::Checkbox("##Experiment Enabled", &pipelineInfo.experimentEnabled)) {
                        guiInfo.requestInfo.experimentPipeline = pipelineInfo.pipeline.get_handle();
                        guiInfo.requestInfo.experimentEnabled = pipelineInfo.experimentEnabled;
                    }

                    // Metrics
                    ImGui::TableNextColumn();
                    if (ImGui::Checkbox("##Metrics Enabled", &pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }

                    #if 0
                    // DEBUGGING :
                    const auto& timestamp = sTimestamps[pipelineInfo.pipeline];
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::to_string(timestamp.first).c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::to_string(timestamp.second).c_str(), pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }
                    #endif

                    ImGui::PopID();
                }

                // Force one pipeline selection for metrics
                for (auto& pipelineInfo : guiInfo.pipelineInfos) {
                    pipelineInfo.second.sampleMetrics = pipelineInfo.first == guiInfo.selectedPipeline;
                }
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

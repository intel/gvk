
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
    guiInfo.requestInfo.refreshActivePipelines = ImGui::Button("Refresh Active Pipelines");
    if (ImGui::BeginChild("##Pipelines Table")) {
        auto tableFlags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
            ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
        if (ImGui::BeginTable("Pipelines-Table", 10, tableFlags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("UUID",                  ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder, 0, 0);
            ImGui::TableSetupColumn("Driver UUID",           ImGuiTableColumnFlags_DefaultHide,                              0, 1);
            ImGui::TableSetupColumn("Handle",                ImGuiTableColumnFlags_DefaultHide,                              0, 2);
            ImGui::TableSetupColumn("Name",                  ImGuiTableColumnFlags_DefaultHide,                              0, 3);
            ImGui::TableSetupColumn("Bind Point",            ImGuiTableColumnFlags_DefaultHide,                              0, 4);
            ImGui::TableSetupColumn("Avg. Executions/Frame",                                                              0, 0, 5);
            ImGui::TableSetupColumn("Avg. Time/Frame (ns)",                                                               0, 0, 6);
            ImGui::TableSetupColumn("Highlight",                                                                          0, 0, 7);
            ImGui::TableSetupColumn("Experiment",            ImGuiTableColumnFlags_DefaultHide,                              0, 8);
            ImGui::TableSetupColumn("Metrics",               ImGuiTableColumnFlags_DefaultHide,                              0, 9);
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

                    // TODO : Documentation
                    ImGui::TableNextColumn();
                    for (const auto& metrics : pipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                            if (ImGui::Selectable(std::to_string(metrics.second->average).c_str(), pipelineInfo.sampleMetrics)) {
                                guiInfo.selectedPipeline = pipeline;
                            }
                            break;
                        }
                    }

                    // TODO : Documentation
                    ImGui::TableNextColumn();
                    for (const auto& metrics : pipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                            if (ImGui::Selectable(std::to_string(metrics.second->average).c_str(), pipelineInfo.sampleMetrics)) {
                                guiInfo.selectedPipeline = pipeline;
                            }
                            break;
                        }
                    }

                    // TODO : Documentation
                    ImGui::TableNextColumn();
                    ImGui::BeginDisabled(pipelineInfo.bindPoint != VK_PIPELINE_BIND_POINT_GRAPHICS);
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

                    // TODO : Documentation
                    ImGui::TableNextColumn();
                    if (ImGui::Checkbox("##Experiment Enabled", &pipelineInfo.experimentEnabled)) {
                        guiInfo.requestInfo.experimentPipeline = pipelineInfo.pipeline.get_handle();
                        guiInfo.requestInfo.experimentEnabled = pipelineInfo.experimentEnabled;
                    }

                    // TODO : Documentation
                    ImGui::TableNextColumn();
                    if (ImGui::Checkbox("##Metrics Enabled", &pipelineInfo.sampleMetrics)) {
                        guiInfo.selectedPipeline = pipeline;
                    }

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

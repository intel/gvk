
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

#include "gvk-pipeline-explorer/gui/metrics-window.hpp"

#include <functional>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

MetricsWindow::MetricsWindow(Window::Manager& windowManager)
    : Window(windowManager, "Metrics")
{
}

void MetricsWindow::on_gui(GuiInfo& guiInfo)
{
    guiInfo.requestInfo.refreshAvailableMetrics = ImGui::Button("Refresh Available Metrics");

    // TODO : Documentation
    ImGui::Text("Write Metrics Report");
    ImGui::Checkbox("##Report Enabled", &guiInfo.reportEnabled);
    ImGui::SameLine();
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::BeginDisabled();
    auto reportPath = gvk::string::scrub_path((std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string());
    GvkGui::InputPath("##Report Path", &reportPath);
    ImGui::EndDisabled();
    ImGui::PopItemWidth();

    // TODO : Documentation
    auto warmupFrameCount = (int)guiInfo.requestInfo.warmupFrameCount;
    ImGui::InputInt("Warmup Frame Count", &warmupFrameCount, 1, 4);
    guiInfo.requestInfo.warmupFrameCount = (uint32_t)std::min(std::max(0, warmupFrameCount), 128);

    // TODO : Documentation
    auto sampleFrameCount = (int)guiInfo.requestInfo.sampleFrameCount;
    ImGui::InputInt("Sample Frame Count", &sampleFrameCount, 1, 4);
    guiInfo.requestInfo.sampleFrameCount = (uint32_t)std::min(std::max(1, sampleFrameCount), 128);

    // TODO : Documentation
    ImGui::Separator();
    auto applyMetricsFilter = ImGui::Button("Apply Metrics Filter");
    ImGui::SameLine();
    ImGui::Text("Case sensitive, ';' delimited");
    ImGui::InputText("Any Of", &guiInfo.metricsAnyOfFilter);
    ImGui::InputText("All Of", &guiInfo.metricsAllOfFilter);
    if (applyMetricsFilter) {
        if (guiInfo.metricsAnyOfFilter.empty() && guiInfo.metricsAllOfFilter.empty()) {
            guiInfo.filteredMetrics = guiInfo.availableMetrics;
        } else {
            guiInfo.filteredMetrics.clear();
            const auto& anyFilterTokens = gvk::string::split(guiInfo.metricsAnyOfFilter, ";");
            const auto& allFilterTokens = gvk::string::split(guiInfo.metricsAllOfFilter, ";");
            for (const auto& metricsFilterItr : guiInfo.metricsFilters) {
                bool anyFound = false;
                for (const auto& anyFilterToken : anyFilterTokens) {
                    std::boyer_moore_searcher searcher(anyFilterToken.begin(), anyFilterToken.end());
                    if (std::search(metricsFilterItr.first.begin(), metricsFilterItr.first.end(), searcher) != metricsFilterItr.first.end()) {
                        anyFound = true;
                        break;
                    }
                }
                bool allFound = false;
                if (!anyFound) {
                    for (const auto& allFilterToken : allFilterTokens) {
                        std::boyer_moore_searcher searcher(allFilterToken.begin(), allFilterToken.end());
                        if (std::search(metricsFilterItr.first.begin(), metricsFilterItr.first.end(), searcher) == metricsFilterItr.first.end()) {
                            allFound = false;
                            break;
                        }
                        allFound = true;
                    }
                }
                if (anyFound || allFound) {
                    auto availableMettricsItr = guiInfo.availableMetrics.find(metricsFilterItr.second);
                    if (availableMettricsItr != guiInfo.availableMetrics.end()) {
                        guiInfo.filteredMetrics.insert(*availableMettricsItr);
                    }
                }
            }
        }
    }

    // TODO : Documentation
    ImGui::Separator();
    ImGui::Separator();

    // TODO : Setup clipper...

    // TODO : Documentation
    ImGui::Text("%zu available metrics groups", guiInfo.filteredMetrics.size());
    if (ImGui::BeginChild("##Metrics Table")) {
        for (const auto& metricsGroup : guiInfo.filteredMetrics) {
            ImGui::PushID(metricsGroup.first);
            ImGui::BeginDisabled(!guiInfo.selectedPipeline.get_handle());
            {
                if (ImGui::Button("Sample Metrics")) {
                    guiInfo.requestInfo.sampleMetricIdCount = 1;
                    guiInfo.enabledMetricsGroup = metricsGroup.first;
                }
            }
            ImGui::EndDisabled();

            // TODO : Documentation
            auto tableFlags =
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
            if (ImGui::BeginTable("Metrics-Table", 6, tableFlags)) {
                ImGui::TableSetupColumn("Index");
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Unit");
                ImGui::TableSetupColumn("Total");
                ImGui::TableSetupColumn("Average");
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_DefaultHide);
                ImGui::TableHeadersRow();
                auto selectedPipelineInfo = guiInfo.pipelineInfos[guiInfo.selectedPipeline];
                for (uint32_t metric_i = 0; metric_i < metricsGroup.second.size(); ++metric_i) {
                    const auto& metricInfo = metricsGroup.second[metric_i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::to_string(metric_i).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", metricInfo->pName ? metricInfo->pName : "");
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", metricInfo->pUnits ? metricInfo->pUnits : "");
                    if (selectedPipelineInfo.pipeline.get_handle()) {
                        const auto& metricResultInfo = selectedPipelineInfo.metrics[metricInfo->id];
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", std::to_string(metricResultInfo->total).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", std::to_string(metricResultInfo->average).c_str());
                    } else {
                        ImGui::TableNextColumn();
                        ImGui::Text("0.0");
                        ImGui::TableNextColumn();
                        ImGui::Text("0.0");
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", metricInfo->pDescription ? metricInfo->pDescription : "");
                }
                ImGui::EndTable();
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

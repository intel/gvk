
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

#include "gvk-pipeline-explorer/gui/metrics/pipeline-statistics-metrics-tab.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

PipelineStatisticsMetricsTab::PipelineStatisticsMetricsTab(MetricsWindow& metricsWindow)
    : MetricsTab(metricsWindow)
{
    mName = "Pipeline Statistics";
}

bool PipelineStatisticsMetricsTab::idle(GuiInfo& guiInfo) const
{
    (void)guiInfo;
    return true;
}

bool PipelineStatisticsMetricsTab::enabled(GuiInfo& guiInfo) const
{
    return guiInfo.pipelineStatisticsQueryInfo.available->count;
}

void PipelineStatisticsMetricsTab::on_update(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void PipelineStatisticsMetricsTab::on_gui(GuiInfo& guiInfo)
{
    if (ImGui::BeginChild("##Pipeline Statistics Table")) {
        ImGui::BeginDisabled(!guiInfo.selectedPipeline.get_handle());
        {
            // Check request/result
            if (guiInfo.pipelineStatisticsQueryInfo.requestResult.pending()) {
                guiInfo.pipelineStatisticsQueryInfo.requestResult.check_result(guiInfo.workspaceInfo.workspace);
            }

            // Process request/result
            if (guiInfo.pipelineStatisticsQueryInfo.requestResult.ready()) {
                auto result = guiInfo.pipelineStatisticsQueryInfo.requestResult.get_result();
                auto& pipelineInfo = guiInfo.pipelineInfos[{ result->pipelineInfo.device, result->pipelineInfo.pipeline }];
                if (pipelineInfo.pipeline) {
                    for (uint32_t counter_i = 0; counter_i < result->counterResultCount; ++counter_i) {
                        const auto& counterResults = result->pCounterResults[counter_i];
                        VkQueryPipelineStatisticFlagBits pipelineStatistic{ };
                        assert(sizeof(pipelineStatistic) <= sizeof(counterResults.counter.uuid));
                        memcpy(&pipelineStatistic, counterResults.counter.uuid, sizeof(pipelineStatistic));
                        pipelineInfo.pipelineStatisticsQueryResults[pipelineStatistic].total = counterResults.total;
                        pipelineInfo.pipelineStatisticsQueryResults[pipelineStatistic].average = counterResults.average;
                    }
                }
                guiInfo.pipelineStatisticsQueryInfo.requestResult.reset();
            }

            // Draw query button
            ImGui::BeginDisabled(guiInfo.pipelineStatisticsQueryInfo.requestResult.pending());
            {
                if (ImGui::Button("Query Pipeline Statistics")) {
                    auto request = gvk::get_default<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo>();
                    std::string reportPath = guiInfo.reportEnabled ? (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string() : std::string();
                    request.pReportPath = !reportPath.empty() ? reportPath.c_str() : nullptr;
                    request.device = guiInfo.selectedPipeline.get_dispatchable_handle();
                    request.pipeline = guiInfo.selectedPipeline.get_handle();
                    request.warmupRangeCount = guiInfo.requestInfo.warmupRangeCount;
                    request.queryRangeCount = guiInfo.requestInfo.queryRangeCount;
                    guiInfo.pipelineStatisticsQueryInfo.requestResult.submit_request(guiInfo.workspaceInfo.workspace, request);
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndDisabled();

        // Draw results table
        auto tableFlags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
        if (ImGui::BeginTable("Pipeline Statistics Table", 3, tableFlags)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Total");
            ImGui::TableSetupColumn("Average");
            ImGui::TableHeadersRow();
            auto selectedPipelineInfo = guiInfo.pipelineInfos[guiInfo.selectedPipeline];
            for (uint32_t counter_i = 0; counter_i < guiInfo.pipelineStatisticsQueryInfo.available->count; ++counter_i) {
                const auto& counter = guiInfo.pipelineStatisticsQueryInfo.available->pCounters[counter_i];
                const auto& description = guiInfo.pipelineStatisticsQueryInfo.available->pDescriptions[counter_i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", description.name);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("%s", description.description);
                }
                if (selectedPipelineInfo.pipeline.get_handle()) {
                    VkQueryPipelineStatisticFlagBits pipelineStatistic{ };
                    assert(sizeof(pipelineStatistic) <= sizeof(counter.uuid));
                    memcpy(&pipelineStatistic, counter.uuid, sizeof(pipelineStatistic));
                    const auto& counterResults = selectedPipelineInfo.pipelineStatisticsQueryResults[pipelineStatistic];
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::to_string(counterResults.total).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::to_string(counterResults.average).c_str());
                }
                else {
                    ImGui::TableNextColumn();
                    ImGui::Text("0.0");
                    ImGui::TableNextColumn();
                    ImGui::Text("0.0");
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

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

#include "gvk-pipeline-explorer/gui/api-call-timeline-window.hpp"
#include "gvk-pipeline-explorer/gui/pipelines-window.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

ApiCallTimelineWindow::ApiCallTimelineWindow(Window::Manager& windowManager)
    : Window(windowManager, "Pipeline Executions")
{
}

ApiCallTimelineWindow::~ApiCallTimelineWindow()
{
}

void ApiCallTimelineWindow::reset()
{
#if 0
    mCommandPlotInfos.clear();
#endif
    mPrevCmdCount = 0;
    mFitRequested = true;
}

void ApiCallTimelineWindow::on_update(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef GVK_PLATFORM_WINDOWS
    // Process available counters reported from backend
    const auto& messageItr = guiInfo.incomingIpcMessages.find("GvkPipelineExplorerTimelineQueryResultInfo");
    if (messageItr != guiInfo.incomingIpcMessages.end() && !messageItr->second.empty()) {
        const auto& message = messageItr->second.back();
        std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
        gvk::Auto<GvkPipelineExplorerTimelineQueryResultInfo> timelineQueryResultInfo;
        gvk::deserialize(istrm, nullptr, timelineQueryResultInfo);
        guiInfo.rangeInfo.add_timeline_query_result_info(std::move(timelineQueryResultInfo));
#if 0
        // HACK : Backend sends this message on queue submit, so using that to increment
        //  submit count here, but backend really should send a dedicated message for submit
        ++guiInfo.rangeInfo.submitCount;

        auto count = std::min(timelineQueryResultInfo->commandInfoCount, timelineQueryResultInfo->commands.commandCount);
        if (timelineQueryResultInfo->commandInfoCount != timelineQueryResultInfo->commands.commandCount) {
            std::cout << "Warning : Timestamp query incomplete" << std::endl;
        }
        for (uint32_t cmd_i = 0; cmd_i < count; ++cmd_i) {

            // TODO : Documentation
            auto pCmd = timelineQueryResultInfo->commands.ppCommands[cmd_i];
            const auto& timelineCmdInfo = timelineQueryResultInfo->pCommandInfos[cmd_i];
            auto duration = timelineCmdInfo.end - timelineCmdInfo.begin;
            CmdInfo cmdInfo{ };
            cmdInfo.label = gvk::to_string(pCmd->sType, gvk::Printer::Default & ~gvk::Printer::EnumValue);
            cmdInfo.content = gvk::to_string(*pCmd, gvk::Printer::Default & ~gvk::Printer::EnumValue);
            cmdInfo.threadId = timelineCmdInfo.threadId;
            cmdInfo.device = timelineQueryResultInfo->device;
            cmdInfo.queue = timelineCmdInfo.queue;
            cmdInfo.pipeline = timelineCmdInfo.pipeline;
            cmdInfo.begin = timelineCmdInfo.begin;
            cmdInfo.end = timelineCmdInfo.end;
            guiInfo.rangeInfo.cmdInfos.push_back(cmdInfo);
            guiInfo.rangeInfo.minCmdTimestamp = std::min(guiInfo.rangeInfo.minCmdTimestamp, cmdInfo.begin);
            guiInfo.rangeInfo.maxCmdTimestamp = std::max(guiInfo.rangeInfo.maxCmdTimestamp, cmdInfo.end);
            guiInfo.rangeInfo.maxCmdDuration = std::max(guiInfo.rangeInfo.maxCmdDuration, duration);

            // TODO : Documentation
            auto& pipelineInfo = guiInfo.pipelineInfos[{ timelineQueryResultInfo->device, timelineCmdInfo.pipeline }];
            auto& executionCountMetric = pipelineInfo.metrics[{ GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT }];
            auto& executionCountMetricResult = const_cast<GvkPipelineExplorerMetricResultInfo&>(*executionCountMetric);
            ++executionCountMetricResult.total;

            // TODO : Documentation
            auto& durationMetric = pipelineInfo.metrics[{ GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY }];
            auto& durationMetricResult = const_cast<GvkPipelineExplorerMetricResultInfo&>(*durationMetric);
            durationMetricResult.total += duration;
            durationMetricResult.average = durationMetricResult.total / executionCountMetricResult.total;

            // TODO : Documentation
            auto& pipelineExecutionInfo = guiInfo.rangeInfo.pipelineExecutionInfos[pipelineInfo.pipeline];
            ++pipelineExecutionInfo.executionCount;
            pipelineExecutionInfo.totalDurationNs += duration;
        }
#endif
    }
#endif // GVK_PLATFORM_WINDOWS
}

void ApiCallTimelineWindow::on_gui(GuiInfo& guiInfo)
{
    if (guiInfo.rangeInfo.cmdInfos.empty()) {
        #if 0
        ImGui::TextUnformatted("No command data available.");
        #endif
        return;
    }

    if (ImGui::Button("Center")) {
        mFitRequested = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::SliderFloat("Bar Width", &mBarWidthScale, 1.0f, 10.0f, "%.1fx")) {
        mFitRequested = true;
    }

#ifdef GVK_PLATFORM_WINDOWS
    // TODO : Correctly display size_t/uint64_t
    ImGui::Text("Commands : %d", guiInfo.rangeInfo.cmdInfos.size());
    ImGui::Text("Submits : %d", guiInfo.rangeInfo.submitCount);
    ImGui::Text("Presents : %d", guiInfo.rangeInfo.presentCount);
#endif

    const double nsToUs = 1.0 / 1000.0;
    int totalCmds = static_cast<int>(guiInfo.rangeInfo.cmdInfos.size());
    double maxDuration = guiInfo.rangeInfo.maxCmdDuration * nsToUs;

    // Detect data changes to re-fit axis limits
    bool dataChanged = (totalCmds != mPrevCmdCount);
    mPrevCmdCount = totalCmds;
    if (dataChanged) {
        mFitRequested = true;
        #if 0
        mSelectedCmdIndex = -1;
        #endif
    }

    // Lay out bars: each bar's width = its duration, with a minimum gap between them
    // Precompute cumulative X positions so bars are placed sequentially
    const double minGap = maxDuration * 0.005; // gap = 0.5% of max duration
    const double minBarWidth = maxDuration * 0.002; // minimum visible bar width
    thread_local std::vector<double> barXStarts;
    thread_local std::vector<double> barWidths;
    barXStarts.resize(totalCmds);
    barWidths.resize(totalCmds);
    double xCursor = 0.0;
    for (int i = 0; i < totalCmds; ++i) {
        const auto& cmdInfo = guiInfo.rangeInfo.cmdInfos[i];
        double durationUs = (cmdInfo.endNs - cmdInfo.beginNs) * nsToUs;
        double w = std::max(durationUs, minBarWidth) * mBarWidthScale;
        barXStarts[i] = xCursor;
        barWidths[i] = w;
        xCursor += w + minGap;
    }
    double totalWidth = xCursor > minGap ? xCursor - minGap : 0.0;

    const ImU32 barColorHovered = IM_COL32(255, 200, 80, 240);
    const ImU32 barColorSelected = IM_COL32(66, 133, 244, 220);
    const ImU32 barColorSelectedHovered = IM_COL32(100, 160, 255, 240);
    const ImU32 barColorActivePipeline = IM_COL32(255, 0, 0, 220);
    const ImU32 barColorActivePipelineHovered = IM_COL32(255, 50, 50, 240);

    ImPlotFlags plotFlags = ImPlotFlags_NoBoxSelect;
    if (ImPlot::BeginPlot("Pipeline Executions", ImVec2(-1, -1), plotFlags)) {
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxis(ImAxis_Y1, "Duration (us)");
        if (mFitRequested) {
            ImPlot::SetupAxisLimits(ImAxis_X1, -totalWidth * 0.02, totalWidth * 1.02, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, maxDuration * 1.2, ImGuiCond_Always);
            mFitRequested = false;
        }

        if (ImPlot::BeginItem("Commands")) {
            ImDrawList* pDrawList = ImPlot::GetPlotDrawList();
            ImPlotRect plotLimits = ImPlot::GetPlotLimits();

            int hoveredCmd = -1;

            const auto& selectedPipelineInfo = guiInfo.pipelineInfos[guiInfo.selectedPipeline];

            for (int i = 0; i < totalCmds; ++i) {
                double durationUs = (guiInfo.rangeInfo.cmdInfos[i].endNs - guiInfo.rangeInfo.cmdInfos[i].beginNs) * nsToUs;
                double x0 = barXStarts[i];
                double x1 = x0 + barWidths[i];

                ImU32 barColor = IM_COL32(245, 166, 35, 220);
                const auto& cmdPipeline = guiInfo.pipelineInfos[{ guiInfo.rangeInfo.cmdInfos[i].device, guiInfo.rangeInfo.cmdInfos[i].pipeline }];
                if (cmdPipeline.highlightEnabled) {
                    barColor = ImGui::ColorConvertFloat4ToU32(cmdPipeline.highlightColor);
                }

                // Skip bars outside the visible X range
                if (x1 < plotLimits.X.Min || x0 > plotLimits.X.Max) {
                    continue;
                }

                ImVec2 pxMin = ImPlot::PlotToPixels(x0, durationUs);
                ImVec2 pxMax = ImPlot::PlotToPixels(x1, 0);

                // Ensure minimum 1px width
                if (pxMax.x - pxMin.x < 1.0f) {
                    pxMax.x = pxMin.x + 1.0f;
                }
                // Ensure minimum 1px height
                if (pxMin.y >= pxMax.y - 1.0f) {
                    pxMin.y = pxMax.y - 1.0f;
                }

                // Hover detection
                ImVec2 mousePos = ImGui::GetMousePos();
                bool hovered = mousePos.x >= pxMin.x && mousePos.x <= pxMax.x &&
                               mousePos.y >= pxMin.y && mousePos.y <= pxMax.y;
                if (hovered) {
                    hoveredCmd = i;
                }

                // Highlight all commands from the selected pipeline with a colored background
                if (selectedPipelineInfo.pipeline.get_handle() == guiInfo.rangeInfo.cmdInfos[i].pipeline) {
                    ImU32 highlightColor = ImGui::ColorConvertFloat4ToU32(selectedPipelineInfo.highlightColor);
                    pDrawList->AddRectFilled(pxMin, pxMax, highlightColor);
                }

                // Determine color based on selection and hover state
                bool isSelected = ((uint64_t)i == guiInfo.rangeInfo.selectedCmd);
                bool isActivePipeline = guiInfo.selectedPipeline.get_handle() == guiInfo.rangeInfo.cmdInfos[i].pipeline;
                ImU32 barFillColor;
                if (isSelected) {
                    barFillColor = hovered ? barColorSelectedHovered : barColorSelected;
                } else if (isActivePipeline) {
                    barFillColor = hovered ? barColorActivePipelineHovered : barColorActivePipeline;
                } else {
                    barFillColor = hovered ? barColorHovered : barColor;
                }

                pDrawList->AddRectFilled(pxMin, pxMax, barFillColor);
            }

            ImPlot::EndItem();

            // Handle click to select command or deselect if clicking empty space
            // Only handle if mouse was released without dragging (to avoid triggering on pan/zoom)
            if (ImPlot::IsPlotHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
                bool wasDragging = (dragDelta.x != 0.0f || dragDelta.y != 0.0f);
                if (!wasDragging) {
                    if (hoveredCmd >= 0) {
                        PipelinesWindow::set_selected_pipeline(guiInfo, { guiInfo.rangeInfo.cmdInfos[hoveredCmd].device, guiInfo.rangeInfo.cmdInfos[hoveredCmd].pipeline });
                        guiInfo.rangeInfo.selectedCmd = hoveredCmd;
                    } else {
                        PipelinesWindow::set_selected_pipeline(guiInfo, { });
                    }
                }
            }

            // Tooltip
            if (hoveredCmd >= 0 && ImPlot::IsPlotHovered()) {
                const auto& cmdInfo = guiInfo.rangeInfo.cmdInfos[hoveredCmd];
                double durationUs = (cmdInfo.endNs - cmdInfo.beginNs) * nsToUs;
                ImGui::BeginTooltip();
                ImGui::Text("Cmd #%d", hoveredCmd);
                if (!cmdInfo.content.empty()) {
                    ImGui::Text("%s", cmdInfo.content.c_str());
                } else if (!cmdInfo.label.empty()) {
                    ImGui::Text("%s", cmdInfo.label.c_str());
                }
                ImGui::Text("Duration: %.3f us", durationUs);
                if ((uint64_t)hoveredCmd == guiInfo.rangeInfo.selectedCmd) {
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "(Selected)");
                }
                ImGui::EndTooltip();
            }
        }

        ImPlot::EndPlot();
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

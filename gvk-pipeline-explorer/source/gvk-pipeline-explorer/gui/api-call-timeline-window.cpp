
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

namespace gvk {
namespace pipeline_explorer {
namespace gui {

ApiCallTimelineWindow::ApiCallTimelineWindow(Window::Manager& windowManager)
    : Window(windowManager, "API Call Timeline")
{
}

ApiCallTimelineWindow::~ApiCallTimelineWindow()
{
}

void ApiCallTimelineWindow::on_gui(GuiInfo& guiInfo)
{
    if (guiInfo.apiCallInfo.commandCollection->commandCount) {

        if (!guiInfo.apiCallInfo.populateGpuCallPairs) {
            //populate gpuCallInfos for Tooltips
            for (uint32_t i = 0; i < guiInfo.apiCallInfo.commandDurations.size(); ++i) {
                if (guiInfo.apiCallInfo.commandCollection->ppCommands[i]) {
                    guiInfo.apiCallInfo.gpuCallInfos.push_back(std::make_pair(gvk::get_cname(guiInfo.apiCallInfo.commandCollection->ppCommands[i]->sType), guiInfo.apiCallInfo.commandDurations[i]));
                }
            }

            guiInfo.apiCallInfo.populateGpuCallPairs = true;

        }

        BarChart::PlotInfo plotInfo;
        plotInfo.pLabelName = "GPU Cmd Duration (ns)";
        plotInfo.pYData = guiInfo.apiCallInfo.commandDurations.data();
        plotInfo.yDataCount = guiInfo.apiCallInfo.commandCollection->commandCount;
        plotInfo.gpuCallInfos = guiInfo.apiCallInfo.gpuCallInfos;
        //TODO : Documentation
        mBarChart.chartName = "GPU Cmd Timeline";
        mBarChart.xAxisName = "GPU Cmd";
        mBarChart.yAxisName = "GPU Duration (ns)";
        mBarChart.barWidth = 0.8f;
#if 0
        mBarChart.flagValue |= static_cast<int>(BarChartFlags::NON_CONTINUOUS_SELECTION) |
            static_cast<int>(BarChartFlags::CONTINUOUS_SELECTION) |
            static_cast<int>(BarChartFlags::BORDER) |
            static_cast<int>(BarChartFlags::TOOL_TIPS_FOR_GPU_CALLS) |
            static_cast<int>(BarChartFlags::TIMING_BRACKET);
#else
        mBarChart.flagValue |= static_cast<int>(BarChartFlags::WIP);
#endif

        //Would like to unify selectedBars/selectedCalls, but selectedBars is per chart
        //And api timeline + explorer window do not share a chart class. 
        mBarChart.selectedBars = guiInfo.workspaceInfo.streamInfo.selectedCalls;
        mBarChart.plot_bar_chart(plotInfo);
        guiInfo.workspaceInfo.streamInfo.selectedCalls = mBarChart.selectedBars;
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

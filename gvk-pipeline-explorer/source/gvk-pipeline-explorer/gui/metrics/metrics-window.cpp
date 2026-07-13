
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

#include "gvk-pipeline-explorer/gui/metrics/metrics-window.hpp"

#include <functional>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

MetricsWindow::MetricsWindow(Window::Manager& windowManager)
    : Window(windowManager, "Metrics")
    , mPipelineStatisticsMetricsTab(*this)
    , mPerformanceQueryMetricsTab(*this)
    , mPluginMetricsTab(*this)
{
}

void MetricsWindow::reset()
{
    mPipelineStatisticsMetricsTab.reset();
    mPerformanceQueryMetricsTab.reset();
    mPluginMetricsTab.reset();
}

MetricsTab* MetricsWindow::get_active_metrics_tab()
{
    return mpActiveMetricsTab;
}

void MetricsWindow::on_update(GuiInfo& guiInfo)
{
    // Update metrics tabs
    mPipelineStatisticsMetricsTab.on_update(guiInfo);
    mPerformanceQueryMetricsTab.on_update(guiInfo);
    mPluginMetricsTab.on_update(guiInfo);
}

void MetricsWindow::on_gui(GuiInfo& guiInfo)
{
#if 0
    // Draw report info
    ImGui::Text("Write Metrics Report");
    ImGui::Checkbox("##Report Enabled", &guiInfo.reportEnabled);
    ImGui::SameLine();
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::BeginDisabled();
    auto reportPath = gvk::string::scrub_path((std::filesystem::path(guiInfo.workspace) / "reports").string());
    GvkGui::InputPath("##Report Path", &reportPath);
    ImGui::EndDisabled();
    ImGui::PopItemWidth();

    // Draw warmup range count field
    auto warmupRangeCount = (int)guiInfo.requestInfo.warmupRangeCount;
    ImGui::InputInt("Warmup Range Count", &warmupRangeCount, 1, 4);
    guiInfo.requestInfo.warmupRangeCount = (uint32_t)std::min(std::max(0, warmupRangeCount), 128);

    // Draw query range count field
    auto queryRangeCount = (int)guiInfo.requestInfo.queryRangeCount;
    ImGui::InputInt("Query Range Count", &queryRangeCount, 1, 4);
    guiInfo.requestInfo.queryRangeCount = (uint32_t)std::min(std::max(1, queryRangeCount), 128);
#endif

    // Draw metrics tab bar
    if (ImGui::BeginTabBar("Metrics Collectors Tab Bar")) {
        draw_tab(guiInfo, mPipelineStatisticsMetricsTab);
        draw_tab(guiInfo, mPerformanceQueryMetricsTab);
        #if 0
        draw_tab(guiInfo, mPluginMetricsTab);
        #endif
        ImGui::EndTabBar();
    }
}

void MetricsWindow::draw_tab(GuiInfo& guiInfo, MetricsTab& metricsTab)
{
    if (metricsTab.enabled(guiInfo)) {
        if (ImGui::BeginTabItem(metricsTab.get_name().c_str())) {
            mpActiveMetricsTab = &metricsTab;
            metricsTab.on_gui(guiInfo);
            ImGui::EndTabItem();
        }
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

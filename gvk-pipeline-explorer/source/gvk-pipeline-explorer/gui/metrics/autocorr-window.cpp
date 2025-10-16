
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

#include "gvk-pipeline-explorer/gui/metrics/autocorr-window.hpp"
#include "gvk-pipeline-explorer/gui/metrics/plugin-metrics-tab.hpp"

#include <fstream>
#include <functional>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

gvk::python::Context AutocorrWindow::mPythonContext;

AutocorrWindow::AutocorrWindow(Window::Manager& windowManager, const std::string&, PluginMetricsTab& pluginMetricsTab)
    : Window(windowManager, "Autocorr")
    , mPluginMetricsTab{ pluginMetricsTab }
{
    // Create Python context
    if (!mPythonContext) {
        GvkPythonContextCreateInfo pythonContextCreateInfo{ };
        (void)gvk::python::Context::create(&pythonContextCreateInfo, &mPythonContext);
    }
}

void AutocorrWindow::on_gui(GuiInfo& guiInfo)
{
    (void)guiInfo;
    // Begin request
    ImGui::BeginDisabled(!mRequests.empty() || mRequestResult.pending() || mPluginMetricsTab.get_selected_set() == UINT32_MAX);
    {
        if (ImGui::Button("Begin Autocorr Analysis")) {
#if GVK_MDAPI_ENABLED
            (void)mMetricsDeviceParams;
            (void)mAdapterParams;
#endif
        }
    }
    ImGui::EndDisabled();
    if (mPluginMetricsTab.get_selected_group() != 3) {
        ImGui::Text("Please select an OA-group MDAPI metric set to perform Autocorr Analysis\n (A MDAPI metric group tab other than OA is selected)");
    } else if (mPluginMetricsTab.get_selected_set() == UINT32_MAX) {
        ImGui::Text("Please select an OA-group MDAPI metric set to perform Autocorr Analysis\n (No set is currently selected)");
    }

    //Imgui Autocorr presentation
#if 0
    //Text-based presentation of result
    if (!mOutput.stage_info.empty()) {
        ImGui::Text("\nAutocorr Analysis Results:\n\n");
        for (int i = 0; i < mOutput.stage_info.size(); i++) {
            ImGui::Separator();
            std::string name = mOutput.stage_info[i].is_bottleneck ? "\nBottleneck - " : "";
            name = name + mOutput.stage_info[i].name + ": ";
            ImGui::TextWrapped(name.c_str());
            ImGui::TextWrapped(mOutput.stage_info[i].description.c_str());
            if (!mOutput.stage_info[i].metric_names.empty()) {
                ImGui::Text("\nRelevant Metrics:");
            }
            for (int metricIndex = 0; metricIndex < mOutput.stage_info[i].metric_names.size(); metricIndex++) {
                std::string metric = " " + mOutput.stage_info[i].metric_names[metricIndex];
                ImGui::Text(metric.c_str());
                if (metricIndex < mOutput.stage_info[i].metric_names.size() - 1) ImGui::Text(", ");
            }
            ImGui::Text(mOutput.stage_info[i].is_asserted ? "\nThis was asserted" : "\nNot asserted");
            ImGui::Text("\n");
        }
    }
#else
    //Table presentation of results
    auto tableFlags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
    ImGui::BeginTable("Signature Output", 5, tableFlags);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Description");
    ImGui::TableSetupColumn("Metric Names");
    ImGui::TableSetupColumn("Asserted Status", ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Bottleneck Status", ImGuiTableColumnFlags_DefaultHide);
    //ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();
    ImGui::EndTable();
#endif
}

std::string AutocorrWindow::detect_similar_metrics(std::string expectedMetricName)
{
    std::vector<std::string> splitMetricName = gvk::string::split_snake_case(expectedMetricName);
    auto lastBlock = !splitMetricName.empty() ? splitMetricName.back() : "";
    bool aggregation = lastBlock == "SLICE1" || lastBlock == "XECORE1" || lastBlock == "L3NODE0" || lastBlock == "UTILIZATION";
    if(aggregation || (lastBlock.find("SQIDI") != std::string::npos || lastBlock.find("L3BANK") != std::string::npos)) {
        std::ostringstream out;
        if (1 < splitMetricName.size()) {
            auto end(std::prev(splitMetricName.end()));
            std::copy(splitMetricName.begin(), --end, std::ostream_iterator<std::string>(out, "_"));
            out << *end;
            return out.str();
        }
    }
    //We don't have logic to convert this metric into a recongizable one.
    return expectedMetricName;
}

template <size_t UUID_SIZE>
static void set_uuid_indices(uint8_t uuid[UUID_SIZE], uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
    static_assert(sizeof(uint32_t) * 4 <= UUID_SIZE);
    auto pCounterIndices = (uint32_t*)uuid;
    pCounterIndices[0] = x;
    pCounterIndices[1] = y;
    pCounterIndices[2] = z;
    pCounterIndices[3] = w;
}

gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> AutocorrWindow::create_request(GuiInfo& guiInfo, uint32_t group_i, uint32_t set_i)
{
    auto counter = gvk::get_default<VkPerformanceCounterKHR>();
    set_uuid_indices<VK_UUID_SIZE>(counter.uuid, group_i, set_i, 0, 0);
    auto requestInfo = gvk::get_default<GvkPipelineExplorerPerformanceQueryRequestInfo>();
    std::string reportPath = guiInfo.reportEnabled ? (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string() : std::string();
    requestInfo.pReportPath = !reportPath.empty() ? reportPath.c_str() : nullptr;
    requestInfo.device = guiInfo.selectedPipeline.get_dispatchable_handle();
    requestInfo.pipeline = guiInfo.selectedPipeline.get_handle();
    requestInfo.warmupRangeCount = guiInfo.requestInfo.warmupRangeCount;
    requestInfo.queryRangeCount = guiInfo.requestInfo.queryRangeCount;
    requestInfo.counterCount = 1;
    requestInfo.pCounters = &counter;
    return requestInfo;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

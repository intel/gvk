
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

#include "gvk-pipeline-explorer/gui/metrics/metrics-tab.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

MetricsTab::MetricsTab()
{
}

MetricsTab::~MetricsTab()
{
}
bool MetricsTab::idle(GuiInfo& guiInfo) const
{
    (void)guiInfo;
    return true;
}

void MetricsTab::submit_metrics_query_request(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

bool MetricsTab::enabled(GuiInfo& guiInfo) const
{
    (void)guiInfo;
    return false;
}

const std::string& MetricsTab::get_name() const
{
    return mName;
}

void MetricsTab::on_update(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void MetricsTab::on_gui(GuiInfo& guiInfo)
{
    // Draw category selector if multiple categories are available
    if (!mCategories.empty()) {
        bool applyCategoryFilter = false;
        if (ImGui::Button("Categories")) {
            ImGui::OpenPopup("Categories");
        }
        if (ImGui::BeginPopup("Categories")) {
            if (ImGui::Button("+")) {
                applyCategoryFilter = true;
                for (auto& categoryItr : mCategories) {
                    categoryItr.second = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("-")) {
                applyCategoryFilter = true;
                for (auto& categoryItr : mCategories) {
                    categoryItr.second = false;
                }
            }
            ImGui::SameLine();
            ImGui::Text("All");
            for (auto& categoryItr : mCategories) {
                if (ImGui::Checkbox(categoryItr.first.c_str(), &categoryItr.second)) {
                    applyCategoryFilter = true;
                }
            }
            ImGui::EndPopup();
        }
        if (applyCategoryFilter) {
            filter_counters(guiInfo);
            sort_counters(guiInfo);
        }
    }

    // Draw query metrics button
    ImGui::BeginDisabled(!guiInfo.selectedPipeline.get_handle());
    {
        ImGui::BeginDisabled(!idle(guiInfo));
        {
            if (ImGui::Button("Query Metrics")) {
                submit_metrics_query_request(guiInfo);
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndDisabled();
}

void MetricsTab::filter_counters(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void MetricsTab::sort_counters(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

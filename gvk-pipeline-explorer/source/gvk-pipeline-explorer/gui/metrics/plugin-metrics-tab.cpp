
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

#include "gvk-pipeline-explorer/gui/metrics/plugin-metrics-tab.hpp"
#include "gvk-pipeline-explorer/gui/metrics/autocorr-window.hpp"
#include "gvk-pipeline-explorer/gui/metrics/metrics-window.hpp"
#include "gvk-pipeline-explorer/gui/window-manager.hpp"

#include <sstream>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

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

template <size_t UUID_SIZE>
static void get_uuid_indices(const uint8_t uuid[UUID_SIZE], uint32_t* pX, uint32_t* pY, uint32_t* pZ, uint32_t* pW)
{
    static_assert(sizeof(uint32_t) * 4 <= UUID_SIZE);
    auto pCounterIndices = (const uint32_t*)uuid;
    if (pX) {
        *pX = pCounterIndices[0];
    }
    if (pY) {
        *pY = pCounterIndices[1];
    }
    if (pZ) {
        *pZ = pCounterIndices[2];
    }
    if (pW) {
        *pW = pCounterIndices[3];
    }
}

PluginMetricsTab::PluginMetricsTab(MetricsWindow& metricsWindow)
    : MetricsTab(metricsWindow)
{
    mName = "Intel MDAPI";
}

bool PluginMetricsTab::idle(GuiInfo& guiInfo) const
{
    (void)guiInfo;
    return !mRequestResult.pending();
}

bool PluginMetricsTab::enabled(GuiInfo& guiInfo) const
{
    return guiInfo.pluginPerformanceCounterInfo.available->groupCount;
}

uint32_t PluginMetricsTab::get_selected_group()
{
    return mSelectedGroup;
}

uint32_t PluginMetricsTab::get_selected_set()
{ 
    auto result = mSelectedSets.find(mSelectedGroup);
    if (result != mSelectedSets.end()) {
        return mSelectedSets.find(mSelectedGroup)->second;
    }
    //return UINT32_MAX if no selection
    return UINT32_MAX;
}

void PluginMetricsTab::submit_metrics_query_request(GuiInfo& guiInfo)
{
    auto selectedSetItr = mSelectedSets.find(mSelectedGroup);
    if (selectedSetItr != mSelectedSets.end()) {
        auto counter = gvk::get_default<VkPerformanceCounterKHR>();

        set_uuid_indices<VK_UUID_SIZE>(counter.uuid, mSelectedGroup, selectedSetItr->second, 0, 0);

        auto queryRequestInfo = gvk::get_default<GvkPipelineExplorerPerformanceQueryRequestInfo>();
        std::string reportPath = guiInfo.reportEnabled ? (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string() : std::string();
        queryRequestInfo.pReportPath = !reportPath.empty() ? reportPath.c_str() : nullptr;
        queryRequestInfo.device = guiInfo.selectedPipeline.get_dispatchable_handle();
        queryRequestInfo.pipeline = guiInfo.selectedPipeline.get_handle();
        queryRequestInfo.warmupRangeCount = guiInfo.requestInfo.warmupRangeCount;
        queryRequestInfo.queryRangeCount = guiInfo.requestInfo.queryRangeCount;
        queryRequestInfo.counterCount = 1;
        queryRequestInfo.pCounters = &counter;
        (void)mRequestResult.submit_request(guiInfo.workspaceInfo.workspace, queryRequestInfo, "MDAPI_REQUEST", "MDAPI_RESULT");
    }
}

#define DEBUG_QUERY_RESULTS 0
void PluginMetricsTab::on_update(GuiInfo& guiInfo)
{
    // TODO : Documentation
    gvk::Auto<GvkPipelineExplorerPluginCounterInfo> pluginCounterInfo;
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", pluginCounterInfo)) {
    case VK_SUCCESS: {
        guiInfo.pluginPerformanceCounterInfo.available = pluginCounterInfo;

        #if 0
        std::cout << "GvkPipelineExplorerPluginCounterInfo" << std::endl;
        std::cout << gvk::to_string(guiInfo.pluginPerformanceCounterInfo.available, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        std::ofstream counterFile(guiInfo.workspaceInfo.workspace + "/GvkPipelineExplorerPluginCounterInfoEx.json");
        counterFile << gvk::to_string(guiInfo.pluginPerformanceCounterInfo.available, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        #endif

        mCategories.clear();
        for (uint32_t group_i = 0; group_i < guiInfo.pluginPerformanceCounterInfo.available->groupCount; ++group_i) {
            const auto& group = guiInfo.pluginPerformanceCounterInfo.available->pGroups[group_i];
            for (uint32_t set_i = 0; set_i < group.setCount; ++set_i) {
                const auto& set = group.pSets[set_i];
                for (uint32_t metric_i = 0; metric_i < set.counterCount; ++metric_i) {
                    const auto& description = set.pDescriptions[metric_i];
                    std::string category = description.category;
                    if (!category.empty()) {
                        mCategories[category] = true;
                    }
                }
            }
        }

        filter_counters(guiInfo);
        sort_counters(guiInfo);
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

    // Check request/result
    if (mRequestResult.pending()) {
        mRequestResult.check_result(guiInfo.workspaceInfo.workspace);
    }

    // Process request/result
    if (mRequestResult.ready()) {
        const auto& result = mRequestResult.get_result();
#if DEBUG_QUERY_RESULTS
        std::cout << "================================================================================" << std::endl;
        std::cout << "Result" << std::endl;
        std::cout << gvk::to_string(result, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        std::cout << "================================================================================" << std::endl;
#endif // DEBUG_QUERY_RESULTS

        uint32_t group_i = 0;
        uint32_t set_i = 0;
        assert(result->groupResultCount == 1);
        for (uint32_t groupResult_i = 0; groupResult_i < result->groupResultCount; ++groupResult_i) {
            const auto& groupResult = result->pGroupResults[groupResult_i];

            for (uint32_t counterResult_i = 0; counterResult_i < groupResult.counterResultCount; ++counterResult_i) {
                const auto& counterResult = groupResult.pCounterResults[counterResult_i];
                uint32_t counter_i = 0;

                if (!groupResult_i && !counterResult_i) {
                    get_uuid_indices<VK_UUID_SIZE>(counterResult.counter.uuid, &group_i, &set_i, &counter_i, nullptr);
                }
                uint32_t group_i_ex = 0;
                uint32_t set_i_ex = 0;
                get_uuid_indices<VK_UUID_SIZE>(counterResult.counter.uuid, &group_i_ex, &set_i_ex, &counter_i, nullptr);
                assert(group_i == group_i_ex);
                assert(set_i == set_i_ex);
                assert(counter_i == counterResult_i);
                auto& results = mResults[{ result->pipelineInfo.device, result->pipelineInfo.pipeline }][group_i][set_i][counter_i];
                results.total = counterResult.total;
                results.average = counterResult.average;
            }
        }
        mRequestResult.reset();
    }
}


void PluginMetricsTab::on_gui(GuiInfo& guiInfo)
{
    // TODO : Documentation
    MetricsTab::on_gui(guiInfo);

    // TODO : Closing app while this is open causes crash... 
    if (ImGui::Button("Autocorr analysis")) {
        get_metrics_window().get_window_manager().open<AutocorrWindow>("Autocorr", * this);
    }

    // TODO : Documentation
    if (ImGui::BeginChild("##Draw-Gui")) {
        draw_gui(guiInfo);
    }
    ImGui::EndChild();
}

void PluginMetricsTab::filter_counters(GuiInfo& guiInfo)
{
    mFiltered.clear();
    for (uint32_t group_i = 0; group_i < guiInfo.pluginPerformanceCounterInfo.available->groupCount; ++group_i) {
        const auto& group = guiInfo.pluginPerformanceCounterInfo.available->pGroups[group_i];
        for (uint32_t set_i = 0; set_i < group.setCount; ++set_i) {
            const auto& set = group.pSets[set_i];
            for (uint32_t metric_i = 0; metric_i < set.counterCount; ++metric_i) {
                const auto& description = set.pDescriptions[metric_i];
                if (mCategories[description.category]) {
                    mFiltered[group_i].insert(set_i);
                    break;
                }
            }
        }
    }

    // HACK :
    // TODO : Reorganize metrics so Ex collection isn't necessary
    mFilteredEx.clear();
    for (const auto& groupItr : mFiltered) {
        auto& set = mFilteredEx[groupItr.first];
        set.reserve(groupItr.second.size());
        for (const auto& set_i : groupItr.second) {
            set.push_back(set_i);
        }
    }
}

void PluginMetricsTab::draw_gui(GuiInfo& guiInfo)
{
    // TODO : Documentation
    mSelectedGroup = 0;
    if (ImGui::BeginTabBar("Metrics Groups Tab Bar")) {
        for (const auto& groupItr : mFiltered) {
            auto group_i = groupItr.first;
            assert(group_i < guiInfo.pluginPerformanceCounterInfo.available->groupCount);
            const auto& group = guiInfo.pluginPerformanceCounterInfo.available->pGroups[group_i];
            if (ImGui::BeginTabItem(group.pName)) {
                GvkGui::ScopeID groupId(group_i);
                mSelectedGroup = group_i;

                if (ImGui::BeginChild("Metrics Group", { ImGui::GetWindowWidth() * 0.2f, 0 })) {
                    draw_metric_group(groupItr, guiInfo);
                } ImGui::EndChild();

                ImGui::SameLine();

                if (ImGui::BeginChild("Metrics Set")) {
                    draw_metric_set(groupItr, guiInfo);
                } ImGui::EndChild();

                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void PluginMetricsTab::draw_metric_group(const std::pair<uint32_t, std::set<uint32_t>>& groupItr, GuiInfo& guiInfo)
{
    //Get group
    auto group_i = groupItr.first;
    assert(group_i < guiInfo.pluginPerformanceCounterInfo.available->groupCount);
    const auto& group = guiInfo.pluginPerformanceCounterInfo.available->pGroups[group_i];

    // HACK :
    auto groupEx = mFilteredEx[group_i];

    // Draw set selection
    ImGuiListClipper clipper;
    clipper.Begin((int)groupEx.size());
    while (clipper.Step()) {
        for (int clipper_i = clipper.DisplayStart; clipper_i < clipper.DisplayEnd; ++clipper_i) {
            assert((size_t)clipper_i < groupEx.size());
            auto set_i = groupEx[clipper_i];
            assert(set_i < group.setCount);
            const auto& set = group.pSets[set_i];

            auto selectedSetItr = mSelectedSets.find(group_i);
            auto selected = selectedSetItr != mSelectedSets.end() && selectedSetItr->second == set_i;
            if (ImGui::Checkbox(set.pName, &selected)) {
                if (selected) {
                    mSelectedSets[group_i] = set_i;
                } else {
                    mSelectedSets.erase(group_i);
                }
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                std::stringstream strStrm;
                for (uint32_t counter_i = 0; counter_i < set.counterCount; ++counter_i) {
                    const auto& description = set.pDescriptions[counter_i];
                    strStrm << description.name << " : " << description.description << std::endl;
                }
                ImGui::SetTooltip("%s", strStrm.str().c_str());
            }
        }
    }
}

void PluginMetricsTab::draw_metric_set(const std::pair<uint32_t, std::set<uint32_t>>& groupItr, GuiInfo& guiInfo)
{
    auto tableFlags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("Performance Counters", 6, tableFlags)) {
        ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_DefaultHide);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Category");
        ImGui::TableSetupColumn("Unit");
        ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_DefaultHide);
        ImGui::TableSetupColumn("Average");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Drill into group and set for individual metrics
        auto selectedPipelineInfo = guiInfo.pipelineInfos[guiInfo.selectedPipeline];

        // TODO : Documentation
        auto group_i = groupItr.first;
        assert(group_i < guiInfo.pluginPerformanceCounterInfo.available->groupCount);
        const auto& group = guiInfo.pluginPerformanceCounterInfo.available->pGroups[group_i];
        auto selectedSetItr = mSelectedSets.find(group_i);
        if (selectedSetItr != mSelectedSets.end()) {
            auto set_i = selectedSetItr->second;
            assert(set_i < group.setCount);
            const auto& set = group.pSets[set_i];
            for (uint32_t counter_i = 0; counter_i < set.counterCount; ++counter_i) {
                GvkGui::ScopeID counterID(group_i + set_i + counter_i);
                const auto& counter = set.pCounters[counter_i];
                (void)counter;
                const auto& description = set.pDescriptions[counter_i];

                // Counter info
                ImGui::TableNextColumn();
                ImGui::Text("%s", uuid_to_string(counter.uuid).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", description.name);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("%s", description.description);
                }
                ImGui::TableNextColumn();
                ImGui::Text("%s", description.category);

                // Counter unit
                ImGui::TableNextColumn();
                switch (counter.unit) {
                case VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR: { ImGui::Text("%s", "Generic"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_PERCENTAGE_KHR: { ImGui::Text("%s", "Percentage"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_NANOSECONDS_KHR: { ImGui::Text("%s", "Nanoseconds"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_BYTES_KHR: { ImGui::Text("%s", "Bytes"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_BYTES_PER_SECOND_KHR: { ImGui::Text("%s", "Bytes/Second"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_KELVIN_KHR: { ImGui::Text("%s", "Kelvin"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_WATTS_KHR: { ImGui::Text("%s", "Watts"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_VOLTS_KHR: { ImGui::Text("%s", "Volts"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_AMPS_KHR: { ImGui::Text("%s", "Amps"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_HERTZ_KHR: { ImGui::Text("%s", "Hertz"); } break;
                case VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR: { ImGui::Text("%s", "Cycles"); } break;
                default: { assert(false && "VK_ERROR_FEATURE_NOT_PRESENT"); } break;
                }

                // Counter values
                if (selectedPipelineInfo.pipeline) {
                    std::array<uint8_t, VK_UUID_SIZE> uuid{ };
                    assert(sizeof(uuid) == sizeof(counter.uuid));
                    memcpy(uuid.data(), counter.uuid, sizeof(uuid));
                    const auto& resultInfo = mResults[selectedPipelineInfo.pipeline][group_i][set_i][counter_i];
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::to_string(resultInfo.total).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::to_string(resultInfo.average).c_str());
                } else {
                    ImGui::TableNextColumn();
                    ImGui::Text("0.0");
                    ImGui::TableNextColumn();
                    ImGui::Text("0.0");
                }
            }
        }
        ImGui::EndTable();
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

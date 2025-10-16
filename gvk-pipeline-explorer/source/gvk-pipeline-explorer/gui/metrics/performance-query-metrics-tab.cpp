
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

#include "gvk-pipeline-explorer/gui/metrics/performance-query-metrics-tab.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

PerformanceQueryMetricsTab::PerformanceQueryMetricsTab(MetricsWindow& metricsWindow)
    : MetricsTab(metricsWindow)
{
    mName = "VK_KHR_performance_query";
}

bool PerformanceQueryMetricsTab::idle(GuiInfo& guiInfo) const
{
    (void)guiInfo;
    return true;
}

bool PerformanceQueryMetricsTab::enabled(GuiInfo& guiInfo) const
{
    return guiInfo.performanceCountersInfo.available->count;
}

template <typename T>
static int sort_compare(const T& lhs, const T& rhs)
{
    return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0);
}

static void sort_performance_counters(GuiInfo& guiInfo)
{
    std::sort(
        guiInfo.performanceCountersInfo.active.begin(),
        guiInfo.performanceCountersInfo.active.end(),
        [&](uint32_t lhs, uint32_t rhs)
        {
            bool lhsEnabled = guiInfo.performanceCountersInfo.enabled[lhs];
            const auto& lhsCounter = guiInfo.performanceCountersInfo.available->pCounters[lhs];
            const auto& lhsDescription = guiInfo.performanceCountersInfo.available->pDescriptions[lhs];
            UUID lhsUuid{ };
            boost::multiprecision::import_bits(lhsUuid, lhsCounter.uuid, lhsCounter.uuid + VK_UUID_SIZE);

            bool rhsEnabled = guiInfo.performanceCountersInfo.enabled[rhs];
            const auto& rhsCounter = guiInfo.performanceCountersInfo.available->pCounters[rhs];
            const auto& rhsDescription = guiInfo.performanceCountersInfo.available->pDescriptions[rhs];
            UUID rhsUuid{ };
            boost::multiprecision::import_bits(rhsUuid, rhsCounter.uuid, rhsCounter.uuid + VK_UUID_SIZE);

            for (const auto& sortSpec : guiInfo.performanceCountersInfo.sortSpecs) {
                int value = 0;
                switch (sortSpec.ColumnUserID) {
                case 0: { value = sort_compare(lhsEnabled, rhsEnabled); } break;
                case 1: { value = sort_compare(lhs, rhs); } break;
                case 2: { value = sort_compare(lhsUuid, rhsUuid); } break;
                case 3: { value = sort_compare(std::string(lhsDescription.name), std::string(rhsDescription.name)); } break;
                case 4: { value = sort_compare(std::string(lhsDescription.category), std::string(rhsDescription.category)); } break;
                case 5: { value = sort_compare(lhsCounter.scope, rhsCounter.scope); } break;
                case 6: { value = sort_compare(lhsCounter.unit, rhsCounter.unit); } break;
                case 7: { value = sort_compare(0, 0); } break;
                case 8: { value = sort_compare(0, 0); } break;
                default: {
                } break;
                }
                if (value) {
                    return (sortSpec.SortDirection == ImGuiSortDirection_Ascending ? value : -value) < 0;
                }
            }
            return lhs < rhs;
        }
    );
}

static void filter_performance_counters(GuiInfo& guiInfo)
{
    guiInfo.performanceCountersInfo.active.clear();
    for (uint32_t counter_i = 0; counter_i < guiInfo.performanceCountersInfo.available->count; ++counter_i) {
        if (guiInfo.performanceCountersInfo.scopes[guiInfo.performanceCountersInfo.available->pCounters[counter_i].scope] &&
            guiInfo.performanceCountersInfo.categories[guiInfo.performanceCountersInfo.available->pDescriptions[counter_i].category]) {
            guiInfo.performanceCountersInfo.active.push_back(counter_i);
        }
    }
    if (!guiInfo.performanceCountersInfo.anyOfFilter.empty() || !guiInfo.performanceCountersInfo.allOfFilter.empty()) {
        const auto& anyFilterTokens = gvk::string::split(guiInfo.performanceCountersInfo.anyOfFilter, ";");
        const auto& allFilterTokens = gvk::string::split(guiInfo.performanceCountersInfo.allOfFilter, ";");
        for (const auto& filterItr : guiInfo.performanceCountersInfo.filters) {
            bool anyFound = false;
            for (const auto& anyFilterToken : anyFilterTokens) {
                std::boyer_moore_searcher searcher(anyFilterToken.begin(), anyFilterToken.end());
                if (std::search(filterItr.first.begin(), filterItr.first.end(), searcher) != filterItr.first.end()) {
                    anyFound = true;
                    break;
                }
            }
            bool allFound = false;
            if (!anyFound) {
                for (const auto& allFilterToken : allFilterTokens) {
                    std::boyer_moore_searcher searcher(allFilterToken.begin(), allFilterToken.end());
                    if (std::search(filterItr.first.begin(), filterItr.first.end(), searcher) == filterItr.first.end()) {
                        allFound = false;
                        break;
                    }
                    allFound = true;
                }
            }
            if (anyFound || allFound) {
                guiInfo.performanceCountersInfo.active.push_back(filterItr.second);
            }
        }
    }
}

void PerformanceQueryMetricsTab::on_update(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void PerformanceQueryMetricsTab::on_gui(GuiInfo& guiInfo)
{
    #if 0
    // TODO :
    auto applyFilter = ImGui::Button("Apply Filter");
    ImGui::SameLine();
    if (ImGui::Button("Clear Filter")) {
        guiInfo.performanceCountersInfo.anyOfFilter.clear();
        guiInfo.performanceCountersInfo.allOfFilter.clear();
        applyFilter = true;
    }
    ImGui::SameLine();
    ImGui::Text("Case sensitive, ';' delimited");
    ImGui::InputText("Any Of", &guiInfo.performanceCountersInfo.anyOfFilter);
    ImGui::InputText("All Of", &guiInfo.performanceCountersInfo.allOfFilter);
    #else
    bool applyFilter = false;
    #endif

    // Draw scopes filter
    if (ImGui::Button("Scopes")) {
        ImGui::OpenPopup("Scopes");
    }
    if (ImGui::BeginPopup("Scopes")) {
        if (ImGui::Button("+")) {
            applyFilter = true;
            for (auto& scopeItr : guiInfo.performanceCountersInfo.scopes) {
                scopeItr.second = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            applyFilter = true;
            for (auto& scopeItr : guiInfo.performanceCountersInfo.scopes) {
                scopeItr.second = false;
            }
        }
        ImGui::SameLine();
        ImGui::Text("All");
        for (auto& scopeItr : guiInfo.performanceCountersInfo.scopes) {
            auto pScopeCheckboxLabel = "Unknown";
            switch (scopeItr.first) {
            case VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR: { pScopeCheckboxLabel = "Command Buffer"; } break;
            case VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR: { pScopeCheckboxLabel = "Render Pass"; } break;
            case VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR: { pScopeCheckboxLabel = "Command"; } break;
            default: { assert(false && "VK_ERROR_FEATURE_NOT_PRESENT"); } break;
            }
            if (ImGui::Checkbox(pScopeCheckboxLabel, &scopeItr.second)) {
                applyFilter = true;
            }
        }
        ImGui::EndPopup();
    }

    // Draw categories filter
    ImGui::SameLine();
    if (ImGui::Button("Categories")) {
        ImGui::OpenPopup("Categories");
    }
    if (ImGui::BeginPopup("Categories")) {
        if (ImGui::Button("+")) {
            applyFilter = true;
            for (auto& categoryItr : guiInfo.performanceCountersInfo.categories) {
                categoryItr.second = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            applyFilter = true;
            for (auto& categoryItr : guiInfo.performanceCountersInfo.categories) {
                categoryItr.second = false;
            }
        }
        ImGui::SameLine();
        ImGui::Text("All");
        for (auto& categoryItr : guiInfo.performanceCountersInfo.categories) {
            if (ImGui::Checkbox(categoryItr.first.c_str(), &categoryItr.second)) {
                applyFilter = true;
            }
        }
        ImGui::EndPopup();
    }

    // Apply filter
    if (applyFilter) {
        filter_performance_counters(guiInfo);
        sort_performance_counters(guiInfo);
    }

    // Draw counter info
    ImGui::Separator();
    ImGui::Separator();
    ImGui::Text("Performance Counters");
    ImGui::Text("Available : %zu", (size_t)guiInfo.performanceCountersInfo.available->count);
    ImGui::Text("Filtered  : %zu", guiInfo.performanceCountersInfo.active.size());
    ImGui::Text("Selected  : %zu", (size_t)guiInfo.performanceCountersInfo.activeEnabledCount);

    // Check request/result
    if (guiInfo.performanceCountersInfo.requestResult.pending()) {
        guiInfo.performanceCountersInfo.requestResult.check_result(guiInfo.workspaceInfo.workspace);
    }

    // Process request/result
    if (guiInfo.performanceCountersInfo.requestResult.ready()) {
        auto result = guiInfo.performanceCountersInfo.requestResult.get_result();
        auto& pipelineInfo = guiInfo.pipelineInfos[{ result->pipelineInfo.device, result->pipelineInfo.pipeline }];
        if (pipelineInfo.pipeline) {
            for (uint32_t goupResult_i = 0; goupResult_i < result->groupResultCount; ++goupResult_i) {
                const auto& groupResult = result->pGroupResults[goupResult_i];
                for (uint32_t counterResult_i = 0; counterResult_i < groupResult.counterResultCount; ++counterResult_i) {
                    const auto& counterResult = groupResult.pCounterResults[counterResult_i];
                    std::array<uint8_t, VK_UUID_SIZE> uuid{ };
                    assert(sizeof(uuid) == sizeof(counterResult.counter.uuid));
                    memcpy(uuid.data(), counterResult.counter.uuid, sizeof(uuid));
                    pipelineInfo.performanceCounterResults[uuid].total = counterResult.total;
                    pipelineInfo.performanceCounterResults[uuid].average = counterResult.average;
                }
            }
        }
        guiInfo.performanceCountersInfo.requestResult.reset();
    }

    // Draw query button
    ImGui::BeginDisabled(
        !guiInfo.selectedPipeline.get_handle() ||
        !guiInfo.performanceCountersInfo.activeEnabledCount ||
        guiInfo.performanceCountersInfo.requestResult.pending()
    );
    {
        if (ImGui::Button("Query Metrics")) {
            std::vector<VkPerformanceCounterKHR> performanceCounters;
            performanceCounters.reserve(guiInfo.performanceCountersInfo.activeEnabledCount);
            for (const auto& counter_i : guiInfo.performanceCountersInfo.active) {
                if (guiInfo.performanceCountersInfo.enabled[counter_i]) {
                    performanceCounters.push_back(guiInfo.performanceCountersInfo.available->pCounters[counter_i]);
                }
            }
            auto performanceQueryRequestInfo = gvk::get_default<GvkPipelineExplorerPerformanceQueryRequestInfo>();
            std::string reportPath = guiInfo.reportEnabled ? (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string() : std::string();
            performanceQueryRequestInfo.pReportPath = !reportPath.empty() ? reportPath.c_str() : nullptr;
            performanceQueryRequestInfo.device = guiInfo.selectedPipeline.get_dispatchable_handle();
            performanceQueryRequestInfo.pipeline = guiInfo.selectedPipeline.get_handle();
            performanceQueryRequestInfo.warmupRangeCount = guiInfo.requestInfo.warmupRangeCount;
            performanceQueryRequestInfo.queryRangeCount = guiInfo.requestInfo.queryRangeCount;
            performanceQueryRequestInfo.counterCount = (uint32_t)performanceCounters.size();
            performanceQueryRequestInfo.pCounters = !performanceCounters.empty() ? performanceCounters.data() : nullptr;
            (void)guiInfo.performanceCountersInfo.requestResult.submit_request(guiInfo.workspaceInfo.workspace, performanceQueryRequestInfo);
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!guiInfo.performanceCountersInfo.requestResult.pending());
    if (ImGui::Button("Force Reset Query")) {
        guiInfo.performanceCountersInfo.requestResult.reset();
    }
    ImGui::EndDisabled();

    // Draw deselect all button
    ImGui::BeginDisabled(!guiInfo.performanceCountersInfo.activeEnabledCount);
    if (ImGui::Button("Deselect All")) {
        for (uint32_t i = 0; i < guiInfo.performanceCountersInfo.available->count; ++i) {
            guiInfo.performanceCountersInfo.enabled[i] = false;
        }
    }
    ImGui::EndDisabled();

    // Draw table
    auto tableFlags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
    if (ImGui::BeginTable("Performance Counters", 9, tableFlags)) {
        auto lockedColumnFlags =
            ImGuiTableColumnFlags_NoHeaderLabel |
            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize |
            ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoClip;
        ImGui::TableSetupColumn("Selected", lockedColumnFlags | ImGuiTableColumnFlags_NoHide, 0, 0);
        ImGui::TableSetupColumn("Index",    ImGuiTableColumnFlags_NoHeaderLabel,              0, 1);
        ImGui::TableSetupColumn("UUID",     ImGuiTableColumnFlags_DefaultHide,                0, 2);
        ImGui::TableSetupColumn("Name",     0,                                                0, 3);
        ImGui::TableSetupColumn("Category", 0,                                                0, 4);
        ImGui::TableSetupColumn("Scope",    ImGuiTableColumnFlags_DefaultHide,                0, 5);
        ImGui::TableSetupColumn("Unit",     0,                                                0, 6);
        ImGui::TableSetupColumn("Total",    ImGuiTableColumnFlags_DefaultHide,                0, 7);
        ImGui::TableSetupColumn("Average",  0,                                                0, 8);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Sort table
        auto pTableSortSpecs = ImGui::TableGetSortSpecs();
        if (pTableSortSpecs && pTableSortSpecs->SpecsDirty) {
            pTableSortSpecs->SpecsDirty = false;
            guiInfo.performanceCountersInfo.sortSpecs.clear();
            guiInfo.performanceCountersInfo.sortSpecs.insert(guiInfo.performanceCountersInfo.sortSpecs.end(), pTableSortSpecs->Specs, pTableSortSpecs->Specs + pTableSortSpecs->SpecsCount);
            sort_performance_counters(guiInfo);
        }

        // Count active/enabled counters
        guiInfo.performanceCountersInfo.activeEnabledCount = 0;
        for (uint32_t i = 0; i < guiInfo.performanceCountersInfo.active.size(); ++i) {
            auto counter_i = guiInfo.performanceCountersInfo.active[i];
            if (guiInfo.performanceCountersInfo.enabled[counter_i]) {
                ++guiInfo.performanceCountersInfo.activeEnabledCount;
            }
        }

        // Draw counters
        ImGuiListClipper clipper;
        clipper.Begin((int)guiInfo.performanceCountersInfo.active.size());
        auto selectedPipelineInfo = guiInfo.pipelineInfos[guiInfo.selectedPipeline];
        while (clipper.Step()) {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n) {
                auto counter_i = guiInfo.performanceCountersInfo.active[row_n];
                const auto& counter = guiInfo.performanceCountersInfo.available->pCounters[counter_i];
                const auto& description = guiInfo.performanceCountersInfo.available->pDescriptions[counter_i];
                ImGui::PushID(counter_i);
                ImGui::TableNextRow();

                // Make row selectable and draw selection checkbox
                ImGui::TableNextColumn();
                bool enabled = guiInfo.performanceCountersInfo.enabled[counter_i];
                ImGui::BeginDisabled(guiInfo.performanceCountersInfo.requestResult.pending());
                ImGui::Selectable("##selected-selectable", &enabled, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::SameLine();
                ImGui::Checkbox("##selected-checkbox", &enabled);
                ImGui::EndDisabled();
                guiInfo.performanceCountersInfo.enabled[counter_i] = enabled;

                // Counter info
                ImGui::TableNextColumn();
                ImGui::Text("%s", std::to_string(counter_i).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", uuid_to_string(counter.uuid).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", description.name);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("%s", description.description);
                }
                ImGui::TableNextColumn();
                ImGui::Text("%s", description.category);

                // Counter scope
                ImGui::TableNextColumn();
                switch (counter.scope) {
                case VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR: { ImGui::Text("%s", "Command Buffer"); } break;
                case VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR: { ImGui::Text("%s", "Render Pass"); } break;
                case VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR: { ImGui::Text("%s", "Command"); } break;
                default: { assert(false && "VK_ERROR_FEATURE_NOT_PRESENT"); } break;
                }

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

                // Counter results
                if (selectedPipelineInfo.pipeline) {
                    std::array<uint8_t, VK_UUID_SIZE> uuid{ };
                    assert(sizeof(uuid) == sizeof(counter.uuid));
                    memcpy(uuid.data(), counter.uuid, sizeof(uuid));
                    const auto& resultInfo = selectedPipelineInfo.performanceCounterResults[uuid];
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
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk


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

void PipelinesWindow::set_selected_pipeline(GuiInfo& guiInfo, const gvk::HandleId<VkDevice, VkPipeline>& pipeline)
{
    guiInfo.selectedPipeline = pipeline;
    guiInfo.rangeInfo.selectedCmd = std::numeric_limits<uint64_t>::max();
}

void PipelinesWindow::update_experiment(GuiInfo& guiInfo, const PipelineInfo& pipelineInfo)
{
    (void)guiInfo;
    (void)pipelineInfo;
#ifdef GVK_PLATFORM_WINDOWS
    auto pipelinePath = get_pipeline_path(guiInfo, pipelineInfo.pipeline.get_dispatchable_handle(), pipelineInfo.pipeline.get_handle());
    auto pipelineRequestInfo = gvk::get_default<GvkPipelineExplorerPipelineRequestInfo>();
    pipelineRequestInfo.device = pipelineInfo.pipeline.get_dispatchable_handle();
    pipelineRequestInfo.experimentEnabled = pipelineInfo.experimentEnabled;
    pipelineRequestInfo.pExperimentPipelinePath = !pipelinePath.empty() ? pipelinePath.c_str() : nullptr;
    pipelineRequestInfo.experimentPipeline = pipelineInfo.pipeline.get_handle();
    guiInfo.ipcMessenger.write("GvkPipelineExplorerPipelineRequestInfo", pipelineRequestInfo);
#endif
}

void PipelinesWindow::on_launch(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void PipelinesWindow::on_update(GuiInfo& guiInfo)
{
    // TODO : Unify timestamp query behavior

    (void)guiInfo;

#ifdef GVK_PLATFORM_WINDOWS
    // Process pipelines reported from backend
    const auto& messageItr = guiInfo.incomingIpcMessages.find("GvkPipelineExplorerPipelineInfo");
    if (messageItr != guiInfo.incomingIpcMessages.end() && !messageItr->second.empty()) {
        for (const auto& message : messageItr->second) {
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::Auto<GvkPipelineExplorerPipelineInfo> pipelineExplorerPipelineInfo;
            gvk::deserialize(istrm, nullptr, pipelineExplorerPipelineInfo);
            // TODO : Validate pipelineExplorerPipelineInfo

            // Setup pipeline info
            auto device = pipelineExplorerPipelineInfo->device;
            auto pipeline = pipelineExplorerPipelineInfo->pipeline;
            guiInfo.activePipelines.insert({ device, pipeline });
            auto& pipelineInfo = guiInfo.pipelineInfos[{ device, pipeline }];
            boost::multiprecision::import_bits(pipelineInfo.uuid, pipelineExplorerPipelineInfo->uuid, pipelineExplorerPipelineInfo->uuid + GVK_PIPELINE_EXPLORER_UUID_SIZE, 8);
            boost::multiprecision::import_bits(pipelineInfo.driverUUID, pipelineExplorerPipelineInfo->driverUUID, pipelineExplorerPipelineInfo->driverUUID + GVK_PIPELINE_EXPLORER_UUID_SIZE, 8);
            pipelineInfo.pipeline = { device, pipeline };
            pipelineInfo.bindPoint = pipelineExplorerPipelineInfo->bindPoint;
            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
            pipelineInfo.bindPointStr = gvk::string::remove(gvk::to_string(pipelineExplorerPipelineInfo->bindPoint, printerFlags), "\"");
            pipelineInfo.uuidStr = uuid_to_string(pipelineExplorerPipelineInfo->uuid, 18);
            pipelineInfo.driverUUIDStr = uuid_to_string(pipelineExplorerPipelineInfo->driverUUID, 18);
            pipelineInfo.handleStr = gvk::to_hex_string(pipeline);
            pipelineInfo.name = pipelineExplorerPipelineInfo->pName;

            // Calculate pipeline color
            // TODO : Calculate in backend
            std::stringstream strStrm;
            strStrm << std::hex << pipelineInfo.uuidStr.substr(2, 6);
            uint32_t hexColorValue = 0;
            strStrm >> hexColorValue;
            pipelineInfo.highlightColor.x = (float)(hexColorValue >> 16 & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.y = (float)(hexColorValue >> 8 & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.z = (float)(hexColorValue & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.w = 1.0f;
            if (pipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                pipelineInfo.highlightColor.x = std::min(pipelineInfo.highlightColor.x, 0.5f);
                pipelineInfo.highlightColor.y = std::min(pipelineInfo.highlightColor.y, 0.5f);
                pipelineInfo.highlightColor.z = std::min(pipelineInfo.highlightColor.z, 0.5f);
            }
        }

        // TODO : Insert sorted...
        guiInfo.sortedPipelines.clear();
        for (auto activePipeline : guiInfo.activePipelines) {
            guiInfo.sortedPipelines.push_back(activePipeline);
        }
        sort_pipelines(guiInfo);
    }
#endif // GVK_PLATFORM_WINDOWS
}

void PipelinesWindow::on_gui(GuiInfo& guiInfo)
{
    // Draw pipelines table
    if (ImGui::BeginChild("##Pipelines Table")) {
        auto tableFlags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
        if (ImGui::BeginTable("Pipelines-Table", 10, tableFlags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("UUID",                /* ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder */ 0, 0.0f, 0);
            ImGui::TableSetupColumn("Driver UUID",         ImGuiTableColumnFlags_DefaultHide, 0.0f, 1);
            ImGui::TableSetupColumn("Handle",              ImGuiTableColumnFlags_DefaultHide, 0.0f, 2);
            ImGui::TableSetupColumn("Name",                ImGuiTableColumnFlags_DefaultHide, 0.0f, 3);
            ImGui::TableSetupColumn("Bind Point",                                          0, 0.0f, 4);
            ImGui::TableSetupColumn("Total Executions",                                    0, 0.0f, 5);
            ImGui::TableSetupColumn("Total Duration (ns)",                                 0, 0.0f, 6);
            ImGui::TableSetupColumn("Avg Duration (ns)",                                   0, 0.0f, 7);
            ImGui::TableSetupColumn("Experiment",          ImGuiTableColumnFlags_DefaultHide, 0.0f, 8);
            ImGui::TableSetupColumn("Highlight",                                           0, 0.0f, 9);
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
                    const auto& pipelineExecutionInfo = guiInfo.rangeInfo.pipelineExecutionInfos[pipeline];
                    auto selected = guiInfo.selectedPipeline == pipelineInfo.pipeline;

                    ImGui::PushID(row_n);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.uuidStr.c_str(), selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.driverUUIDStr.c_str(), selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.handleStr.c_str(), selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.name.c_str(), selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(pipelineInfo.bindPointStr.c_str(), selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }

                    // Execution count
                    ImGui::TableNextColumn();
                    ImGui::PushID("Execution Count");
                    if (ImGui::Selectable("", selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::SameLine();
#ifdef GVK_PLATFORM_WINDOWS
                    ImGui::Text("%d", pipelineExecutionInfo.executionCount);
#endif
                    ImGui::PopID();

                    // Total duration
                    ImGui::TableNextColumn();
                    ImGui::PushID("Total Duration");
                    if (ImGui::Selectable("", selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::SameLine();
                    ImGui::Text("%.2f", (double)pipelineExecutionInfo.totalDurationNs);
                    ImGui::PopID();

                    // Avg duration
                    ImGui::TableNextColumn();
                    ImGui::PushID("Avg Duration");
                    if (ImGui::Selectable("", selected)) {
                        set_selected_pipeline(guiInfo, pipeline);
                    }
                    ImGui::SameLine();
                    ImGui::Text("%.2f", pipelineExecutionInfo.executionCount ? (double)pipelineExecutionInfo.totalDurationNs / (double)pipelineExecutionInfo.executionCount : 0);
                    ImGui::PopID();

                    // Experiment
                    ImGui::TableNextColumn();
                    if (ImGui::Checkbox("##Experiment Enabled", &pipelineInfo.experimentEnabled)) {
                        guiInfo.requestInfo.experimentPipeline = pipelineInfo.pipeline.get_handle();
                        guiInfo.requestInfo.experimentEnabled = pipelineInfo.experimentEnabled;
                        update_experiment(guiInfo, pipelineInfo);
                        if (!guiInfo.selectedPipeline.get_handle()) {
                            set_selected_pipeline(guiInfo, pipeline);
                        }
                    }

                    // TODO : Highlight is overloaded...highlight for chart vs in scene. Need to separate these concepts and UI
                    // Highlight
                    ImGui::TableNextColumn();
                    bool highlightStateChanged = false;
                    if (ImGui::ColorEdit4("Highlight Color", (float*)&pipelineInfo.highlightColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                        #if 0
                        // TODO : Want to be able to change color without forcing a shader recompile.
                        //  Maybe a timer that delays the highlight pipeline update until the user is
                        //  done changing the color...ImGui has something like this built in, maybe can
                        //  leverage that.
                        pipelineInfo.highlightEnabled = false;
                        #endif
                        highlightStateChanged = true;
                    }
                    ImGui::SameLine();
                    highlightStateChanged |= ImGui::Checkbox("##Highlight Enabled", &pipelineInfo.highlightEnabled);
                    if (highlightStateChanged) {
                        guiInfo.requestInfo.highlightPipeline = pipelineInfo.pipeline.get_handle();
                        guiInfo.requestInfo.highlightEnabled = pipelineInfo.highlightEnabled;
                        memcpy(guiInfo.requestInfo.highlightColor, &pipelineInfo.highlightColor, sizeof(pipelineInfo.highlightColor));
                        if (!guiInfo.selectedPipeline.get_handle()) {
                            set_selected_pipeline(guiInfo, pipeline);
                        }
                    }

                    ImGui::PopID();
                }
            }

            // Detect click in table but not on any row to deselect
            if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                set_selected_pipeline(guiInfo, { });
            }

            // Force one pipeline selection for metrics
            for (auto& pipelineInfo : guiInfo.pipelineInfos) {
                pipelineInfo.second.sampleMetrics = pipelineInfo.first == guiInfo.selectedPipeline;
            }

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

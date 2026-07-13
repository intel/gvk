
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

#include "gvk-pipeline-explorer/gui/selected-pipeline-window.hpp"
#include "gvk-pipeline-explorer/gui/file-window.hpp"
#include "gvk-pipeline-explorer/gui/pipelines-window.hpp"
#include "gvk-pipeline-explorer/gui/window-manager.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

SelectedPipelineWindow::SelectedPipelineWindow(Window::Manager& windowManager)
    : Window(windowManager, "Selected Pipeline")
{
}

void SelectedPipelineWindow::on_gui(GuiInfo& guiInfo)
{
    ImGui::BeginDisabled(!guiInfo.selectedPipeline.get_handle());
    {
        auto& selectedPipelineInfo = guiInfo.pipelineInfos[guiInfo.selectedPipeline];
#if 0
        ImGui::Text("Write Pipeline Info");
        ImGui::Checkbox("##Info Enabled", &selectedPipelineInfo.infoWriteEnabled);
        ImGui::SameLine();
#endif

        // TODO : Wrangle path creation
        auto infoPath = gvk::string::scrub_path((std::filesystem::path(guiInfo.workspace) / ("VkPipeline-UUID-" + selectedPipelineInfo.uuidStr)).string());
        ImGui::BeginDisabled();
        {
            ImGui::PushItemWidth(-FLT_MIN);
            GvkGui::InputPath("##Info Path", &infoPath);
            ImGui::PopItemWidth();
        }
        ImGui::EndDisabled();

        // Identification fields
        ImGui::BeginDisabled();
        ImGui::InputText("UUID", &selectedPipelineInfo.uuidStr);
        ImGui::InputText("Driver UUID", &selectedPipelineInfo.driverUUIDStr);
        ImGui::InputText("Handle", &selectedPipelineInfo.handleStr);
        ImGui::InputText("Name", &selectedPipelineInfo.name);
        ImGui::InputText("Bind Point", &selectedPipelineInfo.bindPointStr);
        ImGui::EndDisabled();

        // File open
        ImGui::BeginDisabled(!std::filesystem::exists(infoPath));
        {
#if 1
            if (ImGui::Button("Open File")) {
                ImGui::OpenPopup("Pipeline-File-Popup");
            }
            if (ImGui::BeginPopup("Pipeline-File-Popup")) {
                // TODO : Closing app while this is open causes crash...
                for (const auto& dirItr : std::filesystem::recursive_directory_iterator(infoPath)) {
                    if (dirItr.is_regular_file()) {
                        if (ImGui::Selectable(dirItr.path().filename().string().c_str())) {
                            get_window_manager().open<FileWindow>(FileWindow::get_name(dirItr.path()), dirItr.path(), guiInfo.selectedPipeline);
                        }
                    }
                }
                ImGui::EndPopup();
            }
#else
            if (ImGui::BeginMenu("Open File")) {
                // TODO : Closing app while this is open causes crash...
                for (const auto& dirItr : std::filesystem::recursive_directory_iterator(infoPath)) {
                    if (dirItr.is_regular_file()) {
                        if (ImGui::Selectable(dirItr.path().filename().string().c_str())) {
                            get_window_manager().open<FileWindow>(FileWindow::get_name(dirItr.path()), dirItr.path(), guiInfo.selectedPipeline);
                        }
                    }
                }
                ImGui::EndMenu();
            }
#endif
        }
        ImGui::EndDisabled();

#ifdef GVK_PLATFORM_WINDOWS
        bool workloadActive = (bool)guiInfo.workload;
#else
        bool workloadActive = false;
#endif

        // Recompile
        ImGui::BeginDisabled(!workloadActive);
        {
            ImGui::SameLine();
            if (ImGui::Button("Recompile")) {
                guiInfo.requestInfo.recompilePipeline = guiInfo.selectedPipeline.get_handle();
                guiInfo.requestInfo.experimentPipeline = guiInfo.selectedPipeline.get_handle();
                guiInfo.requestInfo.experimentEnabled = true;
                selectedPipelineInfo.experimentEnabled = true;

#ifdef GVK_PLATFORM_WINDOWS
                // TODO : Documentation
                auto pipelinePath = get_pipeline_path(guiInfo, guiInfo.selectedPipeline.get_dispatchable_handle(), guiInfo.selectedPipeline.get_handle());
                auto pipelineRequestInfo = gvk::get_default<GvkPipelineExplorerPipelineRequestInfo>();
                pipelineRequestInfo.device = guiInfo.selectedPipeline.get_dispatchable_handle();
                pipelineRequestInfo.pRecompilePipelinePath = !pipelinePath.empty() ? pipelinePath.c_str() : nullptr;
                pipelineRequestInfo.recompilePipeline = guiInfo.selectedPipeline.get_handle();
                guiInfo.ipcMessenger.write("GvkPipelineExplorerPipelineRequestInfo", pipelineRequestInfo);
#endif
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                ImGui::SetTooltip("[Ctrl]+[Shift]+[B]");
            }
        }
        ImGui::EndDisabled();

        // Experiment
        ImGui::SameLine();
        if (ImGui::Checkbox("Experiment", &selectedPipelineInfo.experimentEnabled)) {
            guiInfo.requestInfo.experimentPipeline = guiInfo.selectedPipeline.get_handle();
            guiInfo.requestInfo.experimentEnabled = selectedPipelineInfo.experimentEnabled;
            PipelinesWindow::update_experiment(guiInfo, selectedPipelineInfo);
        }

        // Highlight
        ImGui::SameLine();
		bool highlightStateChanged = false;
        if (ImGui::ColorEdit4("Color", (float*)&selectedPipelineInfo.highlightColor, ImGuiColorEditFlags_NoInputs)) {
            #if 0
            // TODO : Want to be able to change color without forcing a shader recompile.
            //  Maybe a timer that delays the highlight pipeline update until the user is
            //  done changing the color...ImGui has something like this built in, maybe can
            //  leverage that.
            selectedPipelineInfo.highlightEnabled = false;
            #endif
            highlightStateChanged = true;
        }
        ImGui::SameLine();
        highlightStateChanged |= ImGui::Checkbox("Highlight", &selectedPipelineInfo.highlightEnabled);
        if (highlightStateChanged) {
            guiInfo.requestInfo.highlightPipeline = guiInfo.selectedPipeline.get_handle();
            guiInfo.requestInfo.highlightEnabled = selectedPipelineInfo.highlightEnabled;
            memcpy(guiInfo.requestInfo.highlightColor, &selectedPipelineInfo.highlightColor, sizeof(selectedPipelineInfo.highlightColor));
        }

        // Executions and duration table
        if (ImGui::BeginTable("ExecutionStats", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            const auto& pipelineExecutionInfo = guiInfo.rangeInfo.pipelineExecutionInfos[selectedPipelineInfo.pipeline];

            // Header row
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Executions", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Duration (ns)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            // TODO : Documentation
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Total");
            ImGui::TableNextColumn();
#ifdef GVK_PLATFORM_WINDOWS
            ImGui::Text("%d", pipelineExecutionInfo.executionCount);
#endif
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", (double)pipelineExecutionInfo.totalDurationNs);

            // TODO : Documentation
            if (pipelineExecutionInfo.executionCount) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Avg/Execute");
                ImGui::TableNextColumn();
#ifdef GVK_PLATFORM_WINDOWS
                ImGui::Text("");
#endif
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", (double)pipelineExecutionInfo.totalDurationNs / (double)pipelineExecutionInfo.executionCount);
            }

            // TODO : Documentation
            if (guiInfo.rangeInfo.submitCount) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Avg/Submit");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", (double)pipelineExecutionInfo.executionCount / (double)guiInfo.rangeInfo.submitCount);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", (double)pipelineExecutionInfo.totalDurationNs / (double)guiInfo.rangeInfo.submitCount);
            }

#if 0
            // TODO : Documentation
            if (guiInfo.rangeInfo.presentCount) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Avg/Present");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", (double)pipelineExecutionInfo.executionCount / (double)guiInfo.rangeInfo.presentCount);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", (double)pipelineExecutionInfo.totalDurationNs / (double)guiInfo.rangeInfo.presentCount);
            }
#endif

            ImGui::EndTable();
        }


        // Draw shader compilation buttons and file selector
#if 0
        if (ImGui::Button("Decompile")) {
            guiInfo.requestInfo.decompilePipeline = guiInfo.selectedPipeline.get_handle();
        }
        ImGui::SameLine();
        if (ImGui::Button("Recompile")) {
            guiInfo.requestInfo.recompilePipeline = guiInfo.selectedPipeline.get_handle();
            guiInfo.requestInfo.experimentPipeline = guiInfo.selectedPipeline.get_handle();
            guiInfo.requestInfo.experimentEnabled = true;
            selectedPipelineInfo.experimentEnabled = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
            ImGui::SetTooltip("[Ctrl]+[Shift]+[B]");
        }
#else

        ImGui::BeginDisabled(!workloadActive);
        {
#if 0
            if (ImGui::Button("Decompile")) {
                guiInfo.requestInfo.decompilePipeline = guiInfo.selectedPipeline.get_handle();

                // TODO : Documentation
                auto pipelinePath = get_pipeline_path(guiInfo, guiInfo.selectedPipeline.get_dispatchable_handle(), guiInfo.selectedPipeline.get_handle());
                auto pipelineRequestInfo = gvk::get_default<GvkPipelineExplorerPipelineRequestInfo>();
                pipelineRequestInfo.device = guiInfo.selectedPipeline.get_dispatchable_handle();
                pipelineRequestInfo.pDecompilePipelinePath = !pipelinePath.empty() ? pipelinePath.c_str() : nullptr;
                pipelineRequestInfo.decompilePipeline = guiInfo.selectedPipeline.get_handle();
                guiInfo.ipcMessenger.write("GvkPipelineExplorerPipelineRequestInfo", pipelineRequestInfo);
            }
            ImGui::SameLine();
#endif

        }
        ImGui::EndDisabled();
#endif




        // // NOTE : Currently, selectedPipeline is the only sampleMetrics pipeline
        // selectedPipelineInfo.sampleMetrics = selectedPipelineInfo.pipeline.get_handle() != VK_NULL_HANDLE;
        // if (ImGui::Checkbox("Metrics", &selectedPipelineInfo.sampleMetrics) && selectedPipelineInfo.sampleMetrics) {
        // }



        // TODO : Documentation
    }
	ImGui::EndDisabled();
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

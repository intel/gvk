
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
        ImGui::Text("Write Pipeline Info");
        ImGui::Checkbox("##Info Enabled", &selectedPipelineInfo.infoWriteEnabled);
        ImGui::SameLine();
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::BeginDisabled();
        // TODO : Wrangle path creation
        auto infoPath = gvk::string::scrub_path((std::filesystem::path(guiInfo.workspaceInfo.workspace) / ("VkPipeline-UUID-" + selectedPipelineInfo.uuidStr)).string());
        GvkGui::InputPath("##Info Path", &infoPath);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();

        // Draw UUID fields
        ImGui::BeginDisabled();
        ImGui::InputText("UUID", &selectedPipelineInfo.uuidStr);
        ImGui::InputText("Driver UUID", &selectedPipelineInfo.driverUUIDStr);
        ImGui::EndDisabled();

        // Draw shader compilation buttons and file selector
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

        // Draw pipelne file selection popup
        ImGui::SameLine();
        ImGui::BeginDisabled(!std::filesystem::exists(infoPath));
        if (ImGui::Button("Open File")) {
            ImGui::OpenPopup("Pipeline-File-Popup");
        }
        if (ImGui::BeginPopup("Pipeline-File-Popup")) {
            // TODO : Closing app while this is open causes crash...
            for (const auto& dirItr : std::filesystem::recursive_directory_iterator(infoPath)) {
                if (dirItr.is_regular_file()) {
                    if (ImGui::Selectable(dirItr.path().filename().string().c_str())) {
                        get_window_manager().open<FileWindow>(FileWindow::get_name(dirItr.path()), dirItr.path());
                    }
                }
            }
            ImGui::EndPopup();
        }
        ImGui::EndDisabled();

        // Draw experiment and metrics options
        if (ImGui::Checkbox("Experiment", &selectedPipelineInfo.experimentEnabled)) {
            guiInfo.requestInfo.experimentPipeline = guiInfo.selectedPipeline.get_handle();
            guiInfo.requestInfo.experimentEnabled = selectedPipelineInfo.experimentEnabled;
        }
        ImGui::SameLine();
        // NOTE : Currently, selectedPipeline is the only sampleMetrics pipeline
        selectedPipelineInfo.sampleMetrics = selectedPipelineInfo.pipeline.get_handle() != VK_NULL_HANDLE;
        if (ImGui::Checkbox("Metrics", &selectedPipelineInfo.sampleMetrics) && selectedPipelineInfo.sampleMetrics) {
        }

        // Draw highlight options
        ImGui::BeginDisabled(!(selectedPipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS || selectedPipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR));
        auto highlightStateChanged = ImGui::Checkbox("Highlight", &selectedPipelineInfo.highlightEnabled);
        ImGui::SameLine();
        if (ImGui::ColorEdit4("Color", (float*)&selectedPipelineInfo.highlightColor, ImGuiColorEditFlags_NoInputs)) {
            selectedPipelineInfo.highlightEnabled = false;
            highlightStateChanged = true;
        }
        if (highlightStateChanged) {
            guiInfo.requestInfo.highlightPipeline = guiInfo.selectedPipeline.get_handle();
            guiInfo.requestInfo.highlightEnabled = selectedPipelineInfo.highlightEnabled;
            memcpy(guiInfo.requestInfo.highlightColor, &selectedPipelineInfo.highlightColor, sizeof(selectedPipelineInfo.highlightColor));
        }
        ImGui::EndDisabled();
    }
    ImGui::EndDisabled();
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

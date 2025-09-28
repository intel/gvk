
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

#include "gvk-pipeline-explorer/gui/api-call-explorer-window.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

ApiCallExplorerWindow::ApiCallExplorerWindow(Window::Manager& windowManager)
    : Window(windowManager, "API Call Explorer")
{
}

ApiCallExplorerWindow::~ApiCallExplorerWindow()
{
}

void ApiCallExplorerWindow::on_gui(GuiInfo& guiInfo)
{
    // Check request/result
    if (guiInfo.apiCallInfo.requestResult.pending()) {
        if (guiInfo.apiCallInfo.requestResult.check_result(guiInfo.workspaceInfo.workspace) == VK_SUCCESS) {
            guiInfo.apiCallInfo.commandCollection = guiInfo.apiCallInfo.requestResult.get_result()->commands;
        }
    }

    // Process request/result
    ImGui::BeginDisabled(!guiInfo.applicationInfo.running && !guiInfo.cliProvidedWorkspace);
    ImGui::BeginDisabled(guiInfo.apiCallInfo.requestResult.pending());
    if (ImGui::Button("Refresh API Calls")) {
        auto request = gvk::get_default<GvkPipelineExplorerCommandCollectionRequestInfo>();
        std::string reportPath = guiInfo.reportEnabled ? (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string() : std::string();
        request.pReportPath = !reportPath.empty() ? reportPath.c_str() : nullptr;
        guiInfo.apiCallInfo.requestResult.submit_request(guiInfo.workspaceInfo.workspace, request);
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();

    // Draw table
    auto tableFlags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
    if (ImGui::BeginTable("API Calls", 3, tableFlags)) {
        auto lockedColumnFlags =
            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize |
            ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoClip;
        ImGui::TableSetupColumn("Index", lockedColumnFlags | ImGuiTableColumnFlags_NoHeaderLabel);
        ImGui::TableSetupColumn("Time (ns)");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Sort
        auto pTableSortSpecs = ImGui::TableGetSortSpecs();
        if (pTableSortSpecs && pTableSortSpecs->SpecsDirty) {
            pTableSortSpecs->SpecsDirty = false;
        }

        // Process commands
        for (uint32_t i = 0; i < guiInfo.apiCallInfo.commandCollection->commandCount; ++i) {
            auto pCommand = guiInfo.apiCallInfo.commandCollection->ppCommands[i];
            assert(pCommand);

            ImGui::PushID(i);
            ImGui::TableNextRow();

            // Index
            ImGui::TableNextColumn();
            ImGui::Text("%s", std::to_string(i).c_str());

            // Duration
            ImGui::TableNextColumn();
            if (i < guiInfo.apiCallInfo.commandDurations.size()) {
                ImGui::Text("%s", std::to_string(guiInfo.apiCallInfo.commandDurations[i]).c_str());
            } else {
                ImGui::Text("%s", std::to_string(0).c_str());
            }

            // Command
            ImGui::TableNextColumn();
            if (ImGui::TreeNode(gvk::get_cname(pCommand->sType))) {
                ImGui::Text("%s", gvk::to_string(*pCommand, gvk::Printer::Default ^ gvk::Printer::EnumValue).c_str());
                ImGui::TreePop();
            }
            switch (pCommand->sType) {
            case gvk::get_stype<GvkCommandStructureQueueSubmit>():
            case gvk::get_stype<GvkCommandStructureQueueSubmit2>():
            case gvk::get_stype<GvkCommandStructureQueueSubmit2KHR>(): {
                ImGui::TableNextRow();
                ImGui::TableNextRow();
            } break;
            default: {
            } break;
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

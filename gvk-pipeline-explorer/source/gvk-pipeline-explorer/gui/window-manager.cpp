
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
#include "gvk-pipeline-explorer/gui/api-call-explorer-window.hpp"
#include "gvk-pipeline-explorer/gui/api-call-timeline-window.hpp"
#include "gvk-pipeline-explorer/gui/window-manager.hpp"
#include "gvk-pipeline-explorer/gui/console-window.hpp"
#include "gvk-pipeline-explorer/gui/pipelines-window.hpp"
#include "gvk-pipeline-explorer/gui/selected-pipeline-window.hpp"
#include "gvk-pipeline-explorer/gui/workspace-window.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

Window::Manager::Manager()
{
    auto upApiCallExplorerWindow = std::make_unique<ApiCallExplorerWindow>(*this);
    upApiCallExplorerWindow->mOpen = false;
    mCoreWindows.insert({ "API Call Explorer", std::move(upApiCallExplorerWindow) });

    auto upApiCallTimelineWindow = std::make_unique<ApiCallTimelineWindow>(*this);
    upApiCallTimelineWindow->mOpen = false;
    mCoreWindows.insert({ "API Call Timeline", std::move(upApiCallTimelineWindow) });

    mCoreWindows.insert({ "Selected Pipeline", std::make_unique<SelectedPipelineWindow>(*this) });
    mCoreWindows.insert({ "Pipelines", std::make_unique<PipelinesWindow>(*this) });
    mCoreWindows.insert({ "Metrics", std::make_unique<MetricsWindow>(*this) });
    mCoreWindows.insert({ "Workspace", std::make_unique<WorkspaceWindow>(*this) });
    mCoreWindows.insert({ "Console", std::make_unique<ConsoleWindow>(*this) });
}

void Window::Manager::clear()
{
    mWindows.clear();
    mNewWindows.clear();
}

void Window::Manager::on_gui(GuiInfo& guiInfo)
{
    // Draw main menu bar
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::Button("Save All : [Ctrl]+[Shift]+[S]")) {
            on_save(guiInfo);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows")) {
        for (auto& windowItr : mCoreWindows) {
            ImGui::Checkbox(windowItr.second->mName.c_str(), &windowItr.second->mOpen);
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    // Save all windows on [Ctrl]+[Shift]+[S]
    // if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S)) {
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S)) {
        on_save(guiInfo);
    }

    // Recompile pipelines on [Ctrl]+[Shift]+[BS]
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_B)) {
        guiInfo.pipelineInfos[guiInfo.selectedPipeline].experimentEnabled = true;
        guiInfo.requestInfo.recompilePipeline = guiInfo.selectedPipeline.get_handle();
        guiInfo.requestInfo.experimentPipeline = guiInfo.selectedPipeline.get_handle();
        guiInfo.requestInfo.experimentEnabled = true;
    }

    // Setup root window
    auto pMainViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(pMainViewport->WorkPos);
    ImGui::SetNextWindowSize(pMainViewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("Pipeline Explorer", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoDecoration
    );
    {
        ImGui::PopStyleVar(2);

        // Setup dock space
        auto rootNodeId = ImGui::GetID("Pipeline Explorer Dockspace");
        ImGui::DockSpace(rootNodeId);
        if (guiInfo.buildDefaultDockSpace) {
            guiInfo.buildDefaultDockSpace = false;
            ImGui::DockBuilderRemoveNode(rootNodeId);
            ImGui::DockBuilderAddNode(rootNodeId);

            ImGui::DockBuilderDockWindow("API Call Timeline", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Up, 0.2f, nullptr, &rootNodeId));
            ImGui::DockBuilderDockWindow("API Call Explorer", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Left, 0.2f, nullptr, &rootNodeId));

            ImGuiID workspaceNodeId = 0;
            ImGui::DockBuilderDockWindow("Workspace", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Down, 0.25f, &workspaceNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Console", ImGui::DockBuilderSplitNode(workspaceNodeId, ImGuiDir_Right, 0.5f, nullptr, &workspaceNodeId));

            ImGuiID pipelinesNodeId = 0;
            ImGui::DockBuilderDockWindow("Pipelines", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Left, 0.4f, &pipelinesNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Selected Pipeline", ImGui::DockBuilderSplitNode(pipelinesNodeId, ImGuiDir_Up, 0.3f, nullptr, &pipelinesNodeId));

            ImGui::DockBuilderDockWindow("Metrics", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Right, 0.5f, nullptr, &rootNodeId));
            ImGui::DockBuilderFinish(rootNodeId);
        }
    }
    ImGui::End();

    // Draw windows
    on_gui(guiInfo, mCoreWindows);
    on_gui(guiInfo, mWindows);

    // Merge new Windows spawned this frame into Window collection to start
    //  rendering next frame
    mWindows.merge(mNewWindows);
    mNewWindows.clear();

    #if 0
    // Draw ImGui demo window
    ImGui::ShowDemoWindow();
    #endif
}

void Window::Manager::on_save(GuiInfo& guiInfo)
{
    on_save(guiInfo, mCoreWindows);
    on_save(guiInfo, mWindows);
}

void Window::Manager::on_load(GuiInfo& guiInfo)
{
    on_load(guiInfo, mCoreWindows);
    on_load(guiInfo, mWindows);
}

void Window::Manager::on_gui(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows)
{
    for (auto& windowItr : windows) {
        if (windowItr.second->mOpen) {
            if (ImGui::Begin(windowItr.second->mName.c_str(), &windowItr.second->mOpen)) {
                windowItr.second->on_gui(guiInfo);
            }
            ImGui::End();
        }
    }
}

void Window::Manager::on_save(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows)
{
    for (auto& windowItr : windows) {
        windowItr.second->on_save(guiInfo);
    }
}

void Window::Manager::on_load(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows)
{
    for (auto& windowItr : windows) {
        windowItr.second->on_load(guiInfo);
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

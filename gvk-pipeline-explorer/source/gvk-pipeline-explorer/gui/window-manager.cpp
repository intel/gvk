
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

#include "gvk-pipeline-explorer/gui/metrics/metrics-charts-window.hpp"
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

    auto upWorkspaceWindow = std::make_unique<WorkspaceWindow>(*this);
    mpWorkspaceWindow = upWorkspaceWindow.get();
    mCoreWindows.insert({ "Workspace", std::move(upWorkspaceWindow) });

    mCoreWindows.insert({ "Pipeline Executions", std::make_unique<ApiCallTimelineWindow>(*this) });
    mCoreWindows.insert({ "Selected Pipeline", std::make_unique<SelectedPipelineWindow>(*this) });
    mCoreWindows.insert({ "Pipelines", std::make_unique<PipelinesWindow>(*this) });
    mCoreWindows.insert({ "Console", std::make_unique<ConsoleWindow>(*this) });

    auto upMetricsWindow = std::make_unique<MetricsWindow>(*this);
    upMetricsWindow->mOpen = false;
	auto upMetricsChartsWindow = std::make_unique<MetricsChartsWindow>(*this, *upMetricsWindow);
    upMetricsChartsWindow->mOpen = false;
    mCoreWindows.insert({ "Metrics Charts", std::move(upMetricsChartsWindow) });
    mCoreWindows.insert({ "Metrics", std::move(upMetricsWindow) });
}

void Window::Manager::reset()
{
    mWindows.clear();
    mNewWindows.clear();
    for (auto& window : mCoreWindows) {
        window.second->reset();
    }
}

void Window::Manager::clear()
{
    mWindows.clear();
    mNewWindows.clear();
    for (auto& window : mCoreWindows) {
        window.second->reset();
    }
}

bool Window::Manager::query_active() const
{
    return mQueryActive;
}

void Window::Manager::on_launch(GuiInfo& guiInfo)
{
    if (mQueryActive) {
        auto pipelineExplorerQueryRequestInfo = gvk::get_default<GvkPipelineExplorerQueryRequestInfo>();
        pipelineExplorerQueryRequestInfo.interval = guiInfo.queryInterval;
        gvk::IpcMessenger::Message message{ };
        create_ipc_message("GvkPipelineExplorerQueryRequestInfo", pipelineExplorerQueryRequestInfo, &message);
#ifdef GVK_PLATFORM_WINDOWS
        guiInfo.startupIpcMessages.push_back(message);
#endif
    }
    on_launch(guiInfo, mCoreWindows);
    on_launch(guiInfo, mWindows);
}

void Window::Manager::on_terminate(GuiInfo& guiInfo)
{
    // TODO : Need to give other windows terminate call
    // TODO : Should this just go in the more generic on_save()
    auto rangesPath = mpWorkspaceWindow ? std::filesystem::path(mpWorkspaceWindow->get_launch_options().workspace) / "ranges" : "";
    if (!rangesPath.empty()) {
        guiInfo.rangeInfo.save(guiInfo, rangesPath);
    }
}

void Window::Manager::on_update(GuiInfo& guiInfo)
{
#ifdef GVK_PLATFORM_WINDOWS

    // Process present info reported from backend
    {
        const auto& messageItr = guiInfo.incomingIpcMessages.find("GvkPipelineExplorerPresentInfo");
        if (messageItr != guiInfo.incomingIpcMessages.end() && !messageItr->second.empty()) {
            const auto& message = messageItr->second.back();
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::Auto<GvkPipelineExplorerPresentInfo> pipelineExplorerPresentInfo;
            gvk::deserialize(istrm, nullptr, pipelineExplorerPresentInfo);

            // Update frame durations and frame rate
            guiInfo.frameDurationAccumulator += pipelineExplorerPresentInfo->frameDurationNS - guiInfo.frameDurations[guiInfo.frameIndex];
            guiInfo.frameDurations[guiInfo.frameIndex] = pipelineExplorerPresentInfo->frameDurationNS;
            guiInfo.frameIndex = (guiInfo.frameIndex + 1) % guiInfo.frameDurations.size();
            guiInfo.frameCount = std::min(guiInfo.frameCount + 1, (uint32_t)guiInfo.frameDurations.size());
            // frameDurationAccumulator is in nanoseconds; convert to seconds for FPS
            double avgFrameSeconds = (guiInfo.frameDurationAccumulator / (double)guiInfo.frameCount) * 1e-9;
            guiInfo.frameRate = (0.0 < avgFrameSeconds) ? (1.0 / avgFrameSeconds) : std::numeric_limits<double>::max();

            #if 0
            // HACK : Backend needs to send all API calls in sequence together for accurate timestamping
            if (mQueryActive) {
                ++guiInfo.rangeInfo.presentCount;
            }
            #endif
        }
    }

    // TODO : Documentation
    {
        const auto& messageItr = guiInfo.incomingIpcMessages.find("GvkPipelineExplorerQueryRequestInfo"); // TODO : StatusInfo
        if (messageItr != guiInfo.incomingIpcMessages.end() && !messageItr->second.empty()) {
            const auto& message = messageItr->second.back();
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::Auto<GvkPipelineExplorerQueryRequestInfo> pipelineExplorerQueryRequestInfo;
            gvk::deserialize(istrm, nullptr, pipelineExplorerQueryRequestInfo);

            // TODO : Documentation
            if (pipelineExplorerQueryRequestInfo->interval.mode == GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE &&
                pipelineExplorerQueryRequestInfo->interval.toggle
            ) {
                mQueryActive = true;
                guiInfo.rangeInfo = { };
            } else {
                mQueryActive = false;
                auto rangesPath = mpWorkspaceWindow ? std::filesystem::path(mpWorkspaceWindow->get_launch_options().workspace) / "ranges" : "";
                if (!rangesPath.empty()) {
                    guiInfo.rangeInfo.save(guiInfo, rangesPath);
                }
            }
        }
    }

#endif // GVK_PLATFORM_WINDOWS

    // TODO : Documentation
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) {
        mImGuiDebugWindowsEnabled = !mImGuiDebugWindowsEnabled;
    }

    // TODO : Documentation
    on_update(guiInfo, mCoreWindows);
    on_update(guiInfo, mWindows);
}

void Window::Manager::on_gui(GuiInfo& guiInfo)
{
    draw_menu_bar(guiInfo);
    draw_tool_bar(guiInfo);
    draw_dockspace(guiInfo);

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

    // Draw windows
    on_gui(guiInfo, mCoreWindows);
    on_gui(guiInfo, mWindows);

    // Merge new Windows spawned this frame into Window collection to start
    //  rendering next frame
    mWindows.merge(mNewWindows);
    mNewWindows.clear();

    // TODO : Documentation
    if (mImGuiDebugWindowsEnabled) {
        if (mImGuiMetricsWindowEnabled) {
            ImGui::ShowMetricsWindow(&mImGuiMetricsWindowEnabled);
        }
        if (mImGuiDebugLogWindowEnabled) {
            ImGui::ShowDebugLogWindow(&mImGuiDebugLogWindowEnabled);
        }
        if (mImGuiIDStackToolWindowEnabled) {
            ImGui::ShowIDStackToolWindow(&mImGuiIDStackToolWindowEnabled);
        }
        if (mImGuiDemoWindowEnabled) {
            ImGui::ShowDemoWindow(&mImGuiDemoWindowEnabled);
        }
        if (mImGuiAboutWindowEnabled) {
            ImGui::ShowAboutWindow(&mImGuiAboutWindowEnabled);
        }
    }
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

void Window::Manager::draw_menu_bar(GuiInfo& guiInfo)
{
    // Draw main menu bar
    if (ImGui::BeginMainMenuBar()) {
        mMenuBarHeight = ImGui::GetWindowHeight();

        // TODO : Documentation
        if (ImGui::BeginMenu("File")) {
            if (ImGui::Selectable("Save All : [Ctrl]+[Shift]+[S]")) {
                on_save(guiInfo);
            }
            ImGui::EndMenu();
        }

        // TODO : Documentation
        if (ImGui::BeginMenu("Windows")) {
            for (auto& windowItr : mCoreWindows) {
                ImGui::Checkbox(windowItr.second->mName.c_str(), &windowItr.second->mOpen);
            }
            if (mImGuiDebugWindowsEnabled) {
                ImGui::Separator();
                ImGui::Checkbox("ImGui Metrics", &mImGuiMetricsWindowEnabled);
                ImGui::Checkbox("ImGui Debug Log", &mImGuiDebugLogWindowEnabled);
                ImGui::Checkbox("ImGui ID Stack Tool", &mImGuiIDStackToolWindowEnabled);
                ImGui::Checkbox("ImGui Demo Window", &mImGuiDemoWindowEnabled);
                ImGui::Checkbox("About ImGui", &mImGuiAboutWindowEnabled);
            }
            ImGui::EndMenu();
        }

        // TODO : Documentation
        auto rangesPath = mpWorkspaceWindow ? std::filesystem::path(mpWorkspaceWindow->get_launch_options().workspace) / "ranges" : "";
        ImGui::BeginDisabled(!std::filesystem::exists(rangesPath));
        {
            if (ImGui::BeginMenu("Ranges")) {
#ifdef GVK_PLATFORM_WINDOWS
                bool workloadEnabled = guiInfo.workload;
#else
                bool workloadEnabled = true;
#endif
                ImGui::BeginDisabled(workloadEnabled);
                {
                    for (const auto& dirItr : std::filesystem::directory_iterator(rangesPath)) {
                        if (dirItr.is_directory()) {
                            if (ImGui::Selectable(dirItr.path().stem().string().c_str())) {
                                guiInfo.rangeInfo.load(guiInfo, dirItr.path());
                                guiInfo.workspace = mpWorkspaceWindow->get_launch_options().workspace;
                            }
                        }
                    }
                }
                ImGui::EndDisabled();
                ImGui::EndMenu();
            }
        }
        ImGui::EndDisabled();

        #if 0
        ImGui::ShowStyleEditor();
        ImGui::ShowStyleSelector("Style Selector");
        ImGui::ShowFontSelector("Font Selector");
        ImGui::ShowUserGuide();
        #endif

        // TODO : Documentation
        if (guiInfo.frameRate) {
            char framerateText[64];
            snprintf(framerateText, sizeof(framerateText), "%.3f ms/frame (%.1f fps)", (float)(1000.0 / guiInfo.frameRate), (float)guiInfo.frameRate);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(framerateText).x - ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%s", framerateText);
        }

        ImGui::EndMainMenuBar();
    }
}

void Window::Manager::draw_tool_bar(GuiInfo& guiInfo)
{
    // Draw tool bar
    auto displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, mMenuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(displaySize.x, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg]);
    if (ImGui::Begin("##Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar      |
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoScrollbar     |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize
    )) {
        // Auto-sized child shrink-wraps all content; re-centered each frame using last frame's size
        static ImVec2 contentSize(0, 0);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - contentSize.x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg]);
        if (ImGui::BeginChild("##ToolbarContent", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        )) {

            // TODO : Documentation
            static std::unordered_map<GvkPipelineExplorerQueryMode, std::string> scQueryModes{
                { GVK_PIPELINE_EXPLORER_QUERY_MODE_DELIMITER, "Delimiter" },
                { GVK_PIPELINE_EXPLORER_QUERY_MODE_DURATION, "Duration (ms)" },
                { GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE, "Toggle" },
            };
            ImGui::SameLine();
            if (ImGui::BeginCombo("##Mode", scQueryModes[guiInfo.queryInterval.mode].c_str())) {
                for (const auto& itr : scQueryModes) {
                    if (itr.first != GVK_PIPELINE_EXPLORER_QUERY_MODE_DELIMITER) {
                        if (ImGui::Selectable(itr.second.c_str(), itr.first == guiInfo.queryInterval.mode)) {
                            guiInfo.queryInterval.mode = itr.first;
                        }
                    }
                }
                ImGui::EndCombo();
            }

            // TODO : Documentation
            switch (guiInfo.queryInterval.mode) {

            case GVK_PIPELINE_EXPLORER_QUERY_MODE_DELIMITER: {
                static std::unordered_map<GvkCommandStructureType, std::string> scQueryRangeDelimiters{
                    { GVK_COMMAND_STRUCTURE_TYPE_QUEUE_SUBMIT, "Queue Submit" },
                    { GVK_COMMAND_STRUCTURE_TYPE_QUEUE_PRESENT_KHR, "Queue Present" },
                };
                ImGui::SameLine();
                if (ImGui::BeginCombo("##Delimiter", scQueryRangeDelimiters[guiInfo.queryInterval.delimiter].c_str())) {
                    for (const auto& itr : scQueryRangeDelimiters) {
                        if (ImGui::Selectable(itr.second.c_str(), itr.first == guiInfo.queryInterval.delimiter)) {
                            guiInfo.queryInterval.delimiter = itr.first;
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();
                auto queryIntervalCount = (int)guiInfo.queryInterval.count;
                ImGui::InputInt("##Count", &queryIntervalCount, 1, 4);
                guiInfo.queryInterval.count = (uint32_t)std::max(0, queryIntervalCount);
            } break;

            case GVK_PIPELINE_EXPLORER_QUERY_MODE_DURATION: {
                ImGui::SameLine();
                ImGui::InputDouble("##Duration", &guiInfo.queryInterval.durationMS, 1.0, 10.0, "%.3f");
            } break;

            case GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE: {
                // NOOP :
            } break;

            default: {
                assert(false);
            } break;
            }

            // TODO : Documentation
            ImGui::SameLine();
            ImGui::BeginDisabled(mQueryActive);
            {
                if (ImGui::Button("Enable Query")) {
#ifdef GVK_PLATFORM_WINDOWS
                    if (guiInfo.workload) {
                        auto pipelineExplorerQueryRequestInfo = gvk::get_default<GvkPipelineExplorerQueryRequestInfo>();
                        pipelineExplorerQueryRequestInfo.interval = guiInfo.queryInterval;
                        guiInfo.ipcMessenger.write("GvkPipelineExplorerQueryRequestInfo", pipelineExplorerQueryRequestInfo);
                    } else {
                        mQueryActive = true;
                    }
#endif
                }
            }
            ImGui::EndDisabled();

            // TODO : Documentation
            ImGui::SameLine();
            ImGui::BeginDisabled(!mQueryActive);
            {
                if (ImGui::Button("Disable Query")) {
#ifdef GVK_PLATFORM_WINDOWS
                    if (guiInfo.workload) {
                        auto pipelineExplorerQueryRequestInfo = gvk::get_default<GvkPipelineExplorerQueryRequestInfo>();
                        pipelineExplorerQueryRequestInfo.interval.mode = GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE;
                        pipelineExplorerQueryRequestInfo.interval.toggle = VK_FALSE;
                        guiInfo.ipcMessenger.write("GvkPipelineExplorerQueryRequestInfo", pipelineExplorerQueryRequestInfo);
                    } else {
                        mQueryActive = false;
                    }
#endif
                }
            }
            ImGui::EndDisabled();

            #if 0
            // TODO : Documentation
            ImGui::Text("Write Metrics Report");
            ImGui::SameLine();
            ImGui::Checkbox("##Report Enabled", &guiInfo.reportEnabled);
            ImGui::SameLine();
            ImGui::PushItemWidth(300.0f);
            ImGui::BeginDisabled();
            auto reportPath = gvk::string::scrub_path((std::filesystem::path(guiInfo.workspace) / "reports").string());
            GvkGui::InputPath("##Report Path", &reportPath);
            ImGui::EndDisabled();
            ImGui::PopItemWidth();
            #endif

            // TODO : Documentation
            contentSize = ImGui::GetWindowSize();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    if (auto* pToolbar = ImGui::FindWindowByName("##Toolbar")) {
        mToolBarHeight = pToolbar->Size.y;
    }
}

void Window::Manager::draw_dockspace(GuiInfo& guiInfo)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    auto pMainViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(pMainViewport->WorkPos.x, pMainViewport->WorkPos.y + mToolBarHeight));
    ImGui::SetNextWindowSize(ImVec2(pMainViewport->WorkSize.x, pMainViewport->WorkSize.y - mToolBarHeight));
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
#if 0
            ImGui::DockBuilderDockWindow("Pipeline Executions", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Up, 0.2f, nullptr, &rootNodeId));
            ImGui::DockBuilderDockWindow("API Call Explorer", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Left, 0.2f, nullptr, &rootNodeId));

            ImGuiID workspaceNodeId = 0;
            ImGui::DockBuilderDockWindow("Workspace", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Down, 0.25f, &workspaceNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Console", ImGui::DockBuilderSplitNode(workspaceNodeId, ImGuiDir_Right, 0.5f, nullptr, &workspaceNodeId));

            ImGuiID pipelinesNodeId = 0;
            ImGui::DockBuilderDockWindow("Pipelines", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Left, 0.333f, &pipelinesNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Selected Pipeline", ImGui::DockBuilderSplitNode(pipelinesNodeId, ImGuiDir_Up, 0.25f, nullptr, &pipelinesNodeId));

            ImGuiID metricsNodeId = 0;
            ImGui::DockBuilderDockWindow("Metrics", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Right, 0.333f, &metricsNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Metrics Charts", ImGui::DockBuilderSplitNode(metricsNodeId, ImGuiDir_Right, 0.5f, nullptr, &metricsNodeId));
#else
            ImGui::DockBuilderDockWindow("Pipeline Executions", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Up, 0.333f, nullptr, &rootNodeId));
            ImGui::DockBuilderDockWindow("API Call Explorer", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Left, 0.2f, nullptr, &rootNodeId));

            ImGuiID workspaceNodeId = 0;
            ImGui::DockBuilderDockWindow("Workspace", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Down, 0.25f, &workspaceNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Console", ImGui::DockBuilderSplitNode(workspaceNodeId, ImGuiDir_Right, 0.5f, nullptr, &workspaceNodeId));

            ImGuiID pipelinesNodeId = 0;
            ImGui::DockBuilderDockWindow("Pipelines", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Left, 0.333f, &pipelinesNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Selected Pipeline", ImGui::DockBuilderSplitNode(pipelinesNodeId, ImGuiDir_Left, 0.25f, nullptr, &pipelinesNodeId));

            ImGuiID metricsNodeId = 0;
            ImGui::DockBuilderDockWindow("Metrics", ImGui::DockBuilderSplitNode(rootNodeId, ImGuiDir_Right, 0.333f, &metricsNodeId, &rootNodeId));
            ImGui::DockBuilderDockWindow("Metrics Charts", ImGui::DockBuilderSplitNode(metricsNodeId, ImGuiDir_Right, 0.5f, nullptr, &metricsNodeId));
#endif
            ImGui::DockBuilderFinish(rootNodeId);
        }
    }
    ImGui::End();
}

void Window::Manager::on_launch(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows)
{
    for (auto& windowItr : windows) {
        windowItr.second->on_launch(guiInfo);
    }
}

void Window::Manager::on_update(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows)
{
    for (auto& windowItr : windows) {
        windowItr.second->on_update(guiInfo);
    }
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

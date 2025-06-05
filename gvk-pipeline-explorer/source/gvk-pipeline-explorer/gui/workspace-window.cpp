
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

#include "gvk-pipeline-explorer/gui/workspace-window.hpp"
#include "gvk-environment.hpp"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace gvk {
namespace pipeline_explorer {
namespace gui {

#ifdef VK_USE_PLATFORM_WIN32_KHR
static void launch_application(GuiInfo& guiInfo)
{
    (void)guiInfo;
    #ifdef VK_USE_PLATFORM_WIN32_KHR

        // TODO : Documentation
        guiInfo.messages.clear();

        // Get the path to this gui .exe, the layer .dll should be next to it
        HMODULE hModule = NULL;
        const size_t CharBufferSize = 16384;
        static std::array<wchar_t, CharBufferSize> wcharBuffer;
        wcharBuffer = { };
        if (get_this_module_handle(&hModule)) {
            GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
        }
        std::filesystem::path guiPath = wcharBuffer[0] ? wcharBuffer.data() : std::filesystem::path();
        std::filesystem::path layerPath = std::filesystem::path(guiPath).remove_filename();

        // TODO : Documentation
        // TODO : Configure this to work with elevated privileges automatically...
#if 1
        gvk::set_env_var("VK_ADD_LAYER_PATH", layerPath.string());
        gvk::set_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_gvk_pipeline_explorer,*validation");
#else
        gvk::set_env_var("ENABLE_VK_LAYER_INTEL_gvk_pipeline_explorer", "1");
        gvk::set_env_var("VK_LOADER_DEBUG", "all");
        gvk::set_env_var("VK_LOADER_LAYERS_ENABLE", "*validation");
#endif

        // NOTE : Disable this to use validation on the tooled app
        // TODO : GVK unique handles
        #if 1
        for (const auto& validationFeatureName : get_validation_layer_setting_names()) {
            if (validationFeatureName != "VK_KHRONOS_VALIDATION_UNIQUE_HANDLES") {
                gvk::set_env_var(validationFeatureName, "false");
            }
        }
        gvk::set_env_var("VK_KHRONOS_VALIDATION_UNIQUE_HANDLES", "true");
        #endif

#if 0
        // TODO : Docmentation
        gvk::set_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE", guiInfo.workspacePath.string());
        gvk::set_env_var("GVK_PIPELINE_EXPLORER_TARGET", get_target(guiInfo.applicationInfo));
        if (guiInfo.applicationInfo.waitForDebugger) {
            gvk::set_env_var("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER", "1");
        }
#else
        // TODO : Docmentation
        gvk::set_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE", guiInfo.workspaceInfo.workspace);
        gvk::set_env_var("GVK_PIPELINE_EXPLORER_TARGET", get_target(guiInfo.workspaceInfo));
        if (guiInfo.workspaceInfo.waitForDebugger) {
            gvk::set_env_var("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER", "1"); // TODO : Turn back off...need to reset all envvars
        }
#endif

        // TODO : Documentation
        STARTUPINFO startupInfo{ };
        startupInfo.cb = sizeof(startupInfo);
        std::string cmdLine = guiInfo.workspaceInfo.launch + " " + guiInfo.workspaceInfo.args;
        if (CreateProcess(
            gvk::string::remove(guiInfo.workspaceInfo.launch, "\"").c_str(),
            cmdLine.data(),
            NULL,
            NULL,
            FALSE,
            0, // (guiInfo.workspaceInfo.openTerminal ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW),
            NULL,
            !guiInfo.workspaceInfo.workingDirectory.empty() ? guiInfo.workspaceInfo.workingDirectory.c_str() : NULL,
            &startupInfo,
            &guiInfo.applicationInfo.processInformation)) {
            guiInfo.messages = "INFO : Launched " + guiInfo.workspaceInfo.launch + "\n";

            for (auto itr = guiInfo.recentWorkspaceInfos.begin(); itr != guiInfo.recentWorkspaceInfos.end(); ++itr) {
                if (guiInfo.workspaceInfo == *itr) {
                    guiInfo.recentWorkspaceInfos.erase(itr);
                    break;
                }
            }
            guiInfo.recentWorkspaceInfos.insert(guiInfo.recentWorkspaceInfos.begin(), guiInfo.workspaceInfo);

            if (!RegisterWaitForSingleObject(
                &guiInfo.applicationInfo.waitHandle,
                guiInfo.applicationInfo.processInformation.hProcess,
                process_wait_callback,
                &guiInfo,
                INFINITE, WT_EXECUTEONLYONCE)) {
                guiInfo.messages = "WARNING : Failed to register application for callback on shutdown\n";
                guiInfo.messages += "    " + get_win32_error_str(GetLastError()) + "\n";
            }
        } else {
            guiInfo.messages = "ERROR : Failed to launch " + guiInfo.workspaceInfo.launch + "\n";
            guiInfo.messages += "    " + get_win32_error_str(GetLastError()) + "\n";
        }
#endif
}
#endif // VK_USE_PLATFORM_WIN32_KHR

WorkspaceWindow::WorkspaceWindow(Window::Manager& windowManager)
    : Window(windowManager, "Workspace")
{
}

void WorkspaceWindow::on_gui(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    // TODO : Documentation
    ImGui::BeginDisabled(guiInfo.applicationInfo.processInformation.hProcess || guiInfo.cliProvidedWorkspace);
    {
        // TODO : Documentation
        if (ImGui::Button("Clear")) {
            guiInfo.workspaceInfo = { };
        }

        // TODO : Documentation
        ImGui::SameLine();
        ImGui::BeginDisabled(guiInfo.recentWorkspaceInfos.empty());
        {
            if (ImGui::Button("Recent")) {
                ImGui::OpenPopup("Recent-Workspace-Popup");
            }
            if (ImGui::BeginPopup("Recent-Workspace-Popup")) {
                auto remove = guiInfo.recentWorkspaceInfos.end();
                for (auto itr = guiInfo.recentWorkspaceInfos.begin(); itr != guiInfo.recentWorkspaceInfos.end(); ++itr) {
                    ImGui::PushID(&*itr);
                    if (ImGui::SmallButton("Remove")) {
                        remove = itr;
                    }
                    ImGui::SameLine();
                    if (ImGui::Selectable((itr->launch + " " + itr->args).c_str())) {
                        guiInfo.workspaceInfo = *itr;
                    }
                    ImGui::PopID();
                }
                if (remove != guiInfo.recentWorkspaceInfos.end()) {
                    guiInfo.recentWorkspaceInfos.erase(remove);
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndDisabled();

        // TODO : Documentation
        ImGui::Checkbox("Wait For Debugger", &guiInfo.workspaceInfo.waitForDebugger);
        #if 0
        ImGui::SameLine();
        ImGui::Checkbox("Open Terminal", &guiInfo.workspaceInfo.openTerminal);
        #endif

        // TODO : Documentation
        ImGui::BeginDisabled(guiInfo.workspaceInfo.launch.empty());
        {
            if (ImGui::Button("Launch")) {
                launch_application(guiInfo);
            }
        }
        ImGui::EndDisabled();

        // TODO : Documentation
        ImGui::PushItemWidth(-FLT_MIN);

        // TODO : Documentation
        ImGui::SameLine();
        if (GvkGui::InputPath("##launch", &guiInfo.workspaceInfo.launch)) {
            if (guiInfo.workspaceInfo.autoWorkingDirectory) {
                guiInfo.workspaceInfo.workingDirectory = get_default_working_directory(guiInfo.workspaceInfo).string();
            }
            if (guiInfo.workspaceInfo.autoWorkspace) {
                guiInfo.workspaceInfo.workspace = get_default_workspace_path(guiInfo.workspaceInfo).string();
            }
        }

        // TODO : Documentation
        ImGui::Text("Target");
        ImGui::SameLine();
        if (GvkGui::InputPath("##target", &guiInfo.workspaceInfo.target) && guiInfo.workspaceInfo.autoWorkspace) {
            guiInfo.workspaceInfo.workspace = get_default_workspace_path(guiInfo.workspaceInfo).string();
        }

        // TODO : Documentation
        ImGui::Text("Args");
        ImGui::SameLine();
        GvkGui::InputPath("##args", &guiInfo.workspaceInfo.args);

        // TODO : Documentation
        ImGui::Text("Working Directory");
        ImGui::SameLine();
        if (ImGui::Checkbox("Auto##workingDirectory", &guiInfo.workspaceInfo.autoWorkingDirectory) && guiInfo.workspaceInfo.autoWorkingDirectory) {
            guiInfo.workspaceInfo.workingDirectory = get_default_working_directory(guiInfo.workspaceInfo).string();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(guiInfo.workspaceInfo.autoWorkingDirectory);
        {
            guiInfo.workspaceInfo.workingDirectory = gvk::string::scrub_path(guiInfo.workspaceInfo.workingDirectory);
            if (GvkGui::InputPath("##workingDirectory", &guiInfo.workspaceInfo.workingDirectory)) {
            }
        }
        ImGui::EndDisabled();

        // TODO : Documentation
        ImGui::Text("Workspace");
        ImGui::SameLine();
        if (ImGui::Checkbox("Auto##workspace", &guiInfo.workspaceInfo.autoWorkspace) && guiInfo.workspaceInfo.autoWorkspace) {
            guiInfo.workspaceInfo.workspace = get_default_workspace_path(guiInfo.workspaceInfo).string();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(guiInfo.workspaceInfo.autoWorkspace);
        {
            auto workspacePathStr = gvk::string::scrub_path(guiInfo.workspaceInfo.workspace);
            if (GvkGui::InputPath("##workspace", &workspacePathStr)) {
                guiInfo.workspaceInfo.workspace = workspacePathStr;
            }
        }
        ImGui::EndDisabled();

        // TODO : Documentation
        ImGui::PopItemWidth();

    }
    ImGui::EndDisabled();
#endif // VK_USE_PLATFORM_WIN32_KHR
}

void WorkspaceWindow::on_save(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void WorkspaceWindow::on_load(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

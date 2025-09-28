
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
#include "gvk-pipeline-explorer/gui/stream-playback-window.hpp"
#include "gvk-pipeline-explorer/gui/window-manager.hpp"
#include "gvk-environment.hpp"
#include "gvk-system.hpp"

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

WorkspaceWindow::WorkspaceWindow(Window::Manager& windowManager)
    : Window(windowManager, "Workspace")
{
}

void WorkspaceWindow::on_gui(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef VK_USE_PLATFORM_WIN32_KHR

    ImGui::BeginDisabled(guiInfo.applicationInfo.processInformation.hProcess || guiInfo.cliProvidedWorkspace || guiInfo.workspaceInfo.streamInfo.running);
    {
        // Workspace options
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

        // Draw tab bar
        if (ImGui::BeginTabBar("Tab Bar"))
        {
            if (ImGui::BeginTabItem("Application")) {
                draw_application_tab(guiInfo);
                ImGui::EndTabItem();
            }
            ImGui::BeginDisabled();
            if (ImGui::BeginTabItem("Stream Playback")) {
                draw_stream_tab(guiInfo);
                ImGui::EndTabItem();
            }
            ImGui::EndDisabled();
            ImGui::EndTabBar();
        }
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

void WorkspaceWindow::draw_application_tab(GuiInfo& guiInfo)
{
    // Clear workspace
    if (ImGui::Button("Clear")) {
        guiInfo.workspaceInfo = { };
    }

    // Recent workspaces
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

    // Launch options
    ImGui::Checkbox("Auto Query", &mAutoQuery);
    ImGui::SameLine();
    ImGui::Checkbox("Wait For Debugger", &guiInfo.workspaceInfo.waitForDebugger);
    #if 0
    ImGui::SameLine();
    ImGui::Checkbox("Open Terminal", &guiInfo.workspaceInfo.openTerminal);
    #endif

    // GITS launch options
    ImGui::SameLine();
    #if GVK_GITS_ENABLED
    ImGui::BeginDisabled(guiInfo.workspaceInfo.gits.empty());
    #else
    ImGui::BeginDisabled();
    #endif // GVK_GITS_ENABLED
    ImGui::Checkbox("Record Stream", &guiInfo.workspaceInfo.record);
    ImGui::EndDisabled();

    // Clear stdout on launch option
    ImGui::SameLine();
    ImGui::Checkbox("Clear StdOut", &mClearStdOut);

    // Layer configuration
    ImGui::SameLine();
    if (ImGui::Button("Layers")) {
        ImGui::OpenPopup("Layers");
    }
    if (ImGui::BeginPopup("Layers")) {
        mActiveLayers.resize(guiInfo.layerProperties.size());
        for (size_t i = 0; i < guiInfo.layerProperties.size(); ++i) {
            ImGui::PushID((int)i);
            bool active = mActiveLayers[i];
            #if 0
            // TODO : Setup layer ordering
            if (ImGui::SmallButton("<")) {
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(">")) {
            }
            ImGui::SameLine();
            #endif
            ImGui::Checkbox(guiInfo.layerProperties[i].layerName, &active);
            mActiveLayers[i] = (int)active;
            ImGui::PopID();
        }
        ImGui::EndPopup();
    }

    // Launch
    ImGui::BeginDisabled(guiInfo.workspaceInfo.launch.empty());
    {
        if (ImGui::Button("Launch")) {
            if (guiInfo.workspaceInfo.gitsStream) {
                #if 0
                launch_gits_stream(guiInfo);
                #endif
            } else {
                launch_application(guiInfo);
            }
        }
    }
    ImGui::EndDisabled();

    // Launch path
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SameLine();
    static bool sOnceDEBUG;
    if (GvkGui::InputPath("##launch", &guiInfo.workspaceInfo.launch) || !sOnceDEBUG) {
        sOnceDEBUG = true;

        #if 0
        guiInfo.workspaceInfo.gitsStream = false;
        if (guiInfo.workspaceInfo.launch.empty()) {
            // TODO : Clear stuff
        } else {
            std::filesystem::path launch = guiInfo.workspaceInfo.launch;
            auto status = std::filesystem::status(launch);
            switch (status.type()) {
            case std::filesystem::file_type::regular: {
                auto exec =
                    std::filesystem::perms::owner_exec |
                    std::filesystem::perms::group_exec |
                    std::filesystem::perms::others_exec;
                if ((status.permissions() & exec) != std::filesystem::perms::none) {

                } else {
                    // TODO : Clear stuff
                }
            } break;
            case std::filesystem::file_type::directory: {
                if (std::filesystem::exists(launch / "stream.gits2")) {
                    guiInfo.workspaceInfo.gitsStream = true;
                    get_window_manager().open<StreamPlaybackWindow>("Stream Playback");
                } else {
                    // TODO : Clear stuff
                }
            } break;
            default: {
                // TODO : Clear stuff
            } break;
            }
        }
        #endif

        ///////////////////////////////////////////////////////////////////////////////
        // TODO : Sort out workspace logic
        ///////////////////////////////////////////////////////////////////////////////
        if (guiInfo.workspaceInfo.autoWorkingDirectory && !guiInfo.cliProvidedWorkspace) {
#ifdef WIN32
            guiInfo.workspaceInfo.workingDirectory = get_default_working_directory(guiInfo.workspaceInfo).string();
#else
            // TODO :
#endif
        }
        if (guiInfo.workspaceInfo.autoWorkspace && !guiInfo.cliProvidedWorkspace) {
#ifdef WIN32
            guiInfo.workspaceInfo.workspace = get_default_workspace_path(guiInfo.workspaceInfo).string();
#else
            // TODO :
#endif
        }
        if (guiInfo.workspaceInfo.autoLogPath && !guiInfo.cliProvidedWorkspace) {
#ifdef WIN32
            guiInfo.workspaceInfo.logPath = get_default_workspace_path(guiInfo.workspaceInfo).string() + "/logs";
#else
            // TODO :
#endif
        }
        ///////////////////////////////////////////////////////////////////////////////
    }

    #if 0
    // NOTE : This is for targeting an app that's run via launcher or script, needs
    //  more work to make it actually usable
    ImGui::Text("Target");
    ImGui::SameLine();
    if (GvkGui::InputPath("##target", &guiInfo.workspaceInfo.target) && guiInfo.workspaceInfo.autoWorkspace) {
        guiInfo.workspaceInfo.workspace = get_default_workspace_path(guiInfo.workspaceInfo).string();
    }
    #endif

    // Workload args
    ImGui::Text("Args");
    ImGui::SameLine();
    GvkGui::InputPath("##args", &guiInfo.workspaceInfo.args);

    ////////////////////////////////////////////////////////////////////////////////
    ImGui::Text("Working Directory");
    ImGui::SameLine();
    if (ImGui::Checkbox("Auto##workingDirectory", &guiInfo.workspaceInfo.autoWorkingDirectory) && guiInfo.workspaceInfo.autoWorkingDirectory) {
#ifdef WIN32
        guiInfo.workspaceInfo.workingDirectory = get_default_working_directory(guiInfo.workspaceInfo).string();
#else
        // TODO :
#endif
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(guiInfo.workspaceInfo.autoWorkingDirectory);
    {
        guiInfo.workspaceInfo.workingDirectory = gvk::string::scrub_path(guiInfo.workspaceInfo.workingDirectory);
        if (GvkGui::InputPath("##workingDirectory", &guiInfo.workspaceInfo.workingDirectory)) {
        }
    }
    ImGui::EndDisabled();

    #if GVK_GITS_ENABLED
    // NOTE : Stream playback features WIP
    ImGui::Text("Gits");
    ImGui::SameLine();
    auto gitsPathStr = gvk::string::scrub_path(guiInfo.workspaceInfo.gits);
    if (GvkGui::InputPath("##gits", &gitsPathStr)) {
        guiInfo.workspaceInfo.gits = gitsPathStr;
        // TODO : Validate gits install
    }
    ImGui::BeginDisabled(guiInfo.workspaceInfo.gitsStream);
    if (guiInfo.workspaceInfo.gitsStream) {
    }
    ImGui::EndDisabled();
    #endif // GVK_GITS_ENABLED

    ////////////////////////////////////////////////////////////////////////////////
    ImGui::Text("Log");
    ImGui::SameLine();
    ImGui::Checkbox("StdOut", &guiInfo.workspaceInfo.logToStdOut);
    ImGui::BeginDisabled(guiInfo.workspaceInfo.logPath.empty());
    ImGui::SameLine();
    if (guiInfo.workspaceInfo.logPath.empty()) {
        guiInfo.workspaceInfo.logToFile = false;
    }
    ImGui::Checkbox("File", &guiInfo.workspaceInfo.logToFile);
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Checkbox("Auto##log", &guiInfo.workspaceInfo.autoLogPath) && guiInfo.workspaceInfo.autoLogPath) {
#ifdef WIN32
        guiInfo.workspaceInfo.logPath = get_default_workspace_path(guiInfo.workspaceInfo).string() + "/logs";
#else
        // TODO :
#endif
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(guiInfo.workspaceInfo.autoLogPath);
    {
        guiInfo.workspaceInfo.logPath = gvk::string::scrub_path(guiInfo.workspaceInfo.logPath);
        if (GvkGui::InputPath("##log", &guiInfo.workspaceInfo.logPath)) {
        }
    }
    ImGui::EndDisabled();

    ////////////////////////////////////////////////////////////////////////////////
    ImGui::PopItemWidth();
}

void WorkspaceWindow::draw_stream_tab(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void WorkspaceWindow::launch_application(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef VK_USE_PLATFORM_WIN32_KHR

    // Clear messages
    guiInfo.messages.clear();

    // Get the path to this gui .exe, the layer .dll should be next to it
    // TODO : DRY with gvk::runtime
    HMODULE hModule = NULL;
    const size_t CharBufferSize = 16384;
    static std::array<wchar_t, CharBufferSize> wcharBuffer;
    wcharBuffer = { };
    if (get_this_module_handle(&hModule)) {
        GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    }
    std::filesystem::path guiPath = wcharBuffer[0] ? wcharBuffer.data() : std::filesystem::path();
    std::filesystem::path layerPath = std::filesystem::path(guiPath).remove_filename();

    // Set active layers
    std::string activeLayers;
    bool validationEnabled = false;
    for (size_t layer_i = 0; layer_i < guiInfo.layerProperties.size() && layer_i < mActiveLayers.size(); ++layer_i) {
        if (mActiveLayers[layer_i]) {
            std::string layerName = guiInfo.layerProperties[layer_i].layerName;
            if (layerName == "VK_LAYER_KHRONOS_validation") {
                validationEnabled = true;
            } else {
                activeLayers += layerName + ",";
            }
        }
    }

    // Prepare Environment
    gvk::Environment env;
    env.set_env(); // TODO : Validate and route existing env to GUI
    env.set_env_var("GVK_PIPELINE_EXPLORER_GUI_PID", std::to_string(GetCurrentProcessId()));

    // Configure Environment for layers
    // TODO : Handle environment that PE is launched from
    env.set_env_var("VK_ADD_LAYER_PATH", layerPath.string());
    if (activeLayers.empty()) {
        env.set_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_gvk_pipeline_explorer,VK_LAYER_KHRONOS_validation");
    } else {
        env.set_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_gvk_pipeline_explorer," + activeLayers + "VK_LAYER_KHRONOS_validation");
    }
    if (validationEnabled) {
        env.set_env_var("VK_LOADER_DEBUG", "all");
    }

    #if 0
    // TODO : Configure this to detect elevated privileges and check registry
    // TODO : Give user option to configure registry
    env.set_env_var("ENABLE_VK_LAYER_INTEL_gvk_pipeline_explorer", "1");
    env.set_env_var("VK_LOADER_DEBUG", "all");
    env.set_env_var("VK_LOADER_LAYERS_ENABLE", "*validation");
    #endif

    #if 0
    if (guiInfo.workspaceInfo.record && !guiInfo.workspaceInfo.gits.empty()) {
        std::filesystem::path gitsLayerPath = guiInfo.workspaceInfo.gits;
        gitsLayerPath /= "Recorder/VulkanLayer";
        env.append_value_to_env_var("VK_ADD_LAYER_PATH", gitsLayerPath.string());
        // env.set_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_gvk_pipeline_explorer,VK_LAYER_INTEL_vulkan_GITS_recorder,*validation");
        // env.set_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_vulkan_GITS_recorder,VK_LAYER_INTEL_gvk_pipeline_explorer,*validation");
        // TODO : WHY is layer order not being honored?
        env.set_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_vulkan_GITS_recorder,VK_LAYER_INTEL_gvk_pipeline_explorer");
    }
    #endif

    // Configure Environment for validation
    // TODO : GVK unique handles
    if (!validationEnabled) {
        for (const auto& validationFeatureName : get_validation_layer_setting_names()) {
            if (validationFeatureName != "VK_KHRONOS_VALIDATION_UNIQUE_HANDLES") {
                env.set_env_var(validationFeatureName, "false");
            }
        }
    }
    env.set_env_var("VK_KHRONOS_VALIDATION_UNIQUE_HANDLES", "true");

    // Configure Environment for PE settings
    env.set_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE", guiInfo.workspaceInfo.workspace);
    env.set_env_var("GVK_PIPELINE_EXPLORER_TARGET", get_target(guiInfo.workspaceInfo));
    if (mAutoQuery) {
        env.set_env_var("GVK_PIPELINE_EXPLORER_AUTO_QUERY", "1");
    }
    if (guiInfo.workspaceInfo.waitForDebugger) {
        env.set_env_var("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER", "1");
    }

    // Get Environment data
    uint32_t envCharCount = 0;
    env.get_env(&envCharCount, nullptr);
    std::vector<char> envData(envCharCount);
    env.get_env(&envCharCount, envData.data());

    // Setup pipes for std in/out/err
    if (!ApplicationInfo::PipePair::create(ApplicationInfo::PipePair::INHERIT_READ, &guiInfo.applicationInfo.stdIn)) {
        assert(false && "Failed to create stdIn pipes : TODO : Error handling");
    }
    if (!ApplicationInfo::PipePair::create(ApplicationInfo::PipePair::INHERIT_WRITE, &guiInfo.applicationInfo.stdOut)) {
        assert(false && "Failed to create stdOut pipes : TODO : Error handling");
    }
    if (!ApplicationInfo::PipePair::create(ApplicationInfo::PipePair::INHERIT_WRITE, &guiInfo.applicationInfo.stdErr)) {
        assert(false && "Failed to create stdErr pipes : TODO : Error handling");
    }
    SetHandleInformation(guiInfo.applicationInfo.stdIn.write, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(guiInfo.applicationInfo.stdOut.read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(guiInfo.applicationInfo.stdErr.read, HANDLE_FLAG_INHERIT, 0);

    // Setup STARTUPINFO
    STARTUPINFO startupInfo{ };
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdInput = guiInfo.applicationInfo.stdIn.read;
    startupInfo.hStdOutput = guiInfo.applicationInfo.stdOut.write;
    startupInfo.hStdError = guiInfo.applicationInfo.stdErr.write;
    if (startupInfo.hStdInput || startupInfo.hStdOutput || startupInfo.hStdError) {
        startupInfo.dwFlags = STARTF_USESTDHANDLES;
    }
    std::string cmdLine = guiInfo.workspaceInfo.launch + " " + guiInfo.workspaceInfo.args;

    // Launch workload
    if (CreateProcess(
        gvk::string::remove(guiInfo.workspaceInfo.launch, "\"").c_str(),
        cmdLine.data(),
        NULL,
        NULL,
        TRUE,
        CREATE_NEW_CONSOLE, // (guiInfo.workspaceInfo.openTerminal ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW),
        !envData.empty() ? envData.data() : NULL,
        !guiInfo.workspaceInfo.workingDirectory.empty() ? guiInfo.workspaceInfo.workingDirectory.c_str() : NULL,
        &startupInfo,
        &guiInfo.applicationInfo.processInformation
    )) {
        guiInfo.messages = "INFO : Launched " + guiInfo.workspaceInfo.launch + "\n";
        for (auto itr = guiInfo.recentWorkspaceInfos.begin(); itr != guiInfo.recentWorkspaceInfos.end(); ++itr) {
            if (guiInfo.workspaceInfo == *itr) {
                guiInfo.recentWorkspaceInfos.erase(itr);
                break;
            }
        }
        guiInfo.recentWorkspaceInfos.insert(guiInfo.recentWorkspaceInfos.begin(), guiInfo.workspaceInfo);

        // Close unnecessary std in/out/err pipes
        if (guiInfo.applicationInfo.stdIn.read) {
            CloseHandle(guiInfo.applicationInfo.stdIn.read);
            guiInfo.applicationInfo.stdIn.read = NULL;
        }
        if (guiInfo.applicationInfo.stdOut.write) {
            CloseHandle(guiInfo.applicationInfo.stdOut.write);
            guiInfo.applicationInfo.stdOut.write = NULL;
        }
        if (guiInfo.applicationInfo.stdErr.write) {
            CloseHandle(guiInfo.applicationInfo.stdErr.write);
            guiInfo.applicationInfo.stdErr.write = NULL;
        }

        // Register callback on workload shutdown
        if (RegisterWaitForSingleObject(
            &guiInfo.applicationInfo.waitHandle,
            guiInfo.applicationInfo.processInformation.hProcess,
            process_wait_callback,
            &guiInfo,
            INFINITE,
            WT_EXECUTEONLYONCE
        )) {
            guiInfo.applicationInfo.running = true;
            if (guiInfo.workspaceInfo.logToFile && !guiInfo.workspaceInfo.logPath.empty()) {
                auto logPath = guiInfo.workspaceInfo.logPath;
                if (!std::filesystem::path(logPath).has_extension()) {
                    auto dateTime = gvk::system::DateTime::now();
                    auto dateStr = dateTime.get_date_str();
                    auto timeStr = dateTime.get_time_str();
                    auto dateTimeStr = gvk::string::replace(dateStr, "/", "-") + "_" + gvk::string::replace(timeStr, ":", "-");
                    logPath += "/" + dateTimeStr + ".log";
                }
                std::filesystem::create_directories(std::filesystem::path(logPath).parent_path());
                guiInfo.logFile.open(logPath, std::ios::out | std::ios::binary);
            }
            if (mClearStdOut) {
                std::cout << "\033[2J\033[1;1H";
            }
            guiInfo.applicationInfo.stdInThread.first = CreateThread(0, 0, process_io_callback, &guiInfo, 0, &guiInfo.applicationInfo.stdInThread.second);
            guiInfo.applicationInfo.stdOutThread.first = CreateThread(0, 0, process_io_callback, &guiInfo, 0, &guiInfo.applicationInfo.stdOutThread.second);
            guiInfo.applicationInfo.stdErrThread.first = CreateThread(0, 0, process_io_callback, &guiInfo, 0, &guiInfo.applicationInfo.stdErrThread.second);
        } else {
            guiInfo.messages = "WARNING : Failed to register workload for callback on shutdown\n";
            guiInfo.messages += "    " + get_win32_error_str(GetLastError()) + "\n";
        }
    } else {
        guiInfo.messages = "ERROR : Failed to launch " + guiInfo.workspaceInfo.launch + "\n";
        guiInfo.messages += "    " + get_win32_error_str(GetLastError()) + "\n";
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

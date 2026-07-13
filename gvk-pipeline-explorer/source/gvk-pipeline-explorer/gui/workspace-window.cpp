
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
#include "gvk-pipeline-explorer/gui/image-window.hpp"
#include "gvk-pipeline-explorer/gui/window-manager.hpp"
#include "gvk-environment.hpp"
#include "gvk-runtime.hpp"
#include "gvk-system.hpp"

#ifdef GVK_PLATFORM_WINDOWS
#include <codecvt>
#include <filesystem>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace gvk {
namespace pipeline_explorer {
namespace gui {

#ifdef GVK_PLATFORM_WINDOWS
static std::string get_target(const LaunchOptions& launchOptions)
{
    if (!launchOptions.exe.empty()) {
        auto applicationName = std::filesystem::path(launchOptions.exe).stem().string();
        return !launchOptions.target.empty() ? launchOptions.target : applicationName;
    }
    return { };
}

static std::filesystem::path get_default_workspace_path(const LaunchOptions& launchOptions)
{
    std::filesystem::path workspacePath;
    if (!launchOptions.exe.empty()) {
        PWSTR pDocumentsPath = NULL;
        auto hResult = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pDocumentsPath);
        auto target = get_target(launchOptions) + "-pipeline-explorer";
        workspacePath = (SUCCEEDED(hResult) && pDocumentsPath) ? std::filesystem::path(pDocumentsPath) / "GPA" / target : target;
        CoTaskMemFree(pDocumentsPath);
    }
    return workspacePath;
}

static std::filesystem::path get_default_working_directory(const LaunchOptions& launchOptions)
{
    std::filesystem::path workingDirectory;
    if (!launchOptions.exe.empty()) {
        workingDirectory = std::filesystem::path(launchOptions.exe).parent_path();
    }
    return workingDirectory;
}
#endif // GVK_PLATFORM_WINDOWS

WorkspaceWindow::WorkspaceWindow(Window::Manager& windowManager)
    : Window(windowManager, "Workspace")
{
    ImGuiSettingsHandler settingsHandler{ };
    settingsHandler.TypeName = "LaunchOptions";
    settingsHandler.TypeHash = ImHashStr(settingsHandler.TypeName);
    settingsHandler.ClearAllFn = im_gui_settings_clear_all;
    settingsHandler.ReadInitFn = im_gui_settings_read_init;
    settingsHandler.ReadOpenFn = im_gui_settings_read_open;
    settingsHandler.ReadLineFn = im_gui_settings_read_line;
    settingsHandler.ApplyAllFn = im_gui_settings_apply_all;
    settingsHandler.WriteAllFn = im_gui_settings_write_all;
    settingsHandler.UserData = this;
    ImGui::AddSettingsHandler(&settingsHandler);
}

const LaunchOptions& WorkspaceWindow::get_launch_options() const
{
    return mLaunchOptions;
}

void WorkspaceWindow::on_gui(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef GVK_PLATFORM_WINDOWS
    bool workloadRunning = guiInfo.workload || guiInfo.cliProvidedWorkspace;
    ImGui::BeginDisabled(workloadRunning);
    {
        // Workspace options
        ImGui::Text("Workspace");
        ImGui::SameLine();
        if (ImGui::Checkbox("Auto##workspace", &mLaunchOptions.autoWorkspace) && mLaunchOptions.autoWorkspace) {
            mLaunchOptions.workspace = get_default_workspace_path(mLaunchOptions).string();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(mLaunchOptions.autoWorkspace);
        {
            auto workspacePathStr = gvk::string::scrub_path(mLaunchOptions.workspace);
            if (GvkGui::InputPath("##workspace", &workspacePathStr)) {
                mLaunchOptions.workspace = workspacePathStr;
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndDisabled();
    // Draw tab bar
    if (ImGui::BeginTabBar("Tab Bar"))
    {
        if (ImGui::BeginTabItem("Application")) {
            draw_application_tab(guiInfo, workloadRunning);
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
#endif // GVK_PLATFORM_WINDOWS
}

void WorkspaceWindow::on_save(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void WorkspaceWindow::on_load(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

void WorkspaceWindow::im_gui_settings_clear_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler)
{
    (void)ctx;
    assert(handler);
    auto& workspaceWindow = *(WorkspaceWindow*)handler->UserData;
    workspaceWindow.mLaunchOptions = { };
    workspaceWindow.mRecentLaunchOptions = { };
}

void WorkspaceWindow::im_gui_settings_read_init(ImGuiContext* ctx, ImGuiSettingsHandler* handler)
{
    (void)ctx;
    assert(handler);
    auto& workspaceWindow = *(WorkspaceWindow*)handler->UserData;
    workspaceWindow.mLaunchOptions = { };
    workspaceWindow.mRecentLaunchOptions = { };
}

void* WorkspaceWindow::im_gui_settings_read_open(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name)
{
    (void)ctx;
    assert(handler);
    assert(handler->UserData);
    assert(name);
    auto& workspaceWindow = *(WorkspaceWindow*)handler->UserData;
    if (!strcmp(name, "WorkspaceWindow::mLaunchOptions")) {
        return &workspaceWindow.mLaunchOptions;
    } else if (gvk::string::contains(name, "WorkspaceWindow::mRecentLaunchOptions")) {
        auto index =
            gvk::string::to_number<uint32_t>(
                gvk::string::remove(gvk::string::remove(gvk::string::remove(
                    name, "WorkspaceWindow::mRecentLaunchOptions"), "["), "]"
                )
            );
        if (workspaceWindow.mRecentLaunchOptions.size() <= index) {
            workspaceWindow.mRecentLaunchOptions.resize(index + 1);
        }
        return &workspaceWindow.mRecentLaunchOptions[index];
    }
    return nullptr;
}

void WorkspaceWindow::im_gui_settings_read_line(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line)
{
    (void)ctx;
    (void)handler;
    assert(entry);
    assert(line);
    auto& launchOptions = *(LaunchOptions*)entry;

    // Get key value pair
    std::string key;
    std::string value;
    std::string str = line;
    auto split = str.find('=');
    if (split != std::string::npos) {
        key = str.substr(0, split);
        if (split + 1 < str.size()) {
            value = str.substr(split + 1);
        }
    }

    // Set launch options
    if (!key.empty()) {
        if (value == "nullptr") {
            value.clear();
        }
        if (key == "exe") {
            launchOptions.exe = value;
        } else if (key == "target") {
            launchOptions.target = value;
        } else if (key == "args") {
            launchOptions.args = value;
        } else if (key == "directory") {
            launchOptions.directory = value;
        } else if (key == "workspace") {
            launchOptions.workspace = value;
        } else if (key == "logPath") {
            launchOptions.logPath = value;
        } else if (key == "waitForDebugger") {
            launchOptions.waitForDebugger = gvk::string::to_number<uint32_t>(value);
        } else if (key == "autoDirectory") {
            launchOptions.autoDirectory = gvk::string::to_number<uint32_t>(value);
        } else if (key == "autoWorkspace") {
            launchOptions.autoWorkspace = gvk::string::to_number<uint32_t>(value);
        } else if (key == "autoLogPath") {
            launchOptions.autoLogPath = gvk::string::to_number<uint32_t>(value);
        } else if (key == "logToStdOut") {
            launchOptions.logToStdOut = gvk::string::to_number<uint32_t>(value);
        } else if (key == "logToFile") {
            launchOptions.logToFile = gvk::string::to_number<uint32_t>(value);
        } else if (key == "loaderDebug") {
            launchOptions.loaderDebug = gvk::string::to_number<uint32_t>(value);
        } else if (key == "clearStdOut") {
            launchOptions.clearStdOut = gvk::string::to_number<uint32_t>(value);
        } else if (key == "uniqueHandles") {
            launchOptions.uniqueHandles = gvk::string::to_number<uint32_t>(value);
        }
    }
}

void WorkspaceWindow::im_gui_settings_apply_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler)
{
    (void)ctx;
    (void)handler;
    // NOOP :
}

void WorkspaceWindow::im_gui_settings_write_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
{
    assert(handler);
    assert(handler->TypeName);
    assert(handler->UserData);
    const auto& workspaceWindow = *(const WorkspaceWindow*)handler->UserData;
    out_buf->appendf("[%s][WorkspaceWindow::mLaunchOptions]\n", handler->TypeName);
    LaunchOptions::im_gui_settings_write(workspaceWindow.mLaunchOptions, ctx, handler, out_buf);
    for (size_t i = 0; i < workspaceWindow.mRecentLaunchOptions.size(); ++i) {
        out_buf->appendf("[%s][WorkspaceWindow::mRecentLaunchOptions[%zu]]\n", handler->TypeName, i);
        LaunchOptions::im_gui_settings_write(workspaceWindow.mRecentLaunchOptions[i], ctx, handler, out_buf);
    }
}

void WorkspaceWindow::draw_application_tab(GuiInfo& guiInfo, bool workloadRunning)
{
    ImGui::BeginDisabled(workloadRunning);
    {
        // Clear workspace
        if (ImGui::Button("Clear")) {
            mLaunchOptions = { };
        }

        // Recent workspaces
        ImGui::SameLine();
        ImGui::BeginDisabled(mRecentLaunchOptions.empty());
        {
            if (ImGui::Button("Recent")) {
                ImGui::OpenPopup("Recent-Workspace-Popup");
            }
            if (ImGui::BeginPopup("Recent-Workspace-Popup")) {
                auto remove = mRecentLaunchOptions.end();
                for (auto itr = mRecentLaunchOptions.begin(); itr != mRecentLaunchOptions.end(); ++itr) {
                    ImGui::PushID(&*itr);
                    if (ImGui::SmallButton("Remove")) {
                        remove = itr;
                    }
                    ImGui::SameLine();
                    if (ImGui::Selectable((itr->exe + " " + itr->args).c_str())) {
                        mLaunchOptions = *itr;
                    }
                    ImGui::PopID();
                }
                if (remove != mRecentLaunchOptions.end()) {
                    mRecentLaunchOptions.erase(remove);
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

#if 0
#ifdef GVK_PLATFORM_WINDOWS
    std::filesystem::path screenshotDir = std::filesystem::path(guiInfo.workspaceInfo.workspace) / "screens";
    ImGui::BeginDisabled(!appRunningState);
    {
        if (ImGui::Button("Take Screenshot")) {
            auto hwnd = find_window_by_pid(guiInfo.workload.get_pid()); //Do we need to pass guiInfo.applicationInfo.processInformation.hProcess too?
            //Get timestamp
            auto dateTime = gvk::system::DateTime::now();
            auto dateStr = dateTime.get_date_str();
            auto timeStr = dateTime.get_time_str();
            auto dateTimeStr = gvk::string::replace(dateStr, "/", "-") + "_" + gvk::string::replace(timeStr, ":", "-");

            //===For future use
            auto windowTitle = get_window_title(hwnd);
            
            if (windowTitle.length() > 50) {
                windowTitle[50] = '\0';
            }
                (void)windowTitle;
            std::string filename = screenshotDir.string() +"/" + dateTimeStr + ".png";
            std::filesystem::create_directories(screenshotDir);
            ImageData windowImage = capture_window_pixels(hwnd);
            if (windowImage.pixels.empty())
            {
                guiInfo.messages += "ERROR: A problem occured when taking a screenshot.";
            }
            else
            {
                save_pixels_to_png(windowImage, filename);
            }
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!std::filesystem::exists(screenshotDir) || !std::filesystem::is_directory(screenshotDir)
        || std::filesystem::is_empty(screenshotDir));
    {
        if (ImGui::Button("Screenshots")) {
            ImGui::OpenPopup("Screenshots-Popup");

            for (const auto& entry : std::filesystem::directory_iterator(screenshotDir)) {
                if (entry.is_regular_file()) {
                    if (gvk::string::to_lower(entry.path().extension().string()) == ".png") {
                        guiInfo.screenshots.insert(entry.path().filename().string());
                    }
                }
            }
        }
        if (ImGui::BeginPopup("Screenshots-Popup")) {
            auto remove = guiInfo.screenshots.end();
            for (auto itr = guiInfo.screenshots.begin(); itr != guiInfo.screenshots.end(); ++itr) {
                ImGui::PushID(&*itr);
                if (ImGui::SmallButton("Delete")) {
                    remove = itr;
                }
                ImGui::SameLine();
                if (ImGui::Selectable((*itr).c_str())) {
                    get_window_manager().open<ImageWindow>(ImageWindow::get_name(*itr), *itr);
                }
                ImGui::PopID();
            }
            if (remove != guiInfo.screenshots.end()) {
                try {
                    if (std::filesystem::remove(screenshotDir.string() + "/" + *remove)) {
                        guiInfo.messages += "File removed successfully: " + *remove + "\n";
                    } else {
                        guiInfo.messages += "File not found or could not be removed: " + *remove + "\n";
                    }
                }
                catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
                guiInfo.screenshots.erase(remove);
            }
            ImGui::EndPopup();
        }
    }
    ImGui::EndDisabled();
#endif // GVK_PLATFORM_WINDOWS
#endif

    ImGui::BeginDisabled(workloadRunning);
    {
        // Launch options
#if 0
        ImGui::Checkbox("Auto Query", &mAutoQuery);
        ImGui::SameLine();
#endif
        ImGui::Checkbox("Wait For Debugger", &mLaunchOptions.waitForDebugger);
#if 0
        ImGui::SameLine();
        ImGui::Checkbox("Open Terminal", &guiInfo.workspaceInfo.openTerminal);
#endif

#if 0
        // GITS launch options
        ImGui::SameLine();
#if GVK_GITS_ENABLED
        ImGui::BeginDisabled(guiInfo.workspaceInfo.gits.empty());
#else
        ImGui::BeginDisabled();
#endif // GVK_GITS_ENABLED
        ImGui::Checkbox("Record Stream", &guiInfo.workspaceInfo.record);
        ImGui::EndDisabled();
#endif

        // Vulkan loader debug
        ImGui::SameLine();
        ImGui::Checkbox("Vulkan Loader Debug", &mLaunchOptions.loaderDebug);

        // Clear stdout on launch option
        ImGui::SameLine();
        ImGui::Checkbox("Clear StdOut", &mLaunchOptions.clearStdOut);

        // Enable validation layer unique handles
        ImGui::SameLine();
        ImGui::Checkbox("Unique Handles", &mLaunchOptions.uniqueHandles);

        // Layer configuration
        ImGui::SameLine();
        if (ImGui::Button("Layers")) {
            ImGui::OpenPopup("Layers");
        }
        if (ImGui::BeginPopup("Layers")) {
            mEnabledLayers.resize(guiInfo.layerProperties.size());
            for (size_t i = 0; i < guiInfo.layerProperties.size(); ++i) {
                ImGui::PushID((int)i);
                bool active = mEnabledLayers[i];
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
                mEnabledLayers[i] = (int)active;
                ImGui::PopID();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::EndDisabled();

    ImGui::PushItemWidth(-FLT_MIN);

#ifdef GVK_PLATFORM_WINDOWS
    // Launch/stop
    if (!guiInfo.workload.running() && !guiInfo.onWorkloadShutdown) {
        if (ImGui::Button("Launch")) { // TODO : Disable if field is empty?
            launch_workload(guiInfo);
        }
    } else if (guiInfo.workload.running()) {
        if (ImGui::Button("Stop")) {
            guiInfo.workload.reset();
        }
    }
#endif // GVK_PLATFORM_WINDOWS

    ImGui::BeginDisabled(workloadRunning);
    {
        // Launch path
        ImGui::SameLine();
        if (GvkGui::InputPath("##launch", &mLaunchOptions.exe)) {

#if 0
            guiInfo.workspaceInfo.gitsStream = false;
            if (guiInfo.workspaceInfo.launch.empty()) {
                // TODO : Clear stuff
            }
            else {
                std::filesystem::path launch = guiInfo.workspaceInfo.launch;
                auto status = std::filesystem::status(launch);
                switch (status.type()) {
                case std::filesystem::file_type::regular: {
                    auto exec =
                        std::filesystem::perms::owner_exec |
                        std::filesystem::perms::group_exec |
                        std::filesystem::perms::others_exec;
                    if ((status.permissions() & exec) != std::filesystem::perms::none) {

                    }
                    else {
                        // TODO : Clear stuff
                    }
                } break;
                case std::filesystem::file_type::directory: {
                    if (std::filesystem::exists(launch / "stream.gits2")) {
                        guiInfo.workspaceInfo.gitsStream = true;
                        get_window_manager().open<StreamPlaybackWindow>("Stream Playback");
                    }
                    else {
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
            if (mLaunchOptions.autoDirectory && !guiInfo.cliProvidedWorkspace) {
#ifdef WIN32
                mLaunchOptions.directory = get_default_working_directory(mLaunchOptions).string();
#else
                // TODO :
#endif
            }
            if (mLaunchOptions.autoWorkspace && !guiInfo.cliProvidedWorkspace) {
#ifdef WIN32
                mLaunchOptions.workspace = get_default_workspace_path(mLaunchOptions).string();
#else
                // TODO :
#endif
            }
            if (mLaunchOptions.autoLogPath && !guiInfo.cliProvidedWorkspace) {
#ifdef WIN32
                mLaunchOptions.logPath = get_default_workspace_path(mLaunchOptions).string() + "/logs";
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
#if 0
        GvkGui::InputPath("##args", &guiInfo.workspaceInfo.args);
#else
        GvkGui::InputPath("##args", &mLaunchOptions.args);
#endif

        ////////////////////////////////////////////////////////////////////////////////
        ImGui::Text("Working Directory");
        ImGui::SameLine();
        if (ImGui::Checkbox("Auto##workingDirectory", &mLaunchOptions.autoDirectory) && mLaunchOptions.autoDirectory) {
#ifdef WIN32
            mLaunchOptions.directory = get_default_working_directory(mLaunchOptions).string();
#else
            // TODO :
#endif
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(mLaunchOptions.autoDirectory);
        {
            mLaunchOptions.directory = gvk::string::scrub_path(mLaunchOptions.directory);
            if (GvkGui::InputPath("##workingDirectory", &mLaunchOptions.directory)) {
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
        ImGui::Checkbox("StdOut", &mLaunchOptions.logToStdOut);
        ImGui::BeginDisabled(mLaunchOptions.logPath.empty());
        ImGui::SameLine();
        if (mLaunchOptions.logPath.empty()) {
            mLaunchOptions.logToFile = false;
        }
        ImGui::Checkbox("File", &mLaunchOptions.logToFile);
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Checkbox("Auto##log", &mLaunchOptions.autoLogPath) && mLaunchOptions.autoLogPath) {
#ifdef WIN32
            mLaunchOptions.logPath = get_default_workspace_path(mLaunchOptions).string() + "/logs";
#else
            // TODO :
#endif
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(mLaunchOptions.autoLogPath);
        {
            mLaunchOptions.logPath = gvk::string::scrub_path(mLaunchOptions.logPath);
            if (GvkGui::InputPath("##log", &mLaunchOptions.logPath)) {
            }
        }
        ImGui::EndDisabled();

        ////////////////////////////////////////////////////////////////////////////////
    }
    ImGui::PopItemWidth();
    ImGui::EndDisabled();
}

void WorkspaceWindow::draw_stream_tab(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

#ifdef GVK_PLATFORM_WINDOWS
void WorkspaceWindow::on_workload_io(const gvk::ChildProcess& childProcess, size_t dataSize, const char* pData)
{
    auto pGuiInfo = (GuiInfo*)childProcess.get_user_data();
    assert(pGuiInfo);
    pGuiInfo->logFile.write(pData, dataSize);
    pGuiInfo->logFile.flush();
    std::cout.write(pData, dataSize);
    std::cout.flush();
}

void WorkspaceWindow::on_workload_shutdown(const gvk::ChildProcess& childProcess)
{
    auto pGuiInfo = (GuiInfo*)childProcess.get_user_data();
    assert(pGuiInfo);
    std::lock_guard<std::mutex> lock(pGuiInfo->workloadMutex);
    pGuiInfo->onWorkloadShutdown = [pGuiInfo]()
    {
        if (gvk::is_process_elevated()) {
            gvk::unregister_explicit_layer(GVK_PIPELINE_EXPLORER_LAYER_NAME);
        }

        // assert(pGuiInfo->pWindowManager);
        // pGuiInfo->pWindowManager->reset();
#if 0
        // TODO : Keeping workspace up on shutdown, make this optional
        pGuiInfo->workspace.clear();
#endif
#ifdef GVK_PLATFORM_WINDOWS
        pGuiInfo->ipcPipe.reset();
        pGuiInfo->ipcMessenger.reset();
        pGuiInfo->logFile.close();
#endif
    };
    pGuiInfo->messages += "[INFO] : Terminated '" + childProcess.get_cmd_line() + "'\n";
    std::cout << std::endl << std::endl << "[INFO] : Terminated '" + childProcess.get_cmd_line() << std::endl << std::endl;
}
#endif // GVK_PLATFORM_WINDOWS

void WorkspaceWindow::launch_workload(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef GVK_PLATFORM_WINDOWS

    // TODO : Need to break this out to a utility class so that other tools (ie. UC/VTune)
    //  can launch workloads with the same configuration and IPC mechanism

#if 0
    // Clear messages
    guiInfo.messages.clear();
#else
    guiInfo.reset();
    guiInfo.rangeInfo = { };
    get_window_manager().reset();
    guiInfo.onWorkloadShutdown = nullptr;
#endif

    // Clear stdout
    if (mLaunchOptions.clearStdOut) {
        std::cout << "\033[2J\033[1;1H" << std::flush;
    }

    // Set workspace
    guiInfo.workspace = mLaunchOptions.workspace;

    // Check if the workload is already in the recently launched list, and if so erase
    //  it, then add the launched workload to the front of the list
    for (auto itr = mRecentLaunchOptions.begin(); itr != mRecentLaunchOptions.end(); ++itr) {
        if (itr->exe == mLaunchOptions.exe && itr->args == mLaunchOptions.args) {
            mRecentLaunchOptions.erase(itr);
            break;
        }
    }
    mRecentLaunchOptions.insert(mRecentLaunchOptions.begin(), mLaunchOptions);

    // Create workpsace directory if it doesn't exist
    auto workspaceDataDirectory = std::filesystem::path(guiInfo.workspace) / ".data";
    if (!std::filesystem::exists(workspaceDataDirectory)) {
        std::error_code errorCode{ };
        std::filesystem::create_directories(workspaceDataDirectory, errorCode);
        if (errorCode || !std::filesystem::exists(workspaceDataDirectory)) {

            // TODO : Errors should halt launch and be more visible to the user, this is
            //  just a placeholder for now
            guiInfo.messages += "[ERROR] : Failed to create workspace data directory '" + guiInfo.workspace + "' : " + errorCode.message() + "\n";
        }
    }

    // TODO : Documentation
    get_window_manager().on_launch(guiInfo);

    // Prepare environment object and load current environment
    // NOTE : If running with elevated privileges, layers must be added to the Windows
    //  Vulkan layer registry to satisfy Vulkan loader security requirements
    //  HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\ExplicitLayers
    gvk::Environment environment{ };
    environment.load_env();

    // Clear layer variables from environment
    environment.unset_env_var("VK_INSTANCE_LAYERS");
    environment.unset_env_var("VK_LOADER_LAYERS_ENABLE");
    for (const auto& validationLayerSetting : gvk::get_validation_layer_settings()) {
        environment.unset_env_var(validationLayerSetting);
    }

    // Get layer path (layer binary and JSON should be next to this executable binary)
    //  and set VK_ADD_LAYER_PATH
    std::filesystem::path layerPath;
    (void)gvk::get_this_module_path(&layerPath);
    layerPath.remove_filename();
    if (is_process_elevated()) {

        // Enable pipeline explorer as explicit layer
        // TODO : Capture .hiv and restore on exit to avoid leaving layer registered if
        //  process is shutdown ungracefully
        auto pipelineExplorerJsonPath = layerPath / GVK_PIPELINE_EXPLORER_LAYER_NAME ".json";
        if (std::filesystem::exists(pipelineExplorerJsonPath)) {
            gvk::unregister_explicit_layer(GVK_PIPELINE_EXPLORER_LAYER_NAME);
            if (gvk::register_explicit_layer(pipelineExplorerJsonPath)) {
                guiInfo.messages += "[INFO] : Running with elevated privileges; " GVK_PIPELINE_EXPLORER_LAYER_NAME " added to Windows Vulkan layer registry\n";
            } else {

                // TODO : Errors should halt launch and be more visible to the user
                guiInfo.messages += "[ERROR] : Failed to add " GVK_PIPELINE_EXPLORER_LAYER_NAME " to Windows Vulkan layer registry\n";
            }
        }
    } else {
        environment.append_value_to_env_var("VK_ADD_LAYER_PATH", layerPath.string());
    }

    // Prepare collection of layers to enable when launching the workload
    std::vector<std::string> layers{ GVK_PIPELINE_EXPLORER_LAYER_NAME };

    // Add user enabled layers
    bool apiDumpEnabled = false;
    bool validationEnabled = false;
    for (size_t layer_i = 0; layer_i < mEnabledLayers.size(); ++layer_i) {
        assert(layer_i < guiInfo.layerProperties.size());
        if (mEnabledLayers[layer_i]) {
            std::string layerName = guiInfo.layerProperties[layer_i].layerName;
            if (layerName == "VK_LAYER_LUNARG_api_dump") {
                apiDumpEnabled = true;
            } else if (layerName == "VK_LAYER_KHRONOS_validation") {
                validationEnabled = guiInfo.validationLayerAvailable;
            } else {
                layers.push_back(layerName);
            }
        }
    }

    // Add api dump and validation to the end of the list if enabled
    if (apiDumpEnabled) {
        layers.push_back("VK_LAYER_LUNARG_api_dump");
    }
    if (apiDumpEnabled || validationEnabled || mLaunchOptions.uniqueHandles) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        if (!validationEnabled) {
            // NOTE : When api dump or unique handles are enabled, the validation layer is loaded
            //  with all features disabled.  When VK_LAYER_INTEL_gvk_pipeline_explorer is loaded,
            //  it will check if VK_LAYER_KHRONOS_validation is loaded and enable the validation
            //  layer's unique handles feature.
            // NOTE : The logic here turns on the validation layer when api dump is enabled
            //  so that all layers above it have the same handles as the api dump output.
            for (const auto& validationLayerSetting : gvk::get_validation_layer_settings()) {
                environment.set_env_var(validationLayerSetting, "false");
            }
        }
    }

    // Warn if validation layer is unavailable or unique handles is disabled
    if (!guiInfo.validationLayerAvailable) {
        guiInfo.messages += "[WARNING] : Validation layer unavailable (required for unique handles); may result in instability\n";
        guiInfo.messages += "    Install the Vulkan SDK or set environment variable `VK_ADD_LAYER_PATH`\n";
        guiInfo.messages += "    If GVK was built from source, the Vulkan SDK installer can be found in `gvk/build/_deps/VulkanSDK`\n";
    }
    if (!mLaunchOptions.uniqueHandles) {
        guiInfo.messages += "[WARNING] : Unique handles disabled; may result in instability\n";
    }

    // Create enabled layer list and set environment variable
    std::string layersStr;
    for (const auto& layer : layers) {
        layersStr += !layersStr.empty() ? ("," + layer) : layer;
    }
    if (is_process_elevated()) {
        layersStr = gvk::string::replace(layersStr, ",", ";");
        environment.set_env_var("VK_INSTANCE_LAYERS", layersStr);
    } else {
        environment.set_env_var("VK_LOADER_LAYERS_ENABLE", layersStr);
    }

    // Set VK_LOADER_DEBUG
    if (mLaunchOptions.loaderDebug) {
        environment.set_env_var("VK_LOADER_DEBUG", "all");
    }

    // Create named IPC pipe — name is a string so it survives intermediate script launchers
    auto ipcPipeName = std::string("\\\\.\\pipe\\gvk-pe-") + std::to_string(GetCurrentProcessId());
    gvk::NamedPipe::ServerCreateInfo ipcPipeCreateInfo{ };
    ipcPipeCreateInfo.pName = ipcPipeName.c_str();
    auto ipcPipeSuccess = gvk::NamedPipe::create_server(&ipcPipeCreateInfo, &guiInfo.ipcPipe);

    // Set Pipeline Explorer environment variables
    environment.set_env_var("GVK_PIPELINE_EXPLORER_TARGET", get_target(mLaunchOptions));
    environment.set_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE", mLaunchOptions.workspace);
    environment.set_env_var("GVK_PIPELINE_EXPLORER_GUI_PID", std::to_string(GetCurrentProcessId()));
    environment.set_env_var("GVK_PIPELINE_EXPLORER_IPC_PIPE_NAME", ipcPipeName);
    environment.set_env_var("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER", std::to_string(mLaunchOptions.waitForDebugger));

    // Turn on VVL unique handles unconditionally, if the validation layer is loaded
    //  then it needs to be on, if it's not loaded setting it doesn't impact anything
    environment.set_env_var("VK_KHRONOS_VALIDATION_UNIQUE_HANDLES", "true");

    // Get Environment data
    uint32_t envCharCount = 0;
    environment.get_env(&envCharCount, nullptr);
    std::vector<char> envData(envCharCount);
    environment.get_env(&envCharCount, envData.data());

    // Setup gvk::ChildProcess::CreateInfo
    gvk::ChildProcess::CreateInfo childProcessCreateInfo{ };
    auto cmdLine = mLaunchOptions.exe + (!mLaunchOptions.args.empty() ? " " + mLaunchOptions.args : std::string());
    childProcessCreateInfo.pCmdLine = cmdLine.data();
    childProcessCreateInfo.pDirectory = !mLaunchOptions.directory.empty() ? mLaunchOptions.directory.c_str() : nullptr;
    childProcessCreateInfo.pEnvironment = !envData.empty() ? envData.data() : nullptr;
    if (mLaunchOptions.logToFile && !mLaunchOptions.logPath.empty()) {
        std::filesystem::path logPath = mLaunchOptions.logPath;
        if (!std::filesystem::path(logPath).has_extension()) {
            auto exeName = std::filesystem::path(mLaunchOptions.exe).stem();
            auto dateTime = gvk::system::DateTime::now();
            auto dateTimeStr = dateTime.get_date_str('-') + "_" + dateTime.get_time_str('-');
            // TODO : Log should land in workspace...name needs less info
            logPath /= exeName.string() + "_gvk-pipeline-explorer_" + dateTimeStr + ".log";
        }
        std::filesystem::create_directories(std::filesystem::path(logPath).parent_path());
        guiInfo.logFile.open(logPath, std::ios::out | std::ios::binary);
        if (guiInfo.logFile.is_open()) {
            guiInfo.messages += "[INFO] : Opened log \"" + logPath.string() + "\"\n";

            // TODO : Make any combination of stdout/log work as expected
            childProcessCreateInfo.pFnOnStdOut = on_workload_io;
            childProcessCreateInfo.pFnOnStdErr = on_workload_io;
        } else {
            guiInfo.messages += "[WARNING] : Failed to open log \"" + logPath.string() + "\"\n";
        }
    }
    childProcessCreateInfo.pFnOnShutdown = on_workload_shutdown;
    childProcessCreateInfo.pUserData = &guiInfo;

    // Create child process
    if (ipcPipeSuccess && gvk::ChildProcess::create(&childProcessCreateInfo, &guiInfo.workload)) {
        guiInfo.messages += "[INFO] : Launched '" + cmdLine + "'\n";
        std::cout << std::endl << std::endl << "[INFO] : Launched '" + cmdLine << std::endl << std::endl;

        // Save settings
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);

        // Start waiting for the layer to connect as a named pipe client
        guiInfo.ipcPipe.begin_accept();

        // TODO : Documentation
        get_window_manager().on_launch(guiInfo);
    } else {
        if (!ipcPipeSuccess) {
            guiInfo.messages += "[ERROR] : Failed to create IPC named pipe\n";
        }
        guiInfo.messages += "[ERROR] : Failed to launch '" + cmdLine + "'\n";
        guiInfo.messages += "    " + gvk::get_win32_error_str(GetLastError()) + "\n";
        guiInfo.ipcPipe.reset();
    }

#endif // GVK_PLATFORM_WINDOWS
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

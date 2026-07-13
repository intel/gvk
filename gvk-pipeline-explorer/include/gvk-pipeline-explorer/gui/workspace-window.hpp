
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

#pragma once

#include "gvk-pipeline-explorer/gui/launch-options.hpp"
#include "gvk-pipeline-explorer/gui/window.hpp"
#include "gvk-runtime/child-process.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

class WorkspaceWindow final
    : public Window
{
public:
    WorkspaceWindow(Window::Manager& windowManager);
    const LaunchOptions& get_launch_options() const;

protected:
    void on_gui(GuiInfo& guiInfo) override final;
    void on_save(GuiInfo& guiInfo) override final;
    void on_load(GuiInfo& guiInfo) override final;

private:
    static void im_gui_settings_clear_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    static void im_gui_settings_read_init(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    static void* im_gui_settings_read_open(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
    static void im_gui_settings_read_line(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
    static void im_gui_settings_apply_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    static void im_gui_settings_write_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf);
    void draw_application_tab(GuiInfo& guiInfo, bool appRunningState);
    void draw_stream_tab(GuiInfo& guiInfo);
#ifdef GVK_PLATFORM_WINDOWS
    static void on_workload_io(const gvk::ChildProcess& childProcess, size_t dataSize, const char* pData);
    static void on_workload_shutdown(const gvk::ChildProcess& childProcess);
#endif
    void launch_workload(GuiInfo& guiInfo);

    LaunchOptions mLaunchOptions;
    std::vector<LaunchOptions> mRecentLaunchOptions;
    std::vector<int> mEnabledLayers;
};

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

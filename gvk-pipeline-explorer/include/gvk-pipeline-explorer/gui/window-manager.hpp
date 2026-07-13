
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

#include "gvk-pipeline-explorer/gui/window.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

class WorkspaceWindow;

class Window::Manager final
{
public:
    Manager();
    void reset();
    void clear();
    bool query_active() const;
    void on_launch(GuiInfo& guiInfo);
    void on_terminate(GuiInfo& guiInfo);
    void on_update(GuiInfo& guiInfo);
    void on_gui(GuiInfo& guiInfo);
    void on_save(GuiInfo& guiInfo);
    void on_load(GuiInfo& guiInfo);

    template <typename WindowType, typename ...Args>
    inline void open(const std::string& name, Args&&... args)
    {
        auto itr = mWindows.find(name);
        if (itr == mWindows.end()) {
            itr = mNewWindows.find(name);
            if (itr == mNewWindows.end()) {
                auto upWindow = std::make_unique<WindowType>(*this, name, std::forward<Args>(args)...);
                itr = mNewWindows.insert({ upWindow->mName, std::move(upWindow) }).first;
            }
        }
        ImGui::SetWindowFocus(name.c_str());
        itr->second->mOpen = true;
    }

private:
    void draw_menu_bar(GuiInfo& guiInfo);
    void draw_tool_bar(GuiInfo& guiInfo);
    void draw_dockspace(GuiInfo& guiInfo);
    void on_launch(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows);
    void on_update(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows);
    void on_gui(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows);
    void on_save(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows);
    void on_load(GuiInfo& guiInfo, std::map<std::string, std::unique_ptr<Window>>& windows);

    std::map<std::string, std::unique_ptr<Window>> mCoreWindows;
    std::map<std::string, std::unique_ptr<Window>> mWindows;
    std::map<std::string, std::unique_ptr<Window>> mNewWindows;
    WorkspaceWindow* mpWorkspaceWindow{ };

    float mMenuBarHeight{ };
    float mToolBarHeight{ };
    bool mImGuiDebugWindowsEnabled{ };
    bool mImGuiMetricsWindowEnabled{ };
    bool mImGuiDebugLogWindowEnabled{ };
    bool mImGuiIDStackToolWindowEnabled{ };
    bool mImGuiDemoWindowEnabled{ };
    bool mImGuiAboutWindowEnabled{ };
    bool mQueryActive{ };

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
};

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

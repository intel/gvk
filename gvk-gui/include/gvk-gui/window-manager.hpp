
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

#include "gvk-gui/window.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace gvk {
namespace gui {

template <typename GuiInfoType>
class Window<GuiInfoType>::Collection
{
public:
    const Window<GuiInfoType>* get(const std::string& name) const
    {
        auto indexItr = mWindowIndices.find(name);
        if (indexItr != mWindowIndices.end()) {
            assert(indexItr->second < mWindows.size());
            return mWindows[indexItr->second].get();
        }
        return nullptr;
    }

    Window<GuiInfoType>* get(const std::string& name)
    {
        auto pConstThis = const_cast<const Collection*>(this);
        return const_cast<Window<GuiInfoType>*>(pConstThis->get(name));
    }

    bool empty() const
    {
        return mWindows.empty();
    }

    auto begin()
    {
        return mWindows.begin();
    }

    auto end()
    {
        return mWindows.end();
    }

    auto rbegin()
    {
        return mWindows.rbegin();
    }

    auto rend()
    {
        return mWindows.rend();
    }

    template <typename WindowType>
    inline Window<GuiInfoType>* add(std::unique_ptr<WindowType>&& upWindow)
    {
        auto indexItr = mWindowIndices.find(upWindow->get_name());
        if (indexItr == mWindowIndices.end()) {
            indexItr = mWindowIndices.insert({ upWindow->get_name(), mWindows.size() }).first;
            mWindows.push_back(std::move(upWindow));
        } else {
            assert(indexItr->second < mWindows.size());
            mWindows[indexItr->second] = std::move(upWindow);
        }
        return mWindows[indexItr->second].get();
    }

    inline size_t erase(const std::string& name)
    {
        auto indexItr = mWindowIndices.find(name);
        if (indexItr != mWindowIndices.end()) {
            assert(indexItr->second < mWindows.size());
            mWindows.erase(mWindows.begin() + indexItr->second);
            mWindowIndices.erase(name);
            return 1;
        }
        return 0;
    }

    inline void merge(Window<GuiInfoType>::Collection&& other)
    {
        for (auto&& window : other.mWindows) {
            add(std::move(window));
        }
        other.mWindowIndices.clear();
        other.mWindows.clear();
    }

private:
    std::map<std::string, size_t> mWindowIndices;
    std::vector<std::unique_ptr<Window<GuiInfoType>>> mWindows;
};

template <typename GuiInfoType>
class Window<GuiInfoType>::Manager
{
public:
    inline Manager(const std::string& name)
        : mName { name }
    {
    }

    virtual ~Manager() = 0;

    virtual void reset()
    {
        for (auto& window : mWindows) {
            window->reset();
        }
        for (auto& window : mNewWindows) {
            window->reset();
        }
    }

    Window<GuiInfoType>* get(const std::string& name)
    {
        auto pWindow = mWindows.get(name);
        return pWindow ? pWindow : mNewWindows.get(name);
    }

    const Window<GuiInfoType>* get(const std::string& name) const
    {
        auto pWindow = mWindows.get(name);
        return pWindow ? pWindow : mNewWindows.get(name);
    }

    bool empty()
    {
        return mWindows.empty();
    }

    auto begin()
    {
        return mWindows.begin();
    }

    auto end()
    {
        return mWindows.end();
    }

    auto rbegin()
    {
        return mWindows.rbegin();
    }

    auto rend()
    {
        return mWindows.rend();
    }

    inline virtual void on_save(GuiInfoType guiInfo)
    {
        (void)guiInfo;
    }

    inline virtual void on_exit(GuiInfoType guiInfo)
    {
        (void)guiInfo;
    }

    inline virtual void on_gui(GuiInfoType guiInfo)
    {
        // Merge windows spawned on the previous frame
        mWindows.merge(std::move(mNewWindows));

        // Draw main menu bar
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::Button("Save")) {
                on_save(guiInfo);
            }
            if (ImGui::Button("Exit")) {
                on_exit(guiInfo);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            for (auto& window : mWindows) {
                
                ImGui::Checkbox(window->mName.c_str(), &window->mOpen);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        // Setup root window
        auto pMainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(pMainViewport->WorkPos);
        ImGui::SetNextWindowSize(pMainViewport->WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::Begin(mName.c_str(), nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoDocking
        );
        {
            ImGui::PopStyleVar(2);
            auto dockspaceRootId = ImGui::GetID("root-dockspace");
            ImGui::DockSpace(dockspaceRootId);
            if (!mDefaultDockspaceSetup) {
                mDefaultDockspaceSetup = true;
                setup_default_dockspace(dockspaceRootId);
            }
        }
        ImGui::End();

        // Draw Windows
        for (auto& window : mWindows) {
            if (window->mOpen) {
                if (ImGui::Begin(window->mName.c_str(), &window->mOpen)) {
                    window->on_gui(guiInfo);
                }
                ImGui::End();
            }
        }
    }

    template <typename WindowType, typename ...Args>
    inline Window<GuiInfoType>& open(const std::string& name, Args&&... args)
    {
        auto pWindow = mWindows.get(name);
        if (!pWindow) {
            pWindow = mNewWindows.get(name);
            if (!pWindow) {
                auto upWindow = std::make_unique<WindowType>(*this, std::forward<Args>(args)...);
                pWindow = mNewWindows.add(std::move(upWindow));
            }
        }
        assert(pWindow->get_name() == name);
        ImGui::SetWindowFocus(name.c_str());
        pWindow->mOpen = true;
        return *pWindow;
    }

protected:
    inline virtual void setup_default_dockspace(ImGuiID dockspaceRootId)
    {
        ImGui::DockBuilderRemoveNode(dockspaceRootId);
        ImGui::DockBuilderAddNode(dockspaceRootId);
        if (!empty()) {
            auto windowItr = begin();
            ImGui::DockBuilderDockWindow((*windowItr)->get_name().c_str(), dockspaceRootId);
            auto direction = ImGuiDir_Right;
            auto dockspaceId = dockspaceRootId;
            for (; windowItr != end(); ++windowItr) {
                ImGui::DockBuilderDockWindow((*windowItr)->get_name().c_str(), ImGui::DockBuilderSplitNode(dockspaceId, direction, 0.333f, nullptr, nullptr));
                switch (direction) {
                case ImGuiDir_Left: {
                    direction = ImGuiDir_Up;
                } break;
                case ImGuiDir_Right: {
                    direction = ImGuiDir_Down;
                } break;
                case ImGuiDir_Up: {
                    direction = ImGuiDir_Right;
                } break;
                case ImGuiDir_Down: {
                    direction = ImGuiDir_Left;
                } break;
                default: {
                    assert(false);
                } break;
                }
            }
        }
    }

private:
    std::string mName;
    Window<GuiInfoType>::Collection mWindows;
    Window<GuiInfoType>::Collection mNewWindows;
    bool mDefaultDockspaceSetup{ };

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
};

template <typename GuiInfoType>
Window<GuiInfoType>::Manager::~Manager()
{
}

} // namespace gui
} // namespace gvk

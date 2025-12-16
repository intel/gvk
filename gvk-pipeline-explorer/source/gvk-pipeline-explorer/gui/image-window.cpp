
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

#include "gvk-pipeline-explorer/gui/image-window.hpp"

#include <fstream>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

ImageWindow::ImageWindow(Window::Manager& windowManager, const std::string& name, const std::filesystem::path& imagePath)
    : Window(windowManager, name)
    , mImagePath{ imagePath }
{
              
}

std::string ImageWindow::get_name(const std::filesystem::path& imagePath)
{
    return imagePath.string();
}

void ImageWindow::on_gui(GuiInfo& guiInfo)
{
    (void)guiInfo;

    // Spawn new window at center
    auto size = ImGui::GetMainViewport()->Size;
    // Heavy metal window size \m/
    size.x *= 0.666f;
    size.y *= 0.666f;
    ImGui::SetWindowSize(size, ImGuiCond_Once);
    // TODO : Dock new window in active dock node, excluding SelectedPipelineWindow
    auto position = ImGui::GetMainViewport()->GetCenter();
    position.x -= size.x * 0.5f;
    position.y -= size.y * 0.5f;
    ImGui::SetWindowPos(position, ImGuiCond_Once);

    // Set font scale
    // NOTE : As of 16 Jan 2025, ocornut has stated that there will be changes to
    //  font scaling in an upcoming release...
    //  https://github.com/ocornut/imgui/issues/8138#issuecomment-2596387681
    guiInfo.fontScale = std::clamp(guiInfo.fontScale, 0.5f, 2.0f);
    ImGui::InputFloat("Font Scale", &guiInfo.fontScale, 0.1f);
    guiInfo.fontScale = std::clamp(guiInfo.fontScale, 0.5f, 2.0f);
    ImGui::SetWindowFontScale(guiInfo.fontScale);

    // Render the TextEditor
    std::string sampleText = "Imagine " + mImagePath.string() + " is rendered here";
    ImGui::SeparatorText(sampleText.c_str());
}

void ImageWindow::on_save(GuiInfo& guiInfo)
{
    (void)guiInfo;

}

void ImageWindow::on_load(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

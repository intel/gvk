
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

#include "gvk-pipeline-explorer/gui/file-window.hpp"

#include <fstream>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

FileWindow::FileWindow(Window::Manager& windowManager, const std::string& name, const std::filesystem::path& filePath)
    : Window(windowManager, name)
    , mFilePath{ filePath }
{
    std::ifstream file(mFilePath);
    if (file.is_open()) {

        // TODO : Documentation
#if 0
        mTextEditor.SetText(std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())));
#else
        mTextEditor.SetText(std::string(std::istreambuf_iterator<char>(file), { }));
#endif
        auto extension = mFilePath.extension();
        if (extension == ".vert" ||
            extension == ".tesc" ||
            extension == ".tese" ||
            extension == ".geom" ||
            extension == ".frag" ||
            extension == ".comp" ||
            extension == ".rgen" ||
            extension == ".rahit" ||
            extension == ".rchit" ||
            extension == ".rmiss" ||
            extension == ".rint" ||
            extension == ".rcall" ||
            extension == ".task" ||
            extension == ".mesh") {
            mTextEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
        } else {
            mTextEditor.SetReadOnlyEnabled(true);
            if (filePath.extension() == ".json") {
                mTextEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
            }
        }
    } else {
        // TODO : Close window and log error
    }
}

std::string FileWindow::get_name(const std::filesystem::path& filePath)
{
    return filePath.string();
}

void FileWindow::on_gui(GuiInfo& guiInfo)
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
    mTextEditor.Render(Window::get_name().c_str());
}

void FileWindow::on_save(GuiInfo& guiInfo)
{
    (void)guiInfo;

    std::ifstream file(mFilePath);
    if (mTextEditor.GetText() != std::string(std::istreambuf_iterator<char>(file), { })) {
        file.close();
        // TODO : Log save
        std::ofstream(mFilePath) << mTextEditor.GetText();
    }
}

void FileWindow::on_load(GuiInfo& guiInfo)
{
    (void)guiInfo;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

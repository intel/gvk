
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

#include "gvk-gui/utilities.hpp"
#include "gvk-string.hpp"

#include <unordered_map>

namespace GvkGui {

ScopeID::ScopeID(const char* str_id)
{
    ImGui::PushID(str_id);
}

ScopeID::ScopeID(const char* str_id_begin, const char* str_id_end)
{
    ImGui::PushID(str_id_begin, str_id_end);
}

ScopeID::ScopeID(const void* ptr_id)
{
    ImGui::PushID(ptr_id);
}

ScopeID::ScopeID(int int_id)
{
    ImGui::PushID(int_id);
}

ScopeID::~ScopeID()
{
    ImGui::PopID();
}

ScopeIndent::ScopeIndent()
{
    ImGui::Indent();
}

ScopeIndent::~ScopeIndent()
{
    ImGui::Unindent();
}

bool InputText(const char* label, std::string* str)
{
    auto inputTextCallback = [](ImGuiInputTextCallbackData* pCallbackData)
    {
        if (pCallbackData && pCallbackData->UserData) {
            *(ImGuiInputTextCallbackData*)pCallbackData->UserData = *pCallbackData;
        }
        return 0;
    };

    static std::unordered_map<ImGuiID, ImGuiInputTextCallbackData> sInputTextCallbackData;
    auto& inputTextCallbackData = sInputTextCallbackData[ImGui::GetID(label)];
    auto result = ImGui::InputText(label, str, ImGuiInputTextFlags_CallbackAlways, inputTextCallback, &inputTextCallbackData);
    if (ImGui::BeginDragDropTarget()) {
        auto pPayload = ImGui::AcceptDragDropPayload("external");
        if (pPayload) {
            if (inputTextCallbackData.SelectionStart == inputTextCallbackData.SelectionEnd) {
                str->insert(std::min(str->size(), (size_t)inputTextCallbackData.CursorPos), { (const char*)pPayload->Data, (size_t)pPayload->DataSize });
            } else {
                auto index = std::min(inputTextCallbackData.SelectionStart, inputTextCallbackData.SelectionEnd);
                auto count = std::abs(inputTextCallbackData.SelectionEnd - inputTextCallbackData.SelectionStart);
                str->replace(index, count, { (const char*)pPayload->Data, (size_t)pPayload->DataSize });
            }
            result = true;
        }
        ImGui::EndDragDropTarget();
    }
    return result;
}

bool InputPath(const char* label, std::string* str)
{
    auto result = InputText(label, str);
    if (result) {
        *str = gvk::string::scrub_path(*str);
    }
    return result;
}

} // namespace GvkGui

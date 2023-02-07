
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

#include "gvk-gui/renderer.hpp"
#include "gvk-spirv/context.hpp"
#include "gvk-dispatch-table.hpp"
#include "gvk-format-info.hpp"
#include "gvk-structures/defaults.hpp"

#include <algorithm>
#include <iostream>
#include <utility>

namespace gvk {

template <>
inline VkFormat get_vertex_input_attribute_format<ImVec2>()
{
    return VK_FORMAT_R32G32_SFLOAT;
}

template <>
inline VkFormat get_vertex_input_attribute_format<ImU32>()
{
    return VK_FORMAT_R8G8B8A8_UNORM;
}

template <>
inline auto get_vertex_description<ImDrawVert>(uint32_t binding)
{
    return gvk::get_vertex_input_attribute_descriptions<
        ImVec2,
        ImVec2,
        ImU32
    >(binding);
}

namespace gui {

struct PushConstants
{
    ImVec2 scale { };
    ImVec2 translation { };
};

static VkResult validate_shader_info(const gvk::spirv::ShaderInfo& shaderInfo)
{
    if (!shaderInfo.errors.empty()) {
        for (const auto& error : shaderInfo.errors) {
            std::cerr << error << std::endl;
        }
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

Renderer::Renderer(Renderer&& other)
{
    *this = std::move(other);
}

Renderer& Renderer::operator=(Renderer&& other)
{
    if (this != &other) {
        mpImGuiContext = std::move(other.mpImGuiContext);
        mDevice = std::move(other.mDevice);
        mPipeline = std::move(other.mPipeline);
        mFontImageView = std::move(other.mFontImageView);
        mFontSampler = std::move(other.mFontSampler);
        mFontDescriptorSet = std::move(other.mFontDescriptorSet);
        mVertexIndexBuffer = std::move(other.mVertexIndexBuffer);
        mIndexCount = std::move(other.mIndexCount);
        other.mpImGuiContext = nullptr;
    }
    return *this;
}

VkResult Renderer::create(const Device& device, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, const RenderPass& renderPass, const VkAllocationCallbacks* pAllocator, Renderer* pRenderer)
{
    assert(device);
    assert(vkQueue);
    assert(vkCommandBuffer);
    assert(renderPass);
    assert(pRenderer);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pRenderer->mDevice = device;
        pRenderer->mpImGuiContext = ImGui::CreateContext();
        ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        gvk_result(pRenderer->mpImGuiContext ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pRenderer->create_pipeline(renderPass, pAllocator));
        gvk_result(pRenderer->create_image_view_and_sampler(vkQueue, vkCommandBuffer, pAllocator));
        gvk_result(pRenderer->allocate_and_update_descriptor_set(pAllocator));
    } gvk_result_scope_end;
    return gvkResult;
}

Renderer::~Renderer()
{
    if (mpImGuiContext) {
        ImGui::DestroyContext(mpImGuiContext);
    }
}

const Device& Renderer::get_device() const
{
    return mDevice;
}

const Pipeline& Renderer::get_pipeline() const
{
    return mPipeline;
}

static system::Key to_gvk_key(ImGuiKey imGuiKey)
{
    switch (imGuiKey) {
    case ImGuiKey_None:           return system::Key::Unknown;
    case ImGuiKey_Tab:            return system::Key::Tab;
    case ImGuiKey_LeftArrow:      return system::Key::LeftArrow;
    case ImGuiKey_RightArrow:     return system::Key::RightArrow;
    case ImGuiKey_UpArrow:        return system::Key::UpArrow;
    case ImGuiKey_DownArrow:      return system::Key::DownArrow;
    case ImGuiKey_PageUp:         return system::Key::PageUp;
    case ImGuiKey_PageDown:       return system::Key::PageDown;
    case ImGuiKey_Home:           return system::Key::Home;
    case ImGuiKey_End:            return system::Key::End;
    case ImGuiKey_Insert:         return system::Key::Insert;
    case ImGuiKey_Delete:         return system::Key::Delete;
    case ImGuiKey_Backspace:      return system::Key::Backspace;
    case ImGuiKey_Space:          return system::Key::SpaceBar;
    case ImGuiKey_Enter:          return system::Key::Enter;
    case ImGuiKey_Escape:         return system::Key::Escape;
    case ImGuiKey_LeftCtrl:       return system::Key::LeftControl;
    case ImGuiKey_LeftShift:      return system::Key::LeftShift;
    case ImGuiKey_LeftAlt:        return system::Key::LeftAlt;
    case ImGuiKey_LeftSuper:      return system::Key::LeftWindow;
    case ImGuiKey_RightCtrl:      return system::Key::RightControl;
    case ImGuiKey_RightShift:     return system::Key::RightShift;
    case ImGuiKey_RightAlt:       return system::Key::RightAlt;
    case ImGuiKey_RightSuper:     return system::Key::RightWindow;
    case ImGuiKey_Menu:           return system::Key::Menu;
    case ImGuiKey_0:              return system::Key::Zero;
    case ImGuiKey_1:              return system::Key::One;
    case ImGuiKey_2:              return system::Key::Two;
    case ImGuiKey_3:              return system::Key::Three;
    case ImGuiKey_4:              return system::Key::Four;
    case ImGuiKey_5:              return system::Key::Five;
    case ImGuiKey_6:              return system::Key::Six;
    case ImGuiKey_7:              return system::Key::Seven;
    case ImGuiKey_8:              return system::Key::Eight;
    case ImGuiKey_9:              return system::Key::Nine;
    case ImGuiKey_A:              return system::Key::A;
    case ImGuiKey_B:              return system::Key::B;
    case ImGuiKey_C:              return system::Key::C;
    case ImGuiKey_D:              return system::Key::D;
    case ImGuiKey_E:              return system::Key::E;
    case ImGuiKey_F:              return system::Key::F;
    case ImGuiKey_G:              return system::Key::G;
    case ImGuiKey_H:              return system::Key::H;
    case ImGuiKey_I:              return system::Key::I;
    case ImGuiKey_J:              return system::Key::J;
    case ImGuiKey_K:              return system::Key::K;
    case ImGuiKey_L:              return system::Key::L;
    case ImGuiKey_M:              return system::Key::M;
    case ImGuiKey_N:              return system::Key::N;
    case ImGuiKey_O:              return system::Key::O;
    case ImGuiKey_P:              return system::Key::P;
    case ImGuiKey_Q:              return system::Key::Q;
    case ImGuiKey_R:              return system::Key::R;
    case ImGuiKey_S:              return system::Key::S;
    case ImGuiKey_T:              return system::Key::T;
    case ImGuiKey_U:              return system::Key::U;
    case ImGuiKey_V:              return system::Key::V;
    case ImGuiKey_W:              return system::Key::W;
    case ImGuiKey_X:              return system::Key::X;
    case ImGuiKey_Y:              return system::Key::Y;
    case ImGuiKey_Z:              return system::Key::Z;
    case ImGuiKey_F1:             return system::Key::F1;
    case ImGuiKey_F2:             return system::Key::F2;
    case ImGuiKey_F3:             return system::Key::F3;
    case ImGuiKey_F4:             return system::Key::F4;
    case ImGuiKey_F5:             return system::Key::F5;
    case ImGuiKey_F6:             return system::Key::F6;
    case ImGuiKey_F7:             return system::Key::F7;
    case ImGuiKey_F8:             return system::Key::F8;
    case ImGuiKey_F9:             return system::Key::F9;
    case ImGuiKey_F10:            return system::Key::F10;
    case ImGuiKey_F11:            return system::Key::F11;
    case ImGuiKey_F12:            return system::Key::F12;
    case ImGuiKey_Apostrophe:     return system::Key::OEM_Quote;
    case ImGuiKey_Comma:          return system::Key::OEM_Comma;
    case ImGuiKey_Minus:          return system::Key::OEM_Minus;
    case ImGuiKey_Period:         return system::Key::OEM_Period;
    case ImGuiKey_Slash:          return system::Key::OEM_ForwardSlash;
    case ImGuiKey_Semicolon:      return system::Key::OEM_SemiColon;
    case ImGuiKey_Equal:          return system::Key::OEM_Plus;
    case ImGuiKey_LeftBracket:    return system::Key::OEM_OpenBracket;
    case ImGuiKey_Backslash:      return system::Key::OEM_BackSlash;
    case ImGuiKey_RightBracket:   return system::Key::OEM_CloseBracket;
    case ImGuiKey_GraveAccent:    return system::Key::OEM_Tilde;
    case ImGuiKey_CapsLock:       return system::Key::CapsLock;
    case ImGuiKey_ScrollLock:     return system::Key::ScrollLock;
    case ImGuiKey_NumLock:        return system::Key::NumLock;
    case ImGuiKey_PrintScreen:    return system::Key::PrintScreen;
    case ImGuiKey_Pause:          return system::Key::Pause;
    case ImGuiKey_Keypad0:        return system::Key::NumPad0;
    case ImGuiKey_Keypad1:        return system::Key::NumPad1;
    case ImGuiKey_Keypad2:        return system::Key::NumPad2;
    case ImGuiKey_Keypad3:        return system::Key::NumPad3;
    case ImGuiKey_Keypad4:        return system::Key::NumPad4;
    case ImGuiKey_Keypad5:        return system::Key::NumPad5;
    case ImGuiKey_Keypad6:        return system::Key::NumPad6;
    case ImGuiKey_Keypad7:        return system::Key::NumPad7;
    case ImGuiKey_Keypad8:        return system::Key::NumPad8;
    case ImGuiKey_Keypad9:        return system::Key::NumPad9;
    case ImGuiKey_KeypadDecimal:  return system::Key::Decimal;
    case ImGuiKey_KeypadDivide:   return system::Key::Divide;
    case ImGuiKey_KeypadMultiply: return system::Key::Multiply;
    case ImGuiKey_KeypadSubtract: return system::Key::Subtract;
    case ImGuiKey_KeypadAdd:      return system::Key::Add;
    case ImGuiKey_KeypadEnter:    return system::Key::Enter;
    case ImGuiKey_KeypadEqual:    return system::Key::Enter;
    default:                      return system::Key::Unknown;
    }
}

static void add_key_event(const system::Input& input, ImGuiKey imGuiKey)
{
    auto gvkKey = to_gvk_key(imGuiKey);
    if (input.keyboard.pressed(gvkKey)) {
        ImGui::GetIO().AddKeyEvent(imGuiKey, true);
    } else if (input.keyboard.released(gvkKey)) {
        ImGui::GetIO().AddKeyEvent(imGuiKey, false);
    }
}

void Renderer::begin_gui(const BeginInfo& beginInfo)
{
    auto& imGuiIo = ImGui::GetIO();
    imGuiIo.DeltaTime = beginInfo.deltaTime;
    imGuiIo.DisplaySize.x = beginInfo.extent[0];
    imGuiIo.DisplaySize.y = beginInfo.extent[1];
    imGuiIo.DisplayFramebufferScale.x = 1.0f;
    imGuiIo.DisplayFramebufferScale.y = 1.0f;
    if (beginInfo.pInput) {
        // TODO : Hook up input event queue
        // NOTE : Possibly "system::InputStream" object that can be connected between
        //  system::Surface and gui::Renderer
        // NOTE : https://github.com/ocornut/imgui/issues/4921
        imGuiIo.AddMousePosEvent(beginInfo.pInput->mouse.position.current[0], beginInfo.pInput->mouse.position.current[1]);
        imGuiIo.AddMouseButtonEvent(0, beginInfo.pInput->mouse.buttons.down(system::Mouse::Button::Left));
        imGuiIo.AddMouseButtonEvent(1, beginInfo.pInput->mouse.buttons.down(system::Mouse::Button::Left));
        imGuiIo.AddMouseWheelEvent(beginInfo.pInput->mouse.scroll.delta()[0], beginInfo.pInput->mouse.scroll.delta()[1]);
        imGuiIo.AddKeyEvent(ImGuiMod_Ctrl, beginInfo.pInput->keyboard.down(system::Key::LeftControl) || beginInfo.pInput->keyboard.down(system::Key::RightControl));
        imGuiIo.AddKeyEvent(ImGuiMod_Shift, beginInfo.pInput->keyboard.down(system::Key::LeftShift) || beginInfo.pInput->keyboard.down(system::Key::RightShift));
        imGuiIo.AddKeyEvent(ImGuiMod_Alt, beginInfo.pInput->keyboard.down(system::Key::LeftAlt) || beginInfo.pInput->keyboard.down(system::Key::RightAlt));
        imGuiIo.AddKeyEvent(ImGuiMod_Super, beginInfo.pInput->keyboard.down(system::Key::LeftWindow) || beginInfo.pInput->keyboard.down(system::Key::RightWindow));
        add_key_event(*beginInfo.pInput, ImGuiKey_None);
        add_key_event(*beginInfo.pInput, ImGuiKey_Tab);
        add_key_event(*beginInfo.pInput, ImGuiKey_LeftArrow);
        add_key_event(*beginInfo.pInput, ImGuiKey_RightArrow);
        add_key_event(*beginInfo.pInput, ImGuiKey_UpArrow);
        add_key_event(*beginInfo.pInput, ImGuiKey_DownArrow);
        add_key_event(*beginInfo.pInput, ImGuiKey_PageUp);
        add_key_event(*beginInfo.pInput, ImGuiKey_PageDown);
        add_key_event(*beginInfo.pInput, ImGuiKey_Home);
        add_key_event(*beginInfo.pInput, ImGuiKey_End);
        add_key_event(*beginInfo.pInput, ImGuiKey_Insert);
        add_key_event(*beginInfo.pInput, ImGuiKey_Delete);
        add_key_event(*beginInfo.pInput, ImGuiKey_Backspace);
        add_key_event(*beginInfo.pInput, ImGuiKey_Space);
        add_key_event(*beginInfo.pInput, ImGuiKey_Enter);
        add_key_event(*beginInfo.pInput, ImGuiKey_Escape);
        add_key_event(*beginInfo.pInput, ImGuiKey_LeftCtrl);
        add_key_event(*beginInfo.pInput, ImGuiKey_LeftShift);
        add_key_event(*beginInfo.pInput, ImGuiKey_LeftAlt);
        add_key_event(*beginInfo.pInput, ImGuiKey_LeftSuper);
        add_key_event(*beginInfo.pInput, ImGuiKey_RightCtrl);
        add_key_event(*beginInfo.pInput, ImGuiKey_RightShift);
        add_key_event(*beginInfo.pInput, ImGuiKey_RightAlt);
        add_key_event(*beginInfo.pInput, ImGuiKey_RightSuper);
        add_key_event(*beginInfo.pInput, ImGuiKey_Menu);
        add_key_event(*beginInfo.pInput, ImGuiKey_0);
        add_key_event(*beginInfo.pInput, ImGuiKey_1);
        add_key_event(*beginInfo.pInput, ImGuiKey_2);
        add_key_event(*beginInfo.pInput, ImGuiKey_3);
        add_key_event(*beginInfo.pInput, ImGuiKey_4);
        add_key_event(*beginInfo.pInput, ImGuiKey_5);
        add_key_event(*beginInfo.pInput, ImGuiKey_6);
        add_key_event(*beginInfo.pInput, ImGuiKey_7);
        add_key_event(*beginInfo.pInput, ImGuiKey_8);
        add_key_event(*beginInfo.pInput, ImGuiKey_9);
        add_key_event(*beginInfo.pInput, ImGuiKey_A);
        add_key_event(*beginInfo.pInput, ImGuiKey_B);
        add_key_event(*beginInfo.pInput, ImGuiKey_C);
        add_key_event(*beginInfo.pInput, ImGuiKey_D);
        add_key_event(*beginInfo.pInput, ImGuiKey_E);
        add_key_event(*beginInfo.pInput, ImGuiKey_F);
        add_key_event(*beginInfo.pInput, ImGuiKey_G);
        add_key_event(*beginInfo.pInput, ImGuiKey_H);
        add_key_event(*beginInfo.pInput, ImGuiKey_I);
        add_key_event(*beginInfo.pInput, ImGuiKey_J);
        add_key_event(*beginInfo.pInput, ImGuiKey_K);
        add_key_event(*beginInfo.pInput, ImGuiKey_L);
        add_key_event(*beginInfo.pInput, ImGuiKey_M);
        add_key_event(*beginInfo.pInput, ImGuiKey_N);
        add_key_event(*beginInfo.pInput, ImGuiKey_O);
        add_key_event(*beginInfo.pInput, ImGuiKey_P);
        add_key_event(*beginInfo.pInput, ImGuiKey_Q);
        add_key_event(*beginInfo.pInput, ImGuiKey_R);
        add_key_event(*beginInfo.pInput, ImGuiKey_S);
        add_key_event(*beginInfo.pInput, ImGuiKey_T);
        add_key_event(*beginInfo.pInput, ImGuiKey_U);
        add_key_event(*beginInfo.pInput, ImGuiKey_V);
        add_key_event(*beginInfo.pInput, ImGuiKey_W);
        add_key_event(*beginInfo.pInput, ImGuiKey_X);
        add_key_event(*beginInfo.pInput, ImGuiKey_Y);
        add_key_event(*beginInfo.pInput, ImGuiKey_Z);
        add_key_event(*beginInfo.pInput, ImGuiKey_F1);
        add_key_event(*beginInfo.pInput, ImGuiKey_F2);
        add_key_event(*beginInfo.pInput, ImGuiKey_F3);
        add_key_event(*beginInfo.pInput, ImGuiKey_F4);
        add_key_event(*beginInfo.pInput, ImGuiKey_F5);
        add_key_event(*beginInfo.pInput, ImGuiKey_F6);
        add_key_event(*beginInfo.pInput, ImGuiKey_F7);
        add_key_event(*beginInfo.pInput, ImGuiKey_F8);
        add_key_event(*beginInfo.pInput, ImGuiKey_F9);
        add_key_event(*beginInfo.pInput, ImGuiKey_F10);
        add_key_event(*beginInfo.pInput, ImGuiKey_F11);
        add_key_event(*beginInfo.pInput, ImGuiKey_F12);
        add_key_event(*beginInfo.pInput, ImGuiKey_Apostrophe);
        add_key_event(*beginInfo.pInput, ImGuiKey_Comma);
        add_key_event(*beginInfo.pInput, ImGuiKey_Minus);
        add_key_event(*beginInfo.pInput, ImGuiKey_Period);
        add_key_event(*beginInfo.pInput, ImGuiKey_Slash);
        add_key_event(*beginInfo.pInput, ImGuiKey_Semicolon);
        add_key_event(*beginInfo.pInput, ImGuiKey_Equal);
        add_key_event(*beginInfo.pInput, ImGuiKey_LeftBracket);
        add_key_event(*beginInfo.pInput, ImGuiKey_Backslash);
        add_key_event(*beginInfo.pInput, ImGuiKey_RightBracket);
        add_key_event(*beginInfo.pInput, ImGuiKey_GraveAccent);
        add_key_event(*beginInfo.pInput, ImGuiKey_CapsLock);
        add_key_event(*beginInfo.pInput, ImGuiKey_ScrollLock);
        add_key_event(*beginInfo.pInput, ImGuiKey_NumLock);
        add_key_event(*beginInfo.pInput, ImGuiKey_PrintScreen);
        add_key_event(*beginInfo.pInput, ImGuiKey_Pause);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad0);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad1);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad2);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad3);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad4);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad5);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad6);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad7);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad8);
        add_key_event(*beginInfo.pInput, ImGuiKey_Keypad9);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadDecimal);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadDivide);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadMultiply);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadSubtract);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadAdd);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadEnter);
        add_key_event(*beginInfo.pInput, ImGuiKey_KeypadEqual);
    }
    if (beginInfo.textStreamCodePointCount) {
        assert(beginInfo.pTextStreamCodePoints);
        for (uint32_t i = 0; i < beginInfo.textStreamCodePointCount; ++i) {
            ImGui::GetIO().AddInputCharacter(beginInfo.pTextStreamCodePoints[i]);
        }
    }
    ImGui::NewFrame();
}

VkResult Renderer::end_gui(uint32_t fenceCount, const VkFence* pVkFences)
{
    assert(mDevice);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        ImGui::Render();
        auto pImDrawData = ImGui::GetDrawData();
        assert(pImDrawData);
        auto vertexCount = pImDrawData->TotalVtxCount;
        auto vertexDataSize = vertexCount * sizeof(ImDrawVert);
        mIndexDataOffset = vertexDataSize;
        mIndexCount = pImDrawData->TotalIdxCount;
        auto indexDataSize = mIndexCount * sizeof(ImDrawIdx);
        auto dataSize = vertexDataSize + indexDataSize;
        if (dataSize) {
            if (!mVertexIndexBuffer || mVertexIndexBuffer.get<VkBufferCreateInfo>().size < dataSize) {
                auto originalSize = mVertexIndexBuffer ? mVertexIndexBuffer.get<VkBufferCreateInfo>().size : 0;
                (void)originalSize;
                if (mVertexIndexBuffer && fenceCount && pVkFences) {
                    auto dispatchTable = mDevice.get<DispatchTable>();
                    assert(dispatchTable.gvkWaitForFences);
                    dispatchTable.gvkWaitForFences(mDevice, fenceCount, pVkFences, VK_TRUE, UINT64_MAX);
                }
                auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
                bufferCreateInfo.size = dataSize;
                bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                VmaAllocationCreateInfo allocationCreateInfo { };
                allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
                gvk_result(Buffer::create(mDevice, &bufferCreateInfo, &allocationCreateInfo, &mVertexIndexBuffer));
            }
            uint8_t* pData = nullptr;
            gvk_result(vmaMapMemory(mDevice.get<VmaAllocator>(), mVertexIndexBuffer.get<VmaAllocation>(), (void**)&pData));
            auto pVertexData = (ImDrawVert*)pData;
            auto pIndexData = (ImDrawIdx*)(pData + mIndexDataOffset);
            for (int i = 0; i < pImDrawData->CmdListsCount; ++i) {
                auto pCmdList = pImDrawData->CmdLists[i];
                assert(pCmdList);
                memcpy(pVertexData, pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(pIndexData, pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
                pVertexData += pCmdList->VtxBuffer.Size;
                pIndexData += pCmdList->IdxBuffer.Size;
            }
            gvk_result(vmaFlushAllocation(mDevice.get<VmaAllocator>(), mVertexIndexBuffer.get<VmaAllocation>(), 0, dataSize));
            vmaUnmapMemory(mDevice.get<VmaAllocator>(), mVertexIndexBuffer.get<VmaAllocation>());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void Renderer::record_cmds(VkCommandBuffer vkCommandBuffer) const
{
    auto dispatchTable = mDevice.get<DispatchTable>();
    assert(dispatchTable.gvkCmdSetScissor);
    assert(dispatchTable.gvkCmdBindDescriptorSets);
    assert(dispatchTable.gvkCmdDrawIndexed);

    auto pImDrawData = ImGui::GetDrawData();
    assert(pImDrawData);
    if (pImDrawData->CmdListsCount) {
        assert(pImDrawData->CmdLists);
        record_render_state_setup_cmds(vkCommandBuffer, pImDrawData);
        int vertexOffset = 0;
        int indexOffset = 0;
        for (int cmdList_i = 0; cmdList_i < pImDrawData->CmdListsCount; ++cmdList_i) {
            auto pCmdList = pImDrawData->CmdLists[cmdList_i];
            for (int cmd_i = 0; cmd_i < pCmdList->CmdBuffer.Size; ++cmd_i) {
                const auto& cmd = pCmdList->CmdBuffer[cmd_i];
                if (cmd.UserCallback) {
                    if (cmd.UserCallback == ImDrawCallback_ResetRenderState) {
                        record_render_state_setup_cmds(vkCommandBuffer, pImDrawData);
                    } else {
                        cmd.UserCallback(pCmdList, &cmd);
                    }
                } else {
                    auto displayPos = pImDrawData->DisplayPos;
                    auto framebufferScale = pImDrawData->FramebufferScale;
                    ImVec2 clipMin(std::max(0.0f, (cmd.ClipRect.x - displayPos.x) * framebufferScale.x), std::max(0.0f, (cmd.ClipRect.y - displayPos.y) * framebufferScale.y));
                    ImVec2 clipMax(std::max(0.0f, (cmd.ClipRect.z - displayPos.x) * framebufferScale.x), std::max(0.0f, (cmd.ClipRect.w - displayPos.y) * framebufferScale.y));
                    auto scissor = get_default<VkRect2D>();
                    scissor.offset.x = (int32_t)clipMin.x;
                    scissor.offset.y = (int32_t)clipMin.y;
                    scissor.extent.width = (uint32_t)(clipMax.x - clipMin.x);
                    scissor.extent.height = (uint32_t)(clipMax.y - clipMin.y);
                    if (clipMin.x < clipMax.x && clipMin.y < clipMax.y) {
                        dispatchTable.gvkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
                        auto vkDescriptorSet = (VkDescriptorSet)cmd.TextureId;
                        dispatchTable.gvkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.get<PipelineLayout>(), 0, 1, &vkDescriptorSet, 0, nullptr);
                        dispatchTable.gvkCmdDrawIndexed(vkCommandBuffer, cmd.ElemCount, 1, cmd.IdxOffset + indexOffset, cmd.VtxOffset + vertexOffset, 0);
                    }
                }
            }
            vertexOffset += pCmdList->VtxBuffer.Size;
            indexOffset += pCmdList->IdxBuffer.Size;
        }
    }
}

VkResult Renderer::create_pipeline(const RenderPass& renderPass, const VkAllocationCallbacks* pAllocator)
{
    assert(mDevice);
    assert(renderPass);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto vertexShaderInfo = get_default<spirv::ShaderInfo>();
        vertexShaderInfo.language = spirv::ShadingLanguage::Glsl;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.lineOffset = __LINE__;
        vertexShaderInfo.source = R"(
            #version 450

            layout(location = 0) in vec2 inPosition;
            layout(location = 1) in vec2 inTexCoord;
            layout(location = 2) in vec4 inColor;

            layout(push_constant) uniform PushConstants
            {
                vec2 scale;
                vec2 translation;
            } pushConstants;

            layout(location = 0) out vec2 outTexCoord;
            layout(location = 1) out vec4 outColor;

            out gl_PerVertex
            {
                vec4 gl_Position;
            };

            void main()
            {
                gl_Position = vec4(inPosition * pushConstants.scale + pushConstants.translation, 0, 1);
                outTexCoord = inTexCoord;
                outColor = inColor;
            }
        )";
        auto fragmentShaderInfo = get_default<spirv::ShaderInfo>();
        fragmentShaderInfo.language = spirv::ShadingLanguage::Glsl;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.lineOffset = __LINE__;
        fragmentShaderInfo.source = R"(
            #version 450

            layout(set = 0, binding = 0) uniform sampler2D image;

            layout(location = 0) in vec2 inTexCoord;
            layout(location = 1) in vec4 inColor;

            layout(location = 0) out vec4 outColor;

            void main()
            {
                outColor = texture(image, inTexCoord) * inColor;
            }
        )";
        spirv::Context spirvContext;
        gvk_result(spirv::Context::create(&get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext));
        gvk_result(spirvContext.compile(&vertexShaderInfo));
        gvk_result(spirvContext.compile(&fragmentShaderInfo));
        auto vsVkResult = validate_shader_info(vertexShaderInfo);
        auto fsVkResult = validate_shader_info(fragmentShaderInfo);
        gvk_result(vsVkResult);
        gvk_result(fsVkResult);

        auto vertexShaderModuleCreateInfo = get_default<VkShaderModuleCreateInfo>();
        vertexShaderModuleCreateInfo.codeSize = vertexShaderInfo.spirv.size() * sizeof(uint32_t);
        vertexShaderModuleCreateInfo.pCode = !vertexShaderInfo.spirv.empty() ? vertexShaderInfo.spirv.data() : nullptr;
        ShaderModule vertexShaderModule;
        gvk_result(ShaderModule::create(mDevice, &vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule));
        auto vertexPipelineShaderStageCreateInfo = get_default<VkPipelineShaderStageCreateInfo>();
        vertexPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexPipelineShaderStageCreateInfo.module = vertexShaderModule;

        auto fragmentShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        fragmentShaderModuleCreateInfo.codeSize = fragmentShaderInfo.spirv.size() * sizeof(uint32_t);
        fragmentShaderModuleCreateInfo.pCode = !fragmentShaderInfo.spirv.empty() ? fragmentShaderInfo.spirv.data() : nullptr;
        ShaderModule fragmentShaderModule;
        gvk_result(ShaderModule::create(mDevice, &fragmentShaderModuleCreateInfo, nullptr, &fragmentShaderModule));
        auto fragmentPipelineShaderStageCreateInfo = get_default<VkPipelineShaderStageCreateInfo>();
        fragmentPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentPipelineShaderStageCreateInfo.module = fragmentShaderModule;

        std::array<VkPipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos {
            vertexPipelineShaderStageCreateInfo,
            fragmentPipelineShaderStageCreateInfo,
        };

        VkVertexInputBindingDescription vertexInputBindingDescription { 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX };
        auto vertexInputAttributeDescriptions = get_vertex_description<ImDrawVert>(0);
        auto pipelineVertexInputStateCreateInfo = get_default<VkPipelineVertexInputStateCreateInfo>();
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions.size();
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

        auto pipelineColorBlendAttachmentState = get_default<VkPipelineColorBlendAttachmentState>();
        pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
        auto pipelineColorBlendStateCreateInfo = get_default<VkPipelineColorBlendStateCreateInfo>();
        pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

        auto pipelineMultisampleStateCreateInfo = get_default<VkPipelineMultisampleStateCreateInfo>();
        pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        auto pipelineDepthStencilStateCreateInfo = get_default<VkPipelineDepthStencilStateCreateInfo>();

        auto renderPassCreateInfo = renderPass.get<VkRenderPassCreateInfo>();
        if (renderPassCreateInfo.sType == get_stype<VkRenderPassCreateInfo>()) {
            assert(!renderPass.get<VkRenderPassCreateInfo2>().sType);
            for (uint32_t i = 0; i < renderPassCreateInfo.attachmentCount; ++i) {
                pipelineMultisampleStateCreateInfo.rasterizationSamples = std::max(pipelineMultisampleStateCreateInfo.rasterizationSamples, renderPassCreateInfo.pAttachments[i].samples);
            }
        } else {
            auto renderPassCreateInfo2 = renderPass.get<VkRenderPassCreateInfo2>();
            assert(!renderPassCreateInfo.sType);
            assert(renderPassCreateInfo2.sType == get_stype<VkRenderPassCreateInfo2>());
            for (uint32_t i = 0; i < renderPassCreateInfo2.attachmentCount; ++i) {
                pipelineMultisampleStateCreateInfo.rasterizationSamples = std::max(pipelineMultisampleStateCreateInfo.rasterizationSamples, renderPassCreateInfo2.pAttachments[i].samples);
            }
        }

        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(vertexShaderInfo);
        spirvBindingInfo.add_shader(fragmentShaderInfo);
        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(renderPass.get<gvk::Device>(), spirvBindingInfo, nullptr, &pipelineLayout));

        auto graphicsPipelineCreateInfo = get_default<VkGraphicsPipelineCreateInfo>();
        graphicsPipelineCreateInfo.stageCount = (uint32_t)pipelineShaderStageCreateInfos.size();
        graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
        graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.layout = pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = renderPass;
        gvk_result(Pipeline::create(mDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, pAllocator, &mPipeline));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Renderer::create_image_view_and_sampler(VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, const VkAllocationCallbacks* pAllocator)
{
    assert(mpImGuiContext);
    assert(mDevice);
    assert(vkQueue);
    assert(vkCommandBuffer);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        int fontWidth = 0;
        int fontHeight = 0;
        unsigned char* pFontData = nullptr;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pFontData, &fontWidth, &fontHeight);
        assert(fontWidth);
        assert(fontHeight);
        assert(pFontData);

        auto imageCreateInfo = get_default<VkImageCreateInfo>();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.extent.width = (uint32_t)fontWidth;
        imageCreateInfo.extent.height = (uint32_t)fontHeight;
        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        auto allocationCreateInfo = get_default<VmaAllocationCreateInfo>();
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        Image image;
        gvk_result(Image::create(mDevice, &imageCreateInfo, &allocationCreateInfo, &image));

        auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
        bufferCreateInfo.size = fontWidth * fontHeight * 4 * sizeof(unsigned char);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        Buffer buffer;
        gvk_result(Buffer::create(mDevice, &bufferCreateInfo, &allocationCreateInfo, &buffer));

        uint8_t* pData = nullptr;
        gvk_result(vmaMapMemory(mDevice.get<VmaAllocator>(), buffer.get<VmaAllocation>(), (void**)&pData));
        memcpy(pData, pFontData, bufferCreateInfo.size);
        vmaFlushAllocation(mDevice.get<VmaAllocator>(), buffer.get<VmaAllocation>(), 0, bufferCreateInfo.size);
        vmaUnmapMemory(mDevice.get<VmaAllocator>(), buffer.get<VmaAllocation>());

        gvk_result(execute_immediately(mDevice, vkQueue, vkCommandBuffer, VK_NULL_HANDLE,
            [&](auto)
            {
                auto dispatchTable = mDevice.get<DispatchTable>();
                assert(dispatchTable.gvkCmdPipelineBarrier);
                assert(dispatchTable.gvkCmdCopyBufferToImage);

                auto imageMemoryBarrier = get_default<VkImageMemoryBarrier>();
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.oldLayout = imageCreateInfo.initialLayout;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.image = image;
                dispatchTable.gvkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

                auto bufferImageCopy = get_default<VkBufferImageCopy>();
                bufferImageCopy.imageExtent = imageCreateInfo.extent;
                bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                dispatchTable.gvkCmdCopyBufferToImage(vkCommandBuffer, buffer, image, imageMemoryBarrier.newLayout, 1, &bufferImageCopy);

                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                dispatchTable.gvkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
        ));

        auto imageViewCreateInfo = get_default<VkImageViewCreateInfo>();
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = imageCreateInfo.format;
        gvk_result(ImageView::create(mDevice, &imageViewCreateInfo, pAllocator, &mFontImageView));
        gvk_result(Sampler::create(mDevice, &get_default<VkSamplerCreateInfo>(), pAllocator, &mFontSampler));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Renderer::allocate_and_update_descriptor_set(const VkAllocationCallbacks* pAllocator)
{
    assert(mpImGuiContext);
    assert(mDevice);
    assert(mPipeline);
    auto pipelineLayout = mPipeline.get<PipelineLayout>();
    assert(pipelineLayout);
    const auto& descriptorSetLayouts = pipelineLayout.get<DescriptorSetLayouts>();
    assert(descriptorSetLayouts.size() == 1);
    assert(descriptorSetLayouts[0]);
    assert(mFontImageView);
    assert(mFontSampler);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto descriptorPoolSize = get_default<VkDescriptorPoolSize>();
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorPoolSize.descriptorCount = 1;
        auto descriptorPoolCreateInfo = get_default<VkDescriptorPoolCreateInfo>();
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
        DescriptorPool descriptorPool;
        gvk_result(DescriptorPool::create(mDevice, &descriptorPoolCreateInfo, pAllocator, &descriptorPool));

        auto descriptorSetAllocateInfo = get_default<VkDescriptorSetAllocateInfo>();
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayouts[0].get<const VkDescriptorSetLayout&>();
        gvk_result(DescriptorSet::allocate(mDevice, &descriptorSetAllocateInfo, &mFontDescriptorSet));

        auto descriptorImageInfo = get_default<VkDescriptorImageInfo>();
        descriptorImageInfo.sampler = mFontSampler;
        descriptorImageInfo.imageView = mFontImageView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        auto writeDescriptorSet = get_default<VkWriteDescriptorSet>();
        writeDescriptorSet.dstSet = mFontDescriptorSet;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.pImageInfo = &descriptorImageInfo;
        auto dispatchTable = mDevice.get<DispatchTable>();
        assert(dispatchTable.gvkUpdateDescriptorSets);
        dispatchTable.gvkUpdateDescriptorSets(mDevice, 1, &writeDescriptorSet, 0, nullptr);
        ImGui::GetIO().Fonts->SetTexID(mFontDescriptorSet);
    } gvk_result_scope_end;
    return gvkResult;
}

void Renderer::record_render_state_setup_cmds(VkCommandBuffer vkCommandBuffer, const ImDrawData* pImDrawData) const
{
    auto dispatchTable = mDevice.get<DispatchTable>();
    assert(dispatchTable.gvkCmdSetViewport);
    assert(dispatchTable.gvkCmdBindPipeline);
    assert(dispatchTable.gvkCmdPushConstants);
    assert(dispatchTable.gvkCmdBindVertexBuffers);
    assert(dispatchTable.gvkCmdBindIndexBuffer);

    auto viewport = get_default<VkViewport>();
    viewport.width = pImDrawData->DisplaySize.x * pImDrawData->FramebufferScale.x;
    viewport.height = pImDrawData->DisplaySize.y * pImDrawData->FramebufferScale.y;
    dispatchTable.gvkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);

    dispatchTable.gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

    PushConstants pushContstants { };
    pushContstants.scale[0] = 2.0f / pImDrawData->DisplaySize.x;
    pushContstants.scale[1] = 2.0f / pImDrawData->DisplaySize.y;
    pushContstants.translation[0] = -1.0f - pImDrawData->DisplayPos.x * pushContstants.scale[0];
    pushContstants.translation[1] = -1.0f - pImDrawData->DisplayPos.y * pushContstants.scale[1];
    dispatchTable.gvkCmdPushConstants(vkCommandBuffer, mPipeline.get<PipelineLayout>(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushContstants), &pushContstants);

    if (pImDrawData->TotalVtxCount) {
        VkDeviceSize vertexDataOffset = 0;
        dispatchTable.gvkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &mVertexIndexBuffer.get<const VkBuffer&>(), &vertexDataOffset);
        dispatchTable.gvkCmdBindIndexBuffer(vkCommandBuffer, mVertexIndexBuffer, mIndexDataOffset, get_index_type<ImDrawIdx>());
    }
}

} // namespace gui
} // namespace gvk

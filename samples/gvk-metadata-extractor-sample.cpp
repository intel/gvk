
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

#include "gvk-metadata-extractor-sample-utilities.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-containers/streambuf.hpp"
#include "gvk-environment.hpp"
#include "gvk-gui.hpp"
#include "gvk-handles.hpp"
#include "gvk-runtime.hpp"
#include "gvk-string.hpp"
#include "gvk-structures.hpp"
#include "gvk-system.hpp"

#ifdef GVK_COMPILER_MSVC
#pragma warning(push, 0)
#endif
#include "TextEditor.h"
#ifdef GVK_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <Psapi.h>
#include <Windows.h>

#include <array>
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

class GvkMetadataExtractorSampleWindowManager;

static constexpr gvk::Printer::Flags GvkPrinterFlags{ gvk::Printer::Default ^ gvk::Printer::EnumValue };

static void on_workload_io(const gvk::ChildProcess& childProcess, size_t dataSize, const char* pData);
static void on_workload_shutdown(const gvk::ChildProcess& childProcess);

////////////////////////////////////////////////////////////////////////////////
class GuiInfo final
{
public:
    GvkMetadataExtractorSampleWindowManager* pWindowManager{ };
    std::vector<VkLayerProperties> layerProperties;
    bool validationLayerAvailable{ };
    gvk::ChildProcess workload;
    std::function<void()> onWorkloadShutdown;
    gvk::IoPipe ipcReadPipe;
    gvk::IoPipe ipcWritePipe;
    GvkMetadataExtractorSampleIpcMessenger ipcMessenger;
    gvk::Auto<VkInstanceCreateInfo> instanceCreateInfo;
    std::vector<gvk::Auto<VkDeviceCreateInfo>> deviceCreateInfos;
    gvk::Auto<GvkCommandCollection> cmds;
    gvk::Auto<GvkBindingInfo> bindingInfo;
    std::ofstream log;
    std::mutex mutex;
};

////////////////////////////////////////////////////////////////////////////////
class ContextWindow final
    : public gvk::gui::Window<GuiInfo&>
{
public:
    ContextWindow(Window<GuiInfo&>::Manager& windowManager)
        : Window(windowManager, "Context")
    {
    }

    void reset() override final
    {
        mInstanceCreateInfoJson = { };
        mDeviceCreateInfoJsons.clear();
    }

    void on_receive_instance_create_info(const VkInstanceCreateInfo& instanceCreateInfo)
    {
        mInstanceCreateInfoJson.SetReadOnlyEnabled(true);
        mInstanceCreateInfoJson.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
        mInstanceCreateInfoJson.SetText(gvk::to_string(instanceCreateInfo, GvkPrinterFlags));
    }

    void on_receive_device_create_info(const VkDeviceCreateInfo& deviceCreateInfo)
    {
        mDeviceCreateInfoJsons.push_back({ });
        mDeviceCreateInfoJsons.back().SetReadOnlyEnabled(true);
        mDeviceCreateInfoJsons.back().SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
        mDeviceCreateInfoJsons.back().SetText(gvk::to_string(deviceCreateInfo, GvkPrinterFlags));
    }

protected:
    void on_gui(GuiInfo& guiInfo) override final
    {
        (void)guiInfo;
        if (ImGui::BeginTable("Context Info Table", 1 + (int)mDeviceCreateInfoJsons.size(), ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {

            // Draw VkInstanceCreateInfo
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("VkInstanceCreateInfo");
            ImGui::Separator();
            mInstanceCreateInfoJson.Render("VkInstanceCreateInfo");

            // Draw VkDeviceCreateInfos
            for (size_t i = 0; i < mDeviceCreateInfoJsons.size(); ++i) {
                ImGui::TableNextColumn();
                std::string label = "VkDeviceCreateInfo [" + std::to_string(i) + "]";
                ImGui::TextUnformatted(label.c_str());
                ImGui::Separator();
                mDeviceCreateInfoJsons[i].Render("VkDeviceCreateInfo");
            }
            ImGui::EndTable();
        }
    }

private:
    TextEditor mInstanceCreateInfoJson;
    std::vector<TextEditor> mDeviceCreateInfoJsons;
};

////////////////////////////////////////////////////////////////////////////////
class CmdInspectorWindow final
    : public gvk::gui::Window<GuiInfo&>
{
public:
    CmdInspectorWindow(Window<GuiInfo&>::Manager& windowManager)
        : Window(windowManager, "Cmd Inspector")
    {
    }

    void reset() override final
    {
        mCmdLabels.clear();
        mCmdIndex = 0;
        mCmdJson = { };
    }

    void on_receive_command_collection(const GvkCommandCollection& commandCollection)
    {
        // Add indentation to labels so that cmd scope is nicely visble
        uint32_t cmdScopeDepth = 1;
        const uint32_t IndentationSize = 4;
        std::string indentation(cmdScopeDepth * IndentationSize, ' ');
        mCmdLabels.resize(commandCollection.commandCount);
        for (uint32_t cmd_i = 0; cmd_i < commandCollection.commandCount; ++cmd_i) {
            auto sType = commandCollection.ppCommands[cmd_i]->sType;
            auto cType = gvk::get_ctype(sType);
            auto cName = gvk::get_cname(sType);
            if (cType & GVK_COMMAND_STRUCTURE_COMMAND_TYPE_COMMAND_BUFFER) {
                if (gvk::string::contains(cName, "End")) {
                    assert(1 < cmdScopeDepth);
                    indentation.resize(--cmdScopeDepth * IndentationSize, ' ');
                }
                mCmdLabels[cmd_i] = indentation + cName;
                if (gvk::string::contains(cName, "Begin")) {
                    indentation.resize(++cmdScopeDepth * IndentationSize, ' ');
                }
            } else {
                mCmdLabels[cmd_i] = cName;
            }
        }
    }

protected:
    void on_gui(GuiInfo& guiInfo) override final
    {
        // When user clicks [Refresh Cmds] send request to layer
        ImGui::BeginDisabled(!guiInfo.workload.running());
        if (ImGui::Button("Refresh Cmds")) {

            // Send request to refresh cmds to layer
            guiInfo.ipcMessenger.write(guiInfo.ipcWritePipe.get_write_handle(), "Refresh Cmds");
            guiInfo.cmds.reset();
            guiInfo.bindingInfo.reset();
            reset();
        }
        ImGui::EndDisabled();

        // Draw table with nested table to put cmd explorer and selected cmd side-by-side
        if (ImGui::BeginTable("Cmd Explorer Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {

            // Draw cmds table
            ImGui::TableNextColumn();
            if (ImGui::BeginTable("Cmds Table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
                auto columnFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder;
                ImGui::TableSetupColumn("Index", columnFlags);
                ImGui::TableSetupColumn("Cmd", columnFlags);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                // Clipper is used to iterate over visible cmds
                ImGuiListClipper clipper;
                clipper.Begin((int)mCmdLabels.size());
                while (clipper.Step()) {

                    // Draw cmds
                    assert(mCmdIndex < mCmdLabels.size());
                    for (int cmd_i = clipper.DisplayStart; cmd_i < clipper.DisplayEnd; ++cmd_i) {
                        ImGui::PushID(cmd_i);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text(std::to_string(cmd_i).c_str());
                        ImGui::TableNextColumn();
                        if (ImGui::Selectable(mCmdLabels[cmd_i].c_str(), mCmdIndex == (uint32_t)cmd_i, ImGuiSelectableFlags_SpanAllColumns)) {

                            // Set selected cmd index
                            mCmdIndex = (uint32_t)cmd_i;

                            // Set text editor with cmd JSON
                            mCmdJson.SetReadOnlyEnabled(true);
                            mCmdJson.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
                            // NOTE : Binary data fed directly to Vulkan commands (eg. push constant data) gets
                            //  treated as Unicode by ImGuiColorTextEditor (reasonable), which causes segfaults
                            //  because it's not actually Unicode, just arbitrary bags-of-bytes.  So sanitize
                            //  the string before giving it to the TextEditor.
                            // TODO : Expose some options for how to print data fields
                            mCmdJson.SetText(sanitize_utf8_string(gvk::to_string(*guiInfo.cmds->ppCommands[cmd_i], GvkPrinterFlags)));

                            // Send request for the selected cmd's binding info to layer
                            guiInfo.ipcMessenger.write(guiInfo.ipcWritePipe.get_write_handle(), "Refresh Binding Info " + std::to_string(mCmdIndex));
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }

            // Draw selected cmd
            ImGui::TableNextColumn();
            if (mCmdIndex < guiInfo.cmds->commandCount) {
                mCmdJson.Render(mCmdLabels[mCmdIndex].c_str());
            }

            ImGui::EndTable();
        }
    }

private:
    std::string sanitize_utf8_string(const std::string& input)
    {
        std::string output;
        size_t i = 0;
        while (i < input.size()) {
            auto c = (unsigned char)(input[i]);

            // ASCII
            if (c < 0x80) {
                output += c;
                ++i;

            // 2 byte sequence
            } else if ((c & 0xE0) == 0xC0 && i + 1 < input.size() &&
                ((unsigned char)(input[i+1]) & 0xC0) == 0x80) {
                output += input.substr(i, 2);
                i += 2;

            // 3 byte sequence
            } else if ((c & 0xF0) == 0xE0 && i + 2 < input.size() &&
                ((unsigned char)(input[i+1]) & 0xC0) == 0x80 &&
                ((unsigned char)(input[i+2]) & 0xC0) == 0x80) {
                output += input.substr(i, 3);
                i += 3;

            // 4 byte sequence
            } else if ((c & 0xF8) == 0xF0 && i + 3 < input.size() &&
                ((unsigned char)(input[i+1]) & 0xC0) == 0x80 &&
                ((unsigned char)(input[i+2]) & 0xC0) == 0x80 &&
                ((unsigned char)(input[i+3]) & 0xC0) == 0x80) {
                output += input.substr(i, 4);
                i += 4;

            // Invalid byte, skip it
            } else {
                ++i;
            }
        }
        return output;
    }

    std::vector<std::string> mCmdLabels;
    uint32_t mCmdIndex{ };
    TextEditor mCmdJson;
};

////////////////////////////////////////////////////////////////////////////////
class BindingsWindow final
    : public gvk::gui::Window<GuiInfo&>
{
public:
    BindingsWindow(Window<GuiInfo&>::Manager& windowManager)
        : Window(windowManager, "Bindings")
    {
    }

    void reset() override final
    {
        mSelectedHandle = 0;
        mBindingInfoJson = { };
    }

protected:
    void on_gui(GuiInfo& guiInfo) override final
    {
        if (ImGui::BeginTabBar("VkPipelineBindPoint")) {
            auto tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
            auto pComputeBindingInfo = guiInfo.bindingInfo->pComputeBindingInfo;
            if (pComputeBindingInfo && ImGui::BeginTabItem("VK_PIPELINE_BIND_POINT_COMPUTE")) {
                if (ImGui::BeginTable("Compute Bindings Table", 3, tableFlags)) {
                    ImGui::TableNextColumn();
                    draw_pipeline_state(pComputeBindingInfo->pPipelineBindingInfo, pComputeBindingInfo->pShaderBindingInfo ? 1 : 0, pComputeBindingInfo->pShaderBindingInfo);
                    ImGui::TableNextColumn();
                    draw_descriptor_state(pComputeBindingInfo->descriptorCount, pComputeBindingInfo->pDescriptorBindingInfos);
                    ImGui::TableNextColumn();
                    mBindingInfoJson.Render("Binding Info");
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            auto pGraphicsBindingInfo = guiInfo.bindingInfo->pGraphicsBindingInfo;
            if (guiInfo.bindingInfo->pGraphicsBindingInfo && ImGui::BeginTabItem("VK_PIPELINE_BIND_POINT_GRAPHICS")) {
                if (ImGui::BeginTable("Graphics Bindings Table", 5, tableFlags)) {
                    ImGui::TableNextColumn();
                    draw_pipeline_state(pGraphicsBindingInfo->pPipelineBindingInfo, pGraphicsBindingInfo->shaderCount, pGraphicsBindingInfo->pShaderBindingInfos);
                    ImGui::TableNextColumn();
                    draw_descriptor_state(pGraphicsBindingInfo->descriptorCount, pGraphicsBindingInfo->pDescriptorBindingInfos);
                    ImGui::TableNextColumn();
                    draw_input_assembly_state(pGraphicsBindingInfo->pIndexBufferBindingInfo, pGraphicsBindingInfo->vertexBufferCount, pGraphicsBindingInfo->pVertexBufferBindingInfos);
                    ImGui::TableNextColumn();
                    draw_output_merger_state(pGraphicsBindingInfo->renderTargetCount, pGraphicsBindingInfo->pRenderTargetBindingInfos);
                    ImGui::TableNextColumn();
                    mBindingInfoJson.Render("Binding Info");
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            auto pRayTracingBindingInfo = guiInfo.bindingInfo->pRayTracingBindingInfo;
            if (guiInfo.bindingInfo->pRayTracingBindingInfo && ImGui::BeginTabItem("VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR")) {
                if (ImGui::BeginTable("Ray Tracing Bindings Table", 3, tableFlags)) {
                    ImGui::TableNextColumn();
                    draw_pipeline_state(pRayTracingBindingInfo->pPipelineBindingInfo, pRayTracingBindingInfo->shaderCount, pRayTracingBindingInfo->pShaderBindingInfos);
                    ImGui::TableNextColumn();
                    draw_descriptor_state(pRayTracingBindingInfo->descriptorCount, pRayTracingBindingInfo->pDescriptorBindingInfos);
                    ImGui::TableNextColumn();
                    mBindingInfoJson.Render("Binding Info");
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

private:
    void set_selected_binding_info(const GvkBindingInfoBaseStructure* pBindingInfo)
    {
        mBindingInfoJson.SetText(pBindingInfo ? gvk::to_string(*pBindingInfo, GvkPrinterFlags) : std::string());
        mSelectedHandle = pBindingInfo && pBindingInfo->pResourceInfo ? pBindingInfo->pResourceInfo->handle : 0;
    }

    void draw_pipeline_state(const GvkPipelineBindingInfo* pPipelineBindingInfo, uint32_t shaderCount, const GvkShaderBindingInfo* pShaderBindingInfo)
    {
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Pipeline Binding Info");
        ImGui::Separator();
        {
            GvkGui::ScopeIndent scopeIndent{ };
            auto handle = pPipelineBindingInfo && pPipelineBindingInfo->pResourceInfo ? pPipelineBindingInfo->pResourceInfo->handle : 0;
            auto label = "VkPipeline : " + gvk::to_string((VkPipeline)handle);
            GvkGui::ScopeID id("Pipeline");
            if (ImGui::Selectable(label.c_str(), mSelectedHandle == handle)) {
                set_selected_binding_info((const GvkBindingInfoBaseStructure*)pPipelineBindingInfo);
            }
        }

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Shader Binding Info");
        ImGui::Separator();
        {
            GvkGui::ScopeIndent scopeIndent0{ };
            for (uint32_t shader_i = 0; shader_i < shaderCount; ++shader_i) {
                const auto& shaderBindingInfo = pShaderBindingInfo[shader_i];

                ImGui::Text(gvk::string::remove(gvk::to_string(shaderBindingInfo.stage, GvkPrinterFlags), "\"").c_str());

                std::string label;
                auto handle = shaderBindingInfo.pResourceInfo ? shaderBindingInfo.pResourceInfo->handle : 0;
                switch (shaderBindingInfo.pResourceInfo ? shaderBindingInfo.pResourceInfo->type : VK_OBJECT_TYPE_UNKNOWN) {
                case VK_OBJECT_TYPE_SHADER_EXT: {
                    label = "VkShaderEXT : ";
                } break;
                case VK_OBJECT_TYPE_SHADER_MODULE: {
                    label = "VkShaderModule : ";
                } break;
                default: {
                    assert(false && "Unserviced shader type encountered; gvk maintenance required");
                } break;
                }
                GvkGui::ScopeIndent scopeIndent1{ };
                label += gvk::to_string((VkShaderModule)handle);
                auto selected = mSelectedHandle && shaderBindingInfo.pResourceInfo && mSelectedHandle == shaderBindingInfo.pResourceInfo->handle;
                GvkGui::ScopeID id((int)shader_i);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    set_selected_binding_info((const GvkBindingInfoBaseStructure*)&shaderBindingInfo);
                }
            }
        }
    }

    void draw_descriptor_state(uint32_t descriptorCount, const GvkDescriptorBindingInfo* pDescriptorBindingInfos)
    {
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Descriptor Binding Info");
        ImGui::Separator();
        {
            GvkGui::ScopeIndent scopeIndent0{ };
            for (uint32_t descriptor_i = 0; descriptor_i < descriptorCount; ++descriptor_i) {
                const auto& descriptorBindingInfo = pDescriptorBindingInfos[descriptor_i];
                std::string label = "VkObject : ";
                auto handle = descriptorBindingInfo.pResourceInfo ? descriptorBindingInfo.pResourceInfo->handle : 0;
                switch (descriptorBindingInfo.pResourceInfo ? descriptorBindingInfo.pResourceInfo->type : 0) {
                case VK_OBJECT_TYPE_BUFFER: {
                    label = "VkBuffer : ";
                } break;
                case VK_OBJECT_TYPE_BUFFER_VIEW: {
                    label = "VkBufferView : ";
                } break;
                case VK_OBJECT_TYPE_IMAGE: {
                    label = "VkImage : ";
                } break;
                case VK_OBJECT_TYPE_IMAGE_VIEW: {
                    label = "VkImageView : ";
                } break;
                case VK_OBJECT_TYPE_SAMPLER: {
                    label = "VkSampler : ";
                } break;
                default: {
                    assert(false && "Unserviced descriptor type encountered; gvk maintenance required");
                } break;
                }
                label += gvk::to_string((VkBuffer)handle);
                GvkGui::ScopeID id((int)descriptor_i);
                if (ImGui::Selectable(label.c_str(), mSelectedHandle == handle)) {
                    set_selected_binding_info((const GvkBindingInfoBaseStructure*)&descriptorBindingInfo);
                }
            }
        }
    }

    void draw_input_assembly_state(const GvkIndexBufferBindingInfo* pIndexBufferBindingInfo, uint32_t vertexBufferCount, const GvkVertexBufferBindingInfo* pVertexBufferBindingInfo)
    {
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Index Buffer Binding Info");
        ImGui::Separator();
        {
            GvkGui::ScopeIndent scopeIndent{ };
            auto handle = pIndexBufferBindingInfo && pIndexBufferBindingInfo->pResourceInfo ? pIndexBufferBindingInfo->pResourceInfo->handle : 0;
            auto label = "VkBuffer : " + gvk::to_string((VkBuffer)handle);
            GvkGui::ScopeID id("Index Buffer");
            if (ImGui::Selectable(label.c_str(), mSelectedHandle == handle)) {
                set_selected_binding_info((const GvkBindingInfoBaseStructure*)pIndexBufferBindingInfo);
            }
        }

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Vertex Buffer Binding Info");
        ImGui::Separator();
        {
            GvkGui::ScopeIndent scopeIndent0{ };
            for (uint32_t vertexBuffer_i = 0; vertexBuffer_i < vertexBufferCount; ++vertexBuffer_i) {
                const auto& vertexBufferBindingInfo = pVertexBufferBindingInfo[vertexBuffer_i];
                auto handle = vertexBufferBindingInfo.pResourceInfo ? vertexBufferBindingInfo.pResourceInfo->handle : 0;
                auto label = "VkBuffer : " + gvk::to_string((VkBuffer)handle);
                GvkGui::ScopeID id((int)vertexBuffer_i);
                if (ImGui::Selectable(label.c_str(), mSelectedHandle == handle)) {
                    set_selected_binding_info((const GvkBindingInfoBaseStructure*)&vertexBufferBindingInfo);
                }
            }
        }
    }

    void draw_output_merger_state(uint32_t renderTargetCount, const GvkRenderTargetBindingInfo* pRenderTargetBindingInfo)
    {
            ImGui::Separator();
            ImGui::Separator();
            ImGui::Text("Render Target Binding Info");
            ImGui::Separator();
            {
                GvkGui::ScopeIndent scopeIndent{ };
                for (uint32_t renderTarget_i = 0; renderTarget_i < renderTargetCount; ++renderTarget_i) {
                    const auto& renderTargetBindingInfo = pRenderTargetBindingInfo[renderTarget_i];
                    auto handle = renderTargetBindingInfo.pResourceInfo ? renderTargetBindingInfo.pResourceInfo->handle : 0;
                    auto label = "VkImage : " + gvk::to_string((VkImage)handle);
                    GvkGui::ScopeID id((int)renderTarget_i);
                    if (ImGui::Selectable(label.c_str(), mSelectedHandle == handle)) {
                        set_selected_binding_info((const GvkBindingInfoBaseStructure*)&renderTargetBindingInfo);
                    }
                }
            }
    }

    uint64_t mSelectedHandle{ };
    TextEditor mBindingInfoJson;
};

////////////////////////////////////////////////////////////////////////////////
class LaunchOptions final
{
public:
    std::string exe;
    std::string args;
    std::string directory;
    std::string logPath;
    bool autoWorkingDirectory{ true };
    bool waitForDebugger{ };
    bool loaderDebug{ };
    bool clearStdOut{ true };
    bool uniqueHandles{ true };

    static void im_gui_settings_write(const LaunchOptions& launchOptions, ImGuiContext* pCtx, ImGuiSettingsHandler* pSettingsHandler, ImGuiTextBuffer* pTextBuffer)
    {
        (void)pCtx;
        (void)pSettingsHandler;
        assert(pTextBuffer);
        auto getCStr = [](const std::string& str) { return !str.empty() ? str.c_str() : "nullptr"; };
        pTextBuffer->appendf("exe=%s\n", getCStr(launchOptions.exe));
        pTextBuffer->appendf("args=%s\n", getCStr(launchOptions.args));
        pTextBuffer->appendf("directory=%s\n", getCStr(launchOptions.directory));
        pTextBuffer->appendf("logPath=%s\n", getCStr(launchOptions.logPath));
        pTextBuffer->appendf("autoWorkingDirectory=%d\n", (int)launchOptions.autoWorkingDirectory);
        pTextBuffer->appendf("waitForDebugger=%d\n", (int)launchOptions.waitForDebugger);
        pTextBuffer->appendf("loaderDebug=%d\n", (int)launchOptions.loaderDebug);
        pTextBuffer->appendf("clearStdOut=%d\n", (int)launchOptions.clearStdOut);
        pTextBuffer->appendf("uniqueHandles=%d\n", (int)launchOptions.uniqueHandles);
        pTextBuffer->appendf("\n");
    };
};

////////////////////////////////////////////////////////////////////////////////
class WorkloadWindow final
    : public gvk::gui::Window<GuiInfo&>
{
public:
    WorkloadWindow(Window<GuiInfo&>::Manager& windowManager)
        : Window(windowManager, "Workload")
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

protected:
    void on_gui(GuiInfo& guiInfo) override final
    {
        ImGui::BeginDisabled(guiInfo.workload.running());
        {
            // Clear launch options
            if (ImGui::Button("Clear")) {
                mLaunchOptions = { };
            }

            // Recent launch options
            ImGui::BeginDisabled(mRecentLaunchOptions.empty());
            {
                if (ImGui::Button("Recent")) {
                    ImGui::OpenPopup("Recent-Workspace-Popup");
                }
                if (!mRecentLaunchOptions.empty() && ImGui::BeginPopup("Recent-Workspace-Popup")) {
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

            // Layers
            ImGui::SameLine();
            uint32_t enabledLayerCount = 0;
            for (const auto& layer : mEnabledLayers) {
                if (layer) {
                    ++enabledLayerCount;
                }
            }
            std::string layersLabel = "Layers [" + std::to_string(enabledLayerCount) + "]";
            if (ImGui::Button(layersLabel.c_str())) {
                ImGui::OpenPopup("Layers");
            }
            if (ImGui::BeginPopup("Layers")) {
                mEnabledLayers.resize(guiInfo.layerProperties.size());
                for (size_t layer_i = 0; layer_i < guiInfo.layerProperties.size(); ++layer_i) {
                    ImGui::PushID((int)layer_i);
                    bool active = mEnabledLayers[layer_i];
                    ImGui::Checkbox(guiInfo.layerProperties[layer_i].layerName, &active);
                    mEnabledLayers[layer_i] = (int)active;
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }

            // Wait for debugger
            ImGui::SameLine();
            ImGui::Checkbox("Wait For Debugger", &mLaunchOptions.waitForDebugger);

            // Vulkan loader debug
            ImGui::SameLine();
            ImGui::Checkbox("Vulkan Loader Debug", &mLaunchOptions.loaderDebug);

            // Clear stdout on launch
            ImGui::SameLine();
            ImGui::Checkbox("Clear StdOut", &mLaunchOptions.clearStdOut);

            // Enable validation layer unique handles
            ImGui::SameLine();
            ImGui::Checkbox("Unique Handles", &mLaunchOptions.uniqueHandles);
        }
        ImGui::EndDisabled();

        // Launch/stop
        ImGui::PushItemWidth(-FLT_MIN);
        if (!guiInfo.workload.running()) {
            if (ImGui::Button("Launch")) {
                launch_workload(guiInfo);
            }
        } else {
            if (ImGui::Button("Stop")) {
                guiInfo.workload.reset();
            }
        }

        ImGui::BeginDisabled(guiInfo.workload.running());
        {
            // Exe
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::SameLine();
            if (GvkGui::InputPath("##launch", &mLaunchOptions.exe) && mLaunchOptions.autoWorkingDirectory) {
                mLaunchOptions.directory = get_default_working_directory(mLaunchOptions.exe);
            }

            // Args
            ImGui::Text("Args");
            ImGui::SameLine();
            GvkGui::InputPath("##args", &mLaunchOptions.args);

            // Directory
            ImGui::Text("Working Directory");
            ImGui::SameLine();
            if (ImGui::Checkbox("Auto##workingDirectory", &mLaunchOptions.autoWorkingDirectory) && mLaunchOptions.autoWorkingDirectory) {
                mLaunchOptions.directory = get_default_working_directory(mLaunchOptions.exe);
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(mLaunchOptions.autoWorkingDirectory);
            {
                mLaunchOptions.directory = gvk::string::scrub_path(mLaunchOptions.directory);
                if (GvkGui::InputPath("##workingDirectory", &mLaunchOptions.directory)) {
                }
            }
            ImGui::EndDisabled();

            // Log
            ImGui::Text("Log");
            ImGui::SameLine();
            GvkGui::InputPath("##log", &mLaunchOptions.logPath);
        }
        ImGui::EndDisabled();
    }

private:
    void launch_workload(GuiInfo& guiInfo)
    {
        if (mLaunchOptions.clearStdOut) {
            // TODO : Double check that this actually clears the terminal, is cross-platform, and move to runtime...
            std::cout << "\033[2J\033[1;1H" << std::flush;
        }

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
        environment.append_value_to_env_var("VK_ADD_LAYER_PATH", layerPath.string());

        // Prepare collection of layers to enable when launching the workload
        std::vector<std::string> layers{
            "VK_LAYER_INTEL_gvk_state_tracker",
            "VK_LAYER_INTEL_gvk_metadata_extractor_sample",
        };

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
                //  with all features disabled.  When VK_LAYER_INTEL_gvk_state_tracker is loaded,
                //  it will check if VK_LAYER_KHRONOS_validation is loaded and enable the validation
                //  layer's unique handles feature.
                // NOTE : The logic here turns on the validation layer when api dump is enabled
                //  so that all layers above it utilize the same handles as the api dump output.
                for (const auto& validationLayerSetting : gvk::get_validation_layer_settings()) {
                    environment.set_env_var(validationLayerSetting, "false");
                }
            }
        }

        // Warn if validation layer is unavailable or unique handles is disabled
        if (!guiInfo.validationLayerAvailable) {
            std::cout << "[WARNING] : Validation layer unavailable (required for unique handles); may result in instability" << std::endl;
            std::cout << "    Install the Vulkan SDK or set environment variable `VK_ADD_LAYER_PATH`" << std::endl;
            std::cout << "    If GVK was built from source, the Vulkan SDK installer can be found in `gvk/build/_deps/VulkanSDK`" << std::endl;
        }
        if (!mLaunchOptions.uniqueHandles) {
            std::cout << "[WARNING] : Unique handles disabled; may result in instability" << std::endl;
        }

        // Create enabled layer list and set environment variable
        std::string layersStr;
        for (const auto& layer : layers) {
            layersStr += !layersStr.empty() ? ("," + layer) : layer;
        }
        environment.set_env_var("VK_LOADER_LAYERS_ENABLE", layersStr);

        // Set GVK_METADATA_EXTRACTOR_WAIT_FOR_DEBUGGER
        if (mLaunchOptions.waitForDebugger) {
            environment.set_env_var("GVK_METADATA_EXTRACTOR_WAIT_FOR_DEBUGGER", "true");
        }

        // Set VK_LOADER_DEBUG
        if (mLaunchOptions.loaderDebug) {
            environment.set_env_var("VK_LOADER_DEBUG", "all");
        }

        // Create IPC pipes
        gvk::IoPipe::CreateInfo ioPipeCreateInfo{ };
        ioPipeCreateInfo.inherit = gvk::IoPipe::Write;
        (void)gvk::IoPipe::create(&ioPipeCreateInfo, &guiInfo.ipcReadPipe);
        ioPipeCreateInfo.inherit = gvk::IoPipe::Read;
        (void)gvk::IoPipe::create(&ioPipeCreateInfo, &guiInfo.ipcWritePipe);

        // Set envvars with IPC handles for layer to connect
        environment.set_env_var("GVK_METADATA_EXTRACTOR_SAMPLE_LAYER_IPC_READ_HANDLE", std::to_string((uint64_t)guiInfo.ipcWritePipe.get_read_handle()));
        environment.set_env_var("GVK_METADATA_EXTRACTOR_SAMPLE_LAYER_IPC_WRITE_HANDLE", std::to_string((uint64_t)guiInfo.ipcReadPipe.get_write_handle()));

        // Get Environment data
        uint32_t envCharCount = 0;
        environment.get_env(&envCharCount, nullptr);
        std::vector<char> envData(envCharCount);
        environment.get_env(&envCharCount, envData.data());

        // Prepare gvk::ChildProcess::CreateInfo
        gvk::ChildProcess::CreateInfo childProcessCreateInfo{ };
        auto cmdLine = mLaunchOptions.exe + (!mLaunchOptions.args.empty() ? " " + mLaunchOptions.args : std::string());
        childProcessCreateInfo.pCmdLine = cmdLine.data();
        childProcessCreateInfo.pDirectory = !mLaunchOptions.directory.empty() ? mLaunchOptions.directory.c_str() : nullptr;
        childProcessCreateInfo.pEnvironment = !envData.empty() ? envData.data() : nullptr;
        if (!mLaunchOptions.logPath.empty()) {
            std::filesystem::path logPath = mLaunchOptions.logPath;
            if (!std::filesystem::path(logPath).has_extension()) {
                auto exeName = std::filesystem::path(mLaunchOptions.exe).stem();
                auto dateTime = gvk::system::DateTime::now();
                auto dateTimeStr = dateTime.get_date_str('-') + "_" + dateTime.get_time_str('-');
                logPath /= exeName.string() + "_gvk-metadata-extractor_" + dateTimeStr + ".log";
            }
            std::filesystem::create_directories(std::filesystem::path(logPath).parent_path());
            guiInfo.log.open(logPath, std::ios::out | std::ios::binary);
            if (guiInfo.log.is_open()) {
                std::cerr << "Opened log \"" << mLaunchOptions.logPath << "\"" << std::endl;
                childProcessCreateInfo.pFnOnStdOut = on_workload_io;
                childProcessCreateInfo.pFnOnStdErr = on_workload_io;
            } else {
                std::cerr << "Failed to open log \"" << mLaunchOptions.logPath << "\"" << std::endl;
            }
        }
        childProcessCreateInfo.pFnOnShutdown = on_workload_shutdown;
        childProcessCreateInfo.pUserData = &guiInfo;

        // Create child process
        if (gvk::ChildProcess::create(&childProcessCreateInfo, &guiInfo.workload)) {
            std::cout << "Launched '" << cmdLine << "'" << std::endl;

            // Check if the workload is already in the recently launched list, and if so erase
            //  it, then add the launched workload to the front of the recently launched list
            for (auto itr = mRecentLaunchOptions.begin(); itr != mRecentLaunchOptions.end(); ++itr) {
                if (itr->exe == mLaunchOptions.exe && itr->args == mLaunchOptions.args) {
                    mRecentLaunchOptions.erase(itr);
                    break;
                }
            }
            mRecentLaunchOptions.insert(mRecentLaunchOptions.begin(), mLaunchOptions);

            // Close parent handle to child process pipe ends
            guiInfo.ipcReadPipe.close(gvk::IoPipe::Write);
            guiInfo.ipcWritePipe.close(gvk::IoPipe::Read);
        } else {
            std::cerr << "Failed to launch '" << cmdLine << "'" << std::endl;
            std::cerr << "    " + gvk::get_win32_error_str(GetLastError()) << std::endl;
            guiInfo.ipcReadPipe.reset();
            guiInfo.ipcWritePipe.reset();
        }
    }

    std::string get_default_working_directory(const std::string& exe)
    {
        return !exe.empty() ? std::filesystem::path(exe).parent_path().string() : std::string();
    }

    static void im_gui_settings_clear_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler)
    {
        (void)ctx;
        assert(handler);
        auto& workspaceWindow = *(WorkloadWindow*)handler->UserData;
        workspaceWindow.mLaunchOptions = { };
        workspaceWindow.mRecentLaunchOptions = { };
    };

    static void im_gui_settings_read_init(ImGuiContext* ctx, ImGuiSettingsHandler* handler)
    {
        (void)ctx;
        assert(handler);
        auto& workspaceWindow = *(WorkloadWindow*)handler->UserData;
        workspaceWindow.mLaunchOptions = { };
        workspaceWindow.mRecentLaunchOptions = { };
    };

    static void* im_gui_settings_read_open(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name)
    {
        (void)ctx;
        assert(handler);
        assert(handler->UserData);
        assert(name);
        auto& workspaceWindow = *(WorkloadWindow*)handler->UserData;
        if (!strcmp(name, "WorkloadWindow::mLaunchOptions")) {
            return &workspaceWindow.mLaunchOptions;
        } else if (gvk::string::contains(name, "WorkloadWindow::mRecentLaunchOptions")) {
            auto index =
                gvk::string::to_number<uint32_t>(
                    gvk::string::remove(gvk::string::remove(gvk::string::remove(
                        name, "WorkloadWindow::mRecentLaunchOptions"), "["), "]"
                    )
                );
            if (workspaceWindow.mRecentLaunchOptions.size() <= index) {
                workspaceWindow.mRecentLaunchOptions.resize(index + 1);
            }
            return &workspaceWindow.mRecentLaunchOptions[index];
        }
        return nullptr;
    };

    static void im_gui_settings_read_line(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line)
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
            } else if (key == "args") {
                launchOptions.args = value;
            } else if (key == "directory") {
                launchOptions.directory = value;
            } else if (key == "logPath") {
                launchOptions.logPath = value;
            } else if (key == "autoWorkingDirectory") {
                launchOptions.autoWorkingDirectory = gvk::string::to_number<uint32_t>(value);
            } else if (key == "waitForDebugger") {
                launchOptions.waitForDebugger = gvk::string::to_number<uint32_t>(value);
            } else if (key == "loaderDebug") {
                launchOptions.loaderDebug = gvk::string::to_number<uint32_t>(value);
            } else if (key == "clearStdOut") {
                launchOptions.clearStdOut = gvk::string::to_number<uint32_t>(value);
            } else if (key == "uniqueHandles") {
                launchOptions.uniqueHandles = gvk::string::to_number<uint32_t>(value);
            }
        }
    };

    static void im_gui_settings_apply_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler)
    {
        (void)ctx;
        (void)handler;
        // NOOP :
    };

    static void im_gui_settings_write_all(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
    {
        assert(handler);
        assert(handler->TypeName);
        assert(handler->UserData);
        const auto& workspaceWindow = *(const WorkloadWindow*)handler->UserData;
        out_buf->appendf("[%s][WorkloadWindow::mLaunchOptions]\n", handler->TypeName);
        LaunchOptions::im_gui_settings_write(workspaceWindow.mLaunchOptions, ctx, handler, out_buf);
        for (int i = 0; i < workspaceWindow.mRecentLaunchOptions.size(); ++i) {
            out_buf->appendf("[%s][WorkloadWindow::mRecentLaunchOptions[%d]]\n", handler->TypeName, i);
            LaunchOptions::im_gui_settings_write(workspaceWindow.mRecentLaunchOptions[i], ctx, handler, out_buf);
        }
    };

    LaunchOptions mLaunchOptions;
    std::vector<LaunchOptions> mRecentLaunchOptions;
    std::vector<int> mEnabledLayers;
};

////////////////////////////////////////////////////////////////////////////////
class GvkMetadataExtractorSampleWindowManager final
    : public gvk::gui::Window<GuiInfo&>::Manager
{
public:
    GvkMetadataExtractorSampleWindowManager()
        : gvk::gui::Window<GuiInfo&>::Manager("gvk-metadata-extractor-sample")
    {
        open<ContextWindow>("Context");
        open<WorkloadWindow>("Workload");
        open<CmdInspectorWindow>("Cmd Inspector");
        open<BindingsWindow>("Bindings");
    }
};

////////////////////////////////////////////////////////////////////////////////
void process_incoming_messages(GuiInfo& guiInfo)
{
    for (const auto& message : guiInfo.ipcMessenger.read(guiInfo.ipcReadPipe.get_read_handle())) {

        if (message.text == "VkInstanceCreateInfo") {
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::deserialize(istrm, nullptr, guiInfo.instanceCreateInfo);
            assert(guiInfo.pWindowManager);
            auto pWindow = (ContextWindow*)guiInfo.pWindowManager->get("Context");
            assert(pWindow);
            pWindow->on_receive_instance_create_info(*guiInfo.instanceCreateInfo);

        } else if (message.text == "VkDeviceCreateInfo") {
            gvk::Auto<VkDeviceCreateInfo> deviceCreateInfo;
            guiInfo.deviceCreateInfos.push_back({ });
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::deserialize(istrm, nullptr, guiInfo.deviceCreateInfos.back());
            assert(guiInfo.pWindowManager);
            auto pWindow = (ContextWindow*)guiInfo.pWindowManager->get("Context");
            assert(pWindow);
            pWindow->on_receive_device_create_info(*guiInfo.deviceCreateInfos.back());

        } else if (message.text == "GvkBindingInfo") {
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::deserialize(istrm, nullptr, guiInfo.bindingInfo);

        } else if (message.text == "GvkCommandCollection") {
            std::istringstream istrm(std::string((char*)message.data.data(), message.data.size()));
            gvk::deserialize(istrm, nullptr, guiInfo.cmds);
            assert(guiInfo.pWindowManager);
            auto pWindow = (CmdInspectorWindow*)guiInfo.pWindowManager->get("Cmd Inspector");
            assert(pWindow);
            pWindow->on_receive_command_collection(*guiInfo.cmds);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
static void on_workload_io(const gvk::ChildProcess& childProcess, size_t dataSize, const char* pData)
{
    auto pGuiInfo = (GuiInfo*)childProcess.get_user_data();
    assert(pGuiInfo);
    pGuiInfo->log.write(pData, dataSize);
}

////////////////////////////////////////////////////////////////////////////////
static void on_workload_shutdown(const gvk::ChildProcess& childProcess)
{
    auto pGuiInfo = (GuiInfo*)childProcess.get_user_data();
    assert(pGuiInfo);
    std::lock_guard<std::mutex> lock(pGuiInfo->mutex);
    pGuiInfo->onWorkloadShutdown = [pGuiInfo]()
    {
        assert(pGuiInfo->pWindowManager);
        pGuiInfo->pWindowManager->reset();
        pGuiInfo->ipcReadPipe.reset();
        pGuiInfo->ipcWritePipe.reset();
        pGuiInfo->ipcMessenger = { };
        pGuiInfo->instanceCreateInfo.reset();
        pGuiInfo->deviceCreateInfos.clear();
        pGuiInfo->cmds.reset();
        pGuiInfo->bindingInfo.reset();
        pGuiInfo->log.close();
    };
    std::cout << "Terminated '" << childProcess.get_cmd_line() << "'" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char* ppArgv[])
{
    (void)argc;
    (void)ppArgv;
    GuiInfo guiInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Create gvk::Context
        auto applicationInfo = gvk::get_default<VkApplicationInfo>();
        applicationInfo.pApplicationName = "gvk-metadata-extractor-sample";
        auto instanceCreateInfo = gvk::get_default<VkInstanceCreateInfo>();
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        auto contextCreateInfo = gvk::get_default<gvk::Context::CreateInfo>();
        contextCreateInfo.pInstanceCreateInfo = &instanceCreateInfo;
        contextCreateInfo.loadWsiExtensions = VK_TRUE;
        gvk::Context context = VK_NULL_HANDLE;
        gvk_result(gvk::Context::create(&contextCreateInfo, nullptr, &context));

        // Get gvk::Context objects
        const auto& instance = context.get<gvk::Instance>();
        const auto& device = context.get<gvk::Devices>()[0];
        const auto& queue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
        const auto& commandBuffer = context.get<gvk::CommandBuffers>()[0];

        // Get available layers
        uint32_t layerPropertyCount = 0;
        gvk_result(context.get<gvk::Instance>().get<gvk::DispatchTable>().gvkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr));
        guiInfo.layerProperties.resize(layerPropertyCount, gvk::get_default<VkLayerProperties>());
        gvk_result(context.get<gvk::Instance>().get<gvk::DispatchTable>().gvkEnumerateInstanceLayerProperties(&layerPropertyCount, guiInfo.layerProperties.data()));
        for (const auto& layerProperties : guiInfo.layerProperties) {
            if (!strcmp(layerProperties.layerName, "VK_LAYER_KHRONOS_validation")) {
                guiInfo.validationLayerAvailable = true;
            }
        }

        // Create gvk::system::Surface
        auto systemSurfaceCreateInfo = gvk::get_default<gvk::system::Surface::CreateInfo>();
        systemSurfaceCreateInfo.pTitle = applicationInfo.pApplicationName;
        gvk::system::Surface systemSurface = VK_NULL_HANDLE;
        gvk_result((VkResult)gvk::system::Surface::create(&systemSurfaceCreateInfo, &systemSurface));

        // Create gvk::SurfaceKHR
        const VkBaseInStructure* pSurfaceCreateInfo = nullptr;
        auto win32SurfaceCreateInfo = gvk::get_default<VkWin32SurfaceCreateInfoKHR>();
        win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
        win32SurfaceCreateInfo.hwnd = systemSurface.get<gvk::system::Surface::PlatformInfo>().hwnd;
        pSurfaceCreateInfo = (VkBaseInStructure*)&win32SurfaceCreateInfo;
        gvk::SurfaceKHR surface = VK_NULL_HANDLE;
        gvk_result(gvk::SurfaceKHR::create(instance, pSurfaceCreateInfo, nullptr, &surface));

        // Create gvk::wsi::Context
        auto wsiContextCreateInfo = gvk::get_default<gvk::wsi::Context::CreateInfo>();
        wsiContextCreateInfo.queueFamilyIndex = gvk::get_queue_family(device, 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        wsiContextCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        wsiContextCreateInfo.sampleCount = VK_SAMPLE_COUNT_64_BIT;
        gvk::wsi::Context wsiContext = VK_NULL_HANDLE;
        gvk_result(gvk::wsi::Context::create(device, surface, &wsiContextCreateInfo, nullptr, &wsiContext));

        // Create gvk::gui::Renderer
        gvk::gui::Renderer guiRenderer = VK_NULL_HANDLE;
        gvk_result(gvk::gui::Renderer::create(device, queue, commandBuffer, wsiContext.get<gvk::RenderPass>(), nullptr, &guiRenderer));
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Create WindowManager
        // NOTE : Must be created after gvk::gui::Renderer because ImGui must be initialized
        GvkMetadataExtractorSampleWindowManager windowManager;
        guiInfo.pWindowManager = &windowManager;

        // Main loop
        gvk::system::Clock clock;
        while (!(systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::CloseRequested)) {
            gvk::system::Surface::update();
            clock.update();
            auto deltaTime = clock.elapsed<gvk::system::Seconds<float>>();

            // Acquire next image
            gvk::wsi::AcquiredImageInfo acquiredImageInfo{};
            gvk::RenderTarget acquiredImageRenderTarget = VK_NULL_HANDLE;
            auto wsiStatus = wsiContext.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &acquiredImageInfo, &acquiredImageRenderTarget);
            if (wsiStatus == VK_SUCCESS || wsiStatus == VK_SUBOPTIMAL_KHR) {
                const auto& extent = wsiContext.get<gvk::SwapchainKHR>().get<VkSwapchainCreateInfoKHR>().imageExtent;
                const auto& input = systemSurface.get<gvk::system::Input>();

                // Handle ImGui io and events
                auto imguiCursor = ImGui::GetMouseCursor();
                if (imguiCursor == ImGuiMouseCursor_None || ImGui::GetIO().MouseDrawCursor) {
                    systemSurface.set(gvk::system::Surface::CursorMode::Hidden);
                } else {
                    switch (imguiCursor) {
                    case ImGuiMouseCursor_Arrow: { systemSurface.set(gvk::system::Surface::CursorType::Arrow); } break;
                    case ImGuiMouseCursor_TextInput: { systemSurface.set(gvk::system::Surface::CursorType::IBeam); } break;
                    case ImGuiMouseCursor_Hand: { systemSurface.set(gvk::system::Surface::CursorType::Hand); } break;
                    case ImGuiMouseCursor_ResizeNS: { systemSurface.set(gvk::system::Surface::CursorType::ResizeNS); } break;
                    case ImGuiMouseCursor_ResizeEW: { systemSurface.set(gvk::system::Surface::CursorType::ResizeEW); } break;
                    case ImGuiMouseCursor_ResizeAll: { systemSurface.set(gvk::system::Surface::CursorType::ResizeAll); } break;
                    case ImGuiMouseCursor_ResizeNESW: { systemSurface.set(gvk::system::Surface::CursorType::ResizeNESW); } break;
                    case ImGuiMouseCursor_ResizeNWSE: { systemSurface.set(gvk::system::Surface::CursorType::ResizeNWSE); } break;
                    case ImGuiMouseCursor_NotAllowed: { systemSurface.set(gvk::system::Surface::CursorType::NotAllowed); } break;
                    default: { } break;
                    }
                }
                if (systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::GainedFocus) {
                    ImGui::GetIO().AddFocusEvent(true);
                }
                if (systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::LostFocus) {
                    ImGui::GetIO().AddFocusEvent(false);
                }

                // Update GUI
                const auto& textStream = systemSurface.get<gvk::system::Surface::TextStream>();
                const auto& droppedPaths = systemSurface.get<gvk::system::Surface::DroppedPaths>();
                auto guiRendererBeginInfo = gvk::get_default<gvk::gui::Renderer::BeginInfo>();
                guiRendererBeginInfo.deltaTime = deltaTime;
                guiRendererBeginInfo.extent = { (float)extent.width, (float)extent.height };
                guiRendererBeginInfo.pInput = &input;
                guiRendererBeginInfo.textStreamCodePointCount = (uint32_t)textStream.size();
                guiRendererBeginInfo.pTextStreamCodePoints = !textStream.empty() ? textStream.data() : nullptr;
                guiRendererBeginInfo.pDragDropPath = !droppedPaths.empty() ? droppedPaths[0].c_str() : nullptr;
                guiRenderer.begin_gui(guiRendererBeginInfo);
                windowManager.on_gui(guiInfo);
                #if 0
                ImGui::ShowDemoWindow();
                #endif
                gvk_result(guiRenderer.end_gui(acquiredImageInfo.index));

                // Draw GUI
                auto renderPassBeginInfo = acquiredImageRenderTarget.get<VkRenderPassBeginInfo>();
                gvk::CommandBuffer acquiredImageCommandBuffer = acquiredImageInfo.commandBuffer;
                gvk_result(acquiredImageCommandBuffer.BeginCommandBuffer(&gvk::get_default<VkCommandBufferBeginInfo>()));
                {
                    acquiredImageCommandBuffer.CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        VkRect2D scissor{ { }, renderPassBeginInfo.renderArea.extent };
                        acquiredImageCommandBuffer.CmdSetScissor(0, 1, &scissor);
                        VkViewport viewport{ 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                        acquiredImageCommandBuffer.CmdSetViewport(0, 1, &viewport);
                        guiRenderer.record_cmds(acquiredImageCommandBuffer, acquiredImageInfo.index);
                    }
                    acquiredImageCommandBuffer.CmdEndRenderPass();
                }
                gvk_result(acquiredImageCommandBuffer.EndCommandBuffer());

                // Submit and present
                gvk_result(queue.QueueSubmit(1, &wsiContext.get<VkSubmitInfo>(acquiredImageInfo), acquiredImageInfo.fence));
                wsiStatus = wsiContext.queue_present(queue, &acquiredImageInfo);
                gvk_result((wsiStatus == VK_SUBOPTIMAL_KHR || wsiStatus == VK_ERROR_OUT_OF_DATE_KHR) ? VK_SUCCESS : wsiStatus);
            }

            // Lock and check if workload is running, if so process incoming messages, otherwise
            //  execute and clear the onChildProcessShutdown callback if necessary
            std::lock_guard<std::mutex> lock(guiInfo.mutex);
            if (guiInfo.workload) {
                process_incoming_messages(guiInfo);
            } else if (guiInfo.onWorkloadShutdown) {
                guiInfo.onWorkloadShutdown();
                guiInfo.onWorkloadShutdown = nullptr;
            }
        }

        // Shutdown child process
        guiInfo.workload.reset();

        // Destroy gvk::gui::Renderer
        // NOTE : Explicitly destroying guiRenderer so ImGuiSettingsHandler::WriteAllFn
        //  will be called for the last time before any dtors are called
        guiRenderer = gvk::nullref;

        // Device wait idle
        gvk_result(device.DeviceWaitIdle());
    } gvk_result_scope_end;
    return gvkResult;
}

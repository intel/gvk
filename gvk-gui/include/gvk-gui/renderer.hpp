
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

#include "gvk-system/input.hpp"
#include "gvk-system/time.hpp"
#include "gvk-defines.hpp"
#include "gvk-handles/handles.hpp"
#include "gvk-handles/mesh.hpp"
#include "gvk-handles/utilities.hpp"

#include "imgui.h"

namespace gvk {
namespace gui {

class Renderer final
{
public:
    struct BeginInfo
    {
        float deltaTime { 0 };
        std::array<float, 2> extent { };
        const system::Input* pInput { };
        uint32_t textStreamCodePointCount { 0 };
        const uint32_t* pTextStreamCodePoints { nullptr };
    };

    static VkResult create(const Device& device, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, const RenderPass& renderPass, const VkAllocationCallbacks* pAllocator, Renderer* pRenderer);
    
    template <typename T>
    inline const T& get() const
    {
        assert(mReference && "Attempting to dereference nullref gvk::gui::Renderer");
        if constexpr (std::is_same_v<T, Device>) { return mReference->mDevice; }
        if constexpr (std::is_same_v<T, Pipeline>) { return mReference->mPipeline; }
    }

    void begin_gui(const BeginInfo& beginInfo);
    VkResult end_gui(uint32_t resourceId);
    void record_cmds(VkCommandBuffer vkCommandBuffer, uint32_t resourceId) const;

private:
    VkResult create_pipeline(const RenderPass& renderPass, const VkAllocationCallbacks* pAllocator);
    VkResult create_image_view_and_sampler(VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, const VkAllocationCallbacks* pAllocator);
    VkResult allocate_and_update_descriptor_set(const VkAllocationCallbacks* pAllocator);
    void record_render_state_setup_cmds(VkCommandBuffer vkCommandBuffer, const ImDrawData* pImDrawData, VkBuffer vkVertexIndexBuffer, VkDeviceSize indexDataOffset) const;

    class VertexIndexBufferResources final
    {
    public:
        Buffer buffer{ VK_NULL_HANDLE };
        VkDeviceSize indexDataOffset{ };
        VkDeviceSize indexCount{ };
    };

    class ControlBlock final
    {
    public:
        ControlBlock();
        ~ControlBlock();
        ImGuiContext* mpImGuiContext{ nullptr };
        Device mDevice{ VK_NULL_HANDLE };
        Pipeline mPipeline{ VK_NULL_HANDLE };
        ImageView mFontImageView{ VK_NULL_HANDLE };
        Sampler mFontSampler{ VK_NULL_HANDLE };
        DescriptorSet mFontDescriptorSet{ VK_NULL_HANDLE };
        std::unordered_map<uint32_t, VertexIndexBufferResources> mVertexIndexBufferResources;
    private:
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock& operator=(const ControlBlock&) = delete;
    };

    gvk_reference_type(Renderer)
};

} // namespace gui
} // namespace gvk


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

    Renderer() = default;
    Renderer(Renderer&& other);
    Renderer& operator=(Renderer&& other);
    static VkResult create(const Device& device, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, const RenderPass& renderPass, const VkAllocationCallbacks* pAllocator, Renderer* pRenderer);
    ~Renderer();

    const Device& get_device() const;
    const Pipeline& get_pipeline() const;

    void begin_gui(const BeginInfo& beginInfo);
    VkResult end_gui(uint32_t fenceCount, const VkFence* pVkFences);
    void record_cmds(VkCommandBuffer vkCommandBuffer) const;

private:
    VkResult create_pipeline(const RenderPass& renderPass, const VkAllocationCallbacks* pAllocator);
    VkResult create_image_view_and_sampler(VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, const VkAllocationCallbacks* pAllocator);
    VkResult allocate_and_update_descriptor_set(const VkAllocationCallbacks* pAllocator);
    void record_render_state_setup_cmds(VkCommandBuffer vkCommandBuffer, const ImDrawData* pImDrawData) const;

    ImGuiContext* mpImGuiContext { nullptr };
    Device mDevice;
    Pipeline mPipeline;
    ImageView mFontImageView;
    Sampler mFontSampler;
    DescriptorSet mFontDescriptorSet;
    Buffer mVertexIndexBuffer;
    VkDeviceSize mIndexDataOffset { };
    uint32_t mIndexCount { };
};

} // namespace gui
} // namespace gvk

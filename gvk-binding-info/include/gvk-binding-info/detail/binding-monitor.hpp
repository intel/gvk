
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

#include "gvk-binding-info/detail/binding-registry.hpp"

#include <vector>

namespace gvk {
namespace detail {

typedef void(*PFN_getObjectCreateInfo)(uint64_t dispatchableHandle, VkObjectType objectType, uint64_t handle, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo);

class BindingRegistry::BindingMonitor final
{
public:
    BindingMonitor() = default;
    BindingMonitor(BindingMonitor&& other) noexcept;
    BindingMonitor& operator=(BindingMonitor&& other) noexcept;
    ~BindingMonitor();
    void reset();

private:
    VkPipelineBindPoint mBindPoint{ };
    std::pair<uint64_t, uint64_t> mInterval{ };
    const GvkResourceInfo* mpResourceInfo{ };
    const GvkBindingInfoBaseStructure* mpBindingInfo{ };
    BindingRegistry* mpResourceRegistry{ };

    BindingMonitor(const BindingMonitor&) = delete;
    BindingMonitor& operator=(BindingMonitor&) = delete;
    friend class BindingRegistry;
};

struct BindingSlot
{
    uint32_t setIndex{ };
    uint32_t bindingIndex{ };
    uint32_t arrayIndex{ };
};

class DescriptorBindingMonitorCollection final
{
public:
    DescriptorBindingMonitorCollection() = default;
    void reset();
    void reset(uint32_t setIndex);
    BindingRegistry::BindingMonitor& operator[](const BindingSlot& bindingSlot);

private:
    std::vector<std::vector<std::vector<BindingRegistry::BindingMonitor>>> mBindingMonitors;
    DescriptorBindingMonitorCollection(const DescriptorBindingMonitorCollection&) = delete;
    DescriptorBindingMonitorCollection& operator=(DescriptorBindingMonitorCollection&) = delete;
};

class RenderPassBindingMonitorCollection final
{
public:
    class Attachment final
    {
    public:
        operator bool() const;
        VkImage image{ };
        gvk::Auto<VkImageCreateInfo> imageCreateInfo;
        VkImageView imageView{ };
        gvk::Auto<VkImageViewCreateInfo> imageViewCreateInfo;
    };

    RenderPassBindingMonitorCollection() = default;
    ~RenderPassBindingMonitorCollection();
    void reset();
    uint32_t get_attachment_count() const;
    VkAttachmentDescription2 get_attachment_description(uint32_t attachmentIndex) const;
    const Attachment& get_attachment(uint32_t attachmentIndex) const;
    const std::vector<uint32_t>& get_first_time_load_attachment_indices() const;
    void begin_render_pass(VkDevice device, const VkRenderPassBeginInfo& renderPassBeginInfo, PFN_getObjectCreateInfo pfnGetObjectCreateInfo);
    void next_subpass();
    void end_render_pass();
    void bind_load_and_clear_attachments(BindingRegistry& bindingRegistry);
    void bind_resolve_attachments(BindingRegistry& bindingRegistry);
    void bind_framebuffer_attachments(BindingRegistry& bindingRegistry);
    void bind_framebuffer_attachment(BindingRegistry& bindingRegistry, uint32_t attachmentIndex);

    VkDevice mDevice{ };
    VkRenderPass mRenderPass{ };
    gvk::Auto<VkRenderPassCreateInfo2> mRenderPassCreateInfo;
    VkFramebuffer mFramebuffer{ };
    gvk::Auto<VkFramebufferCreateInfo> mFramebufferCreateInfo;
    std::vector<Attachment> mAttachments;
    const VkSubpassDescription2* mpPreviousSubpassDescription{ };
    const VkSubpassDescription2* mpCurrentSubpassDescription{ };
    std::vector<BindingRegistry::BindingMonitor> mResolveBindingMonitors;
    std::vector<BindingRegistry::BindingMonitor> mAttachmentBindingMonitors;

private:
    void update_subpass();
    void update_first_time_load_attachment_index(uint32_t attachmentIndex);

    uint32_t mCurrentSubpassIndex{ };
    std::set<uint32_t> mUnusedAttachmentIndices;
    std::vector<uint32_t> mFirstTimeLoadAttachmentIndices;

    RenderPassBindingMonitorCollection(RenderPassBindingMonitorCollection const&) = delete;
    RenderPassBindingMonitorCollection& operator=(RenderPassBindingMonitorCollection const&) = delete;
};

class BindingMonitorCollection
{
public:
    BindingMonitorCollection() = default;
    virtual ~BindingMonitorCollection() = 0;
    virtual void reset();

    BindingRegistry::BindingMonitor pipelineBindingMonitor;
    std::vector<BindingRegistry::BindingMonitor> shaderBindingMonitors;
    DescriptorBindingMonitorCollection descriptorBindingMonitors;

private:
    BindingMonitorCollection(const BindingMonitorCollection&) = delete;
    BindingMonitorCollection& operator=(BindingMonitorCollection&) = delete;
};

class ComputeBindingMonitorCollection final
    : public BindingMonitorCollection
{
};

class GraphicsBindingMonitorCollection final
    : public BindingMonitorCollection
{
public:
    void reset() override final;

    BindingRegistry::BindingMonitor indexBufferBindingMonitor;
    std::vector<BindingRegistry::BindingMonitor> vertexBufferBindingMonitors;
    RenderPassBindingMonitorCollection renderPassBindingMonitors;
};

class RayTracingBindingMonitorCollection final
    : public BindingMonitorCollection
{
};

} // namespace detail
} // namespace gvk

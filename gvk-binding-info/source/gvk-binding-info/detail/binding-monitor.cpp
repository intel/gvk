
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

#include "gvk-binding-info/detail/binding-monitor.hpp"
#include "gvk-containers/utilities.hpp"
#include "gvk-format-info.hpp"

#include <utility>

namespace gvk {
namespace detail {

BindingRegistry::BindingMonitor::BindingMonitor(BindingMonitor&& other) noexcept
{
    *this = std::move(other);
}

BindingRegistry::BindingMonitor& BindingRegistry::BindingMonitor::operator=(BindingMonitor&& other) noexcept
{
    if (this != &other) {
        reset();
        mBindPoint = std::exchange(other.mBindPoint, mBindPoint);
        mInterval = std::exchange(other.mInterval, mInterval);
        mpResourceInfo = std::exchange(other.mpResourceInfo, mpResourceInfo);
        mpBindingInfo = std::exchange(other.mpBindingInfo, mpBindingInfo);
        mpResourceRegistry = std::exchange(other.mpResourceRegistry, mpResourceRegistry);
    }
    return *this;
}

BindingRegistry::BindingMonitor::~BindingMonitor()
{
    reset();
}

void BindingRegistry::BindingMonitor::reset()
{
    if (mpResourceRegistry) {
        mpResourceRegistry->on_binding_monitor_reset(*this);
    }
    mBindPoint = { };
    mInterval = { };
    mpResourceInfo = nullptr;
    mpBindingInfo = nullptr;
    mpResourceRegistry = nullptr;
}

void DescriptorBindingMonitorCollection::reset()
{
    for (auto& setMonitors : mBindingMonitors) {
        for (auto& arrayMonitors : setMonitors) {
            arrayMonitors.clear();
        }
    }
}

void DescriptorBindingMonitorCollection::reset(uint32_t setIndex)
{
    if (setIndex < mBindingMonitors.size()) {
        for (auto& arrayMonitors : mBindingMonitors[setIndex]) {
            for (auto& arrayMonitor : arrayMonitors) {
                arrayMonitor.reset();
            }
        }
    }
}

BindingRegistry::BindingMonitor& DescriptorBindingMonitorCollection::operator[](const BindingSlot& bindingSlot)
{
    validate_size_for_index(bindingSlot.setIndex, mBindingMonitors);
    validate_size_for_index(bindingSlot.bindingIndex, mBindingMonitors[bindingSlot.setIndex]);
    validate_size_for_index(bindingSlot.arrayIndex, mBindingMonitors[bindingSlot.setIndex][bindingSlot.bindingIndex]);
    return mBindingMonitors[bindingSlot.setIndex][bindingSlot.bindingIndex][bindingSlot.arrayIndex];
}

RenderPassBindingMonitorCollection::Attachment::operator bool() const
{
    return
        image && imageCreateInfo->sType == gvk::get_stype<VkImageCreateInfo>() &&
        imageView && imageViewCreateInfo->sType == gvk::get_stype<VkImageViewCreateInfo>();
}

RenderPassBindingMonitorCollection::~RenderPassBindingMonitorCollection()
{
    reset();
}

void RenderPassBindingMonitorCollection::reset()
{
    mDevice = VK_NULL_HANDLE;
    mRenderPass = VK_NULL_HANDLE;
    mRenderPassCreateInfo.reset();
    mFramebuffer = VK_NULL_HANDLE;
    mFramebufferCreateInfo.reset();
    mAttachments.clear();
    mpPreviousSubpassDescription = nullptr;
    mpCurrentSubpassDescription = nullptr;
    mResolveBindingMonitors.clear();
    mAttachmentBindingMonitors.clear();
    mCurrentSubpassIndex = 0;
    mUnusedAttachmentIndices.clear();
    mFirstTimeLoadAttachmentIndices.clear();
}

uint32_t RenderPassBindingMonitorCollection::get_attachment_count() const
{
    return mRenderPassCreateInfo->attachmentCount;
}

VkAttachmentDescription2 RenderPassBindingMonitorCollection::get_attachment_description(uint32_t attachmentIndex) const
{
    return attachmentIndex < mRenderPassCreateInfo->attachmentCount ? mRenderPassCreateInfo->pAttachments[attachmentIndex] : VkAttachmentDescription2{ };
}

const RenderPassBindingMonitorCollection::Attachment& RenderPassBindingMonitorCollection::get_attachment(uint32_t attachmentIndex) const
{
    static Attachment sNullAttachment;
    return attachmentIndex != VK_ATTACHMENT_UNUSED && attachmentIndex < mAttachments.size() ? mAttachments[attachmentIndex] : sNullAttachment;
}

const std::vector<uint32_t>& RenderPassBindingMonitorCollection::get_first_time_load_attachment_indices() const
{
    return mFirstTimeLoadAttachmentIndices;
}

template <typename CreateInfoType, typename ObjectType>
static gvk::Auto<CreateInfoType> get_object_create_info(VkDevice device, ObjectType object, PFN_getObjectCreateInfo pfnGetObjectCreateInfo)
{
    assert(pfnGetObjectCreateInfo);
    auto createInfoType = gvk::get_stype<CreateInfoType>();
    CreateInfoType createInfo{ };
    pfnGetObjectCreateInfo((uint64_t)device, gvk::detail::get_object_type<ObjectType>(), (uint64_t)object, &createInfoType, (VkBaseOutStructure*)&createInfo);
    return createInfo.sType == gvk::get_stype<CreateInfoType>() ? createInfo : CreateInfoType{ };
}

static gvk::Auto<VkRenderPassCreateInfo2> get_render_pass_create_info(VkDevice device, VkRenderPass renderPass, PFN_getObjectCreateInfo pfnGetObjectCreateInfo)
{
    assert(pfnGetObjectCreateInfo);
    VkStructureType createInfoType{ };
    pfnGetObjectCreateInfo((uint64_t)device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderPass, &createInfoType, nullptr);
    switch (createInfoType) {
    case gvk::get_stype<VkRenderPassCreateInfo>(): {
        auto createInfo = gvk::get_default<VkRenderPassCreateInfo>();
        pfnGetObjectCreateInfo((uint64_t)device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderPass, &createInfoType, (VkBaseOutStructure*)&createInfo);
        return gvk::convert<VkRenderPassCreateInfo, VkRenderPassCreateInfo2>(createInfo);
    } break;
    case gvk::get_stype<VkRenderPassCreateInfo2>(): {
        auto createInfo = gvk::get_default<VkRenderPassCreateInfo2>();
        pfnGetObjectCreateInfo((uint64_t)device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderPass, &createInfoType, (VkBaseOutStructure*)&createInfo);
        return createInfo;
    } break;
    default: {
        assert(false && "Unserviced VkStructureType encountered; gvk maintenance required");
    } break;
    }
    return { };
}

void RenderPassBindingMonitorCollection::begin_render_pass(VkDevice device, const VkRenderPassBeginInfo& renderPassBeginInfo, PFN_getObjectCreateInfo pfnGetObjectCreateInfo)
{
    reset();
    mDevice = device;
    mRenderPass = renderPassBeginInfo.renderPass;
    mRenderPassCreateInfo = get_render_pass_create_info(device, mRenderPass, pfnGetObjectCreateInfo);
    mFramebuffer = renderPassBeginInfo.framebuffer;
    mFramebufferCreateInfo = get_object_create_info<VkFramebufferCreateInfo>(device, mFramebuffer, pfnGetObjectCreateInfo);
    assert(mFramebufferCreateInfo->sType == gvk::get_stype<VkFramebufferCreateInfo>());
    mAttachments.resize(mFramebufferCreateInfo->attachmentCount);
    for (uint32_t attachment_i = 0; attachment_i < mFramebufferCreateInfo->attachmentCount; ++attachment_i) {
        auto& attachment = mAttachments[attachment_i];
        attachment.imageView = mFramebufferCreateInfo->pAttachments[attachment_i];
        attachment.imageViewCreateInfo = get_object_create_info<VkImageViewCreateInfo>(device, attachment.imageView, pfnGetObjectCreateInfo);
        attachment.image = attachment.imageViewCreateInfo->image;
        attachment.imageCreateInfo = get_object_create_info<VkImageCreateInfo>(device, attachment.image, pfnGetObjectCreateInfo);
        mUnusedAttachmentIndices.insert(attachment_i);
    }
    update_subpass();
}

void RenderPassBindingMonitorCollection::next_subpass()
{
    ++mCurrentSubpassIndex;
    update_subpass();
    mResolveBindingMonitors.clear();
    mAttachmentBindingMonitors.clear();
}

void RenderPassBindingMonitorCollection::end_render_pass()
{
    reset();
}

void RenderPassBindingMonitorCollection::bind_load_and_clear_attachments(BindingRegistry& bindingRegistry)
{
    for (auto attachmentIndex : get_first_time_load_attachment_indices()) {
        const auto& attachmentDescription = get_attachment_description(attachmentIndex);
        const auto& attachment = get_attachment(attachmentIndex);
        if (attachment) {
            auto imageAspectFlags = gvk::get_image_aspect_flags(attachmentDescription.format);
            bool depth = imageAspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT;
            bool stencil = imageAspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT;
            bool color = !depth && !stencil;
            (void)color;

            GvkResourceInfo imageResourceInfo{ };
            imageResourceInfo.type = VK_OBJECT_TYPE_IMAGE;
            imageResourceInfo.handle = (uint64_t)attachment.image;
            imageResourceInfo.dispatchableHandle = (uint64_t)mDevice;
            imageResourceInfo.pCreateInfo = (const VkBaseOutStructure*)&*attachment.imageCreateInfo;
            auto pImageResourceInfo = bindingRegistry.register_resource_info(&imageResourceInfo);
            (void)pImageResourceInfo;

            GvkResourceInfo imageViewResourceInfo{ };
            imageViewResourceInfo.type = VK_OBJECT_TYPE_IMAGE_VIEW;
            imageViewResourceInfo.handle = (uint64_t)attachment.imageView;
            imageViewResourceInfo.dispatchableHandle = (uint64_t)mDevice;
            imageViewResourceInfo.pCreateInfo = (const VkBaseOutStructure*)&*attachment.imageViewCreateInfo;
            auto pImageViewResourceInfo = bindingRegistry.register_resource_info(&imageViewResourceInfo);
            (void)pImageViewResourceInfo;

            // TODO : GvkResourceTransferBindingInfo
        }
    }
}

void RenderPassBindingMonitorCollection::bind_resolve_attachments(BindingRegistry& bindingRegistry)
{
    (void)bindingRegistry;
    // TODO : GvkResourceTransferBindingInfo
}

void RenderPassBindingMonitorCollection::bind_framebuffer_attachments(BindingRegistry& bindingRegistry)
{
    if (mpCurrentSubpassDescription) {
        if (mpCurrentSubpassDescription->pColorAttachments) {
            for (uint32_t attachment_i = 0; attachment_i < mpCurrentSubpassDescription->colorAttachmentCount; ++attachment_i) {
                bind_framebuffer_attachment(bindingRegistry, mpCurrentSubpassDescription->pColorAttachments[attachment_i].attachment);
            }
        }
        if (mpCurrentSubpassDescription->pDepthStencilAttachment) {
            bind_framebuffer_attachment(bindingRegistry, mpCurrentSubpassDescription->pDepthStencilAttachment->attachment);
        }
    }
}

void RenderPassBindingMonitorCollection::bind_framebuffer_attachment(BindingRegistry& bindingRegistry, uint32_t attachmentIndex)
{
    const auto& attachment = get_attachment(attachmentIndex);
    if (attachment) {
        GvkResourceInfo imageResourceInfo{ };
        imageResourceInfo.type = VK_OBJECT_TYPE_IMAGE;
        imageResourceInfo.handle = (uint64_t)attachment.image;
        imageResourceInfo.dispatchableHandle = (uint64_t)mDevice;
        imageResourceInfo.pCreateInfo = (const VkBaseOutStructure*)&*attachment.imageCreateInfo;
        auto pImageResourceInfo = bindingRegistry.register_resource_info(&imageResourceInfo);

        GvkResourceInfo imageViewResourceInfo{ };
        imageViewResourceInfo.type = VK_OBJECT_TYPE_IMAGE_VIEW;
        imageViewResourceInfo.handle = (uint64_t)attachment.imageView;
        imageViewResourceInfo.dispatchableHandle = (uint64_t)mDevice;
        imageViewResourceInfo.pCreateInfo = (const VkBaseOutStructure*)&*attachment.imageViewCreateInfo;
        auto pImageViewResourceInfo = bindingRegistry.register_resource_info(&imageViewResourceInfo);

        auto renderTargetBindingInfo = gvk::get_default<GvkRenderTargetBindingInfo>();
        renderTargetBindingInfo.pResourceInfo = pImageResourceInfo;
        renderTargetBindingInfo.pResourceViewInfo = pImageViewResourceInfo;
        auto pRenderTargetBindingInfo = bindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&renderTargetBindingInfo);

        mAttachmentBindingMonitors.push_back({ });
        bindingRegistry.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, pImageResourceInfo, pRenderTargetBindingInfo, &mAttachmentBindingMonitors.back());
    }
}

void RenderPassBindingMonitorCollection::update_subpass()
{
    mpPreviousSubpassDescription = nullptr;
    mpCurrentSubpassDescription = nullptr;
    mFirstTimeLoadAttachmentIndices.clear();
    if (mCurrentSubpassIndex < mRenderPassCreateInfo->subpassCount) {
        mpCurrentSubpassDescription = &mRenderPassCreateInfo->pSubpasses[mCurrentSubpassIndex];
        if (mpCurrentSubpassDescription->pInputAttachments) {
            for (uint32_t i = 0; i < mpCurrentSubpassDescription->inputAttachmentCount; ++i) {
                update_first_time_load_attachment_index(mpCurrentSubpassDescription->pInputAttachments[i].attachment);
            }
        }
        if (mpCurrentSubpassDescription->pColorAttachments) {
            for (uint32_t i = 0; i < mpCurrentSubpassDescription->colorAttachmentCount; ++i) {
                update_first_time_load_attachment_index(mpCurrentSubpassDescription->pColorAttachments[i].attachment);
            }
        }
        if (mpCurrentSubpassDescription->pResolveAttachments) {
            for (uint32_t i = 0; i < mpCurrentSubpassDescription->colorAttachmentCount; ++i) {
                update_first_time_load_attachment_index(mpCurrentSubpassDescription->pResolveAttachments[i].attachment);
            }
        }
        if (mpCurrentSubpassDescription->pDepthStencilAttachment) {
            update_first_time_load_attachment_index(mpCurrentSubpassDescription->pDepthStencilAttachment->attachment);
        }
    }
    auto previousSubpassIndex = mCurrentSubpassIndex ? mCurrentSubpassIndex - 1 : 0;
    if (previousSubpassIndex < mRenderPassCreateInfo->subpassCount) {
        mpPreviousSubpassDescription = &mRenderPassCreateInfo->pSubpasses[previousSubpassIndex];
    }
}

void RenderPassBindingMonitorCollection::update_first_time_load_attachment_index(uint32_t attachmentIndex)
{
    auto itr = mUnusedAttachmentIndices.find(attachmentIndex);
    if (itr != mUnusedAttachmentIndices.end()) {
        mFirstTimeLoadAttachmentIndices.push_back(*itr);
        mUnusedAttachmentIndices.erase(itr);
    }
}

BindingMonitorCollection::~BindingMonitorCollection()
{
    reset();
}

void BindingMonitorCollection::reset()
{
    pipelineBindingMonitor.reset();
    shaderBindingMonitors.clear();
    descriptorBindingMonitors.reset();
}

void GraphicsBindingMonitorCollection::reset()
{
    BindingMonitorCollection::reset();
    indexBufferBindingMonitor.reset();
    vertexBufferBindingMonitors.clear();
    renderPassBindingMonitors.reset();
}

} // namespace detail
} // namespace gvk

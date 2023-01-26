
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

#include "gvk-defines.hpp"
#include "gvk-structures/auto.hpp"
#include "gvk-structures/comparison-operators.hpp"
#include "gvk-structures/copy.hpp"
#include "gvk-structures/defaults.hpp"
#include "gvk-structures/get-stype.hpp"

#include <utility>

namespace gvk {
namespace detail {

// NOTE : These functions operate on Auto<> structures that we are allocating
//  storage for and populating manually.  Auto<>'s dtor takes care of cleanup.

template <typename T>
inline void set_stypes(uint32_t objCount, const T* pObjs)
{
    if (objCount && pObjs) {
        for (uint32_t i = 0; i < objCount; ++i) {
            const_cast<T*>(pObjs)[i].sType = get_stype<T>();
        }
    }
}

template<typename SrcType, typename DstType, typename ArrayElementConversionFunctionType>
inline void convert_array(
    uint32_t srcObjCount, const SrcType* pSrcObjs,
    const uint32_t& dstObjCount, const DstType*& pDstObjs,
    ArrayElementConversionFunctionType convertArrayElement
)
{
    if (srcObjCount && pSrcObjs) {
        const_cast<uint32_t&>(dstObjCount) = srcObjCount;
        pDstObjs = (DstType*)malloc(sizeof(DstType) * dstObjCount);
        for (uint32_t i = 0; i < dstObjCount; ++i) {
            convertArrayElement(pSrcObjs[i], const_cast<DstType*>(pDstObjs)[i]);
        }
    }
}

template<typename SrcAttachmentDescriptionType, typename DstAttachmentDescriptionType>
inline void convert_attachment_description(const SrcAttachmentDescriptionType& src, DstAttachmentDescriptionType& dst)
{
    dst = { };
    dst.flags = src.flags;
    dst.format = src.format;
    dst.samples = src.samples;
    dst.loadOp = src.loadOp;
    dst.storeOp = src.storeOp;
    dst.stencilLoadOp = src.stencilLoadOp;
    dst.stencilStoreOp = src.stencilStoreOp;
    dst.initialLayout = src.initialLayout;
    dst.finalLayout = src.finalLayout;
}

template<typename SrcAttachmentReferenceType, typename DstAttachmentReferenceType>
inline void convert_attachment_reference(const SrcAttachmentReferenceType& src, DstAttachmentReferenceType& dst)
{
    dst = { };
    dst.attachment = src.attachment;
    dst.layout = src.layout;
}

template<typename SrcSubpassDescriptionType, typename DstSubpassDescriptionType>
inline void convert_subpass_description(const SrcSubpassDescriptionType& src, DstSubpassDescriptionType& dst)
{
    dst = { };
    dst.flags = src.flags;
    dst.pipelineBindPoint = src.pipelineBindPoint;
    using SrcAttachmentReferenceType = std::remove_const_t<std::remove_pointer_t<decltype(src.pDepthStencilAttachment)>>;
    using DstAttachmentReferenceType = std::remove_const_t<std::remove_pointer_t<decltype(dst.pDepthStencilAttachment)>>;
    convert_array(
        src.inputAttachmentCount, src.pInputAttachments,
        dst.inputAttachmentCount, dst.pInputAttachments,
        convert_attachment_reference<SrcAttachmentReferenceType, DstAttachmentReferenceType>
    );
    convert_array(
        src.colorAttachmentCount, src.pColorAttachments,
        dst.colorAttachmentCount, dst.pColorAttachments,
        convert_attachment_reference<SrcAttachmentReferenceType, DstAttachmentReferenceType>
    );
    convert_array(
        src.colorAttachmentCount, src.pResolveAttachments,
        dst.colorAttachmentCount, dst.pResolveAttachments,
        convert_attachment_reference<SrcAttachmentReferenceType, DstAttachmentReferenceType>
    );
    if (src.pDepthStencilAttachment) {
        auto pDepthStencilAttachment = (DstAttachmentReferenceType*)malloc(sizeof(DstAttachmentReferenceType));
        convert_attachment_reference(*src.pDepthStencilAttachment, *pDepthStencilAttachment);
        dst.pDepthStencilAttachment = pDepthStencilAttachment;
    }
    convert_array(
        src.preserveAttachmentCount, src.pPreserveAttachments,
        dst.preserveAttachmentCount, dst.pPreserveAttachments,
        [](uint32_t srcPreserveAttachment, uint32_t& dstPreserveAttachment)
        {
            dstPreserveAttachment = srcPreserveAttachment;
        }
    );
}

template<typename SrcSubpassDependencyType, typename DstSubpassDependencyType>
inline void convert_subpass_dependency(const SrcSubpassDependencyType& src, DstSubpassDependencyType& dst)
{
    dst = { };
    dst.srcSubpass = src.srcSubpass;
    dst.dstSubpass = src.dstSubpass;
    dst.srcStageMask = src.srcStageMask;
    dst.dstStageMask = src.dstStageMask;
    dst.srcAccessMask = src.srcAccessMask;
    dst.dstAccessMask = src.dstAccessMask;
    dst.dependencyFlags = src.dependencyFlags;
}

template<typename SrcRenderPassCreateInfoType, typename DstRenderPassCreateInfoType>
inline void convert_render_pass_create_info(const SrcRenderPassCreateInfoType& src, DstRenderPassCreateInfoType& dst)
{
    dst.flags = src.flags;
    using SrcAttachmentDescriptionType = std::remove_const_t<std::remove_pointer_t<decltype(src.pAttachments)>>;
    using DstAttachmentDescriptionType = std::remove_const_t<std::remove_pointer_t<decltype(dst.pAttachments)>>;
    convert_array(
        src.attachmentCount, src.pAttachments,
        dst.attachmentCount, dst.pAttachments,
        convert_attachment_description<SrcAttachmentDescriptionType, DstAttachmentDescriptionType>
    );
    using SrcSubpassDescriptionType = std::remove_const_t<std::remove_pointer_t<decltype(src.pSubpasses)>>;
    using DstSubpassDescriptionType = std::remove_const_t<std::remove_pointer_t<decltype(dst.pSubpasses)>>;
    convert_array(
        src.subpassCount, src.pSubpasses,
        dst.subpassCount, dst.pSubpasses,
        convert_subpass_description<SrcSubpassDescriptionType, DstSubpassDescriptionType>
    );
    using SrcSubpassDependencyType = std::remove_const_t<std::remove_pointer_t<decltype(src.pDependencies)>>;
    using DstSubpassDependencyType = std::remove_const_t<std::remove_pointer_t<decltype(dst.pDependencies)>>;
    convert_array(
        src.dependencyCount, src.pDependencies,
        dst.dependencyCount, dst.pDependencies,
        convert_subpass_dependency<SrcSubpassDependencyType, DstSubpassDependencyType>
    );
}

} // namespace detail

template<typename SrcType, typename DstType>
inline Auto<DstType> convert(const SrcType& src)
{
    return src;
}

template<>
inline Auto<VkRenderPassCreateInfo2> convert<VkRenderPassCreateInfo, VkRenderPassCreateInfo2>(const VkRenderPassCreateInfo& src)
{
    Auto<VkRenderPassCreateInfo2> dst;
    auto& renderPassCreateInfo = const_cast<VkRenderPassCreateInfo2&>(*dst);
    detail::convert_render_pass_create_info<VkRenderPassCreateInfo, VkRenderPassCreateInfo2>(src, renderPassCreateInfo);
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    renderPassCreateInfo.pNext = detail::create_pnext_copy(src.pNext, nullptr);
    detail::set_stypes(renderPassCreateInfo.attachmentCount, renderPassCreateInfo.pAttachments);
    detail::set_stypes(renderPassCreateInfo.subpassCount, renderPassCreateInfo.pSubpasses);
    if (renderPassCreateInfo.subpassCount && renderPassCreateInfo.pSubpasses) {
        for (uint32_t i = 0; i < renderPassCreateInfo.subpassCount; ++i) {
            const auto& subpass = renderPassCreateInfo.pSubpasses[i];
            detail::set_stypes(subpass.inputAttachmentCount, subpass.pInputAttachments);
            detail::set_stypes(subpass.colorAttachmentCount, subpass.pColorAttachments);
            detail::set_stypes(subpass.colorAttachmentCount, subpass.pResolveAttachments);
            detail::set_stypes(1, subpass.pDepthStencilAttachment);
        }
    }
    detail::set_stypes(renderPassCreateInfo.dependencyCount, renderPassCreateInfo.pDependencies);
    return dst;
}

template<>
inline Auto<VkRenderPassCreateInfo> convert<VkRenderPassCreateInfo2, VkRenderPassCreateInfo>(const VkRenderPassCreateInfo2& src)
{
    Auto<VkRenderPassCreateInfo> dst;
    auto& renderPassCreateInfo = const_cast<VkRenderPassCreateInfo&>(*dst);
    detail::convert_render_pass_create_info<VkRenderPassCreateInfo2, VkRenderPassCreateInfo>(src, renderPassCreateInfo);
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = detail::create_pnext_copy(src.pNext, nullptr);
    return dst;
}

} // namespace gvk

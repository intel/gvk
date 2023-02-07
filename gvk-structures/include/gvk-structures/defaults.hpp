
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

#include "gvk-structures/get-stype.hpp"
#include "gvk-defines.hpp"

#include <type_traits>

namespace gvk {

template <typename StructureType, typename = int>
struct has_stype : std::false_type
{
};

template <typename StructureType>
struct has_stype<StructureType, decltype((void)StructureType::sType, 0)> : std::true_type
{
};

template <typename StructureType>
inline const StructureType& get_default()
{
    static const StructureType DefaultStructure =
    []()
    {
        StructureType structure { };
        if constexpr (has_stype<StructureType>::value) {
            structure.sType = get_stype<StructureType>();
        }
        return structure;
    }();
    return DefaultStructure;
}

template <> const VkApplicationInfo& get_default<VkApplicationInfo>();
template <> const VkInstanceCreateInfo& get_default<VkInstanceCreateInfo>();
template <> const VkAttachmentDescription& get_default<VkAttachmentDescription>();
template <> const VkAttachmentDescription2& get_default<VkAttachmentDescription2>();
template <> const VkBufferImageCopy& get_default<VkBufferImageCopy>();
template <> const VkDescriptorBufferInfo& get_default<VkDescriptorBufferInfo>();
template <> const VkDescriptorPoolCreateInfo& get_default<VkDescriptorPoolCreateInfo>();
template <> const VkDebugUtilsMessengerCreateInfoEXT& get_default<VkDebugUtilsMessengerCreateInfoEXT>();
template <> const VkDeviceQueueCreateInfo& get_default<VkDeviceQueueCreateInfo>();
template <> const VkFramebufferCreateInfo& get_default<VkFramebufferCreateInfo>();
template <> const VkImageCreateInfo& get_default<VkImageCreateInfo>();
template <> const VkImageMemoryBarrier& get_default<VkImageMemoryBarrier>();
template <> const VkImageMemoryBarrier2& get_default<VkImageMemoryBarrier2>();
template <> const VkImageSubresourceLayers& get_default<VkImageSubresourceLayers>();
template <> const VkImageSubresourceRange& get_default<VkImageSubresourceRange>();
template <> const VkImageViewCreateInfo& get_default<VkImageViewCreateInfo>();
template <> const VkSamplerCreateInfo& get_default<VkSamplerCreateInfo>();
template <> const VkSwapchainCreateInfoKHR& get_default<VkSwapchainCreateInfoKHR>();

////////////////////////////////////////////////////////////////////////////////
// VkGraphicsPipelineCreateInfo defaults
template <> const VkPipelineShaderStageCreateInfo& get_default<VkPipelineShaderStageCreateInfo>();
template <> const VkPipelineInputAssemblyStateCreateInfo& get_default<VkPipelineInputAssemblyStateCreateInfo>();
template <> const VkViewport& get_default<VkViewport>();
template <> const VkPipelineViewportStateCreateInfo& get_default<VkPipelineViewportStateCreateInfo>();
template <> const VkPipelineRasterizationStateCreateInfo& get_default<VkPipelineRasterizationStateCreateInfo>();
template <> const VkPipelineMultisampleStateCreateInfo& get_default<VkPipelineMultisampleStateCreateInfo>();
template <> const VkPipelineColorBlendAttachmentState& get_default<VkPipelineColorBlendAttachmentState>();
template <> const VkPipelineColorBlendStateCreateInfo& get_default<VkPipelineColorBlendStateCreateInfo>();
template <> const VkPipelineDynamicStateCreateInfo& get_default<VkPipelineDynamicStateCreateInfo>();
template <> const VkGraphicsPipelineCreateInfo& get_default<VkGraphicsPipelineCreateInfo>();
template <> const VkComputePipelineCreateInfo& get_default<VkComputePipelineCreateInfo>();

} // namespace gvk


/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include "gvk/generated/get-stype.hpp"
#include "gvk/defines.hpp"

namespace gvk {

template <typename VulkanStructureType, typename = int>
struct has_stype : std::false_type
{
};

template <typename VulkanStructureType>
struct has_stype<VulkanStructureType, decltype((void)VulkanStructureType::sType, 0)> : std::true_type
{
};

template <typename VulkanStructureType>
inline const VulkanStructureType& get_default()
{
    static const VulkanStructureType DefaultVulkanStructure =
    []()
    {
        VulkanStructureType vulkanStructure { };
        if constexpr (has_stype<VulkanStructureType>::value) {
            vulkanStructure.sType = get_stype<VulkanStructureType>();
        }
        return vulkanStructure;
    }();
    return DefaultVulkanStructure;
}

template <> const VkApplicationInfo& get_default<VkApplicationInfo>();
template <> const VkAttachmentDescription& get_default<VkAttachmentDescription>();
template <> const VkAttachmentDescription2& get_default<VkAttachmentDescription2>();
template <> const VkBufferImageCopy& get_default<VkBufferImageCopy>();
template <> const VkDescriptorBufferInfo& get_default<VkDescriptorBufferInfo>();
template <> const VkDebugUtilsMessengerCreateInfoEXT& get_default<VkDebugUtilsMessengerCreateInfoEXT>();
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

} // namespace gvk

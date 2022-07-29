
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

#include "gvk/generated/dispatch-table.hpp"
#include "gvk/generated/handles.hpp"
#include "gvk/defines.hpp"

#include <vector>

namespace gvk {

using DescriptorSetLayouts = const std::vector<DescriptorSetLayout>&;
using ImageViews = const std::vector<ImageView>&;
using PhysicalDevices = const std::vector<PhysicalDevice>&;
using Images = const std::vector<Image>&;
using QueueFamilies = const std::vector<QueueFamily>&;

template <typename GvkHandleType>
inline std::vector<typename GvkHandleType::VkHandleType> get_vk_handles(const std::vector<GvkHandleType>& gvkHandles)
{
    std::vector<typename GvkHandleType::VkHandleType> vkHandles;
    vkHandles.reserve(gvkHandles.size());
    for (const auto& gvkHandle : gvkHandles) {
        vkHandles.push_back(gvkHandle);
    }
    return vkHandles;
}

} // namespace gvk

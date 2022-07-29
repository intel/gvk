
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

#include "gvk/generated/forward-declarations.hpp"
#include "gvk/defines.hpp"

#include <cassert>
#include <vector>

namespace gvk {

class QueueFamily final
{
public:
    uint32_t index{ };
    std::vector<Queue> queues;
};

/**
Gets a gvk::QueueFamily given a gvk::Device and index
@param [in] device The gvk::Device to get the gvk::QueueFamily from
@param [in] queueFamilyIndex The index of the gvk::QueueFamily to get
@return The given gvk::Device object's gvk::QueueFamily at the specified index
*/
const QueueFamily& get_queue_family(const Device& device, uint32_t queueFamilyIndex);

namespace detail {

void* get_transient_storage(size_t size);

template <typename ControlBlockType>
inline VkResult initialize_control_block(ControlBlockType&)
{
    return VK_SUCCESS;
}

template <> VkResult initialize_control_block<DeviceControlBlock>(DeviceControlBlock& controlBlock);
template <> VkResult initialize_control_block<FramebufferControlBlock>(FramebufferControlBlock& controlBlock);
template <> VkResult initialize_control_block<InstanceControlBlock>(InstanceControlBlock& controlBlock);
template <> VkResult initialize_control_block<PipelineLayoutControlBlock>(PipelineLayoutControlBlock& controlBlock);
template <> VkResult initialize_control_block<RenderPassControlBlock>(RenderPassControlBlock& controlBlock);
template <> VkResult initialize_control_block<SwapchainKHRControlBlock>(SwapchainKHRControlBlock& controlBlock);

} // namespace detail
} // namespace gvk

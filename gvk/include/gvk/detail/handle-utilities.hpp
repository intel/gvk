
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
Gets a QueueFamily given a Device and index
@param [in] device The Device to get the QueueFamily from
@param [in] queueFamilyIndex The index of the QueueFamily to get
@return The given Device object's QueueFamily at the specified index
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

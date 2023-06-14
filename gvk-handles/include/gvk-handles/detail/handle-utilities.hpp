
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

#include "gvk-handles/generated/forward-declarations.inl"
#include "gvk-handles/defines.hpp"
#include "gvk-reference.hpp"

#include <cassert>
#include <tuple>
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

template <typename HandleType>
inline VkResult initialize_control_block(HandleType&)
{
    return VK_SUCCESS;
}

template <> VkResult initialize_control_block<Instance>(Instance& instance);
template <> VkResult initialize_control_block<Device>(Device& device);
template <> VkResult initialize_control_block<CommandBuffer>(CommandBuffer& commandBuffer);
template <> VkResult initialize_control_block<Framebuffer>(Framebuffer& framebuffer);
template <> VkResult initialize_control_block<PipelineLayout>(PipelineLayout& pipelineLayout);
template <> VkResult initialize_control_block<ShaderEXT>(ShaderEXT& shader);
template <> VkResult initialize_control_block<SurfaceKHR>(SurfaceKHR& surface);
template <> VkResult initialize_control_block<SwapchainKHR>(SwapchainKHR& swapchain);

} // namespace detail
} // namespace gvk

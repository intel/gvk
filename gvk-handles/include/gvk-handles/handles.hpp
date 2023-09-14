
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

#include "gvk-dispatch-table.hpp"
#include "gvk-handles/generated/handles.hpp"
#include "gvk-defines.hpp"

#include <vector>

namespace gvk {

using DescriptorSetLayouts = std::vector<DescriptorSetLayout>;
using Images = std::vector<Image>;
using ImageViews = std::vector<ImageView>;
using PhysicalDevices = std::vector<PhysicalDevice>;
using QueueFamilies = std::vector<QueueFamily>;

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

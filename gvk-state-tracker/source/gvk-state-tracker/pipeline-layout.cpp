
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

#include "gvk-state-tracker/state-tracker.hpp"
#include "gvk-layer/registry.hpp"

#include <cassert>

namespace gvk {
namespace state_tracker {

VkResult StateTracker::post_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        gvkResult = BasicStateTracker::post_vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, gvkResult);
        assert(gvkResult == VK_SUCCESS);
        PipelineLayout gvkPipelineLayout({ device, *pPipelineLayout });
        assert(gvkPipelineLayout);
        auto& controlBlock = gvkPipelineLayout.mReference.get_obj();
        controlBlock.mDescriptorSetLayouts.resize(pCreateInfo->setLayoutCount);
        for (uint32_t setLayout_i = 0; setLayout_i < pCreateInfo->setLayoutCount; ++setLayout_i) {
            controlBlock.mDescriptorSetLayouts[setLayout_i] = DescriptorSetLayout({ device, pCreateInfo->pSetLayouts[setLayout_i] });
            assert(controlBlock.mDescriptorSetLayouts[setLayout_i]);
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk


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

VkResult StateTracker::post_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, VkResult gvkResult)
{
    if (gvkResult == VK_SUCCESS) {
        Device gvkDevice = device;
        assert(gvkDevice);
        assert(createInfoCount);
        assert(pCreateInfos);
        assert(pShaders);
        for (uint32_t createInfo_i = 0; createInfo_i < createInfoCount; ++createInfo_i) {
            const auto& createInfo = pCreateInfos[createInfo_i];
            ShaderEXT gvkShader;
            gvkShader.mReference.reset(gvk::newref, gvk::HandleId<VkDevice, VkShaderEXT>(device, pShaders[createInfo_i]));
            auto& controlBlock = gvkShader.mReference.get_obj();
            controlBlock.mVkShaderEXT = pShaders[createInfo_i];
            controlBlock.mDevice = gvkDevice;
            controlBlock.mStateTrackedObjectInfo.flags = GVK_STATE_TRACKED_OBJECT_STATUS_ACTIVE_BIT;
            controlBlock.mAllocationCallbacks = pAllocator ? *pAllocator : VkAllocationCallbacks { };
            controlBlock.mShaderCreateInfoEXT = createInfo;
            controlBlock.mDescriptorSetLayouts.reserve(createInfo.setLayoutCount);
            for (uint32_t setLayout_i = 0; setLayout_i < createInfo.setLayoutCount; ++setLayout_i) {
                controlBlock.mDescriptorSetLayouts.push_back(DescriptorSetLayout({ device, createInfo.pSetLayouts[setLayout_i] }));
                assert(controlBlock.mDescriptorSetLayouts.back());
            }
            gvkDevice.mReference.get_obj().mShaderEXTTracker.insert(gvkShader);
        }
    }
    return gvkResult;
}

} // namespace state_tracker
} // namespace gvk

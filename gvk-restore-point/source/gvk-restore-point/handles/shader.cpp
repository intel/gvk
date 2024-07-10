
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

#include "gvk-restore-point/layer.hpp"

namespace gvk {
namespace restore_point {

VkResult Layer::pre_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, VkResult gvkResult)
{
    (void)device;
    (void)createInfoCount;
    (void)pCreateInfos;
    (void)pAllocator;
    (void)pShaders;
    // NOOP :
    return gvkResult;
}

VkResult Layer::post_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, VkResult gvkResult)
{
    (void)pCreateInfos;
    (void)pAllocator;
    for (auto gvkRestorePoint : get_restore_points()) {
        assert(gvkRestorePoint);
        assert(pShaders);
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            auto stateTrackedShader = get_default<GvkStateTrackedObject>();
            stateTrackedShader.type = VK_OBJECT_TYPE_SHADER_EXT;
            stateTrackedShader.handle = (uint64_t)pShaders[i];
            stateTrackedShader.dispatchableHandle = (uint64_t)device;
            gvkRestorePoint->createdObjects.insert(stateTrackedShader);
        }
    }
    return gvkResult;
}

} // namespace restore_point
} // namespace gvk

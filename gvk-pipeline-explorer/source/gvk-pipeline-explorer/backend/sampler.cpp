
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

#include "gvk-pipeline-explorer/backend/pipeline-explorer.hpp"

namespace gvk {

VkResult PipelineExplorer::execute_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicPipelineExplorer::execute_vkCreateSampler(device, pCreateInfo, pAllocator, pSampler));
        pipeline_explorer::SamplerInfo samplerInfo(gvk::newref, { device, *pSampler });
        samplerInfo->deviceInfo = device;
        samplerInfo->vkHandle = *pSampler;
        samplerInfo->samplerCreateInfo = *pCreateInfo;
        auto inserted = samplerInfos.insert({ { device, *pSampler }, samplerInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        samplerInfo->uuid = pipeline_explorer::get_uuid(device, samplerInfo->samplerCreateInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
    samplerInfos.erase({ device, sampler });
    BasicPipelineExplorer::execute_vkDestroySampler(device, sampler, pAllocator);
}

VkResult PipelineExplorer::create_replacement_sampler(VkDevice vkDevice, VkSampler vkSampler, gvk::Sampler* pGvkSampler)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pipeline_explorer::SamplerInfo samplerInfo({ vkDevice, vkSampler });
        gvk_result(samplerInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(gvk::Sampler::create(vkDevice, &*samplerInfo->samplerCreateInfo, nullptr, pGvkSampler));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

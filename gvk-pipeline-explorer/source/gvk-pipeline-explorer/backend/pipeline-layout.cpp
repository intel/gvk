
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

VkResult PipelineExplorer::execute_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicPipelineExplorer::execute_vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout));
        pipeline_explorer::PipelineLayoutInfo pipelineLayoutInfo(gvk::newref, { device, *pPipelineLayout });
        pipelineLayoutInfo->deviceInfo = device;
        pipelineLayoutInfo->vkHandle = *pPipelineLayout;
        pipelineLayoutInfo->pipelineLayoutCreateInfo = *pCreateInfo;
        for (uint32_t setLayout_i = 0; setLayout_i < pCreateInfo->setLayoutCount; ++setLayout_i) {
            pipeline_explorer::DescriptorSetLayoutInfo descriptorSetLayoutInfo({ device, pCreateInfo->pSetLayouts[setLayout_i] });
            gvk_result(descriptorSetLayoutInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            pipelineLayoutInfo->descriptorSetLayoutInfos.push_back(descriptorSetLayoutInfo);
        }
        auto inserted = pipelineLayoutInfos.insert({ { device, *pPipelineLayout }, pipelineLayoutInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pipelineLayoutInfo->uuid = pipeline_explorer::get_uuid(device, pipelineLayoutInfo->pipelineLayoutCreateInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
    pipelineLayoutInfos.erase({ device, pipelineLayout });
    BasicPipelineExplorer::execute_vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VkResult PipelineExplorer::create_replacement_pipeline_layout(VkDevice vkDevice, VkPipelineLayout vkPipelineLayout, gvk::PipelineLayout* pGvkPipelineLayout)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pipeline_explorer::PipelineLayoutInfo piplineLayoutInfo({ vkDevice, vkPipelineLayout });
        gvk_result(piplineLayoutInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto pipelineLayoutCreateInfo = *piplineLayoutInfo->pipelineLayoutCreateInfo;
        std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts(pipelineLayoutCreateInfo.setLayoutCount);
        std::vector<gvk::DescriptorSetLayout> replacementDescriptorSetLayouts(pipelineLayoutCreateInfo.setLayoutCount);
        for (uint32_t setLayout_i = 0; setLayout_i < pipelineLayoutCreateInfo.setLayoutCount; ++setLayout_i) {
            gvk_result(create_replacement_descriptor_set_layout(vkDevice, pipelineLayoutCreateInfo.pSetLayouts[setLayout_i], &replacementDescriptorSetLayouts[setLayout_i]));
            vkDescriptorSetLayouts[setLayout_i] = replacementDescriptorSetLayouts[setLayout_i];
        }
        pipelineLayoutCreateInfo.pSetLayouts = !vkDescriptorSetLayouts.empty() ? vkDescriptorSetLayouts.data() : nullptr;
        gvk_result(gvk::PipelineLayout::create(vkDevice, &pipelineLayoutCreateInfo, nullptr, pGvkPipelineLayout));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

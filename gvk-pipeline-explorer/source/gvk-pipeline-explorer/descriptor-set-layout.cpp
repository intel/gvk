
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

#include "gvk-pipeline-explorer/pipeline-explorer.hpp"

namespace gvk {

VkResult PipelineExplorer::execute_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout));
        pipeline_explorer::DescriptorSetLayoutInfo descriptorSetLayoutInfo(gvk::newref, { device, *pSetLayout });
        descriptorSetLayoutInfo->deviceInfo = device;
        descriptorSetLayoutInfo->vkHandle = *pSetLayout;
        descriptorSetLayoutInfo->descriptorSetLayoutCreateInfo = *pCreateInfo;
        for (uint32_t binding_i = 0; binding_i < pCreateInfo->bindingCount; ++binding_i) {
            const auto& binding = pCreateInfo->pBindings[binding_i];
            switch (binding.descriptorType) {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                if (binding.pImmutableSamplers) {
                    for (uint32_t descriptor_i = 0; descriptor_i < binding.descriptorCount; ++descriptor_i) {
                        pipeline_explorer::SamplerInfo samplerInfo({ device, binding.pImmutableSamplers[descriptor_i] });
                        gvk_result(samplerInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                        descriptorSetLayoutInfo->immutableSamplerInfos.push_back(samplerInfo);
                    }
                }
            } break;
            default: {
            } break;
            }
        }
        auto inserted = descriptorSetLayoutInfos.insert({ { device, *pSetLayout }, descriptorSetLayoutInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        descriptorSetLayoutInfo->uuid = pipeline_explorer::get_uuid(device, descriptorSetLayoutInfo->descriptorSetLayoutCreateInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
    descriptorSetLayoutInfos.erase({ device, descriptorSetLayout });
    BasicApiCallHandler::execute_vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VkResult PipelineExplorer::create_replacement_descriptor_set_layout(VkDevice vkDevice, VkDescriptorSetLayout vkDescriptorSetLayout, gvk::DescriptorSetLayout* pGvkDescriptorSetLayout)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pipeline_explorer::DescriptorSetLayoutInfo descriptorSetLayoutInfo({ vkDevice, vkDescriptorSetLayout });
        gvk_result(descriptorSetLayoutInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk::Auto<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfo = *descriptorSetLayoutInfo->descriptorSetLayoutCreateInfo;
        std::vector<gvk::Sampler> replacementSamplers;
        for (uint32_t binding_i = 0; binding_i < descriptorSetLayoutCreateInfo->bindingCount; ++binding_i) {
            auto& binding = descriptorSetLayoutCreateInfo->pBindings[binding_i];
            switch (binding.descriptorType) {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                if (binding.pImmutableSamplers) {
                    for (uint32_t descriptor_i = 0; descriptor_i < binding.descriptorCount; ++descriptor_i) {
                        gvk::Sampler replacementSampler = VK_NULL_HANDLE;
                        gvk_result(create_replacement_sampler(vkDevice, binding.pImmutableSamplers[descriptor_i], &replacementSampler));
                        const_cast<VkSampler*>(binding.pImmutableSamplers)[descriptor_i] = replacementSampler;
                        replacementSamplers.push_back(replacementSampler);
                    }
                }
            } break;
            default: {
            } break;
            }
        }
        gvk_result(gvkResult);
        gvk_result(gvk::DescriptorSetLayout::create(vkDevice, &*descriptorSetLayoutCreateInfo, nullptr, pGvkDescriptorSetLayout));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk


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

static VkResult set_shader_module_driver_uuid(pipeline_explorer::ShaderModuleInfo shaderModuleInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(shaderModuleInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        if (shaderModuleInfo->deviceInfo->VK_EXT_shader_module_identifier_enabled) {
            auto shaderModuleIdentifier = gvk::get_default<VkShaderModuleIdentifierEXT>();
            gvk::Device(shaderModuleInfo->deviceInfo->vkHandle).GetShaderModuleIdentifierEXT(shaderModuleInfo->vkHandle, &shaderModuleIdentifier);
            shaderModuleInfo->shaderModuleIdentifier = shaderModuleIdentifier;
            boost::multiprecision::import_bits(shaderModuleInfo->driverUUID, shaderModuleIdentifier.identifier, shaderModuleIdentifier.identifier + shaderModuleIdentifier.identifierSize);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicPipelineExplorer::execute_vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule));
        pipeline_explorer::ShaderModuleInfo shaderModuleInfo(gvk::newref, { device, *pShaderModule });
        shaderModuleInfo->deviceInfo = device;
        shaderModuleInfo->vkHandle = *pShaderModule;
        shaderModuleInfo->shaderModuleCreateInfo = *pCreateInfo;
        auto inserted = shaderModuleInfos.insert({ { device, *pShaderModule }, shaderModuleInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        shaderModuleInfo->uuid = pipeline_explorer::get_uuid(device, shaderModuleInfo->shaderModuleCreateInfo);
        gvk_result(set_shader_module_driver_uuid(shaderModuleInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
    shaderModuleInfos.erase({ device, shaderModule });
    BasicPipelineExplorer::execute_vkDestroyShaderModule(device, shaderModule, pAllocator);
}

VkResult PipelineExplorer::execute_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicPipelineExplorer::execute_vkCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders));
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator)
{
    BasicPipelineExplorer::execute_vkDestroyShaderEXT(device, shader, pAllocator);
}

} // namespace gvk

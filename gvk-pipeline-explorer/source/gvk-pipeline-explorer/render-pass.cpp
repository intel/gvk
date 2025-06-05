
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

VkResult PipelineExplorer::execute_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass));
        pipeline_explorer::RenderPassInfo renderPassInfo(gvk::newref, { device, *pRenderPass });
        renderPassInfo->deviceInfo = device;
        renderPassInfo->vkHandle = *pRenderPass;
        renderPassInfo->renderPassCreateInfo = *pCreateInfo;
        auto inserted = renderPassInfos.insert({ { device, *pRenderPass }, renderPassInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        renderPassInfo->uuid = pipeline_explorer::get_uuid(device, renderPassInfo->renderPassCreateInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass));
        pipeline_explorer::RenderPassInfo renderPassInfo(gvk::newref, { device, *pRenderPass });
        renderPassInfo->deviceInfo = device;
        renderPassInfo->vkHandle = *pRenderPass;
        renderPassInfo->renderPassCreateInfo2 = *pCreateInfo;
        auto inserted = renderPassInfos.insert({ { device, *pRenderPass }, renderPassInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        renderPassInfo->uuid = pipeline_explorer::get_uuid(device, renderPassInfo->renderPassCreateInfo2);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass));
        pipeline_explorer::RenderPassInfo renderPassInfo(gvk::newref, { device, *pRenderPass });
        renderPassInfo->deviceInfo = device;
        renderPassInfo->vkHandle = *pRenderPass;
        renderPassInfo->renderPassCreateInfo2 = *pCreateInfo;
        auto inserted = renderPassInfos.insert({ { device, *pRenderPass }, renderPassInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        renderPassInfo->uuid = pipeline_explorer::get_uuid(device, renderPassInfo->renderPassCreateInfo2);
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
    renderPassInfos.erase({ device, renderPass });
    BasicApiCallHandler::execute_vkDestroyRenderPass(device, renderPass, pAllocator);
}

VkResult PipelineExplorer::create_replacement_render_pass(VkDevice vkDevice, VkRenderPass vkRenderPass, gvk::RenderPass* pGvkRenderPass)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pipeline_explorer::RenderPassInfo renderPassInfo({ vkDevice, vkRenderPass });
        gvk_result(renderPassInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        if (renderPassInfo->renderPassCreateInfo->sType == gvk::get_stype<VkRenderPassCreateInfo>()) {
            gvk_result(gvk::RenderPass::create(vkDevice, &*renderPassInfo->renderPassCreateInfo, nullptr, pGvkRenderPass));
        } else if (renderPassInfo->renderPassCreateInfo2->sType == gvk::get_stype<VkRenderPassCreateInfo2>()) {
            gvk_result(gvk::RenderPass::create(vkDevice, &*renderPassInfo->renderPassCreateInfo2, nullptr, pGvkRenderPass));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

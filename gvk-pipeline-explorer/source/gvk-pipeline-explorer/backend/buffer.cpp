
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

VkResult PipelineExplorer::execute_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        auto usage = pCreateInfo->usage;
        if (usage & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR) {
            const_cast<VkBufferCreateInfo*>(pCreateInfo)->usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        gvk_result(BasicPipelineExplorer::execute_vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer));
        const_cast<VkBufferCreateInfo*>(pCreateInfo)->usage = usage;
        pipeline_explorer::BufferInfo bufferInfo(gvk::newref, { device, *pBuffer });
        bufferInfo->deviceInfo = device;
        bufferInfo->vkHandle = *pBuffer;
        bufferInfo->bufferCreateInfo = *pCreateInfo;
        auto inserted = bufferInfos.insert({ { device, *pBuffer }, bufferInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        bufferInfo->uuid = pipeline_explorer::get_uuid(device, bufferInfo->bufferCreateInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    pipeline_explorer::DeviceInfo deviceInfo(device);
    assert(deviceInfo);
    deviceInfo->deviceAddressTracker.erase_buffer_bindings(device, buffer);
    bufferInfos.erase({ device, buffer });
    BasicPipelineExplorer::execute_vkDestroyBuffer(device, buffer, pAllocator);
}

} // namespace gvk

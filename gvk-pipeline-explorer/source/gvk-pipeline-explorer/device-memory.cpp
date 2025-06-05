
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

VkResult PipelineExplorer::execute_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory));
        pipeline_explorer::DeviceMemoryInfo deviceMemoryInfo(gvk::newref, { device, *pMemory });
        deviceMemoryInfo->deviceInfo = device;
        deviceMemoryInfo->vkHandle = *pMemory;
        deviceMemoryInfo->memoryAllocateInfo = *pAllocateInfo;
        auto inserted = deviceMemoryInfos.insert({ { device, *pMemory }, deviceMemoryInfo }).second;
        gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        deviceMemoryInfo->uuid = pipeline_explorer::get_uuid(device, deviceMemoryInfo->memoryAllocateInfo);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(BasicApiCallHandler::execute_vkBindBufferMemory(device, buffer, memory, memoryOffset));
        pipeline_explorer::DeviceInfo deviceInfo(device);
        gvk_result(deviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pipeline_explorer::BufferInfo bufferInfo({ device, buffer });
        gvk_result(bufferInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        if (bufferInfo->bufferCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            auto bindBufferMemoryInfo = gvk::get_default<VkBindBufferMemoryInfo>();
            bindBufferMemoryInfo.buffer = buffer;
            bindBufferMemoryInfo.memory = memory;
            bindBufferMemoryInfo.memoryOffset = memoryOffset;
            gvk_result(deviceInfo->deviceAddressTracker.add_buffer_binding(device, &bindBufferMemoryInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(BasicApiCallHandler::execute_vkBindBufferMemory2(device, bindInfoCount, pBindInfos));
        pipeline_explorer::DeviceInfo deviceInfo(device);
        gvk_result(deviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        for (uint32_t bindInfo_i = 0; bindInfo_i < bindInfoCount; ++bindInfo_i) {
            pipeline_explorer::BufferInfo bufferInfo({ device, pBindInfos[bindInfo_i].buffer });
            gvk_result(bufferInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            if (bufferInfo->bufferCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                gvk_result(deviceInfo->deviceAddressTracker.add_buffer_binding(device, &pBindInfos[bindInfo_i]));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(BasicApiCallHandler::execute_vkBindBufferMemory2KHR(device, bindInfoCount, pBindInfos));
        pipeline_explorer::DeviceInfo deviceInfo(device);
        gvk_result(deviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        for (uint32_t bindInfo_i = 0; bindInfo_i < bindInfoCount; ++bindInfo_i) {
            pipeline_explorer::BufferInfo bufferInfo({ device, pBindInfos[bindInfo_i].buffer });
            gvk_result(bufferInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            if (bufferInfo->bufferCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                gvk_result(deviceInfo->deviceAddressTracker.add_buffer_binding(device, &pBindInfos[bindInfo_i]));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    pipeline_explorer::DeviceInfo deviceInfo(device);
    assert(deviceInfo);
    deviceInfo->deviceAddressTracker.erase_memory_bindings(device, memory);
    deviceMemoryInfos.erase({ device, memory });
    BasicApiCallHandler::execute_vkFreeMemory(device, memory, pAllocator);
}

} // namespace gvk

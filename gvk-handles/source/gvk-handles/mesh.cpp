
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

#include "gvk-handles/mesh.hpp"

namespace gvk {

void Mesh::reset()
{
    mCpuBuffer.reset();
    mGpuBuffer.reset();
    mIndexDataOffset = 0;
    mIndexType = VK_INDEX_TYPE_NONE_KHR;
    mIndexCount = 0;
}

void Mesh::record_cmds(const gvk::CommandBuffer& commandBuffer) const
{
    const auto& dispatchTable = commandBuffer.get<Device>().get<DispatchTable>();
    assert(dispatchTable.gvkCmdBindVertexBuffers);
    assert(dispatchTable.gvkCmdBindIndexBuffer);
    assert(dispatchTable.gvkCmdDrawIndexed);
    VkDeviceSize vertexDataOffset = 0;
    dispatchTable.gvkCmdBindVertexBuffers(commandBuffer, 0, 1, &mGpuBuffer.get<VkBuffer>(), &vertexDataOffset);
    dispatchTable.gvkCmdBindIndexBuffer(commandBuffer, mGpuBuffer, mIndexDataOffset, mIndexType);
    dispatchTable.gvkCmdDrawIndexed(commandBuffer, (uint32_t)mIndexCount, 1, 0, 0, 0);
}

VkResult resize(VkDeviceSize size, gvk::Buffer* pBuffer)
{
    assert(pBuffer);
    assert(*pBuffer);
    gvk_result_scope_begin(VK_SUCCESS) {
        auto bufferCreateInfo = pBuffer->get<VkBufferCreateInfo>();
        if (bufferCreateInfo.size != size) {
            bufferCreateInfo.size = size;
            const auto& device = pBuffer->get<Device>();
            const auto& allocationCreateInfo = pBuffer->get<VmaAllocationCreateInfo>();
            gvk_result(Buffer::create(device, &bufferCreateInfo, &allocationCreateInfo, pBuffer));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult expand(VkDeviceSize size, gvk::Buffer* pBuffer)
{
    assert(pBuffer);
    assert(*pBuffer);
    return pBuffer->get<VkBufferCreateInfo>().size < size ? resize(size, pBuffer) : VK_SUCCESS;
}

VkResult create_staging_buffer(const gvk::Device& device, VkDeviceSize size, gvk::Buffer* pBuffer)
{
    assert(pBuffer);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (*pBuffer) {
            gvk_result(expand(size, pBuffer));
        } else {
            auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
            bufferCreateInfo.size = size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            auto allocationCreateInfo = get_default<VmaAllocationCreateInfo>();
            allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            gvk_result(Buffer::create(device, &bufferCreateInfo, &allocationCreateInfo, pBuffer));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

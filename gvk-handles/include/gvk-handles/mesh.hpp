
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

#pragma once

#include "gvk-dispatch-table.hpp"
#include "gvk-defines.hpp"
#include "gvk-handles/handles.hpp"
#include "gvk-handles/utilities.hpp"

namespace gvk {

class Mesh final
{
public:
    void reset();
    void record_cmds(const gvk::CommandBuffer& commandBuffer) const;

    template <typename VertexType, typename IndexType>
    VkResult write(
        const Device& device,
        VkQueue vkQueue,
        VkCommandBuffer vkCommandBuffer,
        VkFence vkFence,
        uint32_t vertexCount,
        const VertexType* pVertices,
        uint32_t indexCount,
        const IndexType* pIndices
    )
    {
        gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
            if (device && vkCommandBuffer && vkQueue && vertexCount && pVertices && indexCount && pIndices) {
                auto vertexDataSize = vertexCount * sizeof(VertexType);
                auto indexDataSize = indexCount * sizeof(IndexType);
                mIndexDataOffset = vertexDataSize;
                mIndexType = get_index_type<IndexType>();
                mIndexCount = indexCount;

                auto bufferCreateInfo = get_default<VkBufferCreateInfo>();
                bufferCreateInfo.size = vertexDataSize + indexDataSize;
                bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                VmaAllocationCreateInfo allocationCreateInfo{ };
                allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
                gvk_result(Buffer::create(device, &bufferCreateInfo, &allocationCreateInfo, &mGpuBuffer));

                bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                gvk_result(Buffer::create(device, &bufferCreateInfo, &allocationCreateInfo, &mCpuBuffer));

                uint8_t* pData = nullptr;
                gvk_result(vmaMapMemory(device.get<VmaAllocator>(), mCpuBuffer.get<VmaAllocation>(), (void**)&pData));
                memcpy(pData, pVertices, vertexDataSize);
                memcpy(pData + mIndexDataOffset, pIndices, indexDataSize);
                vmaUnmapMemory(device.get<VmaAllocator>(), mCpuBuffer.get<VmaAllocation>());
                gvk_result(execute_immediately(
                    device, vkQueue, vkCommandBuffer, vkFence,
                    [&](auto)
                    {
                        auto bufferCopy = get_default<VkBufferCopy>();
                        bufferCopy.size = bufferCreateInfo.size;
                        auto dispatchTable = device.get<DispatchTable>();
                        assert(dispatchTable.gvkCmdCopyBuffer);
                        dispatchTable.gvkCmdCopyBuffer(vkCommandBuffer, mCpuBuffer, mGpuBuffer, 1, &bufferCopy);
                    }
                ));
            }
        } gvk_result_scope_end
        return gvkResult;
    }

private:
    gvk::Buffer mCpuBuffer;
    gvk::Buffer mGpuBuffer;
    VkDeviceSize mIndexDataOffset { };
    VkIndexType mIndexType { VK_INDEX_TYPE_NONE_KHR };
    uint32_t mIndexCount { };
};

} // namespace gvk

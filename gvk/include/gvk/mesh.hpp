
/******************************************************************************
© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include "gvk/generated/dispatch-table.hpp"
#include "gvk/defines.hpp"
#include "gvk/handles.hpp"
#include "gvk/spir-v.hpp"

#include <vector>

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
                    vkQueue, vkCommandBuffer, vkFence,
                    [&](auto)
                    {
                        auto bufferCopy = get_default<VkBufferCopy>();
                        bufferCopy.size = bufferCreateInfo.size;
                        auto dispatchTable = DispatchTable::get_global_dispatch_table();
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
    VkDeviceSize mIndexDataOffset{ };
    VkIndexType mIndexType{ VK_INDEX_TYPE_NONE_KHR };
    uint32_t mIndexCount{ };
};

} // namespace gvk

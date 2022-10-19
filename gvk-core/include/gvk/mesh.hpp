
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

#include "gvk/generated/dispatch-table.hpp"
#include "gvk/defines.hpp"
#include "gvk/handles.hpp"

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

/**
Gets the vertex input attribute VkFormat for a given type
@param VertexInputAttributeType The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
@example
    namespace example {

    struct Vector3
    {
        float x { };
        float y { };
        float z { };
    };

    } // namespace example

    template <>
    inline VkFormat gvk::get_vertex_input_attribute_format<example::Vector3>()
    {
        return VK_FORMAT_R32G32B32_SFLOAT;
    }
*/
template <typename VertexInputAttributeType>
inline VkFormat get_vertex_input_attribute_format()
{
#ifdef GVK_COMPILER_MSVC
    static_assert(false, "template <> VkFormat gvk::get_vertex_input_attribute_format<VertexInputAttributeType>() must be specialized for the given type");
#endif
    return VK_FORMAT_UNDEFINED;
}

/**
Gets an std::array<VkVertexInputAttributeDescription, N> for a given set of vertex input attribute types
@typename ...VertexInputAttributeTypes The vertex input attribute types of the vertex type being described
@param [in] binding The binding of the vertex type being described
@return The std::array<VkVertexInputAttributeDescription, N> for the given set of vertex input attribute types
*/
template <typename ...VertexInputAttributeTypes>
inline std::array<VkVertexInputAttributeDescription, sizeof...(VertexInputAttributeTypes)> get_vertex_input_attribute_descriptions(uint32_t binding)
{
    size_t offset = 0;
    std::array<size_t, sizeof...(VertexInputAttributeTypes)> sizes{ sizeof(VertexInputAttributeTypes)... };
    std::array<VkVertexInputAttributeDescription, sizeof...(VertexInputAttributeTypes)> vertexInputAttributeDescriptions{
        VkVertexInputAttributeDescription{
            /*.location = */ 0,
            /*.binding  = */ binding,
            /*.format   = */ get_vertex_input_attribute_format<VertexInputAttributeTypes>(),
            /*.offset   = */ 0,
        }...
    };
    for (size_t i = 0; i < vertexInputAttributeDescriptions.size(); ++i) {
        vertexInputAttributeDescriptions[i].location = (uint32_t)i;
        vertexInputAttributeDescriptions[i].offset = (uint32_t)offset;
        offset += sizes[i];
    }
    return vertexInputAttributeDescriptions;
}

/**
Gets an std::array<VkVertexInputAttributeDescription, N> for a given VertexType
@typename VertexType The type of vertex being described
@param [in] binding The binding of the vertex type being described
@return The std::array<VkVertexInputAttributeDescription, N> for the given VertexType
@example
    namespace example {

    struct VertexPositionTexcoordColor
    {
        Vector3 position;
        Vector2 texcoord;
        Vector4 color;
    };

    } // namespace example

    template <>
    inline auto gvk::get_vertex_description<example::VertexPositionTexcoordColor>(uint32_t binding)
    {
        return gvk::get_vertex_input_attribute_descriptions<
            example::Vector3,
            example::Vector2,
            example::Vector4
        >(binding);
    }
*/
template <typename VertexType>
inline auto get_vertex_description(uint32_t binding)
{
    (void)binding;
#ifdef GVK_COMPILER_MSVC
    static_assert(false, "template <> auto gvk::get_vertex_description<VertexType>() must be specialized for the given type");
#endif
}

/**
Gets the VkIndexType of a given IndexType
@typename IndexType The index type to get the VkIndexType for
@return The VkIndexType of the given IndexType
*/
template <typename IndexType>
inline VkIndexType get_index_type()
{
    switch (sizeof(IndexType))
    {
        case sizeof(uint8_t):  return VK_INDEX_TYPE_UINT8_EXT;
        case sizeof(uint16_t): return VK_INDEX_TYPE_UINT16;
        case sizeof(uint32_t): return VK_INDEX_TYPE_UINT32;
        default:               return VK_INDEX_TYPE_NONE_KHR;
    }
}

/**
Gets the size, in bytes, of the given VkIndexType
@param [in] indexType The VkIndexType to get the size of
@return The size, in bytes, of the given VkIndexType
*/
inline VkDeviceSize get_index_size(VkIndexType indexType)
{
    switch (indexType)
    {
    case VK_INDEX_TYPE_UINT8_EXT: return sizeof(uint8_t);
    case VK_INDEX_TYPE_UINT16:    return sizeof(uint16_t);
    case VK_INDEX_TYPE_UINT32:    return sizeof(uint32_t);
    case VK_INDEX_TYPE_NONE_KHR:
    default: return 0;
    }
}

} // namespace gvk


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
#include "gvk-structures.hpp"

#include <array>

namespace gvk {

/*
Gets the indices of phyiscal device memory types compatible with specified VkMemoryPropertyFlags
@param [in] physicalDevice The PhysicalDevice to request memory properties from
@param [in] memoryTypeBits A bitmask with one bit set for each memory type to check for compatibility
    @note This is generally expected to be the memoryTypeBits member of a resource's VkMemoryRequirements
@param [in] memoryPropertyFlags a bitmask of required memory properties
@param [in,out] pMemoryTypeCount The number of compatible memory type indices
    @note If pMemoryTypeIndices is null, pMemoryTypeCount will be populated with the number of compatible memory type indices
    @note If pMemoryTypeIndices is not null, pMemoryTypeCount indicates the max number of compatible memory type indices to write to pMemoryTypeIndices
@param [out] pMemoryTypeIndices The indices of compatible memory types
*/
void get_compatible_memory_type_indices(const PhysicalDevice& physicalDevice, uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryPropertyFlags, uint32_t* pMemoryTypeCount, uint32_t* pMemoryTypeIndices);

/*
Gets the indices of phyiscal device memory types compatible with specified VkMemoryPropertyFlags
@param [in] pPhysicalDeviceMemoryProperties A pointer to VkPhysicalDeviceMemoryProperties to use when selecting memory types
@param [in] memoryTypeBits A bitmask with one bit set for each memory type to check for compatibility
    @note This is generally expected to be the memoryTypeBits member of a resource's VkMemoryRequirements
@param [in] memoryPropertyFlags a bitmask of required memory properties
@param [in,out] pMemoryTypeCount The number of compatible memory type indices
    @note If pMemoryTypeIndices is null, pMemoryTypeCount will be populated with the number of compatible memory type indices
    @note If pMemoryTypeIndices is not null, pMemoryTypeCount indicates the max number of compatible memory type indices to write to pMemoryTypeIndices
@param [out] pMemoryTypeIndices The indices of compatible memory types
*/
void get_compatible_memory_type_indices(const VkPhysicalDeviceMemoryProperties* pPhysicalDeviceMemoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryPropertyFlags, uint32_t* pMemoryTypeCount, uint32_t* pMemoryTypeIndices);

/**
Gets the number of mip levels for an image with a specified VkExtent3D
@param [in] imageExtent The VkExtent3D of the image to get the specified mip level for
@return The number of mip levels for an image with the specified VkExtent3D
*/
uint32_t get_mip_level_count(const VkExtent3D& imageExtent);

/**
Gets the VkExtent3D of a specified mip level for a VkImage with a specified VkExtent3D
@param [in] imageExtent The VkExtent3D of the VkImage to get the specified mip level for
@param [in] mipLevel The mip level to get the VkExtent3D for
@return The VkExtent3D of the specified VkImage mip level
*/
VkExtent3D get_mip_level_extent(const VkExtent3D& imageExtent, uint32_t mipLevel);

/**
Gets the max VkSampleCountFlagBits for a Framebuffer with specified attachment types
@param [in] vkPhysicalDevice
@param [in] color A value indicating whether or not the Framebuffer has a color attachment
@param [in] depth A value indicating whether or not the Framebuffer has a depth attachment
@param [in] stencil A value indicating whether or not the Framebuffer has a stencil attachment
@return The max VkSampleCountFlagBits for a Framebuffer with the specified attachment types
*/
VkSampleCountFlagBits get_max_framebuffer_sample_count(const PhysicalDevice& physicalDevice, VkBool32 color, VkBool32 depth, VkBool32 stencil);

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

    namespace gvk {

    template <>
    inline VkFormat get_vertex_input_attribute_format<example::Vector3>()
    {
        return VK_FORMAT_R32G32B32_SFLOAT;
    }

    } // namespace gvk
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
    (void)binding;
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

    namespace gvk {

    template <>
    inline auto get_vertex_description<example::VertexPositionTexcoordColor>(uint32_t binding)
    {
        return gvk::get_vertex_input_attribute_descriptions<
            example::Vector3,
            example::Vector2,
            example::Vector4
        >(binding);
    }

    } // namespace gvk
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

/**
Record and execute a VkCommandBuffer immediately
@typename <RecordCommandBufferFunctionType> The type of function to call to record the VkCommandBuffer
    @note The function type must accept a single VkCommandBuffer argument
@param [in] vkQueue The VkQueue to submit the recorded VkCommandBuffer to
@param [in] vkCommandBuffer The vkCommandBuffer to record and submit
@param [in] vkFence The VkFence to signal when the submited VkCommandBuffer completes execution
    @note If this argument is VK_NULL_HANDLE this call will block on vkQueueWaitIdle() after VkCommandBuffer submission
@return The VkResult
*/
template <typename RecordCommandBufferFunctionType>
inline VkResult execute_immediately(
    const Device& device,
    VkQueue vkQueue,
    VkCommandBuffer vkCommandBuffer,
    VkFence vkFence,
    RecordCommandBufferFunctionType recordCommandBuffer
)
{
    assert(vkQueue);
    assert(vkCommandBuffer);
    gvk_result_scope_begin(VK_INCOMPLETE) {
        auto commandBufferBeginInfo = get_default<VkCommandBufferBeginInfo>();
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        const auto& dispatchTable = device.get<DispatchTable>();
        assert(dispatchTable.gvkBeginCommandBuffer);
        gvk_result(dispatchTable.gvkBeginCommandBuffer(vkCommandBuffer, &commandBufferBeginInfo));
        recordCommandBuffer(vkCommandBuffer);
        assert(dispatchTable.gvkEndCommandBuffer);
        gvk_result(dispatchTable.gvkEndCommandBuffer(vkCommandBuffer));

        auto submitInfo = get_default<VkSubmitInfo>();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        assert(dispatchTable.gvkQueueSubmit);
        gvk_result(dispatchTable.gvkQueueSubmit(vkQueue, 1, &submitInfo, vkFence));

        if (!vkFence) {
            assert(dispatchTable.gvkQueueWaitIdle);
            gvk_result(dispatchTable.gvkQueueWaitIdle(vkQueue));
        }
    } gvk_result_scope_end
    return gvkResult;
}

} // namespace gvk

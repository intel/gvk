
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

#include "gvk/math/defines.hpp"
#include "gvk/defines.hpp"
#include "gvk/handles.hpp"

#include <array>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace gvk {
namespace spirv {

/**
Enumeration of high level shading languages
*/
enum class ShadingLanguage
{
    Glsl,
    Hlsl,
};

/**
Encapsulates data necessary for shader compilation
*/
class ShaderInfo final
{
public:
    ShadingLanguage language{ ShadingLanguage::Glsl };
    VkShaderStageFlagBits stage{ VK_SHADER_STAGE_ALL };
    uint32_t lineOffset{ };
    std::string source;
    std::vector<uint32_t> spirv;
    std::vector<std::string> errors;
};

/**
Provides high level control over shader compilation
*/
class Context final
{
public:
    /**
    Creation parameters for spirv::Context
    */
    struct CreateInfo final
    {
    };

    /**
    Creates an instance of spirv::Context
    @param [in] pCreateInfo A pointer to the spirv::Context creation parameters
    @param [out] pContext A pointer to the spirv::Context to create
    @return the VkResult
    */
    static VkResult create(const CreateInfo* pCreateInfo, Context* pContext);

    /**
    Destroys this instance of spirv::Context
    */
    ~Context();

    /**
    Destroys this instance of spirv::Context
    */
    void reset();

    /**
    Gets a value indicating whether or not this spirv::Context is valid
    @return A value indicating whether or not this spirv::Context is valid
    */
    operator bool() const;

    /**
    Compiles SPIR-V from a given spirv::ShaderInfo
    @param [in] pShaderInfo
    */
    VkResult compile(ShaderInfo* pShaderInfo);

private:
    bool mInitialized{ false };
    static std::mutex sMutex;
    static uint32_t sInstanceCount;
};

class BindingInfo final
{
public:
    void add_shader(const ShaderInfo& shaderInfo);
    void add_binding(uint32_t setIndex, const VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding);

    std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;
    std::vector<VkPushConstantRange> pushConstantRanges;
};

/**
Creates DescriptorSetLayout objects from a given spirv::BindingInfo
@param [in] device The Device used to create DescriptorSetLayout objects from
@param [in] bindingInfo The BindingInfo to create DescriptorSetLayout objects from
@param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
@param [in,out] pDescriptorSetLayoutCount The number of DescriptorSetLayout objects to create
    @note if pDescriptorSetLayouts is nullptr, this parameter will be populated with the number of DescriptorSetLayout objects
@param [out] pDescriptorSetLayouts A pointer to an array of DescriptorSetLayout objects to create
@return the VkResult
*/
VkResult create_descriptor_set_layouts(const Device& device, const BindingInfo& bindingInfo, const VkAllocationCallbacks* pAllocator, uint32_t* pDescriptorSetLayoutCount, DescriptorSetLayout* pDescriptorSetLayouts);

/**
Creates a PipelineLayout from a given spirv::BindingInfo
@param [in] device The Device used to create DescriptorSetLayout objects from
@param [in] bindingInfo The BindingInfo to create the PipelineLayout from
@param [in] (optional) pAllocator A pointer to the VkAllocationCallbacks to use
@param [out] pPipelineLayout A pointer to the PipelineLayout to create
@return the VkResult
*/
VkResult create_pipeline_layout(const Device& device, const BindingInfo& bindingInfo, const VkAllocationCallbacks* pAllocator, PipelineLayout* pPipelineLayout);

} // namespace spirv

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
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

    template <>
    inline VkFormat gvk::get_vertex_input_attribute_format<std::array<float, 3>>()
    {
        return VK_FORMAT_R32G32B32_SFLOAT;
    }
*/
template <typename T>
inline VkFormat get_vertex_input_attribute_format()
{
#ifdef GVK_COMPILER_MSVC
    static_assert(false, "template <> VkFormat gvk::get_vertex_input_attribute_format<UserType>() must be specialized for the given type");
#endif
    return VK_FORMAT_UNDEFINED;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<uint8_t>()
{
    return VK_FORMAT_R8_UNORM;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<uint32_t>()
{
    return VK_FORMAT_R32_UINT;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<float>()
{
    return VK_FORMAT_R32_SFLOAT;
}

#ifdef GVK_GLM_ENABLED
/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::u8vec2>()
{
    return VK_FORMAT_R8G8_UNORM;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::u8vec3>()
{
    return VK_FORMAT_R8G8B8_UNORM;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::u8vec4>()
{
    return VK_FORMAT_R8G8B8A8_UNORM;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::u32vec2>()
{
    return VK_FORMAT_R32G32_UINT;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::u32vec3>()
{
    return VK_FORMAT_R32G32B32_UINT;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::u32vec4>()
{
    return VK_FORMAT_R32G32B32A32_UINT;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::vec2>()
{
    return VK_FORMAT_R32G32_SFLOAT;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::vec3>()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

/**
Gets the vertex input attribute VkFormat for a given type
@param T The type to get the vertex input attribute VkFormat for
@return The vertex input attribute VkFormat for the given type
*/
template <>
inline VkFormat get_vertex_input_attribute_format<glm::vec4>()
{
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}
#endif // GVK_GLM_ENABLED

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
        glm::vec3 position;
        glm::vec2 texcoord;
        glm::vec4 color;
    };

    } // namespace example

    template <>
    inline auto gvk::get_vertex_description<example::VertexPositionTexcoordColor>(uint32_t binding)
    {
        return gvk::get_vertex_input_attribute_descriptions<
            glm::vec3,
            glm::vec2,
            glm::vec4
        >(binding);
    }
*/
template <typename VertexType>
inline auto get_vertex_description(uint32_t binding)
{
    (void)binding;
#ifdef GVK_COMPILER_MSVC
    static_assert(false, "template <> auto gvk::get_vertex_description<UserType>() must be specialized for the given type");
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

#if 0

GLSL for a vertex shader that draws a full screen triangle
@code
    #version 450
    layout(location = 0) out vec2 fsTexcoord;
    out gl_PerVertex { vec4 gl_Position; };
    void main()
    {
        fsTexcoord = vec2((gl_vertexIndex << 1) & 2, gl_VertexIndex & 2);
        gl_Position = vec4(fsTexcoord * 2 - 1, 0, 1);
    }
@endcode
@note
    Normalized Device Coordinates (NDC)
    -1,-1        1,-1
      +-----------+
      |           |
      |           |
      |           |
      |           |
      +-----------+
    -1,1         1,1

    Texture Coordinates (TC)
     0,0         1,0
      +-----------+
      |           |
      |           |
      |           |
      |           |
      +-----------+
     0,1         1,1

    -1,-1              1,-1
      +-----------------+
      |(NDC)            |
      |                 |
      |       0,0      1,0
      |        +--------+
      |        |(TC)    |
      |        |        |
      |        |        |
      +--------+--------+
    -1,1      0,1      1,1

             u = (i << 1) &   2   v = i &   2
    =========================================
    i = 0 :    (000 << 1) & 010
                      000 & 010     000 & 010
                            000           000
                          -----         -----
                          u = 0         v = 0
    =========================================
    i = 1 :    (001 << 1) & 010
                      010 & 010     001 & 010
                            010           000
                          -----         -----
                          u = 2         v = 0
    =========================================
    i = 2 :    (010 << 1) & 010
                      100 & 010     010 & 010
                            000           010
                          -----         -----
                          u = 0         v = 2

    0,0 (TC)                                        2,0 (TC)
    v0-----------------------+.......................v1
     |                       |                    .
     |                       |                 .
     |                       |              .
     |                       |           .
     |                       |        .
     |                       |     .
     |                       |  .
     +-----------------------+
     .                    . 1,1 (TC)
     .                 .
     .              .
     .           .
     .        .
     .     .
     .  .
    v2
    0,2 (TC)

#endif


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

#include "gvk-defines.hpp"
#include "gvk-handles/handles.hpp"

#include <array>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <utility>
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
    std::map<uint32_t, VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfos;
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

namespace detail {
namespace api_types {

enum ShaderLanguage
{
    GPA_SHADER_LANGUAGE_UNKNOWN = 0,
    GPA_SHADER_LANGUAGE_GLSL,
    GPA_SHADER_LANGUAGE_HLSL,
    GPA_SHADER_LANGUAGE_CM,
    GPA_SHADER_LANGUAGE_DXBC,
    GPA_SHADER_LANGUAGE_DXIL,
    GPA_SHADER_LANGUAGE_SPIRV,
    GPA_SHADER_LANGUAGE_ISA,
    GPA_SHADER_LANGUAGE_COUNT,
    GPA_SHADER_LANGUAGE_BEGIN = GPA_SHADER_LANGUAGE_UNKNOWN,
    GPA_SHADER_LANGUAGE_END = GPA_SHADER_LANGUAGE_COUNT
};

enum ShaderStageFlagBits : uint32_t
{
    GPA_SHADER_STAGE_UNKNOWN                 = 0,
    GPA_SHADER_STAGE_VERTEX                  = 1,
    GPA_SHADER_STAGE_HULL                    = 1 << 1,
    GPA_SHADER_STAGE_TESSELLATION_CONTROL    = GPA_SHADER_STAGE_HULL,
    GPA_SHADER_STAGE_DOMAIN                  = 1 << 2,
    GPA_SHADER_STAGE_TESSELLATION_EVALUATION = GPA_SHADER_STAGE_DOMAIN,
    GPA_SHADER_STAGE_GEOMETRY                = 1 << 3,
    GPA_SHADER_STAGE_PIXEL                   = 1 << 4,
    GPA_SHADER_STAGE_FRAGMENT                = GPA_SHADER_STAGE_PIXEL,
    GPA_SHADER_STAGE_COMPUTE                 = 1 << 5,
    GPA_SHADER_STAGE_RAYGEN                  = 1 << 6,
    GPA_SHADER_STAGE_ANY_HIT                 = 1 << 7,
    GPA_SHADER_STAGE_CLOSEST_HIT             = 1 << 8,
    GPA_SHADER_STAGE_MISS                    = 1 << 9,
    GPA_SHADER_STAGE_INTERSECTION            = 1 << 10,
    GPA_SHADER_STAGE_CALLABLE                = 1 << 11,
    GPA_SHADER_STAGE_AMPLIFICATION           = 1 << 12,
    GPA_SHADER_STAGE_MESH                    = 1 << 13,
    GPA_SHADER_STAGE_DXIL_LIBRARY            = 1 << 14,
    GPA_SHADER_STAGE_ALL_GRAPHICS =
        GPA_SHADER_STAGE_VERTEX |
        GPA_SHADER_STAGE_HULL |
        GPA_SHADER_STAGE_DOMAIN |
        GPA_SHADER_STAGE_GEOMETRY |
        GPA_SHADER_STAGE_FRAGMENT,
    GPA_SHADER_STAGE_ALL = std::numeric_limits<uint32_t>::max(),
};
using ShaderStageFlags = uint32_t;

class SourceFile final
{
public:
    std::string content;
    std::string filename;
};

} // namespace api_types

void EnableGLSLCompiler();
void DisableGLSLCompiler();
bool CompileFromSPIRV(api_types::ShaderLanguage sourceLanguage, std::vector<uint32_t> const* spirv, std::vector<api_types::SourceFile>& files);
void CompileToSPIRV(api_types::ShaderStageFlagBits stage, api_types::ShaderLanguage language, std::string const& shaderCode, std::vector<uint32_t>* spirv, std::string* infoLog, std::string* debugLog);
bool GetReadableSPIRVfromBinarySPIRV(void* spirv, size_t binarySize, std::string& readableSPIRV);
std::string GetGLSLFromSPIRV(std::vector<uint32_t> const& spirv);
bool GetBinarySPIRVfromReadableSPIRV(std::string const& readableSPIRV, std::vector<uint32_t>& spirv);
void GetSPIRVFromGLSL(api_types::ShaderStageFlagBits stage, std::string const& glsl, std::vector<uint32_t>& spirv, std::string& infoLog, std::string& debugLog);

} // namespace detail
} // namespace spirv
} // namespace gvk

/**
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
*/


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

#include "gvk/spir-v.hpp"
#include "gvk/defaults.hpp"

#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv_glsl.hpp"

#include <cassert>
#include <iostream>

namespace gvk {
namespace spirv {

static const TBuiltInResource& built_in_resource();

std::mutex Context::sMutex;
uint32_t Context::sInstanceCount;

VkResult Context::create(const CreateInfo* pCreateInfo, Context* pContext)
{
    (void)pCreateInfo;
    assert(pCreateInfo);
    assert(pContext);
    pContext->reset();
    if (!pContext->mInitialized) {
        std::lock_guard<std::mutex> lock(sMutex);
        if (sInstanceCount) {
            pContext->mInitialized = true;
        } else {
            pContext->mInitialized = glslang::InitializeProcess();
        }
        sInstanceCount += (uint32_t)pContext->mInitialized;
    }
    return pContext->mInitialized ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

Context::~Context()
{
    reset();
}

void Context::reset()
{
    if (mInitialized) {
        std::lock_guard<std::mutex> lock(sMutex);
        glslang::FinalizeProcess();
        assert(sInstanceCount);
        --sInstanceCount;
    }
}

VkResult Context::compile(ShaderInfo* pShaderInfo)
{
    assert(mInitialized);
    assert(pShaderInfo);
    assert(pShaderInfo->language == ShadingLanguage::Glsl && "TODO : ShadingLanguage::Hlsl");
    pShaderInfo->spirv.clear();
    pShaderInfo->errors.clear();
    EShLanguage eshStage{ };
    switch (pShaderInfo->stage) {
    case VK_SHADER_STAGE_VERTEX_BIT: eshStage = EShLangVertex; break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: eshStage = EShLangTessControl; break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: eshStage = EShLangTessEvaluation; break;
    case VK_SHADER_STAGE_GEOMETRY_BIT: eshStage = EShLangGeometry; break;
    case VK_SHADER_STAGE_FRAGMENT_BIT: eshStage = EShLangFragment; break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR: eshStage = EShLangRayGen; break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: eshStage = EShLangAnyHit; break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: eshStage = EShLangClosestHit; break;
    case VK_SHADER_STAGE_MISS_BIT_KHR: eshStage = EShLangMiss; break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR: eshStage = EShLangIntersect; break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR: eshStage = EShLangCallable; break;
    case VK_SHADER_STAGE_COMPUTE_BIT: eshStage = EShLangCompute; break;
    default: return VK_ERROR_FEATURE_NOT_PRESENT;
    }
    glslang::TShader shader(eshStage);
    auto glsl = pShaderInfo->source;
    if (pShaderInfo->lineOffset) {
        glsl.insert(0, pShaderInfo->lineOffset, '\n');
    }
    auto pGlsl = glsl.c_str();
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
    shader.setStrings(&pGlsl, 1);
    auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    if (shader.parse(&built_in_resource(), 100, false, messages)) {
        glslang::TProgram program;
        program.addShader(&shader);
        if (program.link(messages)) {
            glslang::GlslangToSpv(*program.getIntermediate(eshStage), pShaderInfo->spirv);
        } else {
            pShaderInfo->errors.push_back(program.getInfoLog());
            pShaderInfo->errors.push_back(program.getInfoDebugLog());
        }
    } else {
        pShaderInfo->errors.push_back(shader.getInfoLog());
        pShaderInfo->errors.push_back(shader.getInfoDebugLog());
    }
    return pShaderInfo->errors.empty() ? VK_SUCCESS : VK_ERROR_UNKNOWN;
}

Context::operator bool() const
{
    return mInitialized;
}

void BindingInfo::add_shader(const ShaderInfo& shaderInfo)
{
    spirv_cross::CompilerGLSL compilerGlsl(shaderInfo.spirv.data(), shaderInfo.spirv.size());

    auto createBinding =
    [&](VkDescriptorType descriptorType, const spirv_cross::Resource& resource)
    {
        auto descriptorSetLayoutBinding = get_default<VkDescriptorSetLayoutBinding>();
        descriptorSetLayoutBinding.binding = compilerGlsl.get_decoration(resource.id, spv::DecorationBinding);
        descriptorSetLayoutBinding.descriptorType = descriptorType;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = shaderInfo.stage;
        auto setIndex = compilerGlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        add_binding(setIndex, descriptorSetLayoutBinding);
    };

    spirv_cross::ShaderResources shaderResources = compilerGlsl.get_shader_resources();
    for (const auto& shaderResource : shaderResources.uniform_buffers) {
        createBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderResource);
    }
    for (const auto& shaderResource : shaderResources.storage_buffers) {
        createBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, shaderResource);
    }
    for (const auto& shaderResource : shaderResources.sampled_images) {
        createBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, shaderResource);
    }
    for (const auto& shaderResource : shaderResources.storage_images) {
        createBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, shaderResource);
    }
    for (const auto& shaderResource : shaderResources.acceleration_structures) {
        createBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, shaderResource);
    }
    for (const auto& shaderResource : shaderResources.push_constant_buffers) {
        auto ranges = compilerGlsl.get_active_buffer_ranges(shaderResource.id);
        pushConstantRanges.push_back(VkPushConstantRange{ .stageFlags = (VkShaderStageFlags)shaderInfo.stage });
        for (const auto& range : ranges) {
            pushConstantRanges.back().size += (uint32_t)range.range;
        }
    }
}

void BindingInfo::add_binding(uint32_t setIndex, const VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding)
{
    auto& bindings = descriptorSetLayoutBindings[setIndex];
    bindings.insert(std::upper_bound(bindings.begin(), bindings.end(), descriptorSetLayoutBinding), descriptorSetLayoutBinding);
}

VkResult create_descriptor_set_layouts(const Device& device, const BindingInfo& bindingInfo, const VkAllocationCallbacks* pAllocator, uint32_t* pDescriptorSetLayoutCount, DescriptorSetLayout* pDescriptorSetLayouts)
{
    assert(device);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (pDescriptorSetLayoutCount) {
            gvk_result(VK_SUCCESS);
            if (pDescriptorSetLayouts) {
                size_t i = 0;
                auto itr = bindingInfo.descriptorSetLayoutBindings.begin();
                while (i < *pDescriptorSetLayoutCount && itr != bindingInfo.descriptorSetLayoutBindings.end()) {
                    const auto& descriptorSetLayoutBindings = itr->second;
                    auto descriptorSetLayoutCreateInfo = get_default<VkDescriptorSetLayoutCreateInfo>();
                    descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)descriptorSetLayoutBindings.size();
                    descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
                    gvk_result(DescriptorSetLayout::create(device, &descriptorSetLayoutCreateInfo, pAllocator, pDescriptorSetLayouts + i));
                    ++itr;
                    ++i;
                }
            } else {
                *pDescriptorSetLayoutCount = (uint32_t)bindingInfo.descriptorSetLayoutBindings.size();
            }
        }
    } gvk_result_scope_end
    return gvkResult;
}

VkResult create_pipeline_layout(const Device& device, const BindingInfo& bindingInfo, const VkAllocationCallbacks* pAllocator, gvk::PipelineLayout* pPipelineLayout)
{
    assert(device);
    assert(pPipelineLayout);
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        uint32_t descriptorSetLayoutCount = 0;
        gvk_result(gvk::spirv::create_descriptor_set_layouts(device, bindingInfo, nullptr, &descriptorSetLayoutCount, nullptr));
        std::vector<gvk::DescriptorSetLayout> descriptorSetLayouts(descriptorSetLayoutCount);
        gvk_result(gvk::spirv::create_descriptor_set_layouts(device, bindingInfo, pAllocator, &descriptorSetLayoutCount, descriptorSetLayouts.data()));
        auto vkDescriptorSetLayouts = gvk::get_vk_handles(descriptorSetLayouts);
        auto pipelineLayoutCreateInfo = gvk::get_default<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)vkDescriptorSetLayouts.size();
        pipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)bindingInfo.pushConstantRanges.size();
        pipelineLayoutCreateInfo.pPushConstantRanges = bindingInfo.pushConstantRanges.data();
        gvk_result(gvk::PipelineLayout::create(device, &pipelineLayoutCreateInfo, pAllocator, pPipelineLayout));
    } gvk_result_scope_end
    return gvkResult;
}

const std::array<uint32_t, 240>& full_screen_triangle_vertex_shader()
{
    static const std::array<uint32_t, 240> sSpirv{
        119734787, 65536, 524295, 40,
        0, 131089, 1, 393227,
        1, 1280527431, 1685353262, 808793134,
        0, 196622, 0, 1,
        524303, 0, 4, 1852399981,
        0, 9, 12, 26,
        196611, 2, 450, 262149,
        4, 1852399981, 0, 327685,
        9, 1700033382, 1869562744, 25714,
        393221, 12, 1449094247, 1702130277,
        1684949368, 30821, 393221, 24,
        1348430951, 1700164197, 2019914866, 0,
        393222, 24, 0, 1348430951,
        1953067887, 7237481, 196613, 26,
        0, 262215, 9, 30,
        0, 262215, 12, 11,
        42, 327752, 24, 0,
        11, 0, 196679, 24,
        2, 131091, 2, 196641,
        3, 2, 196630, 6,
        32, 262167, 7, 6,
        2, 262176, 8, 3,
        7, 262203, 8, 9,
        3, 262165, 10, 32,
        1, 262176, 11, 1,
        10, 262203, 11, 12,
        1, 262187, 10, 14,
        1, 262187, 10, 16,
        2, 262167, 23, 6,
        4, 196638, 24, 23,
        262176, 25, 3, 24,
        262203, 25, 26, 3,
        262187, 10, 27, 0,
        262187, 6, 29, 1073741824,
        262187, 6, 31, 1065353216,
        262187, 6, 34, 0,
        262176, 38, 3, 23,
        327734, 2, 4, 0,
        3, 131320, 5, 262205,
        10, 13, 12, 327876,
        10, 15, 13, 14,
        327879, 10, 17, 15,
        16, 262255, 6, 18,
        17, 262205, 10, 19,
        12, 327879, 10, 20,
        19, 16, 262255, 6,
        21, 20, 327760, 7,
        22, 18, 21, 196670,
        9, 22, 262205, 7,
        28, 9, 327822, 7,
        30, 28, 29, 327760,
        7, 32, 31, 31,
        327811, 7, 33, 30,
        32, 327761, 6, 35,
        33, 0, 327761, 6,
        36, 33, 1, 458832,
        23, 37, 35, 36,
        34, 31, 327745, 38,
        39, 26, 27, 196670,
        39, 37, 65789, 65592
    };
    return sSpirv;
}

static const TBuiltInResource& built_in_resource()
{
    static const TBuiltInResource sBuiltInResource{
        .maxLights = 32,
        .maxClipPlanes = 6,
        .maxTextureUnits = 32,
        .maxTextureCoords = 32,
        .maxVertexAttribs = 64,
        .maxVertexUniformComponents = 4096,
        .maxVaryingFloats = 64,
        .maxVertexTextureImageUnits = 32,
        .maxCombinedTextureImageUnits = 80,
        .maxTextureImageUnits = 32,
        .maxFragmentUniformComponents = 4096,
        .maxDrawBuffers = 32,
        .maxVertexUniformVectors = 128,
        .maxVaryingVectors = 8,
        .maxFragmentUniformVectors = 16,
        .maxVertexOutputVectors = 16,
        .maxFragmentInputVectors = 15,
        .minProgramTexelOffset = -8,
        .maxProgramTexelOffset = 7,
        .maxClipDistances = 8,
        .maxComputeWorkGroupCountX = 65535,
        .maxComputeWorkGroupCountY = 65535,
        .maxComputeWorkGroupCountZ = 65535,
        .maxComputeWorkGroupSizeX = 1024,
        .maxComputeWorkGroupSizeY = 1024,
        .maxComputeWorkGroupSizeZ = 64,
        .maxComputeUniformComponents = 1024,
        .maxComputeTextureImageUnits = 16,
        .maxComputeImageUniforms = 8,
        .maxComputeAtomicCounters = 8,
        .maxComputeAtomicCounterBuffers = 1,
        .maxVaryingComponents = 60,
        .maxVertexOutputComponents = 64,
        .maxGeometryInputComponents = 64,
        .maxGeometryOutputComponents = 128,
        .maxFragmentInputComponents = 128,
        .maxImageUnits = 8,
        .maxCombinedImageUnitsAndFragmentOutputs = 8,
        .maxCombinedShaderOutputResources = 8,
        .maxImageSamples = 0,
        .maxVertexImageUniforms = 0,
        .maxTessControlImageUniforms = 0,
        .maxTessEvaluationImageUniforms = 0,
        .maxGeometryImageUniforms = 0,
        .maxFragmentImageUniforms = 8,
        .maxCombinedImageUniforms = 8,
        .maxGeometryTextureImageUnits = 16,
        .maxGeometryOutputVertices = 256,
        .maxGeometryTotalOutputComponents = 1024,
        .maxGeometryUniformComponents = 1024,
        .maxGeometryVaryingComponents = 64,
        .maxTessControlInputComponents = 128,
        .maxTessControlOutputComponents = 128,
        .maxTessControlTextureImageUnits = 16,
        .maxTessControlUniformComponents = 1024,
        .maxTessControlTotalOutputComponents = 4096,
        .maxTessEvaluationInputComponents = 128,
        .maxTessEvaluationOutputComponents = 128,
        .maxTessEvaluationTextureImageUnits = 16,
        .maxTessEvaluationUniformComponents = 1024,
        .maxTessPatchComponents = 120,
        .maxPatchVertices = 32,
        .maxTessGenLevel = 64,
        .maxViewports = 16,
        .maxVertexAtomicCounters = 0,
        .maxTessControlAtomicCounters = 0,
        .maxTessEvaluationAtomicCounters = 0,
        .maxGeometryAtomicCounters = 0,
        .maxFragmentAtomicCounters = 8,
        .maxCombinedAtomicCounters = 8,
        .maxAtomicCounterBindings = 1,
        .maxVertexAtomicCounterBuffers = 0,
        .maxTessControlAtomicCounterBuffers = 0,
        .maxTessEvaluationAtomicCounterBuffers = 0,
        .maxGeometryAtomicCounterBuffers = 0,
        .maxFragmentAtomicCounterBuffers = 1,
        .maxCombinedAtomicCounterBuffers = 1,
        .maxAtomicCounterBufferSize = 16384,
        .maxTransformFeedbackBuffers = 4,
        .maxTransformFeedbackInterleavedComponents = 64,
        .maxCullDistances = 8,
        .maxCombinedClipAndCullDistances = 8,
        .maxSamples = 4,
        .limits = {
            .nonInductiveForLoops = 1,
            .whileLoops = 1,
            .doWhileLoops = 1,
            .generalUniformIndexing = 1,
            .generalAttributeMatrixVectorIndexing = 1,
            .generalVaryingIndexing = 1,
            .generalSamplerIndexing = 1,
            .generalVariableIndexing = 1,
            .generalConstantMatrixVectorIndexing = 1,
        }
    };
    return sBuiltInResource;
}

} // namespace spirv
} // namespace gvk

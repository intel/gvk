
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

#include "gvk-spirv/context.hpp"
#include "gvk-structures.hpp"

#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv_glsl.hpp"

#include <cassert>
#include <iostream>

namespace gvk {
namespace spirv {

static TBuiltInResource get_built_in_resource()
{
    TBuiltInResource builtInResource { };
    builtInResource.maxLights = 32;
    builtInResource.maxClipPlanes = 6;
    builtInResource.maxTextureUnits = 32;
    builtInResource.maxTextureCoords = 32;
    builtInResource.maxVertexAttribs = 64;
    builtInResource.maxVertexUniformComponents = 4096;
    builtInResource.maxVaryingFloats = 64;
    builtInResource.maxVertexTextureImageUnits = 32;
    builtInResource.maxCombinedTextureImageUnits = 80;
    builtInResource.maxTextureImageUnits = 32;
    builtInResource.maxFragmentUniformComponents = 4096;
    builtInResource.maxDrawBuffers = 32;
    builtInResource.maxVertexUniformVectors = 128;
    builtInResource.maxVaryingVectors = 8;
    builtInResource.maxFragmentUniformVectors = 16;
    builtInResource.maxVertexOutputVectors = 16;
    builtInResource.maxFragmentInputVectors = 15;
    builtInResource.minProgramTexelOffset = -8;
    builtInResource.maxProgramTexelOffset = 7;
    builtInResource.maxClipDistances = 8;
    builtInResource.maxComputeWorkGroupCountX = 65535;
    builtInResource.maxComputeWorkGroupCountY = 65535;
    builtInResource.maxComputeWorkGroupCountZ = 65535;
    builtInResource.maxComputeWorkGroupSizeX = 1024;
    builtInResource.maxComputeWorkGroupSizeY = 1024;
    builtInResource.maxComputeWorkGroupSizeZ = 64;
    builtInResource.maxComputeUniformComponents = 1024;
    builtInResource.maxComputeTextureImageUnits = 16;
    builtInResource.maxComputeImageUniforms = 8;
    builtInResource.maxComputeAtomicCounters = 8;
    builtInResource.maxComputeAtomicCounterBuffers = 1;
    builtInResource.maxVaryingComponents = 60;
    builtInResource.maxVertexOutputComponents = 64;
    builtInResource.maxGeometryInputComponents = 64;
    builtInResource.maxGeometryOutputComponents = 128;
    builtInResource.maxFragmentInputComponents = 128;
    builtInResource.maxImageUnits = 8;
    builtInResource.maxCombinedImageUnitsAndFragmentOutputs = 8;
    builtInResource.maxCombinedShaderOutputResources = 8;
    builtInResource.maxImageSamples = 0;
    builtInResource.maxVertexImageUniforms = 0;
    builtInResource.maxTessControlImageUniforms = 0;
    builtInResource.maxTessEvaluationImageUniforms = 0;
    builtInResource.maxGeometryImageUniforms = 0;
    builtInResource.maxFragmentImageUniforms = 8;
    builtInResource.maxCombinedImageUniforms = 8;
    builtInResource.maxGeometryTextureImageUnits = 16;
    builtInResource.maxGeometryOutputVertices = 256;
    builtInResource.maxGeometryTotalOutputComponents = 1024;
    builtInResource.maxGeometryUniformComponents = 1024;
    builtInResource.maxGeometryVaryingComponents = 64;
    builtInResource.maxTessControlInputComponents = 128;
    builtInResource.maxTessControlOutputComponents = 128;
    builtInResource.maxTessControlTextureImageUnits = 16;
    builtInResource.maxTessControlUniformComponents = 1024;
    builtInResource.maxTessControlTotalOutputComponents = 4096;
    builtInResource.maxTessEvaluationInputComponents = 128;
    builtInResource.maxTessEvaluationOutputComponents = 128;
    builtInResource.maxTessEvaluationTextureImageUnits = 16;
    builtInResource.maxTessEvaluationUniformComponents = 1024;
    builtInResource.maxTessPatchComponents = 120;
    builtInResource.maxPatchVertices = 32;
    builtInResource.maxTessGenLevel = 64;
    builtInResource.maxViewports = 16;
    builtInResource.maxVertexAtomicCounters = 0;
    builtInResource.maxTessControlAtomicCounters = 0;
    builtInResource.maxTessEvaluationAtomicCounters = 0;
    builtInResource.maxGeometryAtomicCounters = 0;
    builtInResource.maxFragmentAtomicCounters = 8;
    builtInResource.maxCombinedAtomicCounters = 8;
    builtInResource.maxAtomicCounterBindings = 1;
    builtInResource.maxVertexAtomicCounterBuffers = 0;
    builtInResource.maxTessControlAtomicCounterBuffers = 0;
    builtInResource.maxTessEvaluationAtomicCounterBuffers = 0;
    builtInResource.maxGeometryAtomicCounterBuffers = 0;
    builtInResource.maxFragmentAtomicCounterBuffers = 1;
    builtInResource.maxCombinedAtomicCounterBuffers = 1;
    builtInResource.maxAtomicCounterBufferSize = 16384;
    builtInResource.maxTransformFeedbackBuffers = 4;
    builtInResource.maxTransformFeedbackInterleavedComponents = 64;
    builtInResource.maxCullDistances = 8;
    builtInResource.maxCombinedClipAndCullDistances = 8;
    builtInResource.maxSamples = 4;
    builtInResource.limits.nonInductiveForLoops = 1;
    builtInResource.limits.whileLoops = 1;
    builtInResource.limits.doWhileLoops = 1;
    builtInResource.limits.generalUniformIndexing = 1;
    builtInResource.limits.generalAttributeMatrixVectorIndexing = 1;
    builtInResource.limits.generalVaryingIndexing = 1;
    builtInResource.limits.generalSamplerIndexing = 1;
    builtInResource.limits.generalVariableIndexing = 1;
    builtInResource.limits.generalConstantMatrixVectorIndexing = 1;
    return builtInResource;
}

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
    auto builtInResource = get_built_in_resource();
    if (shader.parse(&builtInResource, 100, false, messages)) {
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
        pushConstantRanges.push_back(VkPushConstantRange{ (VkShaderStageFlags)shaderInfo.stage, 0, 0 } );
        for (const auto& range : ranges) {
            pushConstantRanges.back().size += (uint32_t)range.range;
        }
    }
}

void BindingInfo::add_binding(uint32_t setIndex, const VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding)
{
    auto& bindings = descriptorSetLayoutBindings[setIndex];
    auto bindingItr = std::lower_bound(bindings.begin(), bindings.end(), descriptorSetLayoutBinding,
        [](const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs)
        {
            return lhs.binding < rhs.binding;
        }
    );
    if (bindingItr == bindings.end()) {
        bindings.insert(bindingItr, descriptorSetLayoutBinding);
    } else {
        auto stageFlags = bindingItr->stageFlags | descriptorSetLayoutBinding.stageFlags;
        bindingItr->stageFlags = descriptorSetLayoutBinding.stageFlags;
        assert(*bindingItr == descriptorSetLayoutBinding);
        bindingItr->stageFlags = stageFlags;
    }
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
    } gvk_result_scope_end;
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
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace spirv
} // namespace gvk

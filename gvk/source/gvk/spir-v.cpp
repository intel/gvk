
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

#include "gvk/spir-v.hpp"
#include "gvk/defaults.hpp"

#ifdef GVK_GLSLANG_ENABLED
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#endif

#ifdef GVK_SPIRV_CROSS_ENABLED
#include "spirv_glsl.hpp"
#endif

#include <cassert>
#include <iostream>

namespace gvk {
namespace spirv {

#ifdef GVK_GLSLANG_ENABLED
static const TBuiltInResource& get_built_in_resource()
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
#endif // GVK_GLSLANG_ENABLED

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
            #ifdef GVK_GLSLANG_ENABLED
            pContext->mInitialized = glslang::InitializeProcess();
            #else
            assert(false && "TODO : gvk::spirv::Context currently requires GVK be built with GVK_GLSLANG_ENABLED");
            pContext->mInitialized = false;
            #endif // GVK_GLSLANG_ENABLED
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
        #ifdef GVK_GLSLANG_ENABLED
        glslang::FinalizeProcess();
        #else
        assert(false && "TODO : gvk::spirv::Context currently requires GVK be built with GVK_GLSLANG_ENABLED");
        #endif // GVK_GLSLANG_ENABLED
        assert(sInstanceCount);
        --sInstanceCount;
    }
}

VkResult Context::compile(ShaderInfo* pShaderInfo)
{
#ifdef GVK_GLSLANG_ENABLED
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
    if (shader.parse(&get_built_in_resource(), 100, false, messages)) {
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
#else
    (void)pShaderInfo;
    assert(false && "TODO : gvk::spirv::Context currently requires GVK be built with GVK_GLSLANG_ENABLED");
    return VK_ERROR_FEATURE_NOT_PRESENT;
#endif // GVK_GLSLANG_ENABLED
}

Context::operator bool() const
{
    return mInitialized;
}

void BindingInfo::add_shader(const ShaderInfo& shaderInfo)
{
#ifdef GVK_SPIRV_CROSS_ENABLED
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
#else
    (void)shaderInfo;
    assert(false && "TODO : gvk::spirv::Context currently requires GVK be built with GVK_GLSLANG_ENABLED");
#endif // GVK_GLSLANG_ENABLED
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

} // namespace spirv
} // namespace gvk

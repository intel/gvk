
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

#ifdef GVK_COMPILER_MSVC
#pragma warning(push, 0)
#endif // GVK_COMPILER_MSVC

#define ENABLE_HLSL
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv_common.hpp"
#include "spirv_glsl.hpp"
#include "spirv_hlsl.hpp"
#include "spirv_parser.hpp"
#include "spirv-tools/libspirv.hpp"

// HACK : Both SPV_VERSION and SPV_REVISION are unconditionally defined by
//  SPIRV-Cross and SPIRV-Headers, so both are being cleared before including
//  spirv/1.2/spirv.h.
//
//  build/_deps/spirv-headers-src/include/spirv/1.2/spirv.h:53: error: "SPV_VERSION" redefined [-Werror]  53 | #define SPV_VERSION 0x10200
//  build/_deps/spirv-cross-src/spirv.hpp:54: note: this is the location of the previous definition       54 | #define SPV_VERSION 0x10600
//
//  build/_deps/spirv-headers-src/include/spirv/1.2/spirv.h:54: error: "SPV_REVISION" redefined [-Werror] 54 | #define SPV_REVISION 2
//  build/_deps/spirv-cross-src/spirv.hpp:55: note: this is the location of the previous definition       55 | #define SPV_REVISION 1
//
// TODO : Figure out the correct include order, conditional include, etc. to
//  ensure there's no mismatch.
#undef SPV_VERSION
#undef SPV_REVISION
#include "spirv/1.2/spirv.h"

#ifdef GVK_COMPILER_MSVC
#pragma warning(pop)
#endif // GVK_COMPILER_MSVC

#include <cassert>
#include <iostream>

namespace gvk {
namespace spirv {

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
    auto pDefaultResources = GetDefaultResources();
    assert(pDefaultResources);
    if (shader.parse(pDefaultResources, 100, false, messages)) {
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
                    auto descriptorSetLayoutCreateInfo = gvk::get_default<VkDescriptorSetLayoutCreateInfo>();
                    const auto& descriptorSetLayoutCreateInfoItr = bindingInfo.descriptorSetLayoutCreateInfos.find(itr->first);
                    if (descriptorSetLayoutCreateInfoItr != bindingInfo.descriptorSetLayoutCreateInfos.end()) {
                        descriptorSetLayoutCreateInfo = descriptorSetLayoutCreateInfoItr->second;
                    }
                    const auto& descriptorSetLayoutBindings = itr->second;
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

namespace detail {

void EnableGLSLCompiler()
{
    auto success = glslang::InitializeProcess();
    (void)success;
    assert(success);
}

void DisableGLSLCompiler()
{
    glslang::FinalizeProcess();
}

struct SpirvShaderInfo
{
    std::map<uint32_t, std::string> filename;
    std::map<uint32_t, std::string> source;
    api_types::ShaderLanguage language;
    std::string entryPoint;
};

spv_result_t ExtractShaderSourceCallback(void* user_data, const spv_parsed_instruction_t* instruction)
{
    if (instruction->opcode != SpvOpSource && instruction->opcode != SpvOpString && instruction->opcode != SpvOpEntryPoint) {
        return SPV_SUCCESS;
    }

    SpirvShaderInfo* result = (SpirvShaderInfo*)(user_data);

    switch (instruction->opcode) {
    case SpvOpSource: {
        result->language = api_types::GPA_SHADER_LANGUAGE_UNKNOWN;
        SpvSourceLanguage language = static_cast<SpvSourceLanguage>(instruction->words[instruction->operands[0].offset]);
        switch (language) {
        case SpvSourceLanguage::SpvSourceLanguageGLSL: {
            result->language = api_types::GPA_SHADER_LANGUAGE_GLSL;
            break;
        }
        case SpvSourceLanguage::SpvSourceLanguageHLSL: {
            result->language = api_types::GPA_SHADER_LANGUAGE_HLSL;
            break;
        }
        default: {
            break;
        }
        }
        if (instruction->num_operands == 4) {
            uint32_t fileId = instruction->words[instruction->operands[2].offset];
            result->source[fileId] = (char*)(instruction->words + instruction->operands[3].offset);
        }
        break;
    }
    case SpvOpString: {
        assert(instruction->num_operands == 2);
        uint32_t fileId = instruction->words[instruction->operands[0].offset];
        result->filename[fileId] = (char*)(instruction->words + instruction->operands[1].offset);
        break;
    }
    case SpvOpEntryPoint: {
        for (uint16_t i = 0; i < instruction->num_operands; i++) {
            auto& operand = instruction->operands[i];
            if (operand.type == SPV_OPERAND_TYPE_LITERAL_STRING) {
                result->entryPoint = (char*)(instruction->words + instruction->operands[i].offset);
                break;
            }
        }
        break;
    }
    }
    return SPV_SUCCESS;
}

#ifdef GVK_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

bool CompileFromSPIRV(api_types::ShaderLanguage sourceLanguage, std::vector<uint32_t> const* spirv, std::vector<api_types::SourceFile>& files)
{
    if (!spirv) {
        return false;
    }

    static spv_context context = spvContextCreate(spv_target_env::SPV_ENV_UNIVERSAL_1_3);
    SpirvShaderInfo shaderInfo;
    spvBinaryParse(context, &shaderInfo, spirv->data(), spirv->size(), nullptr, ExtractShaderSourceCallback, nullptr);

    if (shaderInfo.language == sourceLanguage && !shaderInfo.source.empty()) {
        for (auto& file : shaderInfo.source) {
            const std::string firstLine = "#line 1\n";
            std::string content = file.second;
            std::string filename = shaderInfo.filename[file.first];
            size_t pos = content.find(firstLine);
            if (pos != std::string::npos) {
                content = content.substr(pos + firstLine.length());
                files.push_back({ content, filename });
            }
            return true;
        }
    }

    std::unique_ptr<spirv_cross::CompilerGLSL> compiler;
    try {
        using namespace api_types;
        if (sourceLanguage == GPA_SHADER_LANGUAGE_HLSL) {
            spirv_cross::CompilerHLSL* compilerHLSL = new spirv_cross::CompilerHLSL(*spirv);
            spirv_cross::CompilerHLSL::Options optionsHLSL = {};
            optionsHLSL.shader_model = 50;  // Default to Shader Model 5.0.
            compilerHLSL->set_hlsl_options(optionsHLSL);
            compiler = std::unique_ptr<spirv_cross::CompilerGLSL>(compilerHLSL);
        } else {
            compiler = std::unique_ptr<spirv_cross::CompilerGLSL>(new spirv_cross::CompilerGLSL(*spirv));
        }
        auto options = compiler->get_common_options();
        options.vulkan_semantics = true;
        options.separate_shader_objects = true;
        compiler->set_common_options(options);
        std::string res = compiler->compile();
        files.push_back({res, ""});
    } catch (spirv_cross::CompilerError& error) {
        std::string res = error.what();
        res += "\n";
        if (compiler) {
            res += compiler->get_partial_source();
        }
        files.push_back({res, ""});
        return false;
    }
    return true;
}

#ifdef GVK_COMPILER_MSVC
#pragma warning(pop)
#endif

void CompileToSPIRV(
    api_types::ShaderStageFlagBits stage,
    api_types::ShaderLanguage language,
    std::string const& shaderCode,
    std::vector<uint32_t>* spirv,
    std::string* infoLog,
    std::string* debugLog
)
{
    using namespace api_types;
    if (!spirv || !debugLog) {
        return;
    }
    spirv->clear();
    infoLog->clear();
    debugLog->clear();
    EShLanguage eshStage;
    switch (stage) {
    case GPA_SHADER_STAGE_VERTEX:
        eshStage = EShLangVertex;
        break;
    case GPA_SHADER_STAGE_TESSELLATION_CONTROL:
        eshStage = EShLangTessControl;
        break;
    case GPA_SHADER_STAGE_TESSELLATION_EVALUATION:
        eshStage = EShLangTessEvaluation;
        break;
    case GPA_SHADER_STAGE_GEOMETRY:
        eshStage = EShLangGeometry;
        break;
    case GPA_SHADER_STAGE_FRAGMENT:
        eshStage = EShLangFragment;
        break;
    case GPA_SHADER_STAGE_COMPUTE:
        eshStage = EShLangCompute;
        break;
    case GPA_SHADER_STAGE_RAYGEN:
        eshStage = EShLangRayGen;
        break;
    case GPA_SHADER_STAGE_ANY_HIT:
        eshStage = EShLangAnyHit;
        break;
    case GPA_SHADER_STAGE_CLOSEST_HIT:
        eshStage = EShLangClosestHit;
        break;
    case GPA_SHADER_STAGE_MISS:
        eshStage = EShLangMiss;
        break;
    case GPA_SHADER_STAGE_INTERSECTION:
        eshStage = EShLangIntersect;
        break;
    case GPA_SHADER_STAGE_CALLABLE:
        eshStage = EShLangCallable;
        break;
    default:
        *infoLog = "Invalid shader stage";
        *debugLog = "Invalid shader stage";
        return;
    }

    glslang::TShader shader(eshStage);
    const char* sourceCStr[]{ shaderCode.c_str() };
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
    shader.setStrings(sourceCStr, 1);
    
    EShMessages messages = (EShMessages)EShMsgDefault;
    if (language == GPA_SHADER_LANGUAGE_HLSL) {
        shader.setEntryPoint("main");
        shader.setHlslIoMapping(true);
        messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules | EShMsgHlslLegalization | EShMsgReadHlsl);
    }
    else if (language == GPA_SHADER_LANGUAGE_GLSL) {
        messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    }
    if (!shader.parse(GetDefaultResources(), 100, false, messages)) {
        *infoLog = shader.getInfoLog();
        *debugLog = shader.getInfoDebugLog();
        return;
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages)) {
        *infoLog = shader.getInfoLog();
        *debugLog = shader.getInfoDebugLog();
        return;
    }
    glslang::GlslangToSpv(*program.getIntermediate(eshStage), *spirv);
   
}

bool GetReadableSPIRVfromBinarySPIRV(void* spirv, size_t binarySize, std::string& readableSPIRV)
{
    uint32_t options = SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
    spv_target_env env = spv_target_env::SPV_ENV_UNIVERSAL_1_0;

    spvtools::SpirvTools disassembler(env);

    return disassembler.Disassemble((uint32_t*)spirv, binarySize, &readableSPIRV, options);
}

std::string GetGLSLFromSPIRV(std::vector<uint32_t> const& spirv)
{
    std::string result;
    spirv_cross::CompilerGLSL* glsl = nullptr;
    try {
        glsl = new spirv_cross::CompilerGLSL(spirv);
        auto options = glsl->get_common_options();
        options.vulkan_semantics = true;
        options.separate_shader_objects = true;
        glsl->set_common_options(options);
        result = glsl->compile();
    } catch (...) {
        result = " !!! ONLY PARTIAL SHADER AVAILABLE DUE TO SPIRV-CROSS ERROR !!!\n\n";
        if (glsl) {
            result += glsl->get_partial_source();
        }
    }
    delete glsl;
    return result;
}

bool GetBinarySPIRVfromReadableSPIRV(std::string const& readableSPIRV, std::vector<uint32_t>& spirv)
{
    spirv.clear();

    spv_target_env env = spv_target_env::SPV_ENV_UNIVERSAL_1_0;
    uint32_t options = SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS;

    spvtools::SpirvTools assembler(env);
    if (!assembler.Assemble(readableSPIRV, &spirv, options)) {
        return false;
    }

    return true;
}

void GetSPIRVFromGLSL(
    api_types::ShaderStageFlagBits stage,
    std::string const& glsl,
    std::vector<uint32_t>& spirv,
    std::string& infoLog,
    std::string& debugLog
)
{
    using namespace api_types;
    spirv.clear();
    infoLog.clear();
    debugLog.clear();
    EShLanguage eshStage;
    switch (stage) {
    case GPA_SHADER_STAGE_VERTEX:
        eshStage = EShLangVertex;
        break;
    case GPA_SHADER_STAGE_TESSELLATION_CONTROL:
        eshStage = EShLangTessControl;
        break;
    case GPA_SHADER_STAGE_TESSELLATION_EVALUATION:
        eshStage = EShLangTessEvaluation;
        break;
    case GPA_SHADER_STAGE_GEOMETRY:
        eshStage = EShLangGeometry;
        break;
    case GPA_SHADER_STAGE_FRAGMENT:
        eshStage = EShLangFragment;
        break;
    case GPA_SHADER_STAGE_COMPUTE:
        eshStage = EShLangCompute;
        break;
    default:
        infoLog = "Invalid shader stage";
        debugLog = "Invalid shader stage";
        return;
    }
    glslang::TShader shader(eshStage);
    const char* sourceCStr[]{ glsl.c_str() };
    shader.setStrings(sourceCStr, 1);
    auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    if (!shader.parse(GetDefaultResources(), 100, false, messages)) {
        infoLog = shader.getInfoLog();
        debugLog = shader.getInfoDebugLog();
        return;
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages)) {
        infoLog = shader.getInfoLog();
        debugLog = shader.getInfoDebugLog();
        return;
    }
    glslang::GlslangToSpv(*program.getIntermediate(eshStage), spirv);
}

} // namespace detail
} // namespace spirv
} // namespace gvk

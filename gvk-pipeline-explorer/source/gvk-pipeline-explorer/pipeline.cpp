
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

#include "gvk-pipeline-explorer/pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/utilities.hpp"
#include "gvk-spirv.hpp"

#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_parser.hpp"

namespace gvk {

VkResult PipelineExplorer::get_pipeline_executable_properties(pipeline_explorer::PipelineInfo pipelineInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        gvk::Device device = pipelineInfo->deviceInfo->vkHandle;
        gvk_result(device ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        std::vector<gvk::ShaderModule> replacementShaderModules(pipelineInfo->shaderModuleInfos.size());
        for (size_t shaderModule_i = 0; shaderModule_i < pipelineInfo->shaderModuleInfos.size(); ++shaderModule_i) {
            gvk_result(gvk::ShaderModule::create(device, &*pipelineInfo->shaderModuleInfos[shaderModule_i].second->shaderModuleCreateInfo, nullptr, &replacementShaderModules[shaderModule_i]));
        }

        // TODO : Documentation
        gvk::Pipeline replacementPipeline = VK_NULL_HANDLE;
        switch (pipelineInfo->bindPoint) {
        case VK_PIPELINE_BIND_POINT_COMPUTE: {

            // TODO : Documentation
            auto computePipelineCreateInfo = pipelineInfo->computePipelineCreateInfo;
            auto pPipelineCreateFlags2CreateInfo = gvk::get_pnext<VkPipelineCreateFlags2CreateInfo>(*computePipelineCreateInfo);
            if (pPipelineCreateFlags2CreateInfo) {
                const_cast<VkPipelineCreateFlags2CreateInfo*>(pPipelineCreateFlags2CreateInfo)->flags |= VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            } else {
                const_cast<VkComputePipelineCreateInfo&>(*computePipelineCreateInfo).flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            }

            // TODO : Documentation
            gvk_result(replacementShaderModules.size() == 1 ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            const_cast<VkComputePipelineCreateInfo&>(*computePipelineCreateInfo).stage.module = replacementShaderModules.back();

            // TODO : Documentation
            gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
            gvk_result(create_replacement_pipeline_layout(device, computePipelineCreateInfo->layout, &replacementPipelineLayout));
            const_cast<VkComputePipelineCreateInfo&>(*computePipelineCreateInfo).layout = replacementPipelineLayout;

            // TODO : Base pipeline

            // TODO : Documentation
            gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &*computePipelineCreateInfo, nullptr, &replacementPipeline));

        } break;
        case VK_PIPELINE_BIND_POINT_GRAPHICS: {

            // TODO : Documentation
            auto graphicsPipelineCreateInfo = pipelineInfo->graphicsPipelineCreateInfo;
            auto pPipelineCreateFlags2CreateInfo = gvk::get_pnext<VkPipelineCreateFlags2CreateInfo>(*graphicsPipelineCreateInfo);
            if (pPipelineCreateFlags2CreateInfo) {
                const_cast<VkPipelineCreateFlags2CreateInfo*>(pPipelineCreateFlags2CreateInfo)->flags |= VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            } else {
                const_cast<VkGraphicsPipelineCreateInfo&>(*graphicsPipelineCreateInfo).flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            }

            // TODO : Documentation
            gvk_result(replacementShaderModules.size() == graphicsPipelineCreateInfo->stageCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            for (uint32_t stage_i = 0; stage_i < graphicsPipelineCreateInfo->stageCount; ++stage_i) {
                const_cast<VkPipelineShaderStageCreateInfo&>(graphicsPipelineCreateInfo->pStages[stage_i]).module = replacementShaderModules[stage_i];
            }

            // TODO : Documentation
            gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
            gvk_result(create_replacement_pipeline_layout(device, graphicsPipelineCreateInfo->layout, &replacementPipelineLayout));
            const_cast<VkGraphicsPipelineCreateInfo&>(*graphicsPipelineCreateInfo).layout = replacementPipelineLayout;

            // TODO : Documentation
            gvk::RenderPass replacementRenderPass = VK_NULL_HANDLE;
            if (graphicsPipelineCreateInfo->renderPass) {
                gvk_result(create_replacement_render_pass(device, graphicsPipelineCreateInfo->renderPass, &replacementRenderPass));
                const_cast<VkGraphicsPipelineCreateInfo&>(*graphicsPipelineCreateInfo).renderPass = replacementRenderPass;
            }

            // TODO : Base pipeline

            // TODO : Documentation
            gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &*graphicsPipelineCreateInfo, nullptr, &replacementPipeline));

        } break;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {

            // TODO : Documentation
            auto rayTracingPipelineCreateInfo = pipelineInfo->rayTracingPipelineCreateInfo;
            auto pPipelineCreateFlags2CreateInfo = gvk::get_pnext<VkPipelineCreateFlags2CreateInfo>(*rayTracingPipelineCreateInfo);
            if (pPipelineCreateFlags2CreateInfo) {
                const_cast<VkPipelineCreateFlags2CreateInfo*>(pPipelineCreateFlags2CreateInfo)->flags |= VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            } else {
                const_cast<VkRayTracingPipelineCreateInfoKHR&>(*rayTracingPipelineCreateInfo).flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            }

            // TODO : Documentation
            gvk_result(replacementShaderModules.size() == rayTracingPipelineCreateInfo->stageCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            for (uint32_t stage_i = 0; stage_i < rayTracingPipelineCreateInfo->stageCount; ++stage_i) {
                const_cast<VkPipelineShaderStageCreateInfo&>(rayTracingPipelineCreateInfo->pStages[stage_i]).module = replacementShaderModules[stage_i];
            }

            // TODO : Pipeline library

            // TODO : Documentation
            gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
            gvk_result(create_replacement_pipeline_layout(device, rayTracingPipelineCreateInfo->layout, &replacementPipelineLayout));
            const_cast<VkRayTracingPipelineCreateInfoKHR&>(*rayTracingPipelineCreateInfo).layout = replacementPipelineLayout;

            // TODO : Base pipeline

            // TODO : Documentation
            gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &*rayTracingPipelineCreateInfo, nullptr, &replacementPipeline));

        } break;
        default: {
            gvk_result(VK_ERROR_INITIALIZATION_FAILED);
        } break;
        }

        // TODO : Documentation
        auto pipelineInfoKHR = gvk::get_default<VkPipelineInfoKHR>();
        pipelineInfoKHR.pipeline = replacementPipeline;
        uint32_t executableCount = 0;
        gvk_result(device.GetPipelineExecutablePropertiesKHR(&pipelineInfoKHR, &executableCount, nullptr));
        std::vector<VkPipelineExecutablePropertiesKHR> pipelineExecutableProperties(executableCount, gvk::get_default<VkPipelineExecutablePropertiesKHR>());
        gvk_result(device.GetPipelineExecutablePropertiesKHR(&pipelineInfoKHR, &executableCount, pipelineExecutableProperties.data()));

        // TODO : Documentation
        pipelineInfo->executableInfos.resize(executableCount);
        for (uint32_t executable_i = 0; executable_i < executableCount; ++executable_i) {
            auto& executableInfo = pipelineInfo->executableInfos[executable_i];
            executableInfo.properties = pipelineExecutableProperties[executable_i];

            // TODO : Documentation
            auto pipelineExecutableInfo = gvk::get_default<VkPipelineExecutableInfoKHR>();
            pipelineExecutableInfo.pipeline = replacementPipeline;
            pipelineExecutableInfo.executableIndex = executable_i;

            // TODO : Documentation
            uint32_t statisticsCount = 0;
            gvk_result(device.GetPipelineExecutableStatisticsKHR(&pipelineExecutableInfo, &statisticsCount, nullptr));
            std::vector<VkPipelineExecutableStatisticKHR> statistics(statisticsCount, gvk::get_default<VkPipelineExecutableStatisticKHR>());
            gvk_result(device.GetPipelineExecutableStatisticsKHR(&pipelineExecutableInfo, &statisticsCount, statistics.data()));
            executableInfo.statistics.reserve(statisticsCount);
            for (uint32_t statistic_i = 0; statistic_i < statisticsCount; ++statistic_i) {
                executableInfo.statistics.push_back(statistics[statistic_i]);
            }

            // TODO : Documentation
            uint32_t internalRepresentationCount = 0;
            gvk_result(device.GetPipelineExecutableInternalRepresentationsKHR(&pipelineExecutableInfo, &internalRepresentationCount, nullptr));
            std::vector<VkPipelineExecutableInternalRepresentationKHR> internalRepresentations(internalRepresentationCount, gvk::get_default<VkPipelineExecutableInternalRepresentationKHR>());
            gvk_result(device.GetPipelineExecutableInternalRepresentationsKHR(&pipelineExecutableInfo, &internalRepresentationCount, internalRepresentations.data()));
            executableInfo.internalRepresentations.reserve(internalRepresentationCount);
            for (uint32_t internalRepresentation_i = 0; internalRepresentation_i < internalRepresentationCount; ++internalRepresentation_i) {
                executableInfo.internalRepresentations.push_back(internalRepresentations[internalRepresentation_i]);
            }
        }

        // TODO : Documentation
        if (pipelineInfo->executableInfos.empty()) {
            pipelineInfo->executableInfos.push_back({ });
        }
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult set_pipeline_driver_uuid(pipeline_explorer::PipelineInfo pipelineInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        if (pipelineInfo->deviceInfo->VK_KHR_pipeline_properties_enabled) {
            gvk::Device device = pipelineInfo->deviceInfo->vkHandle;
            gvk_result(device ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            auto pipelineInfoKHR = gvk::get_default<VkPipelineInfoKHR>();
            pipelineInfoKHR.pipeline = pipelineInfo->vkHandle;
            auto pipelinePropertiesIdentifier = gvk::get_default<VkPipelinePropertiesIdentifierEXT>();
            gvk_result(device.GetPipelinePropertiesEXT(&pipelineInfoKHR, (VkBaseOutStructure*)&pipelinePropertiesIdentifier));
            pipelineInfo->pipelinePropertiesIdentifier = pipelinePropertiesIdentifier;
            boost::multiprecision::import_bits(pipelineInfo->driverUUID, pipelinePropertiesIdentifier.pipelineIdentifier, pipelinePropertiesIdentifier.pipelineIdentifier + VK_UUID_SIZE);
        }    
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines));
        for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
            pipeline_explorer::PipelineInfo pipelineInfo(gvk::newref, { device, pPipelines[pipeline_i] });
            pipelineInfo->deviceInfo = device;
            pipelineInfo->vkHandle = pPipelines[pipeline_i];
            pipelineInfo->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            pipelineInfo->computePipelineCreateInfo = pCreateInfos[pipeline_i];
            pipelineInfo->pipelineLayoutInfo = pipeline_explorer::PipelineLayoutInfo({ device, pCreateInfos[pipeline_i].layout });
            pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, pCreateInfos[pipeline_i].stage.module });
            gvk_result(shaderModuleInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            pipelineInfo->shaderModuleInfos.push_back({ pCreateInfos[pipeline_i].stage.stage, shaderModuleInfo });
            auto inserted = pipelineInfos.insert({ { device, pPipelines[pipeline_i] }, pipelineInfo }).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            pipelineInfo->uuid = pipeline_explorer::get_uuid(device, pipelineInfo->computePipelineCreateInfo);
            gvk_result(set_pipeline_driver_uuid(pipelineInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(BasicApiCallHandler::execute_vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines));
        for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
            pipeline_explorer::PipelineInfo pipelineInfo(gvk::newref, { device, pPipelines[pipeline_i] });
            pipelineInfo->deviceInfo = device;
            pipelineInfo->vkHandle = pPipelines[pipeline_i];
            pipelineInfo->bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            pipelineInfo->graphicsPipelineCreateInfo = pCreateInfos[pipeline_i];
            pipelineInfo->pipelineLayoutInfo = pipeline_explorer::PipelineLayoutInfo({ device, pCreateInfos[pipeline_i].layout });
            pipelineInfo->renderPassInfo = pipeline_explorer::RenderPassInfo({ device, pCreateInfos[pipeline_i].renderPass });
            for (uint32_t stage_i = 0; stage_i < pCreateInfos[pipeline_i].stageCount; ++stage_i) {
                pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, pCreateInfos[pipeline_i].pStages[stage_i].module });
                gvk_result(shaderModuleInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                pipelineInfo->shaderModuleInfos.push_back({ pCreateInfos[pipeline_i].pStages[stage_i].stage, shaderModuleInfo });
            }
            auto inserted = pipelineInfos.insert({ {device, pPipelines[pipeline_i]}, pipelineInfo }).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            pipelineInfo->uuid = pipeline_explorer::get_uuid(device, pipelineInfo->graphicsPipelineCreateInfo);
            gvk_result(set_pipeline_driver_uuid(pipelineInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk::Device gvkDevice = device;
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvkResult = BasicApiCallHandler::execute_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        switch (gvkResult) {
        case VK_SUCCESS:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR: {
            for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {

                // TODO : Documentation
                pipeline_explorer::PipelineInfo pipelineInfo(gvk::newref, { device, pPipelines[pipeline_i] });
                pipelineInfo->deviceInfo = device;
                pipelineInfo->vkHandle = pPipelines[pipeline_i];
                pipelineInfo->bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
                pipelineInfo->rayTracingPipelineCreateInfo = pCreateInfos[pipeline_i];
                pipelineInfo->pipelineLayoutInfo = pipeline_explorer::PipelineLayoutInfo({ device, pCreateInfos[pipeline_i].layout });

                // TODO : Documentation
                for (uint32_t stage_i = 0; stage_i < pCreateInfos[pipeline_i].stageCount; ++stage_i) {
                    pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, pCreateInfos[pipeline_i].pStages[stage_i].module });
                    if (!shaderModuleInfo) {
                        gvk_result(VK_ERROR_INITIALIZATION_FAILED);
                    }
                    pipelineInfo->shaderModuleInfos.push_back({ pCreateInfos[pipeline_i].pStages[stage_i].stage, shaderModuleInfo });
                }

                // TODO : Documentation
                // NOTE : Explicit conversion to VkPhysicalDevice shouldn't be necessary here
                //  since gvk::PhysicalDevice provides a VkPhysicalDevice conversion operator.
                //  GCC and Clang aren't identifying the conversion though.S
                // TODO : Double check on latest versions of GCC and Clang
                pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo = (VkPhysicalDevice)gvkDevice.get<gvk::PhysicalDevice>();
                gvk_result(physicalDeviceInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                auto shaderGroupHandleSize = physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties->shaderGroupHandleSize;
                gvk_result(shaderGroupHandleSize ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                pipelineInfo->shaderGroupHandles.resize(pCreateInfos[pipeline_i].groupCount);
                for (uint32_t group_i = 0; group_i < pCreateInfos[pipeline_i].groupCount; ++group_i) {
                    pipelineInfo->shaderGroupHandles[group_i].resize(shaderGroupHandleSize);
                    gvk_result(gvkDevice.GetRayTracingShaderGroupHandlesKHR(pPipelines[pipeline_i], group_i, 1, shaderGroupHandleSize, pipelineInfo->shaderGroupHandles[group_i].data()));
                }

                // TODO : Documentation
                auto inserted = pipelineInfos.insert({ { device, pPipelines[pipeline_i] }, pipelineInfo }).second;
                if (!inserted) {
                    gvk_result(VK_ERROR_INITIALIZATION_FAILED);
                }

                // TODO : Documentation
                pipelineInfo->uuid = pipeline_explorer::get_uuid(device, pipelineInfo->rayTracingPipelineCreateInfo);
                if (set_pipeline_driver_uuid(pipelineInfo) != VK_SUCCESS) {
                    gvk_result(VK_ERROR_INITIALIZATION_FAILED);
                }
            }
        } break;
        default: {
        } break;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
    pipelineInfos.erase({ device, pipeline });
    BasicApiCallHandler::execute_vkDestroyPipeline(device, pipeline, pAllocator);
}

void PipelineExplorer::decompile_pipeline(VkDevice device, VkPipeline pipeline)
{
    /*
    TODO : Check for VK_KHR_spirv_1_4
    Vulkan 1.0 supports SPIR-V 1.0
    Vulkan 1.1 supports SPIR-V 1.3 and below
    Vulkan 1.2 supports SPIR-V 1.5 and below
    Vulkan 1.3 supports SPIR-V 1.6 and below
    */

    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo) {

        // TODO : Documentation
        if (pipelineInfo->deviceInfo->VK_KHR_pipeline_executable_properties_enabled && pipelineInfo->executableInfos.empty()) {
            if (get_pipeline_executable_properties(pipelineInfo) != VK_SUCCESS) {
                messages.push_back("ERROR : TODO : Documentation");
            }
        }

        for (auto shaderModuleInfoItr : pipelineInfo->shaderModuleInfos) {
            auto stage = shaderModuleInfoItr.first;
            auto shaderModuleInfo = shaderModuleInfoItr.second;
            const auto& shaderModuleCreateInfo = shaderModuleInfo->shaderModuleCreateInfo;
            if (shaderModuleInfo->glsl.empty() || shaderModuleInfo->spirv.empty()) {

                // TODO : Documentation
                auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
                shaderInfo.version = gvk::spirv::Version::SPIRV_1_4; // TODO : Set programatically
                shaderInfo.bytecode.resize(shaderModuleCreateInfo->codeSize / sizeof(uint32_t));
                memcpy(shaderInfo.bytecode.data(), shaderModuleCreateInfo->pCode, shaderModuleCreateInfo->codeSize);

                // TODO : Documentation
                if (shaderModuleInfo->glsl.empty()) {
                    shaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
                    if (spirvContext.decompile(&shaderInfo) == VK_SUCCESS) {
                        shaderModuleInfo->glsl = shaderInfo.source;
                    } else {
                        // TODO : Fixup these error messages...shouldn't spam this loop for each message
                        for (auto const& error : shaderInfo.errors) {
                            messages.push_back("ERROR : Failed to get GLSL for " + gvk::to_string(stage, pipeline_explorer::PrinterFlags));
                            messages.push_back(error);
                            messages.push_back("");
                        }
                    }
                }

                // TODO : Documentation
                if (shaderModuleInfo->spirv.empty()) {
                    shaderInfo.language = gvk::spirv::ShadingLanguage::SpirV;
                    if (spirvContext.decompile(&shaderInfo) == VK_SUCCESS) {
                        shaderModuleInfo->spirv = shaderInfo.source;
                    } else {
                        // TODO : Fixup these error messages...shouldn't spam this loop for each message
                        for (auto const& error : shaderInfo.errors) {
                            messages.push_back("ERROR : Failed to get SPIR-V text for" + gvk::to_string(stage, pipeline_explorer::PrinterFlags));
                            messages.push_back(error);
                            messages.push_back("");
                        }
                    }
                }
            }
        }
    } else {
        messages.push_back("ERROR : TODO : Documentation");
    }
}

void PipelineExplorer::write_pipeline_info(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path)
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo && !path.empty()) {

        // TODO : Documentation
        std::error_code errorCode;
        std::filesystem::create_directories(path, errorCode);
        if (!errorCode) {

            // TODO : Documentation
            if (pipelineInfo->computePipelineCreateInfo->sType == gvk::get_stype<VkComputePipelineCreateInfo>()) {
                std::ofstream computePipelineCreateInfoJson(path / "VkComputePipelineCreateInfo.json");
                computePipelineCreateInfoJson << gvk::to_string(*pipelineInfo->computePipelineCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
            } else if (pipelineInfo->graphicsPipelineCreateInfo->sType == gvk::get_stype<VkGraphicsPipelineCreateInfo>()) {
                std::ofstream graphicsPipelineCreateInfoJson(path / "VkGraphicsPipelineCreateInfo.json");
                graphicsPipelineCreateInfoJson << gvk::to_string(*pipelineInfo->graphicsPipelineCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
            } else if (pipelineInfo->rayTracingPipelineCreateInfo->sType == gvk::get_stype<VkRayTracingPipelineCreateInfoKHR>()) {
                std::ofstream rayTracingPipelineCreateInfoJson(path / "VkRayTracingPipelineCreateInfoKHR.json");
                rayTracingPipelineCreateInfoJson << gvk::to_string(*pipelineInfo->rayTracingPipelineCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
            }

            // TODO : Documentation
            if (pipelineInfo->pipelineLayoutInfo) {
                std::ofstream pipelineLayoutCreateInfoJson(path / "VkPipelineLayoutCreateInfo.json");
                pipelineLayoutCreateInfoJson << gvk::to_string(*pipelineInfo->pipelineLayoutInfo->pipelineLayoutCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;

                // TODO : Documentation
                for (uint32_t setLayout_i = 0; setLayout_i < pipelineInfo->pipelineLayoutInfo->descriptorSetLayoutInfos.size(); ++setLayout_i) {
                    const auto& descriptorSetLayoutInfo = pipelineInfo->pipelineLayoutInfo->descriptorSetLayoutInfos[setLayout_i];
                    std::ofstream descriptorSetLayoutCreateInfoJson(path / ("VkDescriptorSetLayoutCreateInfo[" + std::to_string(setLayout_i) + "].json"));
                    descriptorSetLayoutCreateInfoJson << gvk::to_string(*descriptorSetLayoutInfo->descriptorSetLayoutCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;

                    // TODO : Documentation
                    for (uint32_t immutableSampler_i = 0; immutableSampler_i < descriptorSetLayoutInfo->immutableSamplerInfos.size(); ++immutableSampler_i) {
                        const auto& immutableSamplerInfo = descriptorSetLayoutInfo->immutableSamplerInfos[immutableSampler_i];
                        std::ofstream immutableSamplerInfoJson(path / ("VkSamplerCreateInfo[" + std::to_string(setLayout_i) + "][" + std::to_string(immutableSampler_i) + "].json"));
                        immutableSamplerInfoJson << gvk::to_string(*immutableSamplerInfo->samplerCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
                    }
                }
            }

            // TODO : Documentation
            if (pipelineInfo->renderPassInfo) {
                if (pipelineInfo->renderPassInfo->renderPassCreateInfo->sType == gvk::get_stype<VkRenderPassCreateInfo>()) {
                    std::ofstream renderPassCreateInfoJson(path / "VkRenderPassCreateInfo.json");
                    renderPassCreateInfoJson << gvk::to_string(*pipelineInfo->renderPassInfo->renderPassCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
                } else if (pipelineInfo->renderPassInfo->renderPassCreateInfo2->sType == gvk::get_stype<VkRenderPassCreateInfo2>()) {
                    std::ofstream renderPassCreateInfo2Json(path / "VkRenderPassCreateInfo2.json");
                    renderPassCreateInfo2Json << gvk::to_string(*pipelineInfo->renderPassInfo->renderPassCreateInfo2, pipeline_explorer::PrinterFlags) << std::endl;
                }
            }

            // TODO : Documentation
            for (uint32_t executable_i = 0; executable_i < pipelineInfo->executableInfos.size(); ++executable_i) {
                auto const& executableInfo = pipelineInfo->executableInfos[executable_i];
                for (uint32_t internalRepresentation_i = 0; internalRepresentation_i < executableInfo.internalRepresentations.size(); ++internalRepresentation_i) {
                    auto const& internalRepresentation = executableInfo.internalRepresentations[internalRepresentation_i];
                    if (internalRepresentation->isText) {
                        std::string name = gvk::string::replace(executableInfo.properties->name, " ", "-");
                        std::string extension = gvk::string::to_lower(gvk::string::replace(internalRepresentation->name, " ", "-"));
                        std::ofstream internalRepresentationFile(path / (name + "." + extension));
                        internalRepresentationFile << std::endl;
                        internalRepresentationFile << "/*" << std::endl;
                        internalRepresentationFile << std::endl;
                        internalRepresentationFile << executableInfo.properties->description << std::endl;
                        internalRepresentationFile << internalRepresentation->description << std::endl;
                        internalRepresentationFile << std::endl;
                        std::vector<std::array<std::string, 3>> inernalRepresentationInfos{
                            { "Subgroup Size", std::to_string(executableInfo.properties->subgroupSize), "The subgroup size with which this pipeline executable is dispatched." }
                        };
                        size_t nameColumnWidth = inernalRepresentationInfos[0][0].length();
                        size_t valueColumnWidth = inernalRepresentationInfos[0][1].length();
                        for (const auto& statistic : executableInfo.statistics) {
                            inernalRepresentationInfos.push_back({ });
                            inernalRepresentationInfos.back()[0] = statistic->name;
                            inernalRepresentationInfos.back()[2] = statistic->description;
                            switch (statistic->format) {
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR: {
                                inernalRepresentationInfos.back()[1] = std::to_string(statistic->value.b32);
                            } break;
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR: {
                                inernalRepresentationInfos.back()[1] = std::to_string(statistic->value.i64);
                            } break;
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR: {
                                inernalRepresentationInfos.back()[1] = std::to_string(statistic->value.u64);
                            } break;
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR: {
                                inernalRepresentationInfos.back()[1] = std::to_string(statistic->value.f64);
                            } break;
                            default: {
                                internalRepresentationFile << "Unserviced VkPipelineExecutableStatisticFormatKHR; gvk maintenance required" << std::endl;
                            } break;
                            }
                            nameColumnWidth = std::max(nameColumnWidth, inernalRepresentationInfos.back()[0].length());
                            valueColumnWidth = std::max(valueColumnWidth, inernalRepresentationInfos.back()[1].length());
                        }
                        for (const auto& inernalRepresentationInfo : inernalRepresentationInfos) {
                            internalRepresentationFile << std::left << std::setw(nameColumnWidth) << inernalRepresentationInfo[0] << " : ";
                            internalRepresentationFile << std::right << std::setw(valueColumnWidth) << inernalRepresentationInfo[1] << " : ";
                            internalRepresentationFile << std::left << inernalRepresentationInfo[2] << std::endl;
                        }
                        internalRepresentationFile << std::endl;
                        internalRepresentationFile << "*/" << std::endl;
                        if (internalRepresentation->pData) {
                            internalRepresentationFile << std::endl;
                            internalRepresentationFile << (const char*)internalRepresentation->pData << std::endl;
                        }
                        internalRepresentationFile << std::endl;
                    }
                }
            }

            // TODO : Documentation
            for (auto shaderModuleInfoItr : pipelineInfo->shaderModuleInfos) {
                auto stage = shaderModuleInfoItr.first;
                auto shaderModuleInfo = shaderModuleInfoItr.second;
                auto shaderFileName = "VkShaderModule-UUID-" + pipeline_explorer::uuid_to_string(shaderModuleInfo->uuid, 9);

                #if 0
                // TODO : Documentation
                std::ofstream shaderModuleCreateInfoJson(path / (shaderFileName + ".VkShaderModuleCreateInfo.json"));
                shaderModuleCreateInfoJson << gvk::to_string(*shaderModuleInfo->shaderModuleCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
                #endif

                // TODO : Documentation
                if (!shaderModuleInfo->glsl.empty()) {
                    std::filesystem::path glslPath = path / (shaderFileName + "." + pipeline_explorer::get_shader_stage_file_extension(stage));
                    if (std::filesystem::exists(glslPath)) {
                        std::ifstream existingGlslFile(glslPath);
                        if (shaderModuleInfo->glsl != std::string(std::istreambuf_iterator<char>(existingGlslFile), { })) {
                            glslPath = path / (shaderFileName + ".unmodified." + pipeline_explorer::get_shader_stage_file_extension(stage));
                        }
                    }
                    std::ofstream glslFile(glslPath);
                    glslFile << shaderModuleInfo->glsl;
                }

                // TODO : Documentation
                if (!shaderModuleInfo->spirv.empty()) {
                    std::filesystem::path spirvPath = path / (shaderFileName + "." + pipeline_explorer::get_shader_stage_file_extension(stage) + ".spirv");
                    if (std::filesystem::exists(spirvPath)) {
                        std::ifstream existingSpirvFile(spirvPath);
                        if (shaderModuleInfo->spirv != std::string(std::istreambuf_iterator<char>(existingSpirvFile), { })) {
                            spirvPath = path / (shaderFileName + ".unmodified." + pipeline_explorer::get_shader_stage_file_extension(stage) + ".spirv");
                        }
                    }
                    std::ofstream spirvFile(spirvPath);
                    spirvFile << shaderModuleInfo->spirv;
                }
            }
        } else {
            messages.push_back("ERROR : TODO : Documentation");
        }
    } else {
        messages.push_back("ERROR : TODO : Documentation");
    }
}

void PipelineExplorer::read_pipeline_info(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, std::unordered_map<pipeline_explorer::UUID, std::string>* pShaderSource)
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo && !path.empty() && std::filesystem::exists(path) && pShaderSource) {
        for (uint32_t stage_i = 0; stage_i < pipelineInfo->shaderModuleInfos.size(); ++stage_i) {
            auto stage = pipelineInfo->shaderModuleInfos[stage_i].first;
            const auto& shaderModuleInfo = pipelineInfo->shaderModuleInfos[stage_i].second;
            auto shaderPath = path / ("VkShaderModule-UUID-" + pipeline_explorer::uuid_to_string(shaderModuleInfo->uuid, 9));
            shaderPath.replace_extension(pipeline_explorer::get_shader_stage_file_extension(stage));
            if (std::filesystem::exists(shaderPath)) {
                std::ifstream glslFile(shaderPath);
                if (glslFile.is_open()) {
                    auto source = std::string((std::istreambuf_iterator<char>(glslFile)), std::istreambuf_iterator<char>());
                    if (!source.empty() && !gvk::string::is_whitespace(source)) {
                        if (!pShaderSource->insert({ shaderModuleInfo->uuid, source }).second) {
#if 0
                            messages.push_back("WARNING : TODO : Documentation, duplicate shader UUID in pipeline");
#endif
                        }
                    } else {
                        messages.push_back("ERROR : \"" + shaderPath.string() + "\" contained no shader source.  Was it successfully decompiled first?");
                    }
                } else {
                    messages.push_back("ERROR : Failed to open \"" + shaderPath.string() + "\".  Was it successfully decompiled first?");
                }
            } else {
                messages.push_back("ERROR : \"" + shaderPath.string() + "\" not found.  Was it successfully decompiled first?");
            }
        }
    } else {
        messages.push_back("ERROR : TODO : Documentation");
    }
}

VkResult PipelineExplorer::create_replacement_pipeline(VkDevice device, VkPipeline pipeline, const std::unordered_map<pipeline_explorer::UUID, std::string>& shaderSource, gvk::Pipeline* pPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
        gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        std::vector<gvk::ShaderModule> replacementShaderModules(pipelineInfo->shaderModuleInfos.size());
        std::vector<VkPipelineShaderStageCreateInfo> replacementPipelineShaderStageCreateInfos(pipelineInfo->shaderModuleInfos.size());
        gvk_result(pipelineInfo->shaderModuleInfos.size() == shaderSource.size() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        for (uint32_t shaderModule_i = 0; shaderModule_i < pipelineInfo->shaderModuleInfos.size(); ++shaderModule_i) {
            auto stage = pipelineInfo->shaderModuleInfos[shaderModule_i].first;
            const auto& shaderModuleInfo = pipelineInfo->shaderModuleInfos[shaderModule_i].second;

            // TODO : Documentation
            auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
            shaderInfo.version = gvk::spirv::Version::SPIRV_1_4; // TODO : Set programatically
            shaderInfo.stage = stage;
            const auto& shaderSourceItr = shaderSource.find(shaderModuleInfo->uuid);
            shaderInfo.source = shaderSourceItr != shaderSource.end() ? shaderSourceItr->second : std::string();
            if (!shaderInfo.source.empty() && !gvk::string::is_whitespace(shaderInfo.source)) {
                gvkResult = spirvContext.compile(&shaderInfo);
                if (gvkResult == VK_SUCCESS && !shaderInfo.bytecode.empty() && shaderInfo.errors.empty()) {
                    auto shaderModuleCreateInfo = *shaderModuleInfo->shaderModuleCreateInfo;
                    shaderModuleCreateInfo.codeSize = shaderInfo.bytecode.size() * sizeof(uint32_t);
                    shaderModuleCreateInfo.pCode = shaderInfo.bytecode.data();
                    gvkResult = gvk::ShaderModule::create(shaderModuleInfo->deviceInfo->vkHandle, &shaderModuleCreateInfo, nullptr, &replacementShaderModules[shaderModule_i]);
                    if (gvkResult == VK_SUCCESS) {
                        replacementPipelineShaderStageCreateInfos[shaderModule_i].module = replacementShaderModules[shaderModule_i];
                        replacementPipelineShaderStageCreateInfos[shaderModule_i].pName = "main"; // TODO : Can the original name be reflected?
                    } else {
                        messages.push_back("ERROR : vkCreateShaderModule(" + gvk::to_string(shaderModuleCreateInfo, pipeline_explorer::PrinterFlags) + ")");
                        messages.push_back("ERROR : VkResult " + gvk::to_string(gvkResult, pipeline_explorer::PrinterFlags));
                        messages.push_back("ERROR : Enable VK_LAYER_KHRONOS_validation for more information.");
                    }
                } else {
                    messages.push_back("ERROR : Failed to compile " + gvk::to_string(stage, pipeline_explorer::PrinterFlags));
                    for (const auto& error : shaderInfo.errors) {
                        messages.push_back(error);
                    }
                }
            } else {
                messages.push_back("ERROR : Failed to compile " + gvk::to_string(stage, pipeline_explorer::PrinterFlags) + "; no source provided.");
            }
        }

        // TODO : Documentation
        if (messages.empty()) {
            switch (pipelineInfo->bindPoint) {
            case VK_PIPELINE_BIND_POINT_COMPUTE: {
                auto computePipelineCreateInfo = *pipelineInfo->computePipelineCreateInfo;
                gvk_result(replacementPipelineShaderStageCreateInfos.size() == 1 ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                auto shaderModule = replacementPipelineShaderStageCreateInfos[0].module;
                replacementPipelineShaderStageCreateInfos[0] = computePipelineCreateInfo.stage;
                replacementPipelineShaderStageCreateInfos[0].module = shaderModule;
                replacementPipelineShaderStageCreateInfos[0].pName = "main"; // TODO : Can the original name be reflected?
                computePipelineCreateInfo.stage = replacementPipelineShaderStageCreateInfos[0];

                // TODO : Documentation
                gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
                gvk_result(create_replacement_pipeline_layout(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->computePipelineCreateInfo->layout, &replacementPipelineLayout));
                computePipelineCreateInfo.layout = replacementPipelineLayout;

                // TODO : Documentation
                gvk_result(gvk::Pipeline::create(pipelineInfo->deviceInfo->vkHandle, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pPipeline));
            } break;
            case VK_PIPELINE_BIND_POINT_GRAPHICS: {
                auto graphicsPipelineCreateInfo = *pipelineInfo->graphicsPipelineCreateInfo;
                for (uint32_t stage_i = 0; stage_i < graphicsPipelineCreateInfo.stageCount; ++stage_i) {
                    auto shaderModule = replacementPipelineShaderStageCreateInfos[stage_i].module;
                    replacementPipelineShaderStageCreateInfos[stage_i] = graphicsPipelineCreateInfo.pStages[stage_i];
                    replacementPipelineShaderStageCreateInfos[stage_i].module = shaderModule;
                    replacementPipelineShaderStageCreateInfos[stage_i].pName = "main"; // TODO : Can the original name be reflected?
                }
                graphicsPipelineCreateInfo.pStages = !replacementPipelineShaderStageCreateInfos.empty() ? replacementPipelineShaderStageCreateInfos.data() : nullptr;

                // TODO : Documentation
                gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
                gvk_result(create_replacement_pipeline_layout(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->graphicsPipelineCreateInfo->layout, &replacementPipelineLayout));
                graphicsPipelineCreateInfo.layout = replacementPipelineLayout;

                // TODO : Documentation
                gvk::RenderPass replacementRenderPass = VK_NULL_HANDLE;
                if (pipelineInfo->graphicsPipelineCreateInfo->renderPass) {
                    gvk_result(create_replacement_render_pass(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->graphicsPipelineCreateInfo->renderPass, &replacementRenderPass));
                    graphicsPipelineCreateInfo.renderPass = replacementRenderPass;
                }

                // TODO : Base pipeline

                // TODO : Documentation
                gvk_result(gvk::Pipeline::create(pipelineInfo->deviceInfo->vkHandle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pPipeline));
            } break;
            case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
                auto rayTracingPipelineCreateInfo = *pipelineInfo->rayTracingPipelineCreateInfo;
                for (uint32_t stage_i = 0; stage_i < rayTracingPipelineCreateInfo.stageCount; ++stage_i) {
                    auto shaderModule = replacementPipelineShaderStageCreateInfos[stage_i].module;
                    replacementPipelineShaderStageCreateInfos[stage_i] = rayTracingPipelineCreateInfo.pStages[stage_i];
                    replacementPipelineShaderStageCreateInfos[stage_i].module = shaderModule;
                    replacementPipelineShaderStageCreateInfos[stage_i].pName = "main"; // TODO : Can the original name be reflected?
                }
                rayTracingPipelineCreateInfo.pStages = !replacementPipelineShaderStageCreateInfos.empty() ? replacementPipelineShaderStageCreateInfos.data() : nullptr;

                // TODO : Pipeline library

                // TODO : Documentation
                gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
                gvk_result(create_replacement_pipeline_layout(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->rayTracingPipelineCreateInfo->layout, &replacementPipelineLayout));
                rayTracingPipelineCreateInfo.layout = replacementPipelineLayout;

                // TODO : Base pipeline

                // TODO : Documentation
                gvk_result(gvk::Pipeline::create(pipelineInfo->deviceInfo->vkHandle, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, pPipeline));
            } break;
            default: {
            } break;
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::create_replacement_shader_binding_table(const gvk::Device& gvkDevice, pipeline_explorer::QueueInfo queueInfo, VkCommandBuffer vkCommandBuffer, pipeline_explorer::PipelineInfo pipelineInfo, VkStridedDeviceAddressRegionKHR* pShaderBindingTable)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (pShaderBindingTable && pShaderBindingTable->deviceAddress && pShaderBindingTable->stride && pShaderBindingTable->size) {
            gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);

            // Get the buffer containing the application's shader binding table so that a
            //  barrier can be recorded before reading from it
            uint32_t bindingCount = 1;
            auto binding = gvk::get_default<VkBindBufferMemoryInfo>();
            queueInfo->deviceInfo->deviceAddressTracker.get_buffer_bindings(pShaderBindingTable->deviceAddress, &bindingCount, &binding);
            pipeline_explorer::BufferInfo shaderBindingTableBufferInfo({ gvkDevice, binding.buffer });
            gvk_result(shaderBindingTableBufferInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // The application's shader binding table may not be at the beginning of the
            //  buffer, so calculate the offset into the buffer it's at
            auto bufferDeviceAddressInfo = gvk::get_default<VkBufferDeviceAddressInfo>();
            bufferDeviceAddressInfo.buffer = binding.buffer;
            auto shaderBindingTableDeviceAddress = gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);
            auto shaderBindingTableOffset = shaderBindingTableDeviceAddress - pShaderBindingTable->deviceAddress;

            // Record barrier to ensure any writes are complete before reading from the
            //  application's shader binding table
            {
                auto bufferMemoryBarrier = gvk::get_default<VkBufferMemoryBarrier>();
                bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                bufferMemoryBarrier.buffer = shaderBindingTableBufferInfo->vkHandle;
                bufferMemoryBarrier.offset = shaderBindingTableOffset;
                bufferMemoryBarrier.size = pShaderBindingTable->size;
                gvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
                    vkCommandBuffer,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0,
                    0, nullptr,
                    1, &bufferMemoryBarrier,
                    0, nullptr
                );
            }

            // Prepare a buffer for the replacement shader binding table
            gvk::Buffer replacementShaderBindingTableBuffer;
            auto shaderBindingTableSize = pShaderBindingTable->size;
            auto shaderGroupBaseAlignment = queueInfo->deviceInfo->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties->shaderGroupBaseAlignment;
            if (shaderGroupBaseAlignment) {
                shaderBindingTableSize += shaderGroupBaseAlignment - 1;
            }
            gvk_result(queueInfo->shaderBindingTableReplacementResources.get_buffer(gvkDevice, shaderBindingTableSize, &replacementShaderBindingTableBuffer));
            bufferDeviceAddressInfo.buffer = replacementShaderBindingTableBuffer;
            auto replacementShaderBindingTableDeviceAddress = gvkDevice.GetBufferDeviceAddressKHR(&bufferDeviceAddressInfo);
            auto misalignment = replacementShaderBindingTableDeviceAddress % shaderGroupBaseAlignment;
            if (misalignment) {
                replacementShaderBindingTableDeviceAddress += shaderGroupBaseAlignment - misalignment;
            }

            // Record barrier to prepare replacement buffer for write.  It's very likely
            //  that this barrier is unnecessary, but since the buffer comes from a pool of
            //  utility buffers, it's here for safe measure.
            {
                auto bufferMemoryBarrier = gvk::get_default<VkBufferMemoryBarrier>();
                bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                bufferMemoryBarrier.buffer = replacementShaderBindingTableBuffer;
                gvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
                    vkCommandBuffer,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0,
                    0, nullptr,
                    1, &bufferMemoryBarrier,
                    0, nullptr
                );
            }

#if 1 // DEBUGGING
            // Copy the application's shader binding table to the replacement buffer
            {
                auto gpuMemcpyInfo = gvk::get_default<GpuMemcpyInfo>();
                gpuMemcpyInfo.dst = replacementShaderBindingTableDeviceAddress;
                gpuMemcpyInfo.src = pShaderBindingTable->deviceAddress;
                gpuMemcpyInfo.size = pShaderBindingTable->size;
                gvk::Pipeline gpuMemcpyPipeline;
                gvk_result(queueInfo->shaderBindingTableReplacementResources.get_gpu_memcpy_pipeline(gvkDevice, &gpuMemcpyPipeline));
                gvkDevice.get<DispatchTable>().gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpuMemcpyPipeline);
                gvkDevice.get<DispatchTable>().gvkCmdPushConstants(vkCommandBuffer, gpuMemcpyPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GpuMemcpyInfo), &gpuMemcpyInfo);
                // TODO : Query GPU to create shader with ideal workgroup size
                static const uint32_t local_size_x = 16;
                auto workgroupSize = (uint32_t)((gpuMemcpyInfo.size + local_size_x - 1) / (uint64_t)local_size_x);
                gvkDevice.get<DispatchTable>().gvkCmdDispatch(vkCommandBuffer, workgroupSize, 1, 1);
            }
#endif

            // Record barrier ensuring copy is done before patching shader group handles
            {
                auto bufferMemoryBarrier = gvk::get_default<VkBufferMemoryBarrier>();
                bufferMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                bufferMemoryBarrier.buffer = replacementShaderBindingTableBuffer;
                gvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
                    vkCommandBuffer,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0,
                    0, nullptr,
                    1, &bufferMemoryBarrier,
                    0, nullptr
                );
            }

            // Record barrier ensuring shader group handle map is written before using it
            {
                auto bufferMemoryBarrier = gvk::get_default<VkBufferMemoryBarrier>();
                bufferMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                bufferMemoryBarrier.buffer = pipelineInfo->shaderGroupHandleMap.gvkBuffer;
                gvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
                    vkCommandBuffer,
                    VK_PIPELINE_STAGE_HOST_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0,
                    0, nullptr,
                    1, &bufferMemoryBarrier,
                    0, nullptr
                );
            }

#if 1 // DEBUGGING
            // Execute shader group map, this will run through the replacement buffer and
            //  replace the application's shader group handles with the replacment handles
            {
                auto gpuAddressMapInfo = gvk::get_default<GpuAddressMapInfo>();
                gpuAddressMapInfo.dst = replacementShaderBindingTableDeviceAddress;
                gpuAddressMapInfo.src = pShaderBindingTable->deviceAddress;
                gpuAddressMapInfo.stride = pShaderBindingTable->stride;
                gpuAddressMapInfo.count = pShaderBindingTable->size / pShaderBindingTable->stride;
                gpuAddressMapInfo.keys = pipelineInfo->shaderGroupHandleMap.keys;
                gpuAddressMapInfo.values = pipelineInfo->shaderGroupHandleMap.values;
                gpuAddressMapInfo.kvpCount = pipelineInfo->shaderGroupHandleMap.kvpCount;
                gvk::Pipeline gpuAddressMapPipeline;
                gvk_result(queueInfo->shaderBindingTableReplacementResources.get_gpu_address_map_pipeline(gvkDevice, &gpuAddressMapPipeline));
                gvkDevice.get<DispatchTable>().gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpuAddressMapPipeline);
                gvkDevice.get<DispatchTable>().gvkCmdPushConstants(vkCommandBuffer, gpuAddressMapPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GpuAddressMapInfo), &gpuAddressMapInfo);
                // TODO : Query GPU to create shader with ideal workgroup size
                static const uint32_t local_size_x = 16;
                auto workgroupSize = (uint32_t)((gpuAddressMapInfo.count + local_size_x - 1) / (uint64_t)local_size_x);
                gvkDevice.get<DispatchTable>().gvkCmdDispatch(vkCommandBuffer, workgroupSize, 1, 1);
            }
#endif

            // Record barrier to ensure shader group handle patching is complete before
            //  ray tracing shader stage begins
            {
                auto bufferMemoryBarrier = gvk::get_default<VkBufferMemoryBarrier>();
                bufferMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                bufferMemoryBarrier.buffer = replacementShaderBindingTableBuffer;
                gvkDevice.get<DispatchTable>().gvkCmdPipelineBarrier(
                    vkCommandBuffer,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                    0,
                    0, nullptr,
                    1, &bufferMemoryBarrier,
                    0, nullptr
                );
            }

#if 1 // DEBUGGING
            // Point the shader binding table address at the replacement
            pShaderBindingTable->deviceAddress = replacementShaderBindingTableDeviceAddress;
#endif
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::create_replacement_shader_binding_tables(pipeline_explorer::QueueInfo queueInfo, pipeline_explorer::PipelineInfo pipelineInfo, GvkCommandStructureCmdTraceRaysKHR* pCmd)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pCmd ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk::Device gvkDevice = queueInfo->deviceInfo->vkHandle;
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pRaygenShaderBindingTable)));
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pMissShaderBindingTable)));
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pHitShaderBindingTable)));
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pCallableShaderBindingTable)));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::create_experiment_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path)
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo && !path.empty()) {
        gvk::Device gvkDevice = device;
        assert(gvkDevice);
        std::unordered_map<pipeline_explorer::UUID, std::string> shaderSource;
        read_pipeline_info(device, pipeline, path, &shaderSource);
        if (shaderSource.size() == pipelineInfo->shaderModuleInfos.size()) {
            (void)create_replacement_pipeline(device, pipeline, shaderSource, &pipelineInfo->experimentPipeline);
            if (pipelineInfo->experimentPipeline && pipelineInfo->bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                auto shaderGroupHandleMapCreateInfo = gvk::get_default<gvk::ShaderGroupHandleMapCreateInfo>();
                shaderGroupHandleMapCreateInfo.keysPipeline = pipelineInfo->vkHandle;
                shaderGroupHandleMapCreateInfo.valuesPipeline = pipelineInfo->experimentPipeline;
                shaderGroupHandleMapCreateInfo.shaderGroupHandleCount = pipelineInfo->rayTracingPipelineCreateInfo->groupCount;
                auto gvkResult = gvk::create_shader_group_handle_map(gvkDevice, &shaderGroupHandleMapCreateInfo, &pipelineInfo->shaderGroupHandleMap);
                (void)gvkResult;
                assert(gvkResult == VK_SUCCESS);
            }
        }
    }
    return { };
}

std::vector<std::string> PipelineExplorer::enable_experiment_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, VkBool32 enabled)
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo) {
        pipelineInfo->experimentEnabled = enabled;
        if (enabled && !pipelineInfo->experimentPipeline) {
            (void)create_experiment_pipeline(device, pipeline, path);
        }
    }
    return { };
}

VkResult PipelineExplorer::create_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4])
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo && pipelineInfo->bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && !path.empty()) {
        decompile_pipeline(device, pipeline);
        write_pipeline_info(device, pipeline, path);
        std::unordered_map<pipeline_explorer::UUID, std::string> shaderSource;
        read_pipeline_info(device, pipeline, path, &shaderSource);
        if (messages.empty()) {

            // TODO : Documentation
            std::string glsl;
            for (uint32_t shaderModule_i = 0; shaderModule_i < pipelineInfo->shaderModuleInfos.size() && glsl.empty(); ++shaderModule_i) {
                const auto& shaderModduleInfoItr = pipelineInfo->shaderModuleInfos[shaderModule_i];
                if (shaderModduleInfoItr.first == VK_SHADER_STAGE_FRAGMENT_BIT) {
                    const auto& shaderModuleInfo = shaderModduleInfoItr.second;
                    const auto& shaderModuleCreateInfo = shaderModuleInfo->shaderModuleCreateInfo;

                    // TODO : Documentation
                    std::stringstream strStrm;
                    strStrm << "\n#version 450" << std::endl;

                    // TODO : Documentation
                    spirv_cross::CompilerGLSL compilerGlsl(shaderModuleCreateInfo->pCode, shaderModuleCreateInfo->codeSize / sizeof(uint32_t));
                    const auto& outputs = compilerGlsl.get_shader_resources().stage_outputs;
                    for (uint32_t output_i = 0; output_i < outputs.size(); ++output_i) {
                        const auto& outupt = outputs[output_i];
                        auto location = compilerGlsl.get_decoration(outupt.id, spv::DecorationLocation);
                        const auto& type = compilerGlsl.get_type(outupt.type_id);

                        // TODO : Documentation
                        strStrm << "layout(location = " << location << ") out " << pipeline_explorer::get_spirv_type_str(type) << " outputValue" << output_i;
                        for (uint32_t array_i = 0; array_i < type.array.size(); ++array_i) {
                            strStrm << "[" << type.array[type.array.size() - array_i - 1] << "]";
                        }
                        strStrm << ";" << std::endl;
                    }

                    // TODO : Documentation
                    strStrm << std::endl;
                    strStrm << "void main()" << std::endl;
                    strStrm << "{" << std::endl;
                    for (uint32_t output_i = 0; output_i < outputs.size(); ++output_i) {
                        const auto& outupt = outputs[output_i];
                        const auto& type = compilerGlsl.get_type(outupt.type_id);
                        std::string outputName = "outputValue" + std::to_string(output_i);
                        if (!type.array.empty()) {
                            gvk::spirv::detail::CreateUnrolledOutputsGLSL(
                                pipeline_explorer::get_spirv_type_str(type),
                                pipeline_explorer::get_spirv_base_type_str(type),
                                type.vecsize,
                                outputName,
                                "",
                                color,
                                type.array.size(),
                                type.array.data(),
                                strStrm
                            );
                        } else {
                            strStrm << outputName << " = ";
                            strStrm << gvk::spirv::detail::ColorToOutputGLSL(
                                pipeline_explorer::get_spirv_type_str(type),
                                pipeline_explorer::get_spirv_base_type_str(type),
                                type.vecsize,
                                color
                            );
                            strStrm << ";" << std::endl;
                        }
                    }
                    strStrm << "}" << std::endl;

                    // TODO : Documentation
                    shaderSource[shaderModuleInfo->uuid] = strStrm.str();
                }
            }

            // TODO : Documentation
            (void)create_replacement_pipeline(device, pipeline, shaderSource, &pipelineInfo->highlightPipeline);
        }
    }
    return { };
}

std::vector<std::string> PipelineExplorer::enable_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, VkBool32 enabled, const float color[4])
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo && !path.empty()) {
        pipelineInfo->highlightEnabled = enabled;
        if (enabled && (!pipelineInfo->highlightPipeline || memcmp(pipelineInfo->highlightColor, color, sizeof(pipelineInfo->highlightColor)))) {
            memcpy(pipelineInfo->highlightColor, color, sizeof(pipelineInfo->highlightColor));
            (void)create_highlight_pipeline(device, pipeline, path, color);
        }
    }
    return { };
}

} // namespace gvk

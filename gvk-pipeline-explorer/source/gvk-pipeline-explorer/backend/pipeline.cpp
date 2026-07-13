
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

#include "gvk-pipeline-explorer/backend/pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/backend/utilities.hpp"
#include "gvk-spirv.hpp"

#include /* spirv_cross/ */ "spirv_glsl.hpp"
#include /* spirv_cross/ */ "spirv_parser.hpp"

namespace gvk {

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

template <typename CreateInfoType>
static VkResult create_pipeline_info(VkDevice device, VkPipeline pipeline, const CreateInfoType& createInfo, const std::filesystem::path& workspacePath, pipeline_explorer::PipelineInfo* pPipelineInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk::Device gvkDevice = device;
        gvk_result_assert(gvkDevice);
        gvk_result_assert(pipeline);
        gvk_result_assert(createInfo.sType == gvk::get_stype<CreateInfoType>());
        gvk_result_assert(pPipelineInfo);

        // Pepare PipelineInfo
        pipeline_explorer::PipelineInfo pipelineInfo(gvk::newref, { device, pipeline });
        pipelineInfo->deviceInfo = device;
        pipelineInfo->vkHandle = pipeline;
        pipelineInfo->name = "VkPipeline " + gvk::string::remove(gvk::to_string(pipelineInfo->vkHandle), "\"");

        // Process compute pipeline specific info
        if constexpr (std::is_same_v<CreateInfoType, VkComputePipelineCreateInfo>) {
            pipelineInfo->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            pipelineInfo->computePipelineCreateInfo = createInfo;
            pipelineInfo->uuid = pipeline_explorer::get_uuid(device, pipelineInfo->computePipelineCreateInfo);
            pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, createInfo.stage.module });
            gvk_result_assert(shaderModuleInfo);
            pipelineInfo->shaderModuleInfos.push_back({ createInfo.stage.stage, shaderModuleInfo });

        // Process graphics pipeline specific info
        } else if constexpr (std::is_same_v<CreateInfoType, VkGraphicsPipelineCreateInfo>) {
            pipelineInfo->bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            pipelineInfo->graphicsPipelineCreateInfo = createInfo;
            pipelineInfo->uuid = pipeline_explorer::get_uuid(device, pipelineInfo->graphicsPipelineCreateInfo);
            pipelineInfo->renderPassInfo = pipeline_explorer::RenderPassInfo({ device, createInfo.renderPass });

            // Get ShaderModuleInfos
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, createInfo.pStages[stage_i].module });
                gvk_result_assert(shaderModuleInfo);
                pipelineInfo->shaderModuleInfos.push_back({ createInfo.pStages[stage_i].stage, shaderModuleInfo });
            }

        // Process ray tracing pipeline specific info
        } else if  constexpr (std::is_same_v<CreateInfoType, VkRayTracingPipelineCreateInfoKHR>) {
            pipelineInfo->bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
            pipelineInfo->rayTracingPipelineCreateInfo = createInfo;
            pipelineInfo->uuid = pipeline_explorer::get_uuid(device, pipelineInfo->rayTracingPipelineCreateInfo);

            // Get ShaderModuleInfos
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                pipeline_explorer::ShaderModuleInfo shaderModuleInfo({ device, createInfo.pStages[stage_i].module });
                gvk_result_assert(shaderModuleInfo);
                pipelineInfo->shaderModuleInfos.push_back({ createInfo.pStages[stage_i].stage, shaderModuleInfo });
            }

            // Get shader group handles
            // NOTE : Explicit conversion to VkPhysicalDevice shouldn't be necessary here
            //  since gvk::PhysicalDevice provides a VkPhysicalDevice conversion operator
            //  but GCC and Clang aren't identifying the conversion
            // TODO : Double check on latest versions of GCC and Clang
            pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo = (VkPhysicalDevice)gvkDevice.get<gvk::PhysicalDevice>();
            gvk_result_assert(physicalDeviceInfo);
            auto shaderGroupHandleSize = physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties->shaderGroupHandleSize;
            gvk_result_assert(shaderGroupHandleSize);
            pipelineInfo->shaderGroupHandles.resize(createInfo.groupCount);
            for (uint32_t group_i = 0; group_i < createInfo.groupCount; ++group_i) {
                pipelineInfo->shaderGroupHandles[group_i].resize(shaderGroupHandleSize);
                gvk_result(gvkDevice.GetRayTracingShaderGroupHandlesKHR(pipeline, group_i, 1, shaderGroupHandleSize, pipelineInfo->shaderGroupHandles[group_i].data()));
            }

        // Unsupported pipeline type
        } else {
            gvk_result(VK_ERROR_FEATURE_NOT_PRESENT);
        }

        // Get PipelineLayoutInfo
        pipelineInfo->pipelineLayoutInfo = pipeline_explorer::PipelineLayoutInfo({ device, createInfo.layout });

        // Get pipeline UUID
        gvk_result(set_pipeline_driver_uuid(pipelineInfo));

        // Get pipeline path
        pipelineInfo->path = gvk::pipeline_explorer::get_pipeline_path(workspacePath, pipelineInfo->uuid);

        // Set PipelineInfo
        *pPipelineInfo = pipelineInfo;
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::report_pipeline_creation(const pipeline_explorer::PipelineInfo& pipelineInfo)
{
    auto pipelineExplorerPipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
    boost::multiprecision::export_bits(pipelineInfo->uuid, pipelineExplorerPipelineInfo.uuid, 8);
    boost::multiprecision::export_bits(pipelineInfo->driverUUID, pipelineExplorerPipelineInfo.driverUUID, 8);
    pipelineExplorerPipelineInfo.pName = pipelineInfo->name.c_str();
    pipelineExplorerPipelineInfo.device = pipelineInfo->deviceInfo->vkHandle;
    pipelineExplorerPipelineInfo.pipeline = pipelineInfo->vkHandle;
    pipelineExplorerPipelineInfo.bindPoint = pipelineInfo->bindPoint;
    pipelineExplorerPipelineInfo.labelCount = 0; // TODO : Get labels from pipelineInfo
    pipelineExplorerPipelineInfo.pLabels = nullptr; // TODO : Get labels from pipelineInfo
    pipelineExplorerPipelineInfo.experimentEnabled = pipelineInfo->experimentEnabled;
    // TODO : pipelineResult.pipelineInfo.experimentUUID;
    pipelineExplorerPipelineInfo.highlightEnabled = pipelineInfo->highlightEnabled;
    memcpy(pipelineExplorerPipelineInfo.highlightColor, pipelineInfo->highlightColor, sizeof(pipelineInfo->highlightColor));
#ifdef GVK_PLATFORM_WINDOWS
    mIpcMessenger.write("GvkPipelineExplorerPipelineInfo", pipelineExplorerPipelineInfo);
#endif

    // TODO : Make this optional and asynchronous to avoid stalling pipeline creation on decompilation
    decompile_pipeline(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->vkHandle);
    write_pipeline_info(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->vkHandle, pipelineInfo->path);
}

void PipelineExplorer::report_pipeline_destruction(const pipeline_explorer::PipelineInfo& pipelineInfo)
{
    // NOTE : Here for completeness but currently not used
    // TODO : Notify frontend of pipeline destruction so that it can be used for sorting/display
    (void)pipelineInfo;
}

VkResult PipelineExplorer::get_pipeline_executable_properties(pipeline_explorer::PipelineInfo pipelineInfo)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Get device
        gvk::Device device = pipelineInfo->deviceInfo->vkHandle;
        gvk_result(device ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Workload may destroy shader modules after pipeline creation so recreate them
        std::vector<gvk::ShaderModule> replacementShaderModules(pipelineInfo->shaderModuleInfos.size());
        for (size_t shaderModule_i = 0; shaderModule_i < pipelineInfo->shaderModuleInfos.size(); ++shaderModule_i) {
            gvk_result(gvk::ShaderModule::create(device, &*pipelineInfo->shaderModuleInfos[shaderModule_i].second->shaderModuleCreateInfo, nullptr, &replacementShaderModules[shaderModule_i]));
        }

        // Recreate pipeline with CAPTURE STATISTICS and INTERNAL_REPRESENTATIONS bits
        gvk::Pipeline replacementPipeline = VK_NULL_HANDLE;
        switch (pipelineInfo->bindPoint) {
        case VK_PIPELINE_BIND_POINT_COMPUTE: {

            // Modify VkComputePipelineCreateInfo
            auto computePipelineCreateInfo = pipelineInfo->computePipelineCreateInfo;
            auto pPipelineCreateFlags2CreateInfo = gvk::get_pnext<VkPipelineCreateFlags2CreateInfo>(*computePipelineCreateInfo);
            if (pPipelineCreateFlags2CreateInfo) {
                const_cast<VkPipelineCreateFlags2CreateInfo*>(pPipelineCreateFlags2CreateInfo)->flags |= VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            } else {
                const_cast<VkComputePipelineCreateInfo&>(*computePipelineCreateInfo).flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            }

            // Set replacement shader
            gvk_result(replacementShaderModules.size() == 1 ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            const_cast<VkComputePipelineCreateInfo&>(*computePipelineCreateInfo).stage.module = replacementShaderModules.back();

            // Create and set replacement pipeline layout
            gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
            gvk_result(create_replacement_pipeline_layout(device, computePipelineCreateInfo->layout, &replacementPipelineLayout));
            const_cast<VkComputePipelineCreateInfo&>(*computePipelineCreateInfo).layout = replacementPipelineLayout;

            // TODO : Base pipeline

            // Create pipeline
            gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &*computePipelineCreateInfo, nullptr, &replacementPipeline));

        } break;
        case VK_PIPELINE_BIND_POINT_GRAPHICS: {

            // Modify VkGraphicsPipelineCreateInfo
            auto graphicsPipelineCreateInfo = pipelineInfo->graphicsPipelineCreateInfo;
            auto pPipelineCreateFlags2CreateInfo = gvk::get_pnext<VkPipelineCreateFlags2CreateInfo>(*graphicsPipelineCreateInfo);
            if (pPipelineCreateFlags2CreateInfo) {
                const_cast<VkPipelineCreateFlags2CreateInfo*>(pPipelineCreateFlags2CreateInfo)->flags |= VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            } else {
                const_cast<VkGraphicsPipelineCreateInfo&>(*graphicsPipelineCreateInfo).flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            }

            // Set replacement shaders
            gvk_result(replacementShaderModules.size() == graphicsPipelineCreateInfo->stageCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            for (uint32_t stage_i = 0; stage_i < graphicsPipelineCreateInfo->stageCount; ++stage_i) {
                const_cast<VkPipelineShaderStageCreateInfo&>(graphicsPipelineCreateInfo->pStages[stage_i]).module = replacementShaderModules[stage_i];
            }

            // Create and set replacement pipeline layout
            gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
            gvk_result(create_replacement_pipeline_layout(device, graphicsPipelineCreateInfo->layout, &replacementPipelineLayout));
            const_cast<VkGraphicsPipelineCreateInfo&>(*graphicsPipelineCreateInfo).layout = replacementPipelineLayout;

            // Create and set replacement render pass
            gvk::RenderPass replacementRenderPass = VK_NULL_HANDLE;
            if (graphicsPipelineCreateInfo->renderPass) {
                gvk_result(create_replacement_render_pass(device, graphicsPipelineCreateInfo->renderPass, &replacementRenderPass));
                const_cast<VkGraphicsPipelineCreateInfo&>(*graphicsPipelineCreateInfo).renderPass = replacementRenderPass;
            }

            // TODO : Base pipeline

            // Create pipeline
            gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, 1, &*graphicsPipelineCreateInfo, nullptr, &replacementPipeline));

        } break;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {

            // Modify VkRayTracingPipelineCreateInfoKHR
            auto rayTracingPipelineCreateInfo = pipelineInfo->rayTracingPipelineCreateInfo;
            auto pPipelineCreateFlags2CreateInfo = gvk::get_pnext<VkPipelineCreateFlags2CreateInfo>(*rayTracingPipelineCreateInfo);
            if (pPipelineCreateFlags2CreateInfo) {
                const_cast<VkPipelineCreateFlags2CreateInfo*>(pPipelineCreateFlags2CreateInfo)->flags |= VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            } else {
                const_cast<VkRayTracingPipelineCreateInfoKHR&>(*rayTracingPipelineCreateInfo).flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
            }

            // Set replacement shaders
            gvk_result(replacementShaderModules.size() == rayTracingPipelineCreateInfo->stageCount ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            for (uint32_t stage_i = 0; stage_i < rayTracingPipelineCreateInfo->stageCount; ++stage_i) {
                const_cast<VkPipelineShaderStageCreateInfo&>(rayTracingPipelineCreateInfo->pStages[stage_i]).module = replacementShaderModules[stage_i];
            }

            // TODO : Pipeline library

            // Create and set replacement pipeline layout
            gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
            gvk_result(create_replacement_pipeline_layout(device, rayTracingPipelineCreateInfo->layout, &replacementPipelineLayout));
            const_cast<VkRayTracingPipelineCreateInfoKHR&>(*rayTracingPipelineCreateInfo).layout = replacementPipelineLayout;

            // TODO : Base pipeline

            // Create pipeline
            gvk_result(gvk::Pipeline::create(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &*rayTracingPipelineCreateInfo, nullptr, &replacementPipeline));

        } break;
        default: {
            gvk_result(VK_ERROR_INITIALIZATION_FAILED);
        } break;
        }

        // Get VkPipelineExecutablePropertiesKHRs
        auto pipelineInfoKHR = gvk::get_default<VkPipelineInfoKHR>();
        pipelineInfoKHR.pipeline = replacementPipeline;
        uint32_t executableCount = 0;
        gvk_result(device.GetPipelineExecutablePropertiesKHR(&pipelineInfoKHR, &executableCount, nullptr));
        std::vector<VkPipelineExecutablePropertiesKHR> pipelineExecutableProperties(executableCount, gvk::get_default<VkPipelineExecutablePropertiesKHR>());
        gvk_result(device.GetPipelineExecutablePropertiesKHR(&pipelineInfoKHR, &executableCount, pipelineExecutableProperties.data()));

        // Process VkPipelineExecutablePropertiesKHRs
        pipelineInfo->executableInfos.resize(executableCount);
        for (uint32_t executable_i = 0; executable_i < executableCount; ++executable_i) {
            auto& executableInfo = pipelineInfo->executableInfos[executable_i];
            executableInfo.properties = pipelineExecutableProperties[executable_i];

            // Setup VkPipelineExecutableInfoKHR
            auto pipelineExecutableInfo = gvk::get_default<VkPipelineExecutableInfoKHR>();
            pipelineExecutableInfo.pipeline = replacementPipeline;
            pipelineExecutableInfo.executableIndex = executable_i;

            // Get VkPipelineExecutableStatisticKHRs
            uint32_t statisticsCount = 0;
            gvk_result(device.GetPipelineExecutableStatisticsKHR(&pipelineExecutableInfo, &statisticsCount, nullptr));
            std::vector<VkPipelineExecutableStatisticKHR> statistics(statisticsCount, gvk::get_default<VkPipelineExecutableStatisticKHR>());
            gvk_result(device.GetPipelineExecutableStatisticsKHR(&pipelineExecutableInfo, &statisticsCount, statistics.data()));
            executableInfo.statistics.reserve(statisticsCount);
            for (uint32_t statistic_i = 0; statistic_i < statisticsCount; ++statistic_i) {
                executableInfo.statistics.push_back(statistics[statistic_i]);
            }

            // Get VkPipelineExecutableInternalRepresentationKHRs
            uint32_t internalRepresentationCount = 0;
            gvk_result(device.GetPipelineExecutableInternalRepresentationsKHR(&pipelineExecutableInfo, &internalRepresentationCount, nullptr));
            std::vector<VkPipelineExecutableInternalRepresentationKHR> internalRepresentations(internalRepresentationCount, gvk::get_default<VkPipelineExecutableInternalRepresentationKHR>());
            gvk_result(device.GetPipelineExecutableInternalRepresentationsKHR(&pipelineExecutableInfo, &internalRepresentationCount, internalRepresentations.data()));
            executableInfo.internalRepresentations.reserve(internalRepresentationCount);
            for (uint32_t internalRepresentation_i = 0; internalRepresentation_i < internalRepresentationCount; ++internalRepresentation_i) {
                executableInfo.internalRepresentations.push_back(internalRepresentations[internalRepresentation_i]);
            }
        }

        // If unavailable, populate collection with single empty ExecutableInfo
        if (pipelineInfo->executableInfos.empty()) {
            pipelineInfo->executableInfos.push_back({ });
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    gvk_result_scope_begin(BasicPipelineExplorer::execute_vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines)) {
        switch (gvkResult) {
        case VK_SUCCESS:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR: {
            for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
                pipeline_explorer::PipelineInfo pipelineInfo;
                gvk_result(create_pipeline_info(device, pPipelines[pipeline_i], pCreateInfos[pipeline_i], workspacePath, &pipelineInfo));
                gvk_result_assert(pipelineInfos.insert({ { device, pPipelines[pipeline_i] }, pipelineInfo }).second);
                report_pipeline_creation(pipelineInfo);
            }
        } break;
        default: {
        } break;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    gvk_result_scope_begin(BasicPipelineExplorer::execute_vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines)) {
        switch (gvkResult) {
        case VK_SUCCESS:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR: {
            for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
                pipeline_explorer::PipelineInfo pipelineInfo;
                gvk_result(create_pipeline_info(device, pPipelines[pipeline_i], pCreateInfos[pipeline_i], workspacePath, &pipelineInfo));
                gvk_result_assert(pipelineInfos.insert({ { device, pPipelines[pipeline_i] }, pipelineInfo }).second);
                report_pipeline_creation(pipelineInfo);
            }
        } break;
        default: {
        } break;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::execute_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    gvk_result_scope_begin(BasicPipelineExplorer::execute_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines)) {
        switch (gvkResult) {
        case VK_SUCCESS:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR: {
            for (uint32_t pipeline_i = 0; pipeline_i < createInfoCount; ++pipeline_i) {
                pipeline_explorer::PipelineInfo pipelineInfo;
                gvk_result(create_pipeline_info(device, pPipelines[pipeline_i], pCreateInfos[pipeline_i], workspacePath, &pipelineInfo));
                gvk_result_assert(pipelineInfos.insert({ { device, pPipelines[pipeline_i] }, pipelineInfo }).second);
                report_pipeline_creation(pipelineInfo);
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
    report_pipeline_destruction(pipeline_explorer::PipelineInfo({ device, pipeline }));
    pipelineInfos.erase({ device, pipeline });
    BasicPipelineExplorer::execute_vkDestroyPipeline(device, pipeline, pAllocator);
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

        // Get pipeline executable properties
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

                // Setup ShaderInfo
                auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
                shaderInfo.version = gvk::spirv::Version::SPIRV_1_4; // TODO : Set programmatically
                shaderInfo.bytecode.resize(shaderModuleCreateInfo->codeSize / sizeof(uint32_t));
                memcpy(shaderInfo.bytecode.data(), shaderModuleCreateInfo->pCode, shaderModuleCreateInfo->codeSize);

                // Get GLSL
                if (shaderModuleInfo->glsl.empty()) {
                    shaderInfo.language = gvk::spirv::ShadingLanguage::Glsl;
                    if (mSpirvContext.decompile(&shaderInfo) == VK_SUCCESS) {
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

                // Get SPIR-V
                if (shaderModuleInfo->spirv.empty()) {
                    shaderInfo.language = gvk::spirv::ShadingLanguage::SpirV;
                    if (mSpirvContext.decompile(&shaderInfo) == VK_SUCCESS) {
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
        std::error_code errorCode;
        std::filesystem::create_directories(path, errorCode);
        if (!errorCode) {

            // Write pipeline create info to workspace
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

            // Write VkPipelineLayoutCreateInfo to workspace
            if (pipelineInfo->pipelineLayoutInfo) {
                std::ofstream pipelineLayoutCreateInfoJson(path / "VkPipelineLayoutCreateInfo.json");
                pipelineLayoutCreateInfoJson << gvk::to_string(*pipelineInfo->pipelineLayoutInfo->pipelineLayoutCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;

                // Write VkDescriptorSetLayoutCreateInfo to workspace
                for (uint32_t setLayout_i = 0; setLayout_i < pipelineInfo->pipelineLayoutInfo->descriptorSetLayoutInfos.size(); ++setLayout_i) {
                    const auto& descriptorSetLayoutInfo = pipelineInfo->pipelineLayoutInfo->descriptorSetLayoutInfos[setLayout_i];
                    std::ofstream descriptorSetLayoutCreateInfoJson(path / ("VkDescriptorSetLayoutCreateInfo[" + std::to_string(setLayout_i) + "].json"));
                    descriptorSetLayoutCreateInfoJson << gvk::to_string(*descriptorSetLayoutInfo->descriptorSetLayoutCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;

                    // Write VkSamplerCreateInfo to workspace
                    for (uint32_t immutableSampler_i = 0; immutableSampler_i < descriptorSetLayoutInfo->immutableSamplerInfos.size(); ++immutableSampler_i) {
                        const auto& immutableSamplerInfo = descriptorSetLayoutInfo->immutableSamplerInfos[immutableSampler_i];
                        std::ofstream immutableSamplerInfoJson(path / ("VkSamplerCreateInfo[" + std::to_string(setLayout_i) + "][" + std::to_string(immutableSampler_i) + "].json"));
                        immutableSamplerInfoJson << gvk::to_string(*immutableSamplerInfo->samplerCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
                    }
                }
            }

            // Write VkRenderPassCreateInfo to workspace
            if (pipelineInfo->renderPassInfo) {
                if (pipelineInfo->renderPassInfo->renderPassCreateInfo->sType == gvk::get_stype<VkRenderPassCreateInfo>()) {
                    std::ofstream renderPassCreateInfoJson(path / "VkRenderPassCreateInfo.json");
                    renderPassCreateInfoJson << gvk::to_string(*pipelineInfo->renderPassInfo->renderPassCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
                } else if (pipelineInfo->renderPassInfo->renderPassCreateInfo2->sType == gvk::get_stype<VkRenderPassCreateInfo2>()) {
                    std::ofstream renderPassCreateInfo2Json(path / "VkRenderPassCreateInfo2.json");
                    renderPassCreateInfo2Json << gvk::to_string(*pipelineInfo->renderPassInfo->renderPassCreateInfo2, pipeline_explorer::PrinterFlags) << std::endl;
                }
            }

            // Write internal representations to workspace
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
                        std::vector<std::array<std::string, 3>> internalRepresentationInfos{
                            { "Subgroup Size", std::to_string(executableInfo.properties->subgroupSize), "The subgroup size with which this pipeline executable is dispatched." }
                        };
                        size_t nameColumnWidth = internalRepresentationInfos[0][0].length();
                        size_t valueColumnWidth = internalRepresentationInfos[0][1].length();
                        for (const auto& statistic : executableInfo.statistics) {
                            internalRepresentationInfos.push_back({ });
                            internalRepresentationInfos.back()[0] = statistic->name;
                            internalRepresentationInfos.back()[2] = statistic->description;
                            switch (statistic->format) {
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR: {
                                internalRepresentationInfos.back()[1] = std::to_string(statistic->value.b32);
                            } break;
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR: {
                                internalRepresentationInfos.back()[1] = std::to_string(statistic->value.i64);
                            } break;
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR: {
                                internalRepresentationInfos.back()[1] = std::to_string(statistic->value.u64);
                            } break;
                            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR: {
                                internalRepresentationInfos.back()[1] = std::to_string(statistic->value.f64);
                            } break;
                            default: {
                                internalRepresentationFile << "Unserviced VkPipelineExecutableStatisticFormatKHR; gvk maintenance required" << std::endl;
                            } break;
                            }
                            nameColumnWidth = std::max(nameColumnWidth, internalRepresentationInfos.back()[0].length());
                            valueColumnWidth = std::max(valueColumnWidth, internalRepresentationInfos.back()[1].length());
                        }
                        for (const auto& internalRepresentationInfo : internalRepresentationInfos) {
                            internalRepresentationFile << std::left << std::setw(nameColumnWidth) << internalRepresentationInfo[0] << " : ";
                            internalRepresentationFile << std::right << std::setw(valueColumnWidth) << internalRepresentationInfo[1] << " : ";
                            internalRepresentationFile << std::left << internalRepresentationInfo[2] << std::endl;
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

            // Process shader module infos
            for (auto shaderModuleInfoItr : pipelineInfo->shaderModuleInfos) {
                auto stage = shaderModuleInfoItr.first;
                auto shaderModuleInfo = shaderModuleInfoItr.second;
                auto shaderFileName = "VkShaderModule-UUID-" + pipeline_explorer::uuid_to_string(shaderModuleInfo->uuid, 9);

                #if 0
                // Report VkShaderModuleCreateInfo
                std::ofstream shaderModuleCreateInfoJson(path / (shaderFileName + ".VkShaderModuleCreateInfo.json"));
                shaderModuleCreateInfoJson << gvk::to_string(*shaderModuleInfo->shaderModuleCreateInfo, pipeline_explorer::PrinterFlags) << std::endl;
                #endif

                // Write GLSL to workspace
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

                // Write SPIR-V to workspace
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
                            // Some VKRT workloads do use multiple copies of the same shader in a single
                            //  pipeline, possibly due to codegen
                            // TODO : Maybe a good idea to warn?
                            // TODO : Need to be able to differentiate anyway so that experiments can be
                            //  applied to only one at a time
                            messages.push_back("WARNING : Duplicate shader UUID in pipeline");
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

void PipelineExplorer::get_unmodified_pipeline_info(VkDevice device, VkPipeline pipeline, std::unordered_map<pipeline_explorer::UUID, std::string>* pShaderSource)
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    if (pipelineInfo && pShaderSource) {
        for (uint32_t stage_i = 0; stage_i < pipelineInfo->shaderModuleInfos.size(); ++stage_i) {
            const auto& shaderModuleInfo = pipelineInfo->shaderModuleInfos[stage_i].second;
            if (!shaderModuleInfo->glsl.empty()) {
                if (!pShaderSource->insert({ shaderModuleInfo->uuid, shaderModuleInfo->glsl }).second) {
                    #if 0
                    // Some VKRT workloads do use multiple copies of the same shader in a single
                    //  pipeline, possibly due to codegen
                    // TODO : Maybe a good idea to warn?
                    // TODO : Need to be able to differentiate anyway so that experiments can be
                    //  applied to only one at a time
                    messages.push_back("WARNING : Duplicate shader UUID in pipeline");
                    #endif
                }
            } else {
                messages.push_back("ERROR : No shader source.  Was it successfully decompiled first?");
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

        // Prepare replacement shader module collection
        std::vector<gvk::ShaderModule> replacementShaderModules(pipelineInfo->shaderModuleInfos.size());
        std::vector<VkPipelineShaderStageCreateInfo> replacementPipelineShaderStageCreateInfos(pipelineInfo->shaderModuleInfos.size());
        for (uint32_t shaderModule_i = 0; shaderModule_i < pipelineInfo->shaderModuleInfos.size(); ++shaderModule_i) {
            auto stage = pipelineInfo->shaderModuleInfos[shaderModule_i].first;
            const auto& shaderModuleInfo = pipelineInfo->shaderModuleInfos[shaderModule_i].second;

            // Create replacement shader module
            auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
            shaderInfo.version = gvk::spirv::Version::SPIRV_1_6; // TODO : Set programmatically
            shaderInfo.stage = stage;
            const auto& shaderSourceItr = shaderSource.find(shaderModuleInfo->uuid);
            shaderInfo.source = shaderSourceItr != shaderSource.end() ? shaderSourceItr->second : std::string();
            if (!shaderInfo.source.empty() && !gvk::string::is_whitespace(shaderInfo.source)) {
                gvkResult = mSpirvContext.compile(&shaderInfo);
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

        // TODO : Messages shouldn't be used to indicate failure
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

                // Create replacement pipeline layout
                gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
                gvk_result(create_replacement_pipeline_layout(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->computePipelineCreateInfo->layout, &replacementPipelineLayout));
                computePipelineCreateInfo.layout = replacementPipelineLayout;

                // Create replacement pipeline
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

                // Create replacement pipeline layout
                gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
                gvk_result(create_replacement_pipeline_layout(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->graphicsPipelineCreateInfo->layout, &replacementPipelineLayout));
                graphicsPipelineCreateInfo.layout = replacementPipelineLayout;

                // Create replacement render pass
                gvk::RenderPass replacementRenderPass = VK_NULL_HANDLE;
                if (pipelineInfo->graphicsPipelineCreateInfo->renderPass) {
                    gvk_result(create_replacement_render_pass(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->graphicsPipelineCreateInfo->renderPass, &replacementRenderPass));
                    graphicsPipelineCreateInfo.renderPass = replacementRenderPass;
                }

                // TODO : Base pipeline

                // Create replacement pipeline
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

                // Create replacement pipeline layout
                gvk::PipelineLayout replacementPipelineLayout = VK_NULL_HANDLE;
                gvk_result(create_replacement_pipeline_layout(pipelineInfo->deviceInfo->vkHandle, pipelineInfo->rayTracingPipelineCreateInfo->layout, &replacementPipelineLayout));
                rayTracingPipelineCreateInfo.layout = replacementPipelineLayout;

                // TODO : Base pipeline

                // Create replacement pipeline
                gvk_result(gvk::Pipeline::create(pipelineInfo->deviceInfo->vkHandle, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, pPipeline));
            } break;
            default: {
            } break;
            }
            auto message = "VkPipeline " + uuid_to_string(pipelineInfo->uuid, 18) + " successfully compiled";
            mIpcMessenger.write(message.c_str());
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::create_replacement_shader_binding_table(const gvk::Device& gvkDevice, pipeline_explorer::QueueInfo queueInfo, VkCommandBuffer vkCommandBuffer, pipeline_explorer::PipelineInfo pipelineInfo, const gvk::ShaderGroupHandleMap& shaderGroupHandleMap, VkStridedDeviceAddressRegionKHR* pShaderBindingTable)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (pShaderBindingTable && pShaderBindingTable->deviceAddress && pShaderBindingTable->stride && pShaderBindingTable->size) {
            gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(queueInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(pipelineInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
            gvk_result(shaderGroupHandleMap ? VK_SUCCESS : VK_ERROR_UNKNOWN);

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
                #if 0
                bufferMemoryBarrier.buffer = pipelineInfo->experimentShaderGroupHandleMap.gvkBuffer;
                #else
                bufferMemoryBarrier.buffer = shaderGroupHandleMap.gvkBuffer;
                #endif
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

            // Execute shader group map, this will run through the replacement buffer and
            //  replace the application's shader group handles with the replacement handles
            {
                auto gpuAddressMapInfo = gvk::get_default<GpuAddressMapInfo>();
                gpuAddressMapInfo.dst = replacementShaderBindingTableDeviceAddress;
                gpuAddressMapInfo.src = pShaderBindingTable->deviceAddress;
                gpuAddressMapInfo.stride = pShaderBindingTable->stride;
                gpuAddressMapInfo.count = pShaderBindingTable->size / pShaderBindingTable->stride;
                #if 0
                gpuAddressMapInfo.keys = pipelineInfo->experimentShaderGroupHandleMap.keys;
                gpuAddressMapInfo.values = pipelineInfo->experimentShaderGroupHandleMap.values;
                gpuAddressMapInfo.kvpCount = pipelineInfo->experimentShaderGroupHandleMap.kvpCount;
                #else
                gpuAddressMapInfo.keys = shaderGroupHandleMap.keys;
                gpuAddressMapInfo.values = shaderGroupHandleMap.values;
                gpuAddressMapInfo.kvpCount = shaderGroupHandleMap.kvpCount;
                #endif
                gvk::Pipeline gpuAddressMapPipeline;
                gvk_result(queueInfo->shaderBindingTableReplacementResources.get_gpu_address_map_pipeline(gvkDevice, &gpuAddressMapPipeline));
                gvkDevice.get<DispatchTable>().gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpuAddressMapPipeline);
                gvkDevice.get<DispatchTable>().gvkCmdPushConstants(vkCommandBuffer, gpuAddressMapPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GpuAddressMapInfo), &gpuAddressMapInfo);
                // TODO : Query GPU to create shader with ideal workgroup size
                static const uint32_t local_size_x = 16;
                auto workgroupSize = (uint32_t)((gpuAddressMapInfo.count + local_size_x - 1) / (uint64_t)local_size_x);
                gvkDevice.get<DispatchTable>().gvkCmdDispatch(vkCommandBuffer, workgroupSize, 1, 1);
            }

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

            // Point the shader binding table address at the replacement
            pShaderBindingTable->deviceAddress = replacementShaderBindingTableDeviceAddress;
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::create_replacement_shader_binding_tables(pipeline_explorer::QueueInfo queueInfo, pipeline_explorer::PipelineInfo pipelineInfo, const gvk::ShaderGroupHandleMap& shaderGroupHandleMap, GvkCommandStructureCmdTraceRaysKHR* pCmd)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result_assert(queueInfo);
        gvk_result_assert(pipelineInfo);
        gvk_result_assert(pCmd);
        gvk::Device gvkDevice = queueInfo->deviceInfo->vkHandle;
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, shaderGroupHandleMap, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pRaygenShaderBindingTable)));
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, shaderGroupHandleMap, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pMissShaderBindingTable)));
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, shaderGroupHandleMap, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pHitShaderBindingTable)));
        gvk_result(create_replacement_shader_binding_table(gvkDevice, queueInfo, pCmd->commandBuffer, pipelineInfo, shaderGroupHandleMap, const_cast<VkStridedDeviceAddressRegionKHR*>(pCmd->pCallableShaderBindingTable)));
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
        if (path.empty() || !std::filesystem::exists(path)) {
            decompile_pipeline(device, pipeline);
            write_pipeline_info(device, pipeline, path);
        }
        read_pipeline_info(device, pipeline, path, &shaderSource);
        if (shaderSource.size() == pipelineInfo->shaderModuleInfos.size()) {
            (void)create_replacement_pipeline(device, pipeline, shaderSource, &pipelineInfo->experimentPipeline);
            if (pipelineInfo->experimentPipeline && pipelineInfo->bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                auto shaderGroupHandleMapCreateInfo = gvk::get_default<gvk::ShaderGroupHandleMapCreateInfo>();
                shaderGroupHandleMapCreateInfo.keysPipeline = pipelineInfo->vkHandle;
                shaderGroupHandleMapCreateInfo.valuesPipeline = pipelineInfo->experimentPipeline;
                shaderGroupHandleMapCreateInfo.shaderGroupHandleCount = pipelineInfo->rayTracingPipelineCreateInfo->groupCount;
                auto gvkResult = gvk::create_shader_group_handle_map(gvkDevice, &shaderGroupHandleMapCreateInfo, &pipelineInfo->experimentShaderGroupHandleMap);
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
    if (pipelineInfo && !path.empty()) {
        switch (pipelineInfo->bindPoint) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS: {
            return create_graphics_highlight_pipeline(device, pipeline, path, color);
        } break;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
            auto vkResult = create_ray_tracing_highlight_pipeline(device, pipeline, path, color);
            if (pipelineInfo->highlightPipeline) {
                auto shaderGroupHandleMapCreateInfo = gvk::get_default<gvk::ShaderGroupHandleMapCreateInfo>();
                shaderGroupHandleMapCreateInfo.keysPipeline = pipelineInfo->vkHandle;
                shaderGroupHandleMapCreateInfo.valuesPipeline = pipelineInfo->highlightPipeline;
                shaderGroupHandleMapCreateInfo.shaderGroupHandleCount = pipelineInfo->rayTracingPipelineCreateInfo->groupCount;
                vkResult = gvk::create_shader_group_handle_map(pipelineInfo->deviceInfo->vkHandle, &shaderGroupHandleMapCreateInfo, &pipelineInfo->highlightShaderGroupHandleMap);
            }
            return vkResult;
        } break;
        default: {
        } break;
        }
    }
    return VK_SUCCESS;
}

VkResult PipelineExplorer::create_graphics_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4])
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    decompile_pipeline(device, pipeline);
    write_pipeline_info(device, pipeline, path);
    std::unordered_map<pipeline_explorer::UUID, std::string> shaderSource;
    get_unmodified_pipeline_info(device, pipeline, &shaderSource);
    if (messages.empty()) {

        // Loop over shaders looking for fragment shader
        std::string glsl;
        for (uint32_t shaderModule_i = 0; shaderModule_i < pipelineInfo->shaderModuleInfos.size() && glsl.empty(); ++shaderModule_i) {
            const auto& shaderModuleInfoItr = pipelineInfo->shaderModuleInfos[shaderModule_i];
            if (shaderModuleInfoItr.first == VK_SHADER_STAGE_FRAGMENT_BIT) {
                const auto& shaderModuleInfo = shaderModuleInfoItr.second;
                const auto& shaderModuleCreateInfo = shaderModuleInfo->shaderModuleCreateInfo;

                // Prepare to generate highlight shader GLSL
                std::stringstream strStrm;
                strStrm << "\n#version 450" << std::endl;

                // Reflect the original shader's stage outputs
                spirv_cross::CompilerGLSL compilerGlsl(shaderModuleCreateInfo->pCode, shaderModuleCreateInfo->codeSize / sizeof(uint32_t));
                const auto& outputs = compilerGlsl.get_shader_resources().stage_outputs;
                for (uint32_t output_i = 0; output_i < outputs.size(); ++output_i) {
                    const auto& output = outputs[output_i];
                    auto location = compilerGlsl.get_decoration(output.id, spv::DecorationLocation);
                    const auto& type = compilerGlsl.get_type(output.type_id);

                    // Declare an output for each of the original shader's outputs
                    strStrm << "layout(location = " << location << ") out " << pipeline_explorer::get_spirv_type_str(type) << " outputValue" << output_i;
                    for (uint32_t array_i = 0; array_i < type.array.size(); ++array_i) {
                        strStrm << "[" << type.array[type.array.size() - array_i - 1] << "]";
                    }
                    strStrm << ";" << std::endl;
                }

                // Generate main() that simply outputs the highlight color to each output
                strStrm << std::endl;
                strStrm << "void main()" << std::endl;
                strStrm << "{" << std::endl;
                for (uint32_t output_i = 0; output_i < outputs.size(); ++output_i) {
                    const auto& output = outputs[output_i];
                    const auto& type = compilerGlsl.get_type(output.type_id);
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

                // Set modified shader source
                shaderSource[shaderModuleInfo->uuid] = strStrm.str();
            }
        }

        // Create replacement pipeline with modified shader source
        return create_replacement_pipeline(device, pipeline, shaderSource, &pipelineInfo->highlightPipeline);
    }
    return VK_SUCCESS;
}

VkResult PipelineExplorer::create_ray_tracing_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4])
{
    pipeline_explorer::PipelineInfo pipelineInfo({ device, pipeline });
    decompile_pipeline(device, pipeline);
    write_pipeline_info(device, pipeline, path);
    std::unordered_map<pipeline_explorer::UUID, std::string> shaderSource;
    get_unmodified_pipeline_info(device, pipeline, &shaderSource);
    if (messages.empty()) {

        auto findMatchingParenthesis = [](const std::string& str) -> size_t
        {
            if (str[0] == '(') {
                size_t depth = 1;
                for (size_t i = 1; i < str.size(); ++i) {
                    if (str[i] == '(') {
                        ++depth;
                    } else if (str[i] == ')') {
                        --depth;
                        if (!depth) {
                            return i;
                        }
                    }
                }
            }
            return std::string::npos;
        };

        // Process shaders
        for (auto& itr : shaderSource) {

            // Search for "imageStore"
            std::vector<std::string> snippets;
            for (const auto& snippet : gvk::string::split(itr.second, "imageStore")) {
                snippets.push_back(snippet);
            }

            // Wherever "imageStore" is used, add the highlight color
            if (1 < snippets.size()) {
                std::string source = snippets[0];
                for (size_t i = 1; i < snippets.size(); ++i) {
                    auto closingParenthesis = findMatchingParenthesis(snippets[i]);
                    if (closingParenthesis != std::string::npos) {
                        std::stringstream strStrm;
                        strStrm << " + vec4(" << color[0] << ", " << color[1] << ", " << color[2] << ", " << color[3] << ")";
                        snippets[i].insert(closingParenthesis, strStrm.str());
                    }
                    snippets[i].insert(0, "imageStore");
                    source += snippets[i];
                }
                itr.second = source;
            }
        }

        // Create replacement pipeline with modified shader source
        return create_replacement_pipeline(device, pipeline, shaderSource, &pipelineInfo->highlightPipeline);
    }
    return VK_SUCCESS;
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

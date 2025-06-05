
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

namespace gvk {

struct DeviceExtensionInfo
{
    VkBool32 pipelineStatisticsQuery_enabled{ };
    VkBool32 VK_EXT_pipeline_creation_cache_control_enabled{ };
    VkBool32 VK_EXT_shader_module_identifier_enabled{ };
    VkBool32 VK_KHR_performance_query_enabled{ };
    VkBool32 VK_KHR_pipeline_binary_enabled{ };
    VkBool32 VK_KHR_pipeline_executable_properties_enabled{ };
    VkBool32 VK_KHR_pipeline_properties_enabled{ };
};

#if 1
template <typename PhysicalDeviceFeatures>
inline PhysicalDeviceFeatures get_available_physical_device_features(const gvk::PhysicalDevice& gvkPhysicalDevice)
{
    assert(gvkPhysicalDevice);
    auto physicalDeviceFeatures = gvk::get_default<PhysicalDeviceFeatures>();
    auto physicalDeviceFeatures2 = gvk::get_default<VkPhysicalDeviceFeatures2>();
    physicalDeviceFeatures2.pNext = &physicalDeviceFeatures;
    gvkPhysicalDevice.GetPhysicalDeviceFeatures2(&physicalDeviceFeatures2);
    return physicalDeviceFeatures;
}
#endif

thread_local DeviceExtensionInfo tlDeviceExtensionInfo;
VkResult PipelineExplorer::execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // TODO : Documentation
        tlDeviceExtensionInfo = { };
        auto deviceCreateInfo = *pCreateInfo;
        pipeline_explorer::ExtensionCollection extensions;
        extensions.add(pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);
        pipeline_explorer::PNextChainEditor pNextChainEditor((VkBaseInStructure&)*pCreateInfo);
        auto pMutableCreateInfo = const_cast<VkDeviceCreateInfo*>(pCreateInfo);

        // TODO : Documentation
        gvk::PhysicalDevice gvkPhysicalDevice(physicalDevice);
        gvk_result(gvkPhysicalDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto availablePhysicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures>();
        gvkPhysicalDevice.GetPhysicalDeviceFeatures(&availablePhysicalDeviceFeatures);

        // TODO : Documentation
        if (availablePhysicalDeviceFeatures.pipelineStatisticsQuery) {
            tlDeviceExtensionInfo.pipelineStatisticsQuery_enabled = VK_TRUE;
            auto pApplicationPhysicalDeviceFeatures2 = pNextChainEditor.get<VkPhysicalDeviceFeatures2>();
            if (pApplicationPhysicalDeviceFeatures2) {
                pApplicationPhysicalDeviceFeatures2->features.pipelineStatisticsQuery = VK_TRUE;
            } else {
                auto enabledPhysicalDeviceFeatures2 = gvk::get_default<VkPhysicalDeviceFeatures2>();
                enabledPhysicalDeviceFeatures2.features = pCreateInfo->pEnabledFeatures ? *pCreateInfo->pEnabledFeatures : gvk::get_default<VkPhysicalDeviceFeatures>();
                enabledPhysicalDeviceFeatures2.features.pipelineStatisticsQuery = VK_TRUE;
                pNextChainEditor.set(enabledPhysicalDeviceFeatures2);
                pMutableCreateInfo->pEnabledFeatures = nullptr;
            }
        }

        // TODO : Documentation
        auto availablePipelineCreationCacheControlFeatures = get_available_physical_device_features<VkPhysicalDevicePipelineCreationCacheControlFeatures>(gvkPhysicalDevice);
        if (availablePipelineCreationCacheControlFeatures.pipelineCreationCacheControl) {
            tlDeviceExtensionInfo.VK_EXT_pipeline_creation_cache_control_enabled = VK_TRUE;
            pNextChainEditor.set(availablePipelineCreationCacheControlFeatures);
            extensions.add(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
        }

        // TODO : Documentation
        auto availablePhysicalDeviceShaderModuleIdentifierFeatures = get_available_physical_device_features<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>(gvkPhysicalDevice);
        if (availablePhysicalDeviceShaderModuleIdentifierFeatures.shaderModuleIdentifier) {
            tlDeviceExtensionInfo.VK_EXT_shader_module_identifier_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDeviceShaderModuleIdentifierFeatures);
            extensions.add(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
        }

        // TODO : Documentation
        auto availablePhysicalDevicePerformanceQueryFeatures = get_available_physical_device_features<VkPhysicalDevicePerformanceQueryFeaturesKHR >(gvkPhysicalDevice);
        if (availablePhysicalDevicePerformanceQueryFeatures.performanceCounterQueryPools) {
            tlDeviceExtensionInfo.VK_KHR_performance_query_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePerformanceQueryFeatures);
            extensions.add(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        }

        // TODO : Documentation
        auto availablePhysicalDevicePipelineBinaryFeatures = get_available_physical_device_features<VkPhysicalDevicePipelineBinaryFeaturesKHR>(gvkPhysicalDevice);
        if (availablePhysicalDevicePipelineBinaryFeatures.pipelineBinaries) {
            tlDeviceExtensionInfo.VK_KHR_pipeline_binary_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePipelineBinaryFeatures);
            extensions.add(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
        }

        // TODO : Documentation
        auto availablePhysicalDevicePipelineExecutablePropertiesFeatures = get_available_physical_device_features<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>(gvkPhysicalDevice);
        if (availablePhysicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo) {
            tlDeviceExtensionInfo.VK_KHR_pipeline_executable_properties_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePipelineExecutablePropertiesFeatures);
            extensions.add(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
        }

        // TODO : Documentation
        auto availablePhysicalDevicePipelinePropertiesFeatures = get_available_physical_device_features<VkPhysicalDevicePipelinePropertiesFeaturesEXT>(gvkPhysicalDevice);
        if (availablePhysicalDevicePipelinePropertiesFeatures.pipelinePropertiesIdentifier) {
            tlDeviceExtensionInfo.VK_KHR_pipeline_properties_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePipelinePropertiesFeatures);
            extensions.add(VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME);
        }

        // TODO : Documentation
        if (pNextChainEditor.get<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>()->rayTracingPipeline) {
            auto availablePhysicalDevice8BitStorageFeatures = get_available_physical_device_features<VkPhysicalDevice8BitStorageFeatures>(gvkPhysicalDevice);
            if (availablePhysicalDevice8BitStorageFeatures.storageBuffer8BitAccess) {
                pNextChainEditor.set(availablePhysicalDevice8BitStorageFeatures);
                extensions.add(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
            }
            auto availablePhysicalDeviceShaderFloat16Int8Features = get_available_physical_device_features<VkPhysicalDeviceShaderFloat16Int8Features>(gvkPhysicalDevice);
            if (availablePhysicalDeviceShaderFloat16Int8Features.shaderInt8) {
                pNextChainEditor.set(availablePhysicalDeviceShaderFloat16Int8Features);
            }
            auto pPhysicalDeviceFeatures2 = pNextChainEditor.get<VkPhysicalDeviceFeatures2>();
            if (pPhysicalDeviceFeatures2) {
                pPhysicalDeviceFeatures2->features.shaderInt64 = VK_TRUE;
            }
        }

        // Point pCreateInfo at custom pNext chain, layers, and extensions
        pMutableCreateInfo->pNext = pNextChainEditor.get();
        pMutableCreateInfo->enabledExtensionCount = extensions.count();
        pMutableCreateInfo->ppEnabledExtensionNames = extensions.data();

        // TODO : Documentation
        if (!workspacePath.empty() && std::filesystem::exists(workspacePath)) {
            std::ofstream deviceCreateInfoJson(workspacePath / ("VkDeviceCreateInfo.json"));
            deviceCreateInfoJson << gvk::to_string(*pCreateInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;

            auto physicalDeviceProperties = gvk::get_default<VkPhysicalDeviceProperties>();
            gvkPhysicalDevice.GetPhysicalDeviceProperties(&physicalDeviceProperties);
            std::ofstream physicalDevicePropertiesJson(workspacePath / ("VkPhysicalDeviceProperties.json"));
            physicalDevicePropertiesJson << gvk::to_string(physicalDeviceProperties, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // TODO : Documentation
        gvk_result(BasicApiCallHandler::execute_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice));

        if (!vkLayer) {

            // TODO : Documentation
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &dispatchTable, *pDevice, &gvkDevice));
            auto inserted = gvkDevices.insert(gvkDevice).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // TODO : Documentation
            pipeline_explorer::DeviceInfo deviceInfo(gvk::newref, *pDevice);
            deviceInfo->physicalDeviceInfo = physicalDevice;
            deviceInfo->vkHandle = *pDevice;
            deviceInfo->deviceCreateInfo = *pCreateInfo;
            deviceInfo->pipelineStatisticsQuery_enabled = tlDeviceExtensionInfo.pipelineStatisticsQuery_enabled;
            deviceInfo->VK_EXT_pipeline_creation_cache_control_enabled = tlDeviceExtensionInfo.VK_EXT_pipeline_creation_cache_control_enabled;
            deviceInfo->VK_EXT_shader_module_identifier_enabled = tlDeviceExtensionInfo.VK_EXT_shader_module_identifier_enabled;
            deviceInfo->VK_KHR_performance_query_enabled = tlDeviceExtensionInfo.VK_KHR_performance_query_enabled;
            deviceInfo->VK_KHR_pipeline_binary_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_binary_enabled;
            deviceInfo->VK_KHR_pipeline_executable_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_executable_properties_enabled;
            deviceInfo->VK_KHR_pipeline_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_properties_enabled;
            for (const auto& queueFamily : gvkDevice.get<gvk::QueueFamilies>()) {
                for (const auto& queue : queueFamily.queues) {
                    pipeline_explorer::QueueInfo queueInfo(gvk::newref, queue);
                    queueInfo->deviceInfo = deviceInfo;
                    queueInfo->vkHandle = queue;
                    queueInfo->deviceQueueCreateInfo = queue.get<VkDeviceQueueCreateInfo>();
                    deviceInfo->queueInfos[queueFamily.index].push_back(queueInfo);
                }
            }
            inserted = deviceInfos.insert({ *pDevice, deviceInfo }).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        }

        // TODO : Documentation
        *const_cast<VkDeviceCreateInfo*>(pCreateInfo) = deviceCreateInfo;
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    (void)pAllocator;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (vkLayer) {

            // TODO : Documentation
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &dispatchTable, *pDevice, &gvkDevice));
            auto inserted = gvkDevices.insert(gvkDevice).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // TODO : Documentation
            pipeline_explorer::DeviceInfo deviceInfo(gvk::newref, *pDevice);
            deviceInfo->physicalDeviceInfo = physicalDevice;
            deviceInfo->vkHandle = *pDevice;
            deviceInfo->deviceCreateInfo = *pCreateInfo;
            deviceInfo->pipelineStatisticsQuery_enabled = tlDeviceExtensionInfo.pipelineStatisticsQuery_enabled;
            deviceInfo->VK_EXT_pipeline_creation_cache_control_enabled = tlDeviceExtensionInfo.VK_EXT_pipeline_creation_cache_control_enabled;
            deviceInfo->VK_EXT_shader_module_identifier_enabled = tlDeviceExtensionInfo.VK_EXT_shader_module_identifier_enabled;
            deviceInfo->VK_KHR_performance_query_enabled = tlDeviceExtensionInfo.VK_KHR_performance_query_enabled;
            deviceInfo->VK_KHR_pipeline_binary_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_binary_enabled;
            deviceInfo->VK_KHR_pipeline_executable_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_executable_properties_enabled;
            deviceInfo->VK_KHR_pipeline_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_properties_enabled;
            for (const auto& queueFamily : gvkDevice.get<gvk::QueueFamilies>()) {
                for (const auto& queue : queueFamily.queues) {
                    pipeline_explorer::QueueInfo queueInfo(gvk::newref, queue);
                    queueInfo->deviceInfo = deviceInfo;
                    queueInfo->vkHandle = queue;
                    queueInfo->deviceQueueCreateInfo = queue.get<VkDeviceQueueCreateInfo>();
                    deviceInfo->queueInfos[queueFamily.index].push_back(queueInfo);
                    if (vkLayer) { // NOTE : This check is redundant, but when post call handlers are refactored this will need to be set correctly
                        *(void**)queueInfo->vkHandle = *(void**)*pDevice;
                    }
                }
            }
            inserted = deviceInfos.insert({ *pDevice, deviceInfo }).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // TODO : Documentation
            if (deviceInfo->pipelineStatisticsQuery_enabled) {
                auto pipelineExplorerMetricInfo = gvk::get_default<GvkPipelineExplorerMetricInfo>();
                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.type = GVK_PIPELINE_EXPLORER_METRIC_TYPE_COUNT;
                pipelineExplorerMetricInfo.pName = "Input Assembly Vertices";
                pipelineExplorerMetricInfo.pGroupName = "VkQueryPipelineStatistics";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Input Assembly Primitives";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Vertex Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Geometry Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Geometry Shader Primitives";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Clipping Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Clipping Primitives";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Fragment Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Tesselation Control Shader Patches";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Tesselation Control Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Copmute Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Task Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Mesh Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
            }

            #if 0
            std::ofstream counterFile(workspacePath / "counters.json");

            // TODO : Documentation
            if (deviceInfo->VK_KHR_performance_query_enabled) {
                gvk::PhysicalDevice gvkPhysicalDevice(physicalDevice);
                gvk_result(gvkPhysicalDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                for (const auto& queueFamily : deviceInfo->queueInfos) {

                    // TODO : Documentation
                    uint32_t counterCount = 0;
                    gvkPhysicalDevice.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(queueFamily.first, &counterCount, nullptr, nullptr);
                    std::vector<VkPerformanceCounterKHR> counters(counterCount, gvk::get_default<VkPerformanceCounterKHR>());
                    std::vector<VkPerformanceCounterDescriptionKHR> counterDescriptions(counterCount, gvk::get_default<VkPerformanceCounterDescriptionKHR>());
                    gvkPhysicalDevice.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(queueFamily.first, &counterCount, counters.data(), counterDescriptions.data());

                    counterFile << "================================================================================" << std::endl;
                    counterFile << "Queue Family " << queueFamily.first << std::endl;
                    for (uint32_t i = 0; i < counterCount; ++i) {
                        counterFile << "--------------------------------------------------------------------------------" << std::endl;
                        counterFile << gvk::to_string(counters[i], gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
                        counterFile << gvk::to_string(counterDescriptions[i], gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
                    }
                }
            }
            #endif
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    gvkDevices.erase(device);
    pipeline_explorer::DeviceInfo(device)->queueInfos.clear();
    deviceInfos.erase(device);
    BasicApiCallHandler::execute_vkDestroyDevice(device, pAllocator);
}

} // namespace gvk

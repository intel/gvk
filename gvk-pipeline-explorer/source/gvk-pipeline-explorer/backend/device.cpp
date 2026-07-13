
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

namespace gvk {

struct DeviceExtensionInfo
{
    VkBool32 pipelineStatisticsQuery_enabled{ };
    VkBool32 VK_EXT_pipeline_creation_cache_control_enabled{ };
    VkBool32 VK_EXT_shader_module_identifier_enabled{ };
    VkBool32 VK_KHR_calibrated_timestamps_enabled{ };
    VkBool32 VK_KHR_performance_query_enabled{ };
    VkBool32 VK_KHR_pipeline_binary_enabled{ };
    VkBool32 VK_KHR_pipeline_executable_properties_enabled{ };
    VkBool32 VK_KHR_pipeline_properties_enabled{ };
};

// NOTE : Duplicated in...
//  gvk/gvk-pipeline-explorer/source/gvk-pipeline-explorer/backend/device.cpp
//  gvk/gvk-state-tracker/tests/state-tracker-test-utilities.cpp
//  gvk/gvk-spirv/tests/spirv-validation-context.hpp
// TODO : Move to a common location
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

thread_local DeviceExtensionInfo tlDeviceExtensionInfo;
VkResult PipelineExplorer::execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Prepare to modify pCreateInfo; using a thread_local structure to track what
        //  extensions/features are enabled across execute and post_execute handlers
        tlDeviceExtensionInfo = { };
        auto deviceCreateInfo = *pCreateInfo;
        gvk::StringArrayIndexMap extensions;
        extensions.add(pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);
        pipeline_explorer::PNextChainEditor pNextChainEditor((VkBaseInStructure&)*pCreateInfo);
        auto pMutableCreateInfo = const_cast<VkDeviceCreateInfo*>(pCreateInfo);

        // Get physical device features
        gvk::PhysicalDevice gvkPhysicalDevice(physicalDevice);
        gvk_result(gvkPhysicalDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto availablePhysicalDeviceFeatures = gvk::get_default<VkPhysicalDeviceFeatures>();
        gvkPhysicalDevice.GetPhysicalDeviceFeatures(&availablePhysicalDeviceFeatures);

        // Get available extensions
        uint32_t extensionCount = 0;
        gvkPhysicalDevice.EnumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        gvkPhysicalDevice.EnumerateDeviceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
        std::set<std::string> availableExtensions;
        for (const auto& extensionProperty : extensionProperties) {
            availableExtensions.insert(extensionProperty.extensionName);
        }

        // Enable pipelineStatisticsQuery if available
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

        // Enable VK_EXT_pipeline_creation_cache_control if available
        auto availablePipelineCreationCacheControlFeatures = get_available_physical_device_features<VkPhysicalDevicePipelineCreationCacheControlFeatures>(gvkPhysicalDevice);
        if (availablePipelineCreationCacheControlFeatures.pipelineCreationCacheControl) {
            tlDeviceExtensionInfo.VK_EXT_pipeline_creation_cache_control_enabled = VK_TRUE;
            pNextChainEditor.set(availablePipelineCreationCacheControlFeatures);
            extensions.add(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
        }

        // Enable VK_EXT_shader_module_identifier if available
        auto availablePhysicalDeviceShaderModuleIdentifierFeatures = get_available_physical_device_features<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>(gvkPhysicalDevice);
        if (availablePhysicalDeviceShaderModuleIdentifierFeatures.shaderModuleIdentifier) {
            tlDeviceExtensionInfo.VK_EXT_shader_module_identifier_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDeviceShaderModuleIdentifierFeatures);
            extensions.add(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
        }

        // Enable VK_KHR_calibrated_timestamps if available
        if (availableExtensions.count(VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME)) {
            tlDeviceExtensionInfo.VK_KHR_calibrated_timestamps_enabled = VK_TRUE;
            extensions.add(VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
        }

        // Enable VK_KHR_performance_query if available
        auto availablePhysicalDevicePerformanceQueryFeatures = get_available_physical_device_features<VkPhysicalDevicePerformanceQueryFeaturesKHR>(gvkPhysicalDevice);
        if (availablePhysicalDevicePerformanceQueryFeatures.performanceCounterQueryPools) {
            tlDeviceExtensionInfo.VK_KHR_performance_query_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePerformanceQueryFeatures);
            extensions.add(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        }

        // Enable VK_KHR_pipeline_binary if available
        auto availablePhysicalDevicePipelineBinaryFeatures = get_available_physical_device_features<VkPhysicalDevicePipelineBinaryFeaturesKHR>(gvkPhysicalDevice);
        if (availablePhysicalDevicePipelineBinaryFeatures.pipelineBinaries) {
            tlDeviceExtensionInfo.VK_KHR_pipeline_binary_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePipelineBinaryFeatures);
            extensions.add(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
        }

        // Enable VK_KHR_pipeline_executable_properties if available
        auto availablePhysicalDevicePipelineExecutablePropertiesFeatures = get_available_physical_device_features<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>(gvkPhysicalDevice);
        if (availablePhysicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo) {
            tlDeviceExtensionInfo.VK_KHR_pipeline_executable_properties_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePipelineExecutablePropertiesFeatures);
            extensions.add(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
        }

        // Enable VK_EXT_pipeline_properties if available
        auto availablePhysicalDevicePipelinePropertiesFeatures = get_available_physical_device_features<VkPhysicalDevicePipelinePropertiesFeaturesEXT>(gvkPhysicalDevice);
        if (availablePhysicalDevicePipelinePropertiesFeatures.pipelinePropertiesIdentifier) {
            tlDeviceExtensionInfo.VK_KHR_pipeline_properties_enabled = VK_TRUE;
            pNextChainEditor.set(availablePhysicalDevicePipelinePropertiesFeatures);
            extensions.add(VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME);
        }

        // Enable VK_KHR_8bit_storage if available
        auto pEnabledPhysicalDeviceRayTracingPipelineFeatures = pNextChainEditor.get<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
        if (pEnabledPhysicalDeviceRayTracingPipelineFeatures && pEnabledPhysicalDeviceRayTracingPipelineFeatures->rayTracingPipeline) {
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

        // Write device info to workspace
        if (!workspacePath.empty() && std::filesystem::exists(workspacePath)) {

            // Report VkDeviceCreateInfo
            std::ofstream deviceCreateInfoJson(workspacePath / ("VkDeviceCreateInfo.json"));
            deviceCreateInfoJson << gvk::to_string(*pCreateInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;

            // Report VkPhysicalDeviceProperties
            auto physicalDeviceProperties = gvk::get_default<VkPhysicalDeviceProperties>();
            gvkPhysicalDevice.GetPhysicalDeviceProperties(&physicalDeviceProperties);
            std::ofstream physicalDevicePropertiesJson(workspacePath / ("VkPhysicalDeviceProperties.json"));
            physicalDevicePropertiesJson << gvk::to_string(physicalDeviceProperties, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // Call plugin pre_process_vkCreateDevice() handlers
        // TODO : Plugin initialization needs to be significantly reworked
        auto commandCreateDevice = gvk::get_default<GvkCommandStructureCreateDevice>();
        commandCreateDevice.physicalDevice = physicalDevice;
        commandCreateDevice.pCreateInfo = pCreateInfo;
        commandCreateDevice.pAllocator = pAllocator;
        commandCreateDevice.pDevice = pDevice;
        auto pCommandBaseStructure = (const GvkCommandBaseStructure*)&commandCreateDevice;
        gvk_result(pluginManager.pre_process_vkCreateDevice(*pCommandBaseStructure));

        // Execute vkCreateDevice() via BasicPipelineExplorer
        gvk_result(BasicPipelineExplorer::execute_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice));

        // Call plugin post_process_vkCreateDevice() handlers
        // TODO : Plugin initialization needs to be significantly reworked
        gvk_result(pluginManager.post_process_vkCreateDevice(*pCommandBaseStructure));

        // NOTE : vkLayer is here to determine whether PE is running standalone or as a
        //  GPA FW (RIP) component, keeping this here for the time being to assist in
        //  re-enabling that use-case if necessary
        // NOTE : There's been some other breaking changes regarding GPA FW integration
        //  so there'd still be some effort required to re-enable
        // NOTE : Some similar logic may be necessary to setup a GITS plugin
        // TODO : Really this sort of check should be available via BasicApiCallHandler
        if (!vkLayer) {

#if 0
            // Create unmanaged device
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &dispatchTable, *pDevice, &gvkDevice));
            auto inserted = gvkDevices.insert(gvkDevice).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
#else
            // TODO : Figure out why elevated privileges break dispatch table loading
            const auto& layerDispatchTableItr = gvk::layer::Registry::get().VkDeviceDispatchTables.find(gvk::layer::get_dispatch_key(*pDevice));
            gvk_result_assert(layerDispatchTableItr != gvk::layer::Registry::get().VkDeviceDispatchTables.end());
            const auto& layerDispatchTable = layerDispatchTableItr->second;

            // Create unmanaged device
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &layerDispatchTable, *pDevice, &gvkDevice));
            auto inserted = mGvkDevices.insert(gvkDevice).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
#endif

            // Setup device info
            pipeline_explorer::DeviceInfo deviceInfo(gvk::newref, *pDevice);
            deviceInfo->physicalDeviceInfo = physicalDevice;
            deviceInfo->vkHandle = *pDevice;
            deviceInfo->deviceCreateInfo = *pCreateInfo;
            deviceInfo->pipelineStatisticsQuery_enabled = tlDeviceExtensionInfo.pipelineStatisticsQuery_enabled;
            deviceInfo->VK_EXT_pipeline_creation_cache_control_enabled = tlDeviceExtensionInfo.VK_EXT_pipeline_creation_cache_control_enabled;
            deviceInfo->VK_EXT_shader_module_identifier_enabled = tlDeviceExtensionInfo.VK_EXT_shader_module_identifier_enabled;
            deviceInfo->VK_KHR_calibrated_timestamps_enabled = tlDeviceExtensionInfo.VK_KHR_calibrated_timestamps_enabled;
            deviceInfo->VK_KHR_performance_query_enabled = tlDeviceExtensionInfo.VK_KHR_performance_query_enabled;
            deviceInfo->VK_KHR_pipeline_binary_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_binary_enabled;
            deviceInfo->VK_KHR_pipeline_executable_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_executable_properties_enabled;
            deviceInfo->VK_KHR_pipeline_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_properties_enabled;

            // Setup queue infos
            for (const auto& queueFamily : gvkDevice.get<gvk::QueueFamilies>()) {
                pipeline_explorer::QueueFamilyInfo queueFamilyInfo{ };
                queueFamilyInfo.index = queueFamily.index;
                for (const auto& queue : queueFamily.queues) {
                    pipeline_explorer::QueueInfo queueInfo(gvk::newref, queue);
                    queueInfo->deviceInfo = deviceInfo;
                    queueInfo->vkHandle = queue;
                    queueInfo->name = "VkQueue " + gvk::string::remove(gvk::to_string(queueInfo->vkHandle), "\"");
                    queueInfo->deviceQueueCreateInfo = queue.get<VkDeviceQueueCreateInfo>();
                    queueFamilyInfo.queueInfos.push_back(queueInfo);
                }
                deviceInfo->queueFamilyInfos[queueFamily.index] = queueFamilyInfo;
            }

            // Cache device info
            inserted = deviceInfos.insert({ *pDevice, deviceInfo }).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        }

        // Revert pCreateInfo
        // NOTE : This doesn't revert changes made to pNext chain
        // TODO : Setup deep copy, modify, revert utilities
        *const_cast<VkDeviceCreateInfo*>(pCreateInfo) = deviceCreateInfo;
    } gvk_result_scope_end;
    return gvkResult;
}

static VkResult setup_pipeline_statistic_counter(
    VkQueryPipelineStatisticFlagBits pipelineStatistic,
    std::vector<VkPerformanceCounterKHR>& counters,
    std::vector<VkPerformanceCounterDescriptionKHR>& descriptions,
    const std::string& description
)
{
    gvk_result_scope_begin(VK_SUCCESS) {

        // Setup VkPerformanceCounterKHR
        counters.push_back(gvk::get_default<VkPerformanceCounterKHR>());
        counters.back().unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        counters.back().scope = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR;
        counters.back().storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR;
        static_assert(sizeof(pipelineStatistic) <= sizeof(counters.back().uuid));
        memcpy(counters.back().uuid, &pipelineStatistic, sizeof(pipelineStatistic));

        // Get counter name
        // TODO : DRY
        //  pipeline-explorer.cpp
        //  pipeline-statistics-query-manager.cpp
        std::string pipelineStatisticNameStr;
        auto pipelineStatisticFlagStr = gvk::to_string(pipelineStatistic, gvk::Printer::Default ^ gvk::Printer::EnumValue);
        pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "VK_QUERY_PIPELINE_STATISTIC_");
        pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "_BIT");
        pipelineStatisticFlagStr = gvk::string::remove(pipelineStatisticFlagStr, "\"");
        for (auto token : gvk::string::split_snake_case(pipelineStatisticFlagStr)) {
            gvk_result_assert(!token.empty());
            token = gvk::string::to_lower(token);
            token[0] = gvk::string::to_upper(token[0]);
            if (!pipelineStatisticNameStr.empty()) {
                pipelineStatisticNameStr += " ";
            }
            pipelineStatisticNameStr += token;
        }

#ifdef GVK_PLATFORM_WINDOWS
        // Setup VkPerformanceCounterDescriptionKHR
        descriptions.push_back(gvk::get_default<VkPerformanceCounterDescriptionKHR>());
        gvk_result_assert(pipelineStatisticNameStr.size() < VK_MAX_DESCRIPTION_SIZE);
        strcpy_s(descriptions.back().name, sizeof(descriptions.back().name), pipelineStatisticNameStr.c_str());
        strcpy_s(descriptions.back().category, sizeof(descriptions.back().category), "Pipeline Statistics");
        gvk_result_assert(description.size() < VK_MAX_DESCRIPTION_SIZE);
        strcpy_s(descriptions.back().description, sizeof(descriptions.back().description), description.c_str());
#else
        // TODO :
        (void)descriptions;
        (void)description;
#endif // GVK_PLATFORM_WINDOWS

    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    (void)pAllocator;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // NOTE : See comment in execute_vkCreateDevice() regarding vkLayer
        if (vkLayer) {

#if 0
            // Create unmanaged device
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &dispatchTable, *pDevice, &gvkDevice));
            auto inserted = gvkDevices.insert(gvkDevice).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
#else
            // TODO : Figure out why elevated privileges break dispatch table loading
            const auto& layerDispatchTableItr = gvk::layer::Registry::get().VkDeviceDispatchTables.find(gvk::layer::get_dispatch_key(*pDevice));
            gvk_result_assert(layerDispatchTableItr != gvk::layer::Registry::get().VkDeviceDispatchTables.end());
            const auto& layerDispatchTable = layerDispatchTableItr->second;

            // Create unmanaged device
            gvk::Device gvkDevice;
            gvk_result(gvk::Device::create_unmanaged(physicalDevice, pCreateInfo, nullptr, &layerDispatchTable, *pDevice, &gvkDevice));
            auto inserted = mGvkDevices.insert(gvkDevice).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
#endif

            // Setup device info
            pipeline_explorer::DeviceInfo deviceInfo(gvk::newref, *pDevice);
            deviceInfo->physicalDeviceInfo = physicalDevice;
            deviceInfo->vkHandle = *pDevice;
            deviceInfo->deviceCreateInfo = *pCreateInfo;
            deviceInfo->pipelineStatisticsQuery_enabled = tlDeviceExtensionInfo.pipelineStatisticsQuery_enabled;
            deviceInfo->VK_EXT_pipeline_creation_cache_control_enabled = tlDeviceExtensionInfo.VK_EXT_pipeline_creation_cache_control_enabled;
            deviceInfo->VK_EXT_shader_module_identifier_enabled = tlDeviceExtensionInfo.VK_EXT_shader_module_identifier_enabled;
            deviceInfo->VK_KHR_calibrated_timestamps_enabled = tlDeviceExtensionInfo.VK_KHR_calibrated_timestamps_enabled;
            deviceInfo->VK_KHR_performance_query_enabled = tlDeviceExtensionInfo.VK_KHR_performance_query_enabled;
            deviceInfo->VK_KHR_pipeline_binary_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_binary_enabled;
            deviceInfo->VK_KHR_pipeline_executable_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_executable_properties_enabled;
            deviceInfo->VK_KHR_pipeline_properties_enabled = tlDeviceExtensionInfo.VK_KHR_pipeline_properties_enabled;

            // Setup queue infos
            for (const auto& queueFamily : gvkDevice.get<gvk::QueueFamilies>()) {
                pipeline_explorer::QueueFamilyInfo queueFamilyInfo{ };
                queueFamilyInfo.index = queueFamily.index;
                for (const auto& queue : queueFamily.queues) {
                    pipeline_explorer::QueueInfo queueInfo(gvk::newref, queue);
                    queueInfo->deviceInfo = deviceInfo;
                    queueInfo->vkHandle = queue;
                    queueInfo->name = "VkQueue " + gvk::string::remove(gvk::to_string(queueInfo->vkHandle), "\"");
                    queueInfo->deviceQueueCreateInfo = queue.get<VkDeviceQueueCreateInfo>();
                    queueFamilyInfo.queueInfos.push_back(queueInfo);
                    if (vkLayer) { // NOTE : This check is redundant, but when post call handlers are refactored this will need to be set correctly
                        *(void**)queueInfo->vkHandle = *(void**)*pDevice;
                    }
                    deviceInfo->queueFamilyInfos[queueFamily.index] = queueFamilyInfo;
                }
            }

            // Map device and info
            inserted = deviceInfos.insert({ *pDevice, deviceInfo }).second;
            gvk_result(inserted ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

            // Setup pipeline statistics counters
            if (deviceInfo->pipelineStatisticsQuery_enabled) {

                std::vector<VkPerformanceCounterKHR> counters;
                std::vector<VkPerformanceCounterDescriptionKHR> descriptions;
                auto pipelineExplorerMetricInfo = gvk::get_default<GvkPipelineExplorerMetricInfo>();
                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.type = GVK_PIPELINE_EXPLORER_METRIC_TYPE_COUNT;
                pipelineExplorerMetricInfo.pName = "Input Assembly Vertices";
                pipelineExplorerMetricInfo.pGroupName = "VkQueryPipelineStatistics";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of vertices processed by the input assembly stage. Vertices corresponding to incomplete primitives may contribute to the count."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Input Assembly Primitives";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of primitives processed by the input assembly stage. If primitive restart is enabled, restarting the primitive topology has no effect on the count. Incomplete primitives may be counted."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Vertex Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of vertex shader invocations."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Geometry Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of geometry shader invocations. In the case of instanced geometry shaders, the geometry shader invocations count is incremented for each separate instanced invocation."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Geometry Shader Primitives";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of primitives generated by geometry shader invocations. Restarting primitive topology using SPIR-V instructions OpEndPrimitive or OpEndStreamPrimitive has no effect on primitive count."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Clipping Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of primitives processed by the primitive clipping stage of the pipeline."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Clipping Primitives";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of primitives output by the primitive clipping stage of the pipeline. The actual number of primitives output for a particular input primitive is implementation-dependent."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Fragment Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of fragment shader invocations."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Tessellation Control Shader Patches";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of patches processed by the tessellation control shader."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Tessellation Evaluation Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of tessellation evaluation shader invocations."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Compute Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of compute shader invocations. Implementations may execute more or less compute shader invocations than reported as long as the results remain unchanged."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Task Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of task shader invocations."
                ));

                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Mesh Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of mesh shader invocations."
                ));

                #if 0
                pipelineExplorerMetricInfo.id = { GVK_PIPELINE_EXPLORER_METRIC_ID_STATISTIC_QUERY, VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI, 0, 0 };
                pipelineExplorerMetricInfo.pName = "Subpass Shader Invocations";
                availableMetrics[pipelineExplorerMetricInfo.id] = pipelineExplorerMetricInfo;
                gvk_result(setup_pipeline_statistic_counter((VkQueryPipelineStatisticFlagBits)pipelineExplorerMetricInfo.id.y, counters, descriptions,
                    "The number of cluster culling shader invocations."
                ));
                #endif

#ifdef GVK_PLATFORM_WINDOWS
                // Report available pipeline statistic counters to frontend
                gvk_result_assert(counters.size() == descriptions.size());
                auto pipelineExplorerPerformanceCounterCollection = gvk::get_default<GvkPipelineExplorerPerformanceCounterCollection>();
                pipelineExplorerPerformanceCounterCollection.count = (uint32_t)counters.size();
                pipelineExplorerPerformanceCounterCollection.pCounters = counters.data();
                pipelineExplorerPerformanceCounterCollection.pDescriptions = descriptions.data();
                mIpcMessenger.write("PipelineStatisticsCounterCollection", pipelineExplorerPerformanceCounterCollection);
#endif
            }

            // Setup performance counters
            if (deviceInfo->VK_KHR_performance_query_enabled) {
                gvk::PhysicalDevice gvkPhysicalDevice(physicalDevice);
                gvk_result(gvkPhysicalDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                for (auto& queueFamilyInfoItr : deviceInfo->queueFamilyInfos) {
                    auto& queueFamilyInfo = queueFamilyInfoItr.second;
                    uint32_t counterCount = 0;
                    gvkPhysicalDevice.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(queueFamilyInfo.index, &counterCount, nullptr, nullptr);
                    queueFamilyInfo.performanceCounters.resize(counterCount, gvk::get_default<VkPerformanceCounterKHR>());
                    queueFamilyInfo.performanceCounterDescriptions.resize(counterCount, gvk::get_default<VkPerformanceCounterDescriptionKHR>());
                    gvkPhysicalDevice.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(queueFamilyInfo.index, &counterCount, queueFamilyInfo.performanceCounters.data(), queueFamilyInfo.performanceCounterDescriptions.data());
                    for (uint32_t counter_i = 0; counter_i < counterCount; ++counter_i) {
                        std::array<uint8_t, VK_UUID_SIZE> uuid{ };
                        gvk_result(sizeof(uuid) == sizeof(queueFamilyInfo.performanceCounters[counter_i].uuid) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
                        memcpy(uuid.data(), queueFamilyInfo.performanceCounters[counter_i].uuid, sizeof(uuid));
                        queueFamilyInfo.performanceCounterIndices.insert({ uuid, counter_i });
                    }
                }

#ifdef GVK_PLATFORM_WINDOWS
                // Report available performance counters to frontend
                // TODO : Expose counters based on queue
                // TODO : Handle multi-device scenarios
                std::set<VkPerformanceCounterKHR> uniquePerformanceCounters;
                std::vector<VkPerformanceCounterKHR> performanceCounters;
                std::vector<VkPerformanceCounterDescriptionKHR> performanceCounterDescriptions;
                deviceInfos.enumerate(
                    [&](const auto& deviceInfoItr)
                    {
                        for (const auto& queueFamilyInfo : deviceInfoItr.second->queueFamilyInfos) {
                            for (size_t i = 0; i < queueFamilyInfo.second.performanceCounters.size() && i < queueFamilyInfo.second.performanceCounterDescriptions.size(); ++i) {
                                if (uniquePerformanceCounters.insert(queueFamilyInfo.second.performanceCounters[i]).second) {
                                    performanceCounters.push_back(queueFamilyInfo.second.performanceCounters[i]);
                                    performanceCounterDescriptions.push_back(queueFamilyInfo.second.performanceCounterDescriptions[i]);
                                }
                            }
                        }
                        return true;
                    }
                );
                auto pipelineExplorerPerformanceCounterCollection = gvk::get_default<GvkPipelineExplorerPerformanceCounterCollection>();
                pipelineExplorerPerformanceCounterCollection.count = (uint32_t)performanceCounters.size();
                pipelineExplorerPerformanceCounterCollection.pCounters = performanceCounters.data();
                pipelineExplorerPerformanceCounterCollection.pDescriptions = performanceCounterDescriptions.data();
                mIpcMessenger.write("PerformanceCounterCollection", pipelineExplorerPerformanceCounterCollection);
#endif
            }

#ifdef GVK_PLATFORM_WINDOWS
            // Report plugin counters to frontend
            auto pluginCounterInfo = gvk::get_default<GvkPipelineExplorerPluginCounterInfo>();
            pluginManager.get_plugin_counter_info(VK_NULL_HANDLE, &pluginCounterInfo);
            if (pluginCounterInfo.groupCount) {
                mIpcMessenger.write("PluginCounterCollection", pluginCounterInfo);
            }
#endif
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    mGvkDevices.erase(device);
    pipeline_explorer::DeviceInfo(device)->queueFamilyInfos.clear();
    deviceInfos.erase(device);
    BasicPipelineExplorer::execute_vkDestroyDevice(device, pAllocator);
}

} // namespace gvk

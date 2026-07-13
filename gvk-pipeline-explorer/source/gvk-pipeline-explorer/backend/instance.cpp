
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

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace gvk {

VkResult PipelineExplorer::execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR

    // Get layer path
    std::filesystem::path layerPath;
    (void)gvk::get_this_module_path(&layerPath);

    // Get process name
    std::filesystem::path processName;
    (void)gvk::get_this_process_name(&processName);

    // Get frontend configuration from environment
    mHeadless = gvk::get_env_var_true("GVK_PIPELINE_EXPLORER_HEADLESS");
    mReportPath = gvk::get_env_var("GVK_PIPELINE_EXPLORER_REPORT_PATH");
    #if 0
    // TODO : GVK_PIPELINE_EXPLORER_REPORT_NAME
    std::filesystem::create_directories(mReportPath.root_directory());
    #endif

    if (!mHeadless) {

        // Set target application name
        targetApplicationName = processName.stem().string();

        // Get default workspacePath
        PWSTR pDocumentsPath = NULL;
        auto hResult = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pDocumentsPath);
        auto target = targetApplicationName + "-pipeline-explorer";
        workspacePath = (SUCCEEDED(hResult) && pDocumentsPath) ? std::filesystem::path(pDocumentsPath) / "GPA" / target : target;
        CoTaskMemFree(pDocumentsPath);

        // Use workspacePath provided via environment if available
        if (!gvk::get_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE").empty()) {
            workspacePath = gvk::get_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE");
        }

        // Get target
        if (!gvk::get_env_var("GVK_PIPELINE_EXPLORER_TARGET").empty()) {
            targetApplicationName = gvk::get_env_var("GVK_PIPELINE_EXPLORER_TARGET");
        }

#if 0
        // Get auto query
        autoQuery = !gvk::get_env_var("GVK_PIPELINE_EXPLORER_AUTO_QUERY").empty();
        if (autoQuery) {
            (void)mTimestampQueryManager.initialize_auto_query();
#if 0
            toolCallbackInfo.pfnPreProcessRange = gvk::pipeline_explorer::Tool::pre_process_range;
            toolCallbackInfo.pfnPreProcessCommandBuffers = gvk::pipeline_explorer::Tool::pre_process_command_buffers;
            toolCallbackInfo.pfnPreProcessCmd = gvk::pipeline_explorer::Tool::pre_process_cmd;
            toolCallbackInfo.pfnPostProcessCmd = gvk::pipeline_explorer::Tool::post_process_cmd;
            toolCallbackInfo.pfnPostProcessCommandBuffers = gvk::pipeline_explorer::Tool::post_process_command_buffers;
            toolCallbackInfo.pfnPreProcessQueueSubmission = gvk::pipeline_explorer::Tool::pre_process_queue_submission;
            toolCallbackInfo.pfnPostProcessQueueSubmission = gvk::pipeline_explorer::Tool::post_process_queue_submission;
            toolCallbackInfo.pfnPostProcessRange = gvk::pipeline_explorer::Tool::post_process_range;
            toolCallbackInfo.pUserData = &timestampQueryManager;
#else
            mTimestampQueryManager.submit_request("", { });
#endif
        }
#endif

        // The frontend sets GVK_PIPELINE_EXPLORER_GUI_PID before launching the tooled
        //  app, so if it's not set it indicates that the layer was loaded without the
        //  GUI so the GUI should be launched; this supports the VTune plugin use-case
        // NOTE : It may be useful to load the layer with no GUI at all in some cases
        //  (ie. automated test scenarios) so this logic will need to be revisited
        if (gvk::get_env_var("GVK_PIPELINE_EXPLORER_GUI_PID").empty()) {
            auto vkResult = launch_gui(layerPath);
            if (vkResult != VK_SUCCESS) {
                return vkResult;
            }
        }
    }

#ifdef GVK_PLATFORM_WINDOWS
    // Connect to the named pipe server in the frontend.
    // The pipe name is a plain string envvar so survives any intermediate script launchers.
    auto ipcPipeName = gvk::get_env_var("GVK_PIPELINE_EXPLORER_IPC_PIPE_NAME");
    if (!ipcPipeName.empty()) {
        gvk::NamedPipe::ClientCreateInfo clientCreateInfo{ };
        clientCreateInfo.pName = ipcPipeName.c_str();
        if (gvk::NamedPipe::connect_client(&clientCreateInfo, &mIpcPipe)) {
            auto h = mIpcPipe.get_handle();
            mIpcMessenger.set_read_pipe(h);
            mIpcMessenger.set_write_pipe(h);
            start_ipc_thread();
        }
    }
#endif // GVK_PLATFORM_WINDOWS

#endif // VK_USE_PLATFORM_WIN32_KHR

    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Prepare layer and extension collections with app values
        auto instanceCreateInfo = *pCreateInfo;
        gvk::StringArrayIndexMap layers;
        gvk::StringArrayIndexMap extensions;
        layers.add(pCreateInfo->enabledLayerCount, pCreateInfo->ppEnabledLayerNames);
        extensions.add(pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);

        // TODO : Plumb version info to SPIRV logic to determine min/max SPIRV support
        auto apiVersion = pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0;
        auto versionMajor = VK_VERSION_MAJOR(apiVersion);
        auto versionMinor = VK_VERSION_MINOR(apiVersion);
        auto versionPatch = VK_VERSION_PATCH(apiVersion);
        (void)versionMajor;
        (void)versionMinor;
        (void)versionPatch;

        // Enable VK_KHR_get_physical_device_properties2 if available
        if (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion < VK_API_VERSION_1_1) {
            extensions.add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        // Validate custom layers and extensions
        gvk::DispatchTable gvkDispatchTable{};
        gvk::DispatchTable::load_global_entry_points(&gvkDispatchTable);
        auto instanceLayerProperties = pipeline_explorer::get_instance_layer_properties(gvkDispatchTable.gvkEnumerateInstanceLayerProperties);
        for (auto const& invalidLayer : layers.validate(
            (uint32_t)instanceLayerProperties.size(),
            instanceLayerProperties.data(),
            [](const VkLayerProperties& layerProperties)
            {
                return layerProperties.layerName;
            }
        )) {
            (void)invalidLayer;
        }
        auto instanceExtensionProperties = pipeline_explorer::get_instance_extension_properties(gvkDispatchTable.gvkEnumerateInstanceExtensionProperties);
        for (auto const& invalidExtension : extensions.validate(
            (uint32_t)instanceExtensionProperties.size(),
            instanceExtensionProperties.data(),
            [](const VkExtensionProperties& extensionProperties)
            {
                return extensionProperties.extensionName;
            }
        )) {
            (void)invalidExtension;
        }

        // Point pCreateInfo at custom layers and extensions
        auto pMutableCreateInfo = const_cast<VkInstanceCreateInfo*>(pCreateInfo);
        pMutableCreateInfo->enabledLayerCount = layers.count();
        pMutableCreateInfo->ppEnabledLayerNames = layers.data();
        pMutableCreateInfo->enabledExtensionCount = extensions.count();
        pMutableCreateInfo->ppEnabledExtensionNames = extensions.data();

        // Write instance info to workspace
        if (!workspacePath.empty()) {
            std::filesystem::create_directories(workspacePath);
            std::filesystem::remove_all(workspacePath / ".data");
            std::ofstream json(workspacePath / ("VkInstanceCreateInfo.json"));
            json << gvk::to_string(*pCreateInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // Call plugin pre_process_vkCreateInstance() handlers
        // TODO : Plugin initialization needs to be significantly reworked
        auto commandCreateInstance = gvk::get_default<GvkCommandStructureCreateInstance>();
        commandCreateInstance.pCreateInfo = pCreateInfo;
        commandCreateInstance.pAllocator = pAllocator;
        commandCreateInstance.pInstance = pInstance;
        auto pCommandBaseStructure = (const GvkCommandBaseStructure*)&commandCreateInstance;
        gvk_result(pluginManager.pre_process_vkCreateInstance(*pCommandBaseStructure));

        // Execute vkCreateInstance() via BasicPipelineExplorer
        gvk_result(BasicPipelineExplorer::execute_vkCreateInstance(pCreateInfo, pAllocator, pInstance));

        // NOTE : vkLayer is here to determine whether PE is running standalone or as a
        //  GPA FW (RIP) component, keeping this here for the time being to assist in
        //  re-enabling that use-case if necessary
        // NOTE : There's been some other breaking changes regarding GPA FW integration
        //  so there'd still be some effort required to re-enable
        // NOTE : Some similar logic may be necessary to setup a GITS plugin
        // TODO : Really this sort of check should be available via BasicApiCallHandler
        if (!vkLayer) {

            // Create unmanaged instance
            gvk_result(gvk::Instance::create_unmanaged(pCreateInfo, nullptr, &dispatchTable, *pInstance, &mGvkInstance));

            // Setup instance info
            instanceInfo = pipeline_explorer::InstanceInfo(gvk::newref, *pInstance);
            instanceInfo->vkHandle = *pInstance;
            instanceInfo->instanceCreateInfo = *pCreateInfo;
            for (const auto& physicalDevice : mGvkInstance.get<gvk::PhysicalDevices>()) {
                pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo(gvk::newref, physicalDevice);
                physicalDeviceInfo->instanceInfo = instanceInfo;
                physicalDeviceInfo->vkHandle = physicalDevice;

                // Get VkPhysicalDeviceMemoryProperties
                auto physicalDeviceMemoryProperties = gvk::get_default<VkPhysicalDeviceMemoryProperties>();
                physicalDevice.GetPhysicalDeviceMemoryProperties(&physicalDeviceMemoryProperties);
                physicalDeviceInfo->physicalDeviceMemoryProperties = physicalDeviceMemoryProperties;

                // Get VkPhysicalDeviceProperties2 and VkPhysicalDeviceMemoryProperties
                auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
                auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
                physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
                physicalDevice.GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
                physicalDeviceInfo->physicalDeviceProperties = physicalDeviceProperties2.properties;
                physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties = physicalDeviceRayTracingProperties;

                // Cache physical device info
                instanceInfo->physicalDevices.push_back(physicalDeviceInfo);
            }
        }

        // Revert pCreateInfo
        // NOTE : This doesn't revert changes made to pNext chain
        // TODO : Setup deep copy, modify, revert utilities
        *const_cast<VkInstanceCreateInfo*>(pCreateInfo) = instanceCreateInfo;

    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // HACK :
        // enable_timeline_query();
#if 0
        gvk::pipeline_explorer::Tool::on_reset = [&]() { toolCallbackInfo = { }; };
#endif

        // TODO : Documentation
        mToolDispatchManager.push_back(&mCommandCollectionRequestManager);
        mToolDispatchManager.push_back(&mTimelineQueryManager);
        mToolDispatchManager.push_back(&mPipelineStatisticsQueryManager);
        mToolDispatchManager.push_back(&mPerformanceQueryManager);

        // TODO : Pull configuration from environment and setup tools based on that configuration
        mTimelineQueryManager.enable("", &mIpcMessenger);

        // Initialize plugins
        // TODO : Plugin initialization needs to be significantly reworked
        GvkPipelineExplorerPluginInitializeInfo pluginInitializeInfo{ };
        auto workspacePathStr = workspacePath.string();
        pluginInitializeInfo.pWorkspace = workspacePathStr.c_str();
        const auto& layerInstanceDispatchTables = gvk::layer::Registry::get().VkInstanceDispatchTables;
        const auto& layerInstanceDispatchTableItr = layerInstanceDispatchTables.find(gvk::layer::get_dispatch_key(*pInstance));
        gvk_result(layerInstanceDispatchTableItr != layerInstanceDispatchTables.end() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        pluginInitializeInfo.pfnGetInstanceProcAddr = layerInstanceDispatchTableItr->second.gvkGetInstanceProcAddr;
        gvk_result(pluginManager.initialize_plugins(&pluginInitializeInfo));

        // Call plugin post_process_vkCreateInstance() handlers
        // TODO : Plugin initialization needs to be significantly reworked
        auto commandCreateInstance = gvk::get_default<GvkCommandStructureCreateInstance>();
        commandCreateInstance.pCreateInfo = pCreateInfo;
        commandCreateInstance.pAllocator = pAllocator;
        commandCreateInstance.pInstance = pInstance;
        auto pCommandBaseStructure = (const GvkCommandBaseStructure*)&commandCreateInstance;
        gvk_result(pluginManager.post_process_vkCreateInstance(*pCommandBaseStructure));

        // NOTE : See comment in execute_vkCreateInstance() regarding vkLayer
        if (vkLayer) {

            // Create unmanaged instance
            gvk_result(gvk::Instance::create_unmanaged(pCreateInfo, nullptr, &dispatchTable, *pInstance, &mGvkInstance));

            // Setup instance info
            instanceInfo = pipeline_explorer::InstanceInfo(gvk::newref, *pInstance);
            instanceInfo->vkHandle = *pInstance;
            instanceInfo->instanceCreateInfo = *pCreateInfo;
            for (const auto& physicalDevice : mGvkInstance.get<gvk::PhysicalDevices>()) {
                pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo(gvk::newref, physicalDevice);
                physicalDeviceInfo->instanceInfo = instanceInfo;
                physicalDeviceInfo->vkHandle = physicalDevice;

                // Get VkPhysicalDeviceMemoryProperties
                auto physicalDeviceMemoryProperties = gvk::get_default<VkPhysicalDeviceMemoryProperties>();
                physicalDevice.GetPhysicalDeviceMemoryProperties(&physicalDeviceMemoryProperties);
                physicalDeviceInfo->physicalDeviceMemoryProperties = physicalDeviceMemoryProperties;

                // Get VkPhysicalDeviceProperties2 and VkPhysicalDeviceMemoryProperties
                auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
                auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
                physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
                physicalDevice.GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
                physicalDeviceInfo->physicalDeviceProperties = physicalDeviceProperties2.properties;
                physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties = physicalDeviceRayTracingProperties;

                // Cache physical device info
                instanceInfo->physicalDevices.push_back(physicalDeviceInfo);
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
#if 0
    // TODO : Documentation
    // TODO : Need to ensure all final messages are processed
    mToolDispatchManager.disable();
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (guiProcessInformation.hProcess) {
        TerminateProcess(guiProcessInformation.hProcess, 0);
        guiProcessInformation = { };
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
    mGvkInstance = gvk::nullref;
    if (instanceInfo) {
        instanceInfo->physicalDevices.clear();
        instanceInfo = gvk::nullref;
    }
    BasicPipelineExplorer::execute_vkDestroyInstance(instance, pAllocator);

    // TODO : Documentation
    stop_ipc_thread();
}

} // namespace gvk

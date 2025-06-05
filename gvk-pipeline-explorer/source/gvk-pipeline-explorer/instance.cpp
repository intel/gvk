
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

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace gvk {

#if 0
#ifdef VK_USE_PLATFORM_WIN32_KHR
    // Get the path to this layer .dll, the gui .exe should be next to it
    HMODULE hModule = NULL;
    const size_t CharBufferSize = 16384;
    std::array<wchar_t, CharBufferSize> wcharBuffer{ };
    if (pipeline_explorer::get_this_module_handle(&hModule)) {
        GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    }
    layerPath = wcharBuffer[0] ? wcharBuffer.data() : std::filesystem::path();
    guiPath = std::filesystem::path(layerPath).remove_filename() / "gvk-pipeline-explorer-gui.exe";

    // Get the application name
    std::array<char, CharBufferSize> charBuffer;
    if (GetProcessImageFileName(GetCurrentProcess(), charBuffer.data(), (DWORD)charBuffer.size())) {
        applicationName = PathFindFileName(charBuffer.data());
        applicationName = std::filesystem::path(applicationName).replace_extension().string();
    }

    // Get the workspace path
    if (workspacePath.empty()) {
        workspacePath = gvk::get_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE");
    }
    if (workspacePath.empty()) {
        PWSTR pDocumentsPath = NULL;
        auto hResult = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pDocumentsPath);
        if (SUCCEEDED(hResult) && pDocumentsPath) {
            workspacePath = std::filesystem::path(pDocumentsPath) / "GPA" / (applicationName + "-pipeline-explorer");
        }
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#endif

VkResult PipelineExplorer::execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR

    // Get the path to this layer .dll
    HMODULE hModule = NULL;
    const size_t CharBufferSize = 16384;
    std::array<wchar_t, CharBufferSize> wcharBuffer{ };
    if (pipeline_explorer::get_this_module_handle(&hModule)) {
        GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    }
    auto layerPath = wcharBuffer[0] ? wcharBuffer.data() : std::filesystem::path();

    // Get the application name
    static std::array<char, CharBufferSize> charBuffer;
    if (GetProcessImageFileName(GetCurrentProcess(), charBuffer.data(), (DWORD)charBuffer.size())) {
        applicationName = PathFindFileName(charBuffer.data());
        applicationName = std::filesystem::path(applicationName).replace_extension().string();
    }

    // Wait for debugger
    if (!gvk::get_env_var("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER").empty()) {
        MessageBox(0, (layerPath.string() + "\n\n" + applicationName).c_str(), "Attach Debugger Now", MB_OK);
    }

    // Get workspace
    if (!gvk::get_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE").empty()) {
        workspacePath = gvk::get_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE");
    }

    // Get target
    if (!gvk::get_env_var("GVK_PIPELINE_EXPLORER_TARGET").empty()) {
        targetApplicationName = gvk::get_env_var("GVK_PIPELINE_EXPLORER_TARGET");
    }
#endif

    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // TODO : Documentation
        auto instanceCreateInfo = *pCreateInfo;
        pipeline_explorer::LayerCollection layers;
        pipeline_explorer::ExtensionCollection extensions;
        layers.add(pCreateInfo->enabledLayerCount, pCreateInfo->ppEnabledLayerNames);
        extensions.add(pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);

#if 0
        // TODO : Documentation
        instanceLayers.add("VK_LAYER_KHRONOS_validation");
#endif

        // TODO : Documentation
        auto versionMajor = VK_VERSION_MAJOR(pCreateInfo->pApplicationInfo->apiVersion);
        auto versionMinor = VK_VERSION_MINOR(pCreateInfo->pApplicationInfo->apiVersion);
        auto versionPatch = VK_VERSION_PATCH(pCreateInfo->pApplicationInfo->apiVersion);
        (void)versionMajor;
        (void)versionMinor;
        (void)versionPatch;

        // TODO : Documentation
        if (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion < VK_API_VERSION_1_1) {
            extensions.add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        // Validate custom layers and extensions
        gvk::DispatchTable gvkDispatchTable{};
        gvk::DispatchTable::load_global_entry_points(&gvkDispatchTable);
        auto instanceLayerProperties = pipeline_explorer::get_instance_layer_properties(gvkDispatchTable.gvkEnumerateInstanceLayerProperties);
        for (auto const& invalidLayer : layers.validate((uint32_t)instanceLayerProperties.size(), instanceLayerProperties.data())) {
            (void)invalidLayer;
        }
        auto instanceExtensionProperties = pipeline_explorer::get_instance_extension_properties(gvkDispatchTable.gvkEnumerateInstanceExtensionProperties);
        for (auto const& invalidExtension : extensions.validate((uint32_t)instanceExtensionProperties.size(), instanceExtensionProperties.data())) {
            (void)invalidExtension;
        }

        // Point pCreateInfo at custom layers and extensions
        auto pMutableCreateInfo = const_cast<VkInstanceCreateInfo*>(pCreateInfo);
        pMutableCreateInfo->enabledLayerCount = layers.count();
        pMutableCreateInfo->ppEnabledLayerNames = layers.data();
        pMutableCreateInfo->enabledExtensionCount = extensions.count();
        pMutableCreateInfo->ppEnabledExtensionNames = extensions.data();

#if 0
        // TODO : Documentation
        if (vkLayer) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
            // Get the path to this layer .dll, the gui .exe should be next to it
            HMODULE hModule = NULL;
            const size_t CharBufferSize = 16384;
            std::array<wchar_t, CharBufferSize> wcharBuffer{ };
            if (pipeline_explorer::get_this_module_handle(&hModule)) {
                GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
            }
            layerPath = wcharBuffer[0] ? wcharBuffer.data() : std::filesystem::path();
            guiPath = std::filesystem::path(layerPath).remove_filename() / "gvk-pipeline-explorer-gui.exe";

            // Get the application name
            std::array<char, CharBufferSize> charBuffer;
            if (GetProcessImageFileName(GetCurrentProcess(), charBuffer.data(), (DWORD)charBuffer.size())) {
                applicationName = PathFindFileName(charBuffer.data());
                applicationName = std::filesystem::path(applicationName).replace_extension().string();
            }

            // Get the workspace path
            if (workspacePath.empty()) {
                workspacePath = gvk::get_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE");
            }
            if (workspacePath.empty()) {
                PWSTR pDocumentsPath = NULL;
                auto hResult = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pDocumentsPath);
                if (SUCCEEDED(hResult) && pDocumentsPath) {
                    workspacePath = std::filesystem::path(pDocumentsPath) / "GPA" / (applicationName + "-pipeline-explorer");
                }
            }
#endif // VK_USE_PLATFORM_WIN32_KHR
        }
#endif

        if (!workspacePath.empty()) {
            std::filesystem::create_directories(workspacePath);
            std::ofstream json(workspacePath / ("VkInstanceCreateInfo.json"));
            json << gvk::to_string(*pCreateInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // TODO : Documentation
        gvk_result(BasicApiCallHandler::execute_vkCreateInstance(pCreateInfo, pAllocator, pInstance));

        if (!vkLayer) {

            // TODO : Documentation
            gvk_result(gvk::Instance::create_unmanaged(pCreateInfo, nullptr, &dispatchTable, *pInstance, &gvkInstance));

            // TODO : Documentation
            instanceInfo = pipeline_explorer::InstanceInfo(gvk::newref, *pInstance);
            instanceInfo->vkHandle = *pInstance;
            instanceInfo->instanceCreateInfo = *pCreateInfo;
            for (const auto& physicalDevice : gvkInstance.get<gvk::PhysicalDevices>()) {
                pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo(gvk::newref, physicalDevice);
                physicalDeviceInfo->instanceInfo = instanceInfo;
                physicalDeviceInfo->vkHandle = physicalDevice;

                auto physicalDeviceMemoryProperties = gvk::get_default<VkPhysicalDeviceMemoryProperties>();
                physicalDevice.GetPhysicalDeviceMemoryProperties(&physicalDeviceMemoryProperties);
                physicalDeviceInfo->physicalDeviceMemoryProperties = physicalDeviceMemoryProperties;

                #if 0
                auto physicalDeviceProperties = gvk::get_default<VkPhysicalDeviceProperties>();
                physicalDevice.GetPhysicalDeviceProperties(&physicalDeviceProperties);
                physicalDeviceInfo->physicalDeviceProperties = physicalDeviceProperties;
                #else
                auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
                auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
                physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
                physicalDevice.GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
                physicalDeviceInfo->physicalDeviceProperties = physicalDeviceProperties2.properties;
                physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties = physicalDeviceRayTracingProperties;
                #endif
                instanceInfo->physicalDevices.push_back(physicalDeviceInfo);
            }
        }

        // TODO : Documentation
        *const_cast<VkInstanceCreateInfo*>(pCreateInfo) = instanceCreateInfo;

    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PipelineExplorer::post_execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    (void)pAllocator;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        if (vkLayer) {

            // TODO : Documentation
            gvk_result(gvk::Instance::create_unmanaged(pCreateInfo, nullptr, &dispatchTable, *pInstance, &gvkInstance));

            // TODO : Documentation
            instanceInfo = pipeline_explorer::InstanceInfo(gvk::newref, *pInstance);
            instanceInfo->vkHandle = *pInstance;
            instanceInfo->instanceCreateInfo = *pCreateInfo;
            for (const auto& physicalDevice : gvkInstance.get<gvk::PhysicalDevices>()) {
                pipeline_explorer::PhysicalDeviceInfo physicalDeviceInfo(gvk::newref, physicalDevice);
                physicalDeviceInfo->instanceInfo = instanceInfo;
                physicalDeviceInfo->vkHandle = physicalDevice;
                #if 0
                auto physicalDeviceProperties = gvk::get_default<VkPhysicalDeviceProperties>();
                physicalDevice.GetPhysicalDeviceProperties(&physicalDeviceProperties);
                physicalDeviceInfo->physicalDeviceProperties = physicalDeviceProperties;
                #else
                auto physicalDeviceRayTracingProperties = gvk::get_default<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
                auto physicalDeviceProperties2 = gvk::get_default<VkPhysicalDeviceProperties2>();
                physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
                physicalDevice.GetPhysicalDeviceProperties2(&physicalDeviceProperties2);
                physicalDeviceInfo->physicalDeviceProperties = physicalDeviceProperties2.properties;
                physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties = physicalDeviceRayTracingProperties;
                #endif
                instanceInfo->physicalDevices.push_back(physicalDeviceInfo);
            }
#if 0
#ifdef VK_USE_PLATFORM_WIN32_KHR
            // TODO : Documentation
            STARTUPINFO startupInfo{};
            startupInfo.cb = sizeof(startupInfo);
            std::string cmdLine = "\"" + guiPath.string() + "\"";
            cmdLine += " -t \"Intel(R) GPA Pipeline Explorer\"";
            cmdLine += " -w \"" + workspacePath.string() + "\"";
            if (!CreateProcess(
                guiPath.string().c_str(),
                cmdLine.data(),
                NULL,
                NULL,
                FALSE,
                CREATE_NO_WINDOW,
                NULL,
                NULL,
                &startupInfo,
                &guiProcessInformation)) {
                gvk_result(VK_ERROR_INITIALIZATION_FAILED);
            }
#endif // VK_USE_PLATFORM_WIN32_KHR
#endif
        }
    } gvk_result_scope_end;
    return gvkResult;
}

void PipelineExplorer::execute_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (guiProcessInformation.hProcess) {
        TerminateProcess(guiProcessInformation.hProcess, 0);
        guiProcessInformation = { };
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
    gvkInstance = gvk::nullref;
    if (instanceInfo) {
        instanceInfo->physicalDevices.clear();
        instanceInfo = gvk::nullref;
    }
    BasicApiCallHandler::execute_vkDestroyInstance(instance, pAllocator);
}

} // namespace gvk

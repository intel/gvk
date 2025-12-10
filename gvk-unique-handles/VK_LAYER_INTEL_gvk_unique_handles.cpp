
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

#include "gvk-defines.hpp"
#include "gvk-environment.hpp"
#include "gvk-layer.hpp"

namespace gvk {
namespace layer {

void on_load(const VkInstanceCreateInfo* pInstanceCreateInfo, Registry& registry)
{
    // Check if VK_LAYER_KHRONOS_validation is enabled
    bool validationEnabled = gvk::string::contains("VK_INSTANCE_LAYERS", "validation") || gvk::string::contains("VK_LOADER_LAYERS_ENABLE", "validation");
    if (!validationEnabled && pInstanceCreateInfo) {
        for (uint32_t layer_i = 0; layer_i < pInstanceCreateInfo->enabledLayerCount; ++layer_i) {
            if (!strcmp(pInstanceCreateInfo->ppEnabledLayerNames[layer_i], "VK_LAYER_KHRONOS_validation")) {
                validationEnabled = true;
                break;
            }
        }
    }

    // If VK_LAYER_KHRONOS_validation is enabled, ensure that VK_KHRONOS_VALIDATION_UNIQUE_HANDLES
    //  is also enabled, otherwise enable UniqueHandlesManager
    if (validationEnabled) {
        gvk::set_env_var("VK_KHRONOS_VALIDATION_UNIQUE_HANDLES", "true");
    } else {
        registry.uniqueHandlesManager.enabled = true;
    }

    // TODO : Option to disable all besides VK_KHRONOS_VALIDATION_UNIQUE_HANDLES

    // TODO : Validate that validation layer is closest to driver
}

} // namespace layer
} // namespace gvk

extern "C" {

VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pNegotiateLayerInterface)
{
    assert(pNegotiateLayerInterface);
    pNegotiateLayerInterface->pfnGetInstanceProcAddr = gvk::layer::get_instance_proc_addr;
    pNegotiateLayerInterface->pfnGetPhysicalDeviceProcAddr = gvk::layer::get_physical_device_proc_addr;
    pNegotiateLayerInterface->pfnGetDeviceProcAddr = gvk::layer::get_device_proc_addr;
    return VK_SUCCESS;
}

} // extern "C"

#if 0
/*
Reference for configuring layers for unique handles using VK_LAYER_KHRONOS_validation
NOTE : This approach doesn't work from within a Vulkan layer since the loader has
    already configured layers by the time the vkCreateInstance() handler is run.
    This approach must be done at the application level, or in a custom layer
    implementation that handles its own entry point interception before the Vulkan
    loader.
*/
thread_local VkInstanceCreateInfo tlInstanceCreateInfo;
thread_local std::set<std::string> tlEnvironmentLayers;
thread_local gvk::StringArrayIndexMap tlInstanceLayers;
thread_local gvk::StringArrayIndexMap tlInstanceExtensions;
VkResult StateTracker::pre_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult gvkResult)
{
    // Cache application create info and populate layers and extensions
    tlInstanceCreateInfo = *pCreateInfo;
    tlInstanceLayers.add(pCreateInfo->enabledLayerCount, pCreateInfo->ppEnabledLayerNames);
    tlInstanceExtensions.add(pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);

    // Check for layers enabled via VkInstanceCreateInfo
    auto stateTrackerEnabled = tlInstanceLayers.contains("VK_LAYER_INTEL_gvk_layer");
    auto apiDumpEnabled = tlInstanceLayers.contains("VK_LAYER_LUNARG_api_dump");
    auto validationEnabled = tlInstanceLayers.contains("VK_LAYER_KHRONOS_validation");

    // Remove layers
    tlInstanceLayers.erase("VK_LAYER_INTEL_gvk_layer");
    tlInstanceLayers.erase("VK_LAYER_LUNARG_api_dump");
    tlInstanceLayers.erase("VK_LAYER_KHRONOS_validation");

    // Check for layers enabled via environment
    auto populateEnvironmentLayers = [&](const std::string& envVar)
    {
        #ifdef VK_USE_PLATFORM_WIN32_KHR
        auto delimiter = ";";
        #else
        auto delimiter = ":";
        #endif
        for (const auto& layer : gvk::string::split(gvk::get_env_var(envVar), delimiter)) {
            if (layer == "VK_LAYER_INTEL_gvk_layer") {
                stateTrackerEnabled = true;
            } else if (gvk::string::contains(layer, /* "VK_LAYER_LUNARG_" */ "api_dump")) {
                apiDumpEnabled = true;
            } else if (gvk::string::contains(layer, /* "VK_LAYER_KHRONOS_" */ "validation")) {
                validationEnabled = true;
            } else {
                tlInstanceLayers.add(tlEnvironmentLayers.insert(layer).first->c_str());
            }
        }
    };
    populateEnvironmentLayers("VK_INSTANCE_LAYERS");
    populateEnvironmentLayers("VK_LOADER_LAYERS_ENABLE");

    // Clear environment layers
    gvk::set_env_var("VK_INSTANCE_LAYERS", "");
    gvk::set_env_var("VK_LOADER_LAYERS_ENABLE", "");

    // Re-add enabled layers to end (closest to driver)
    if (stateTrackerEnabled) {
        tlInstanceLayers.add("VK_LAYER_INTEL_gvk_layer");
    }
    if (apiDumpEnabled) {
        tlInstanceLayers.add("VK_LAYER_LUNARG_api_dump");
    }

    // TODO : Documentation
    tlInstanceLayers.add("VK_LAYER_KHRONOS_validation");
    gvk::set_env_var("VK_KHRONOS_VALIDATION_UNIQUE_HANDLES", "true");
    if (!validationEnabled) {
        gvk::set_env_var("VK_LAYER_FINE_GRAINED_LOCKING",                        "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_CORE",                  "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_CHECK_IMAGE_LAYOUT",             "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_CHECK_COMMAND_BUFFER",           "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_CHECK_OBJECT_IN_USE",            "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_CHECK_QUERY",                    "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_CHECK_SHADERS",                  "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_CHECK_SHADERS_CACHING",          "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_OBJECT_LIFETIME",                "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_STATELESS_PARAM",                "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_THREAD_SAFETY",                  "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_SYNC",                  "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_SYNC_QUEUE_SUBMIT",              "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_PRINTF_TO_STDOUT",               "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_PRINTF_VERBOSE",                 "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_RESERVE_BINDING_SLOT",           "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VMA_LINEAR_OUTPUT",              "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_GPUAV_DESCRIPTOR_CHECKS",        "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_WARN_ON_ROBUST_OOB",             "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_INDIRECT_BUFFER",       "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_USE_INSTRUMENTED_SHADER_CACHE",  "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_SELECT_INSTRUMENTED_SHADERS",    "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES",        "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_ARM",    "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_AMD",    "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_IMG",    "false");
        gvk::set_env_var("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_NVIDIA", "false");
    }

    // Point pCreateInfo at custom layers and extensions
    auto pMutableCreateInfo = const_cast<VkInstanceCreateInfo*>(pCreateInfo);
    pMutableCreateInfo->enabledLayerCount = tlInstanceLayers.count();
    pMutableCreateInfo->ppEnabledLayerNames = tlInstanceLayers.data();
    pMutableCreateInfo->enabledExtensionCount = tlInstanceExtensions.count();
    pMutableCreateInfo->ppEnabledExtensionNames = tlInstanceExtensions.data();
}

VkResult post_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    *const_cast<VkInstanceCreateInfo*>(pCreateInfo) = tlInstanceCreateInfo;
    tlInstanceCreateInfo = { };
    tlEnvironmentLayers.clear();
    tlInstanceLayers.clear();
    tlInstanceExtensions.clear();
}
#endif


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
#include "gvk-layer.hpp"

namespace gvk {
namespace layer {

void on_load(const VkInstanceCreateInfo* pInstanceCreateInfo, Registry& registry)
{

#ifdef GVK_PLATFORM_WINDOWS

    // Get layer path
    std::filesystem::path layerPath;
    (void)gvk::get_this_module_path(&layerPath);

    // Get application name
    std::filesystem::path applicationName;
    (void)gvk::get_this_process_name(&applicationName);

    // Wait for debugger
    if (gvk::get_env_var_true("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER")) {
        MessageBox(0, (layerPath.string() + "\n\n" + applicationName.string()).c_str(), "Attach Debugger Now", MB_OK);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // NOTE : Duplicated from "gvk/gvk-state-tracker/source/gvk-state-tracker/state-tracker.cpp"
    // TODO : Move this logic to gvk-layer so that any/all layers have access to it

    // Check if VK_LAYER_KHRONOS_validation is enabled
    bool validationEnabled =
        gvk::string::contains(gvk::get_env_var("VK_INSTANCE_LAYERS"), "validation") ||
        gvk::string::contains(gvk::get_env_var("VK_LOADER_LAYERS_ENABLE"), "validation");
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
#if 0
        // TODO : Need to get this turned back on
        registry.uniqueHandlesManager.enabled = true;
#endif
    }

    // TODO : Option to disable all besides VK_KHRONOS_VALIDATION_UNIQUE_HANDLES

    // TODO : Validate that validation layer is closest to driver
    ////////////////////////////////////////////////////////////////////////////////

#endif

    (void)pInstanceCreateInfo;
    auto pPipelineExplorer = new gvk::PipelineExplorer;
    pPipelineExplorer->vkLayer = true;
    assert(!registry.apiCallHandler);
    registry.apiCallHandler.reset(pPipelineExplorer);
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

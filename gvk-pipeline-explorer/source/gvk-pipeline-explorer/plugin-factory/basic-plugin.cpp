
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

#include "gvk-pipeline-explorer/plugin-factory/basic-plugin.hpp"
#include "gvk-defines.hpp"
#include "gvk-runtime.hpp"

#include <iostream>
#include <string.h>

namespace gvk {
namespace pipeline_explorer {

BasicPlugin::~BasicPlugin()
{
}

VkResult BasicPlugin::initialize(const GvkPipelineExplorerPluginInitializeInfo* pInitializeInfo, GvkPipelineExplorerPluginInfo* pPluginInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pInitializeInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pInitializeInfo->pWorkspace ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pInitializeInfo->pfnGetInstanceProcAddr ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pPluginInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        workspace = pInitializeInfo->pWorkspace;
        pfnGetInstanceProcAddr = pInitializeInfo->pfnGetInstanceProcAddr;
#if WIN32
        strcpy_s(pPluginInfo->name, VK_MAX_DESCRIPTION_SIZE, get_name().c_str());
        strcpy_s(pPluginInfo->description, VK_MAX_DESCRIPTION_SIZE, get_description().c_str());
#else
        // TODO :
#endif // WIN32
        pPluginInfo->pfnGetCounterInfo = get_plugin_counter_info;
        pPluginInfo->pfnGetStatus = get_status;
        pPluginInfo->pfnSubmitRequest = submit_request;
        pPluginInfo->pfnToolCmd = tool_cmd;
        pPluginInfo->pfnPreProcessCreateInstance = pre_process_vkCreateInstance;
        pPluginInfo->pfnPostProcessCreateInstance = post_process_vkCreateInstance;
        pPluginInfo->pfnPreProcessCreateDevice = pre_process_vkCreateDevice;
        pPluginInfo->pfnPostProcessCreateDevice = post_process_vkCreateDevice;
        pPluginInfo->pfnPreProcessRange = pre_process_range;
        pPluginInfo->pfnPreProcessCommandBuffers = pre_process_command_buffers;
        pPluginInfo->pfnPreProcessCmd = pre_process_cmd;
        pPluginInfo->pfnPostProcessCmd = post_process_cmd;
        pPluginInfo->pfnPostProcessCommandBuffers = post_process_command_buffers;
        pPluginInfo->pfnPreProcessQueueSubmission = pre_process_queue_submission;
        pPluginInfo->pfnPostProcessQueueSubmission = post_process_queue_submission;
        pPluginInfo->pfnPostProcessRange = post_process_range;
        pPluginInfo->pfnDestroyPlugin = destroy;
        pPluginInfo->pUserData = this;
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::get_plugin_counter_info(GvkPipelineExplorerPluginCounterInfo* pCounterInfo)
{
    if (pCounterInfo) {
        *pCounterInfo = { };
    }
    return VK_SUCCESS;
}

VkResult BasicPlugin::get_status() const
{
    return VK_SUCCESS;
}

VkResult BasicPlugin::submit_request(const GvkPipelineExplorerPerformanceQueryRequestInfo* pRequest)
{
    (void)pRequest;
    return VK_SUCCESS;
}

VkBool32 BasicPlugin::tool_cmd(VkDevice device, VkPipeline pipeline)
{
    (void)device;
    (void)pipeline;
    return VK_FALSE;
}

VkResult BasicPlugin::pre_process_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pInstance;
    return VK_SUCCESS;
}

VkResult BasicPlugin::post_process_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pInstance;
    return VK_SUCCESS;
}

VkResult BasicPlugin::pre_process_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    (void)physicalDevice;
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pDevice;
    return VK_SUCCESS;
}

VkResult BasicPlugin::post_process_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    (void)physicalDevice;
    (void)pCreateInfo;
    (void)pAllocator;
    (void)pDevice;
    return VK_SUCCESS;
}

VkResult BasicPlugin::pre_process_range()
{
    return VK_SUCCESS;
}

VkResult BasicPlugin::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult BasicPlugin::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult BasicPlugin::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult BasicPlugin::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult BasicPlugin::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult BasicPlugin::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult BasicPlugin::post_process_range()
{
    return VK_SUCCESS;
}

void BasicPlugin::destroy()
{
}

VkResult BasicPlugin::get_plugin_counter_info(GvkPipelineExplorerPluginCounterInfo* pCounterInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->get_plugin_counter_info(pCounterInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::get_status(void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        return ((BasicPlugin*)pPlugin)->get_status();
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::submit_request(const GvkPipelineExplorerPerformanceQueryRequestInfo* pRequest, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->submit_request(pRequest));
    } gvk_result_scope_end;
    return gvkResult;
}

VkBool32 BasicPlugin::tool_cmd(VkDevice device, VkPipeline pipeline, void* pPlugin)
{
    auto toolCmd = false;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        toolCmd = ((BasicPlugin*)pPlugin)->tool_cmd(device, pipeline);
    } gvk_result_scope_end;
    return !gvkResult && toolCmd;
}

VkResult BasicPlugin::pre_process_vkCreateInstance(const GvkCommandBaseStructure* pCommand, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand->sType == gvk::get_stype<GvkCommandStructureCreateInstance>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        const auto& command = *(const GvkCommandStructureCreateInstance*)pCommand;
        gvk_result(((BasicPlugin*)pPlugin)->pre_process_vkCreateInstance(command.pCreateInfo, command.pAllocator, command.pInstance));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::post_process_vkCreateInstance(const GvkCommandBaseStructure* pCommand, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand->sType == gvk::get_stype<GvkCommandStructureCreateInstance>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        const auto& command = *(const GvkCommandStructureCreateInstance*)pCommand;
        gvk_result(((BasicPlugin*)pPlugin)->post_process_vkCreateInstance(command.pCreateInfo, command.pAllocator, command.pInstance));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::pre_process_vkCreateDevice(const GvkCommandBaseStructure* pCommand, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand->sType == gvk::get_stype<GvkCommandStructureCreateDevice>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        const auto& command = *(const GvkCommandStructureCreateDevice*)pCommand;
        gvk_result(((BasicPlugin*)pPlugin)->pre_process_vkCreateDevice(command.physicalDevice, command.pCreateInfo, command.pAllocator, command.pDevice));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::post_process_vkCreateDevice(const GvkCommandBaseStructure* pCommand, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCommand->sType == gvk::get_stype<GvkCommandStructureCreateDevice>() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        const auto& command = *(const GvkCommandStructureCreateDevice*)pCommand;
        gvk_result(((BasicPlugin*)pPlugin)->post_process_vkCreateDevice(command.physicalDevice, command.pCreateInfo, command.pAllocator, command.pDevice));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::pre_process_range(void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->pre_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->pre_process_command_buffers(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->pre_process_cmd(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->post_process_cmd(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->post_process_command_buffers(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->pre_process_queue_submission(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->post_process_queue_submission(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BasicPlugin::post_process_range(void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(((BasicPlugin*)pPlugin)->post_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

void BasicPlugin::destroy(void* pPlugin)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(pPlugin ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        ((BasicPlugin*)pPlugin)->destroy();
    } gvk_result_scope_end;
}

} // namespace pipeline_explorer
} // namespace gvk

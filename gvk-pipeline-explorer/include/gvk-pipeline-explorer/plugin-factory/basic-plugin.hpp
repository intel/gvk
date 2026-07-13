
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

#pragma once

////////////////////////////////////////////////////////////////////////////////
// TODO : Very annoying that Windows and Linux need different include orders for
//  these...that's a very good indicator that these utilities need a rework
#include "gvk-defines.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer.h"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-enumerations-to-string.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-comparison-operators.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-create-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-deserialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-destroy-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-get-stype.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-serialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-to-string.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "gvk-pipeline-explorer.hpp"
#include "gvk-runtime.hpp"

#include <string>

namespace gvk {
namespace pipeline_explorer {

class BasicPlugin
{
public:
    BasicPlugin() = default;
    virtual ~BasicPlugin() = 0;
    virtual VkResult initialize(const GvkPipelineExplorerPluginInitializeInfo* pInitializeInfo, GvkPipelineExplorerPluginInfo* pPluginInfo);
    virtual const std::string& get_name() const = 0;
    virtual const std::string& get_description() const = 0;
    virtual VkResult get_plugin_counter_info(GvkPipelineExplorerPluginCounterInfo* pCounterInfo);
    virtual VkResult get_status() const;
    virtual VkResult submit_request(const GvkPipelineExplorerPerformanceQueryRequestInfo* pRequest);
    virtual VkBool32 tool_cmd(VkDevice device, VkPipeline pipeline);
    virtual VkResult pre_process_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
    virtual VkResult post_process_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
    virtual VkResult pre_process_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
    virtual VkResult post_process_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
    virtual VkResult pre_process_range();
    virtual VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    virtual VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    virtual VkResult post_process_range();
    virtual void destroy();

    std::filesystem::path workspace;
    PFN_vkGetInstanceProcAddr pfnGetInstanceProcAddr{ };

private:
    static VkResult get_plugin_counter_info(GvkPipelineExplorerPluginCounterInfo* pCounterInfo, void* pPlugin);
    static VkResult get_status(void* pPlugin);
    static VkResult submit_request(const GvkPipelineExplorerPerformanceQueryRequestInfo* pRequest, void* pPlugin);
    static VkBool32 tool_cmd(VkDevice device, VkPipeline pipeline, void* pPlugin);
    static VkResult pre_process_vkCreateInstance(const GvkCommandBaseStructure* pCommand, void* pPlugin);
    static VkResult post_process_vkCreateInstance(const GvkCommandBaseStructure* pCommand, void* pPlugin);
    static VkResult pre_process_vkCreateDevice(const GvkCommandBaseStructure* pCommand, void* pPlugin);
    static VkResult post_process_vkCreateDevice(const GvkCommandBaseStructure* pCommand, void* pPlugin);
    static VkResult pre_process_range(void* pPlugin);
    static VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin);
    static VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin);
    static VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin);
    static VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pPlugin);
    static VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pPlugin);
    static VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pPlugin);
    static VkResult post_process_range(void* pPlugin);
    static void destroy(void* pPlugin);

    BasicPlugin(const BasicPlugin&) = delete;
    BasicPlugin& operator=(const BasicPlugin&) = delete;
    friend class PluginManager;
};

} // namespace pipeline_explorer
} // namespace gvk

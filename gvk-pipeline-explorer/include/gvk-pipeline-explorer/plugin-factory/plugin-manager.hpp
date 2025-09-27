
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

#include <unordered_map>

namespace gvk {
namespace pipeline_explorer {

class PluginManager final
{
public:
    PluginManager();
    ~PluginManager();
    VkResult initialize_plugins(const GvkPipelineExplorerPluginInitializeInfo* pInitializeInfo);
    VkResult get_plugin_status() const;
    const GvkPipelineExplorerPerformanceQueryRequestInfo& get_request() const;
    VkResult submit_request(const std::filesystem::path& workspace, gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo>&& request);
    VkBool32 tool_cmd(VkDevice device, VkPipeline pipeline) const;
    VkResult get_plugin_counter_info(GvkPipelineExplorerPlugin plugin, GvkPipelineExplorerPluginCounterInfo* pCounterInfo);
    VkResult pre_process_vkCreateInstance(const GvkCommandBaseStructure& command) const;
    VkResult post_process_vkCreateInstance(const GvkCommandBaseStructure& command) const;
    VkResult pre_process_vkCreateDevice(const GvkCommandBaseStructure& command) const;
    VkResult post_process_vkCreateDevice(const GvkCommandBaseStructure& command) const;
    VkResult pre_process_range();
    VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const;
    VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const;
    VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const;
    VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const;
    VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const;
    VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const;
    VkResult post_process_range() const;

    static VkResult pre_process_range(void* pUserData);
    static VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData);
    static VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData);
    static VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData);
    static VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData);
    static VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pUserData);
    static VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pUserData);
    static VkResult post_process_range(void* pUserData);

private:
    std::unordered_map<void*, gvk::Auto<GvkPipelineExplorerPluginInfo>> mPlugins;
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> mRequest;
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
};

} // namespace pipeline_explorer
} // namespace gvk

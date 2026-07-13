
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

#include "gvk-pipeline-explorer/plugin-factory/plugin-manager.hpp"
#include "gvk-runtime.hpp"

#include <filesystem>

namespace gvk {
namespace pipeline_explorer {

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    for (const auto& pluginItr : mPlugins) {
        assert(pluginItr.second->pfnDestroyPlugin);
        pluginItr.second->pfnDestroyPlugin(pluginItr.second->pUserData);
        gvk_dlclose(pluginItr.first);
    }
}

VkResult PluginManager::initialize_plugins(const GvkPipelineExplorerPluginInitializeInfo* pInitializeInfo)
{
    (void)pInitializeInfo;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(mPlugins.empty() ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

#if WIN32
        std::filesystem::path modulePath;
        if (get_this_module_path(&modulePath)) {
            for (const auto& dirItr : std::filesystem::recursive_directory_iterator(modulePath.parent_path())) {
                if (dirItr.is_regular_file() && (dirItr.path().extension() == ".dll" || dirItr.path().extension() == ".so")) {
                    auto pPlugin = gvk_dlopen(dirItr.path().string().c_str());
                    if (pPlugin) {
                        auto pipelineExplorerPluginInfo = gvk::get_default<GvkPipelineExplorerPluginInfo>();
                        auto pfnGvkInitializePipelineExplorerPlugin = (PFN_gvkInitializePipelineExplorerPlugin)gvk_dlsym(pPlugin, "gvkInitializePipelineExplorerPlugin");
                        auto vkResult = pfnGvkInitializePipelineExplorerPlugin ? pfnGvkInitializePipelineExplorerPlugin(pInitializeInfo, &pipelineExplorerPluginInfo) : VK_ERROR_INITIALIZATION_FAILED;
                        if (vkResult == VK_SUCCESS) {
                            mPlugins[pPlugin] = pipelineExplorerPluginInfo;
                        } else {
                            gvk_dlclose(pPlugin);
                        }
                    }
                }
            }
        }
#else
        // TODO :
#endif // WIN32

    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::get_plugin_status() const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        if (!mPlugins.empty()) {
            gvk_result(mPlugins.size() == 1 ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT);
            const auto& pluginInfo = *mPlugins.begin()->second;
            gvk_result(pluginInfo.pfnGetStatus ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            return pluginInfo.pfnGetStatus(pluginInfo.pUserData);
        }
    } gvk_result_scope_end;
    return gvkResult;
}

const GvkPipelineExplorerPerformanceQueryRequestInfo& PluginManager::get_request() const
{
    return mRequest;
}

VkResult PluginManager::get_plugin_counter_info(GvkPipelineExplorerPlugin plugin, GvkPipelineExplorerPluginCounterInfo* pCounterInfo)
{
    (void)plugin;
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        if (!mPlugins.empty()) {
            gvk_result(mPlugins.size() == 1 ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT);
            const auto& pluginInfo = *mPlugins.begin()->second;
            gvk_result(pluginInfo.pfnGetCounterInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnGetCounterInfo(pCounterInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::submit_request(const std::filesystem::path& workspace, gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo>&& request)
{
    (void)workspace;
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        if (!mPlugins.empty()) {
            gvk_result(mPlugins.size() == 1 ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT);
            const auto& pluginInfo = *mPlugins.begin()->second;
            gvk_result(pluginInfo.pfnSubmitRequest ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnSubmitRequest(&*request, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkBool32 PluginManager::tool_cmd(VkDevice device, VkPipeline pipeline) const
{
    auto toolCmd = false;
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        if (!mPlugins.empty()) {
            gvk_result(mPlugins.size() == 1 ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT);
            const auto& pluginInfo = *mPlugins.begin()->second;
            gvk_result(pluginInfo.pfnToolCmd ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            if (pluginInfo.pfnToolCmd(device, pipeline, pluginInfo.pUserData)) {
                toolCmd = true;
            }
        }
    } gvk_result_scope_end;
    return !gvkResult && toolCmd;
}

VkResult PluginManager::pre_process_vkCreateInstance(const GvkCommandBaseStructure& command) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPreProcessCreateInstance ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPreProcessCreateInstance(&command, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_vkCreateInstance(const GvkCommandBaseStructure& command) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPostProcessCreateInstance ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPostProcessCreateInstance(&command, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_vkCreateDevice(const GvkCommandBaseStructure& command) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPreProcessCreateDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPreProcessCreateDevice(&command, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_vkCreateDevice(const GvkCommandBaseStructure& command) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPostProcessCreateDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPostProcessCreateDevice(&command, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPreProcessRange ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPreProcessRange(pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPreProcessCommandBuffers ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPreProcessCommandBuffers(&toolInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPreProcessCmd ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPreProcessCmd(&toolInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPostProcessCmd ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPostProcessCmd(&toolInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPostProcessCommandBuffers ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPostProcessCommandBuffers(&toolInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPreProcessQueueSubmission ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPreProcessQueueSubmission(&toolInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPostProcessQueueSubmission ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPostProcessQueueSubmission(&toolInfo, pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_range() const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        // NOTE : Plugin system currently only supports Intel MDAPI plugin
        // TODO : Support for custom plugins will be available in the future
        for (auto itr = mPlugins.begin(); itr != mPlugins.end(); ++itr) {
            const auto& pluginInfo = *itr->second;
            gvk_result(pluginInfo.pfnPostProcessRange ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
            gvk_result(pluginInfo.pfnPostProcessRange(pluginInfo.pUserData));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_range(void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->pre_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->pre_process_command_buffers(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->pre_process_cmd(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->post_process_cmd(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->post_process_command_buffers(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->pre_process_queue_submission(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->post_process_queue_submission(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult PluginManager::post_process_range(void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::PluginManager*)pUserData)->post_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk

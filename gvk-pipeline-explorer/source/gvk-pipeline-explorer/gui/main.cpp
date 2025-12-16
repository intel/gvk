
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

#include "gvk-pipeline-explorer/gui/gui-info.hpp"
#include "gvk-pipeline-explorer/gui/window-manager.hpp"
#include "gvk-pipeline-explorer/backend/utilities.hpp"

#include "gvk-command-structures.hpp"
#include "gvk-defines.hpp"
#include "gvk-environment.hpp"
#include "gvk-gui.hpp"
#include "gvk-handles.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-structures.hpp"
#include "gvk-system.hpp"

#include "boost/multiprecision/integer.hpp"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "implot.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

inline void process_outgoing_messages(GuiInfo& guiInfo)
{
    // Automatically send request to refresh available metrics on startup
    static bool sOnce;
    if (!sOnce) {
        sOnce = true;
        guiInfo.requestInfo.refreshAvailableMetrics = true;
    }

    // Set report path
    std::string reportPath;
    if (guiInfo.reportEnabled) {
        reportPath = (std::filesystem::path(guiInfo.workspaceInfo.workspace) / "reports").string();
        guiInfo.requestInfo.pReportPath = reportPath.c_str();
    }

    // Set selected metric ID
    guiInfo.requestInfo.device = guiInfo.selectedPipeline.get_dispatchable_handle();
    guiInfo.requestInfo.sampleMetricsPipeline = guiInfo.selectedPipeline.get_handle();
    GvkPipelineExplorerMetricId metricId{ guiInfo.enabledMetricsGroup, 0, 0, 0 };
    if (guiInfo.requestInfo.sampleMetricIdCount) {
        guiInfo.requestInfo.sampleMetricIdCount = 1;
        guiInfo.requestInfo.pSampleMetricIds = &metricId;
    }

    // Get paths
    auto decompilePipelinePath = get_pipeline_path(guiInfo, guiInfo.requestInfo.device, guiInfo.requestInfo.decompilePipeline);
    auto recompilePipelinePath = get_pipeline_path(guiInfo, guiInfo.requestInfo.device, guiInfo.requestInfo.recompilePipeline);
    auto experimentPipelinePath = get_pipeline_path(guiInfo, guiInfo.requestInfo.device, guiInfo.requestInfo.experimentPipeline);
    auto highlightPipelinePath = get_pipeline_path(guiInfo, guiInfo.requestInfo.device, guiInfo.requestInfo.highlightPipeline);
    guiInfo.requestInfo.pDecompilePipelinePath = !decompilePipelinePath.empty() ? decompilePipelinePath.c_str() : nullptr;
    guiInfo.requestInfo.pRecompilePipelinePath = !recompilePipelinePath.empty() ? recompilePipelinePath.c_str() : nullptr;
    guiInfo.requestInfo.pExperimentPipelinePath = !experimentPipelinePath.empty() ? experimentPipelinePath.c_str() : nullptr;
    guiInfo.requestInfo.pHighlightPipelinePath = !highlightPipelinePath.empty() ? highlightPipelinePath.c_str() : nullptr;

    // Submit request
    if (guiInfo.requestInfo.refreshActivePipelines ||
        guiInfo.requestInfo.refreshAvailableMetrics ||
        guiInfo.requestInfo.getApiCalls ||
        guiInfo.requestInfo.getGpuCalls ||
        (guiInfo.requestInfo.decompilePipeline && guiInfo.requestInfo.pDecompilePipelinePath) ||
        (guiInfo.requestInfo.recompilePipeline && guiInfo.requestInfo.pRecompilePipelinePath) ||
        (guiInfo.requestInfo.experimentPipeline && guiInfo.requestInfo.pExperimentPipelinePath) ||
        (guiInfo.requestInfo.highlightPipeline && guiInfo.requestInfo.pHighlightPipelinePath) ||
        (guiInfo.requestInfo.sampleMetricIdCount && guiInfo.requestInfo.pSampleMetricIds)) {
        auto vkResult = gvk::write_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", guiInfo.requestInfo);
        if (vkResult == VK_SUCCESS) {
            guiInfo.resultPending = true;
        }
        guiInfo.messages.clear();
    }

    // Reset request
    guiInfo.requestInfo.pReportPath = nullptr;
    guiInfo.requestInfo.device = VK_NULL_HANDLE;
    guiInfo.requestInfo.refreshActivePipelines = false;
    guiInfo.requestInfo.refreshAvailableMetrics = false;
    guiInfo.requestInfo.getApiCalls = false;
    guiInfo.requestInfo.getGpuCalls = false;
    guiInfo.requestInfo.sampleMetricsPipeline = VK_NULL_HANDLE;
    guiInfo.requestInfo.pDecompilePipelinePath = nullptr;
    guiInfo.requestInfo.decompilePipeline = VK_FALSE;
    guiInfo.requestInfo.pRecompilePipelinePath = nullptr;
    guiInfo.requestInfo.recompilePipeline = VK_FALSE;
    guiInfo.requestInfo.pExperimentPipelinePath = nullptr;
    guiInfo.requestInfo.experimentPipeline = VK_FALSE;
    guiInfo.requestInfo.pHighlightPipelinePath = nullptr;
    guiInfo.requestInfo.highlightPipeline = VK_FALSE;
    guiInfo.requestInfo.sampleMetricIdCount = 0;
    guiInfo.requestInfo.pSampleMetricIds = nullptr;
}

inline void process_incoming_messages(GuiInfo& guiInfo)
{
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerResultInfo> resultInfo;
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", resultInfo)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        for (uint32_t message_i = 0; message_i < resultInfo->messageCount; ++message_i) {
            guiInfo.messages += std::string(resultInfo->ppMessages[message_i]) + "\n";
        }
        if (resultInfo->pipelineResultCount) {
            guiInfo.activePipelines.clear();
        }
        for (uint32_t pipeline_i = 0; pipeline_i < resultInfo->pipelineResultCount; ++pipeline_i) {

            // Setup pipeline info
            const auto& pipelineResultInfo = resultInfo->pPipelineResults[pipeline_i];
            auto device = pipelineResultInfo.pipelineInfo.device;
            auto pipeline = pipelineResultInfo.pipelineInfo.pipeline;
            guiInfo.activePipelines.insert({ device, pipeline });
            auto& pipelineInfo = guiInfo.pipelineInfos[{ device, pipeline }];
            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
            boost::multiprecision::import_bits(pipelineInfo.uuid, pipelineResultInfo.pipelineInfo.uuid, pipelineResultInfo.pipelineInfo.uuid + GVK_PIPELINE_EXPLORER_UUID_SIZE);
            pipelineInfo.pipeline = { device, pipeline };
            pipelineInfo.bindPoint = pipelineResultInfo.pipelineInfo.bindPoint;
            pipelineInfo.bindPointStr = gvk::string::remove(gvk::to_string(pipelineResultInfo.pipelineInfo.bindPoint, printerFlags), "\"");
            pipelineInfo.uuidStr = uuid_to_string(pipelineResultInfo.pipelineInfo.uuid, 18);
            pipelineInfo.driverUUIDStr = uuid_to_string(pipelineResultInfo.pipelineInfo.driverUUID, 18);
            pipelineInfo.handleStr = gvk::to_hex_string(pipeline);
            pipelineInfo.name = pipelineResultInfo.pipelineInfo.pName;

            // Calculate pipeline color
            std::stringstream strStrm;
            strStrm << std::hex << pipelineInfo.uuidStr.substr(2, 6);
            uint32_t hexColorValue = 0;
            strStrm >> hexColorValue;
            pipelineInfo.highlightColor.x = (float)(hexColorValue >> 16 & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.y = (float)(hexColorValue >> 8 & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.z = (float)(hexColorValue & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.w = 1.0f;
            if (pipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                pipelineInfo.highlightColor.x = std::min(pipelineInfo.highlightColor.x, 0.5f);
                pipelineInfo.highlightColor.y = std::min(pipelineInfo.highlightColor.y, 0.5f);
                pipelineInfo.highlightColor.z = std::min(pipelineInfo.highlightColor.z, 0.5f);
            }

            // Set metric results
            for (uint32_t metricResult_i = 0; metricResult_i < pipelineResultInfo.metricResultCount; ++metricResult_i) {
                const auto& metricResultInfo = pipelineResultInfo.pMetricResults[metricResult_i];
                pipelineInfo.metrics[metricResultInfo.metricInfo.id] = metricResultInfo;
            }
        }
        // TODO : Insert sorted...
        guiInfo.sortedPipelines.clear();
        for (auto pipeline : guiInfo.activePipelines) {
            guiInfo.sortedPipelines.push_back(pipeline);
        }
        sort_pipelines(guiInfo);
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerAvailableMetricsInfo> pipelineExplorerAvailableMetricsInfo;
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", pipelineExplorerAvailableMetricsInfo)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        guiInfo.availableMetrics.clear();
        for (uint32_t metric_i = 0; metric_i < pipelineExplorerAvailableMetricsInfo->metricInfoCount; ++metric_i) {
            const auto& pipelineExplorerMetricInfo = pipelineExplorerAvailableMetricsInfo->pMetricInfos[metric_i];
            guiInfo.availableMetrics[(uint32_t)pipelineExplorerMetricInfo.id.x].push_back(pipelineExplorerMetricInfo);
        }

        // Get filters from available metrics
        for (const auto& metricsGroupItr : guiInfo.availableMetrics) {
            std::stringstream tokens;
            std::set<std::string> uniqueTokens;
            for (const auto& metricsInfo : metricsGroupItr.second) {
                for (const auto& token : gvk::string::split(metricsInfo->pName, " ")) {
                    if (uniqueTokens.insert(token).second) {
                        tokens << token << ";";
                    }
                }
            }
            guiInfo.metricsFilters.push_back({ tokens.str(), metricsGroupItr.first });
        }

        // Set filtered metrics to all available
        guiInfo.filteredMetrics = guiInfo.availableMetrics;
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerPerformanceCounterCollection> pipelineExplorerPerformanceCounterCollection;
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", pipelineExplorerPerformanceCounterCollection)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        guiInfo.performanceCountersInfo.available = pipelineExplorerPerformanceCounterCollection;
        guiInfo.performanceCountersInfo.filters.clear();
        guiInfo.performanceCountersInfo.scopes.clear();
        guiInfo.performanceCountersInfo.categories.clear();
        guiInfo.performanceCountersInfo.active.clear();
        guiInfo.performanceCountersInfo.enabled.clear();
        for (uint32_t i = 0; i < guiInfo.performanceCountersInfo.available->count; ++i) {
            std::stringstream tokens;
            std::set<std::string> uniqueTokens;
            for (const auto& token : gvk::string::split(guiInfo.performanceCountersInfo.available->pDescriptions[i].name, " ")) {
                if (uniqueTokens.insert(token).second) {
                    tokens << token << ";";
                }
            }
            guiInfo.performanceCountersInfo.filters.push_back({ tokens.str(), i });
            guiInfo.performanceCountersInfo.scopes[guiInfo.performanceCountersInfo.available->pCounters[i].scope] = true;
            guiInfo.performanceCountersInfo.categories[guiInfo.performanceCountersInfo.available->pDescriptions[i].category] = true;
            guiInfo.performanceCountersInfo.active.push_back(i);
            guiInfo.performanceCountersInfo.enabled.push_back(false);
        }
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

#ifdef WIN32
    // TODO : Rework all query request/result logic
    gvk::Auto<GvkCommandCollection> commandCollection;
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", "GvkApiCommandCollection", commandCollection)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        guiInfo.workspaceInfo.streamInfo.commandCollection = std::move(commandCollection);
        std::ofstream commandCollectionFile(std::filesystem::path(guiInfo.workspaceInfo.workspace) / "GvkApiCommandCollection.json");
        commandCollectionFile << gvk::to_string(guiInfo.workspaceInfo.streamInfo.commandCollection, pipeline_explorer::PrinterFlags) << std::endl;
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }

    // TODO : Rework all query request/result logic
    commandCollection.reset();
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", "GvkGpuCommandCollection", commandCollection)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        guiInfo.workspaceInfo.streamInfo.commandCollection = std::move(commandCollection);
        std::ofstream commandCollectionFile(std::filesystem::path(guiInfo.workspaceInfo.workspace) / "GvkGpuCommandCollection.json");
        commandCollectionFile << gvk::to_string(guiInfo.workspaceInfo.streamInfo.commandCollection, pipeline_explorer::PrinterFlags) << std::endl;
    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }
#else
    // TODO : Why doesn't this work on Linux?
    // TODO : Gotta rework structure utility includes anyway
#endif // WIN32

    // TODO : Rework all query request/result logic
    gvk::Auto<GvkPipelineExplorerAutoQueryResultInfo> autoQueryResultInfo;
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data", autoQueryResultInfo)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;

        guiInfo.activePipelines.clear();
        for (uint32_t pipeline_i = 0; pipeline_i < autoQueryResultInfo->pipelineResultCount; ++pipeline_i) {
            const auto& pipelineResultInfo = autoQueryResultInfo->pPipelineResults[pipeline_i];
            auto device = pipelineResultInfo.pipelineInfo.device;
            auto pipeline = pipelineResultInfo.pipelineInfo.pipeline;
            guiInfo.activePipelines.insert({ device, pipeline });
            auto& pipelineInfo = guiInfo.pipelineInfos[{ device, pipeline }];
            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
            boost::multiprecision::import_bits(pipelineInfo.uuid, pipelineResultInfo.pipelineInfo.uuid, pipelineResultInfo.pipelineInfo.uuid + GVK_PIPELINE_EXPLORER_UUID_SIZE);
            pipelineInfo.pipeline = { device, pipeline };
            pipelineInfo.bindPoint = pipelineResultInfo.pipelineInfo.bindPoint;
            pipelineInfo.bindPointStr = gvk::string::remove(gvk::to_string(pipelineResultInfo.pipelineInfo.bindPoint, printerFlags), "\"");
            pipelineInfo.uuidStr = uuid_to_string(pipelineResultInfo.pipelineInfo.uuid, 18);
            pipelineInfo.driverUUIDStr = uuid_to_string(pipelineResultInfo.pipelineInfo.driverUUID, 18);
            pipelineInfo.handleStr = gvk::to_hex_string(pipeline);
            pipelineInfo.name = pipelineResultInfo.pipelineInfo.pName;
            std::stringstream strStrm;
            strStrm << std::hex << pipelineInfo.uuidStr.substr(2, 6);
            uint32_t hexColorValue = 0;
            strStrm >> hexColorValue;
            pipelineInfo.highlightColor.x = (float)(hexColorValue >> 16 & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.y = (float)(hexColorValue >> 8 & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.z = (float)(hexColorValue & 0xFF) / 255.0f;
            pipelineInfo.highlightColor.w = 1.0f;
            if (pipelineInfo.bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                pipelineInfo.highlightColor.x = std::min(pipelineInfo.highlightColor.x, 0.5f);
                pipelineInfo.highlightColor.y = std::min(pipelineInfo.highlightColor.y, 0.5f);
                pipelineInfo.highlightColor.z = std::min(pipelineInfo.highlightColor.z, 0.5f);
            }
            assert(pipelineResultInfo.metricResultCount == 2);
            for (uint32_t metricResult_i = 0; metricResult_i < pipelineResultInfo.metricResultCount; ++metricResult_i) {
                auto metricResultInfo = pipelineResultInfo.pMetricResults[metricResult_i];
                pipelineInfo.metrics[metricResultInfo.metricInfo.id] = metricResultInfo;
            }
        }
        // TODO : Insert sorted...
        guiInfo.sortedPipelines.clear();
        for (auto pipeline : guiInfo.activePipelines) {
            guiInfo.sortedPipelines.push_back(pipeline);
        }
        sort_pipelines(guiInfo);

        guiInfo.apiCallInfo.commandCollection = autoQueryResultInfo->commands;
        guiInfo.apiCallInfo.commandDurations.clear();
        guiInfo.apiCallInfo.commandDurations.resize(autoQueryResultInfo->commandCount);
        for (uint32_t cmd_i = 0; cmd_i < autoQueryResultInfo->commandCount; ++cmd_i) {
            guiInfo.apiCallInfo.commandDurations[cmd_i] = autoQueryResultInfo->pCmdDurations[cmd_i];
        }

    } break;
    case VK_INCOMPLETE: {
        // assert(false && "TODO : Error handling");
    } break;
    case VK_NOT_READY:
    default: {
        // NOOP : No file to process
    } break;
    }
}

inline void load_workspace(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    std::filesystem::path path;
    if (get_this_module_path(&path)) {

        // TODO : Rework all query request/result logic
        gvk::Auto<GvkPipelineExplorerGuiInfo> pipelineExplorerGuiInfo;
        switch (gvk::read_serialized_structure(path.parent_path(), "GvkPipelineExplorerGuiInfo", pipelineExplorerGuiInfo, false)) {
        case VK_SUCCESS: {
            guiInfo.windowExtent = pipelineExplorerGuiInfo->extent;
            guiInfo.windowPosition = pipelineExplorerGuiInfo->position;
            guiInfo.fontScale = pipelineExplorerGuiInfo->fontScale ? pipelineExplorerGuiInfo->fontScale : 1.0f;
            if (!guiInfo.cliProvidedWorkspace) {
                guiInfo.workspaceInfo = pipelineExplorerGuiInfo->workspace;
                guiInfo.recentWorkspaceInfos.clear();
                guiInfo.recentWorkspaceInfos.reserve(pipelineExplorerGuiInfo->recentWorkspaceCount);
                for (uint32_t i = 0; i < pipelineExplorerGuiInfo->recentWorkspaceCount; ++i) {
                    if (pipelineExplorerGuiInfo->pRecentWorkspaces[i].pLaunch) {
                        guiInfo.recentWorkspaceInfos.push_back(pipelineExplorerGuiInfo->pRecentWorkspaces[i]);
                    }
                }
            }
        } break;
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
}

inline void save_workspace(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    guiInfo.saveRequired = false;
    std::filesystem::path path;
    if (get_this_module_path(&path)) {
        auto pipelineExplorerGuiInfo = gvk::get_default<GvkPipelineExplorerGuiInfo>();
        pipelineExplorerGuiInfo.extent = guiInfo.windowExtent;
        pipelineExplorerGuiInfo.position = guiInfo.windowPosition;
        pipelineExplorerGuiInfo.fontScale = guiInfo.fontScale;
        pipelineExplorerGuiInfo.workspace = guiInfo.workspaceInfo;
        std::vector<GvkPipelineExplorerWorkspaceInfo> pipelineExplorerWorkspaceInfos(guiInfo.recentWorkspaceInfos.size());
        for (uint32_t i = 0; i < guiInfo.recentWorkspaceInfos.size(); ++i) {
            if (!guiInfo.recentWorkspaceInfos[i].launch.empty()) {
                pipelineExplorerWorkspaceInfos.push_back(guiInfo.recentWorkspaceInfos[i]);
            }
        }
        pipelineExplorerGuiInfo.recentWorkspaceCount = (uint32_t)pipelineExplorerWorkspaceInfos.size();
        pipelineExplorerGuiInfo.pRecentWorkspaces = pipelineExplorerWorkspaceInfos.data();
        if (guiInfo.cliProvidedWorkspace) {
            pipelineExplorerGuiInfo.workspace = gvk::get_default<GvkPipelineExplorerWorkspaceInfo>();
            pipelineExplorerGuiInfo.recentWorkspaceCount = 0;
            pipelineExplorerGuiInfo.pRecentWorkspaces = nullptr;
        }
        gvk::write_serialized_structure(path.parent_path(), "GvkPipelineExplorerGuiInfo", pipelineExplorerGuiInfo, true);
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

using CmdLine = std::map<std::string, std::string>;
CmdLine get_cmd_line(int argc, const char* ppArgv[], CmdLine cmdLine = { })
{
    for (int i = 0; i < argc; ++i) {
        auto itr = cmdLine.insert({ std::string(ppArgv[i]), std::string()}).first;
        if (i < argc - 1 && gvk::string::starts_with(itr->first, "-")) {
            std::string value = ppArgv[i + 1];
            if (!value.empty()) {
                itr->second = value;
            }
            ++i;
        }
    }
    return cmdLine;
}

int main(int argc, const char* ppArgv[])
{
    auto cmdLine = get_cmd_line(argc, ppArgv, {
        { "-t", "Intel(R) GVK Pipeline Explorer" },
    });
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {

        // Configure guiInfo from cmd line args
        gvk::pipeline_explorer::gui::GuiInfo guiInfo{ };
        guiInfo.windowTitle = cmdLine["-t"];
        guiInfo.workspaceInfo.workspace = cmdLine["-w"];
        guiInfo.workspaceInfo.launch = cmdLine["-a"];
        if (!guiInfo.workspaceInfo.workspace.empty()) {
            std::filesystem::create_directories(std::filesystem::path(guiInfo.workspaceInfo.workspace) / ".data");
            guiInfo.cliProvidedWorkspace = true;
        }

        // TODO : gvk::get_default<>()
        guiInfo.requestInfo.warmupRangeCount = 4;
        guiInfo.requestInfo.queryRangeCount = 16;

        // Create gvk::Context
        auto applicationInfo = gvk::get_default<VkApplicationInfo>();
        applicationInfo.pApplicationName = guiInfo.windowTitle.c_str();
        auto instanceCreateInfo = gvk::get_default<VkInstanceCreateInfo>();
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        auto contextCreateInfo = gvk::get_default<gvk::Context::CreateInfo>();
        contextCreateInfo.pInstanceCreateInfo = &instanceCreateInfo;
        contextCreateInfo.loadWsiExtensions = VK_TRUE;
        gvk::Context context = VK_NULL_HANDLE;
        gvk_result(gvk::Context::create(&contextCreateInfo, nullptr, &context));

        // Get available layers
        uint32_t layerPropertyCount = 0;
        gvk_result(context.get<gvk::Instance>().get<gvk::DispatchTable>().gvkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr));
        guiInfo.layerProperties.resize(layerPropertyCount, gvk::get_default<VkLayerProperties>());
        gvk_result(context.get<gvk::Instance>().get<gvk::DispatchTable>().gvkEnumerateInstanceLayerProperties(&layerPropertyCount, guiInfo.layerProperties.data()));

        // Get gvk::Context objects
        const auto& instance = context.get<gvk::Instance>();
        const auto& device = context.get<gvk::Devices>()[0];
        const auto& queue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
        const auto& commandBuffer = context.get<gvk::CommandBuffers>()[0];

        // Load workspace
        gvk::pipeline_explorer::gui::load_workspace(guiInfo);

        // Declare window manager
        gvk::pipeline_explorer::gui::Window::Manager windowManager;

        // Create gvk::system::Surface
        auto systemSurfaceCreateInfo = gvk::get_default<gvk::system::Surface::CreateInfo>();
        systemSurfaceCreateInfo.extent[0] = guiInfo.windowExtent.width;
        systemSurfaceCreateInfo.extent[1] = guiInfo.windowExtent.height;
        systemSurfaceCreateInfo.position[0] = guiInfo.windowPosition.x;
        systemSurfaceCreateInfo.position[1] = guiInfo.windowPosition.y;
        systemSurfaceCreateInfo.pTitle = applicationInfo.pApplicationName;
        gvk::system::Surface systemSurface = VK_NULL_HANDLE;
        gvk_result((VkResult)gvk::system::Surface::create(&systemSurfaceCreateInfo, &systemSurface));

        // Create gvk::Surface
        const VkBaseInStructure* pSurfaceCreateInfo = nullptr;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        auto win32SurfaceCreateInfo = gvk::get_default<VkWin32SurfaceCreateInfoKHR>();
        win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
        win32SurfaceCreateInfo.hwnd = systemSurface.get<gvk::system::Surface::PlatformInfo>().hwnd;
        pSurfaceCreateInfo = (VkBaseInStructure*)&win32SurfaceCreateInfo;
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        auto xlibSurfaceCreateInfo = gvk::get_default<VkXlibSurfaceCreateInfoKHR>();
        xlibSurfaceCreateInfo.dpy = systemSurface.get<gvk::system::Surface::PlatformInfo>().x11Display;
        xlibSurfaceCreateInfo.window = systemSurface.get<gvk::system::Surface::PlatformInfo>().x11Window;
        pSurfaceCreateInfo = (VkBaseInStructure*)&xlibSurfaceCreateInfo;
#endif
        gvk::SurfaceKHR surface = VK_NULL_HANDLE;
        gvk_result(gvk::SurfaceKHR::create(instance, pSurfaceCreateInfo, nullptr, &surface));

        // Create gvk::wsi::Context
        auto wsiContextCreateInfo = gvk::get_default<gvk::wsi::Context::CreateInfo>();
        wsiContextCreateInfo.queueFamilyIndex = gvk::get_queue_family(device, 0).queues[0].get<VkDeviceQueueCreateInfo>().queueFamilyIndex;
        wsiContextCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        wsiContextCreateInfo.sampleCount = VK_SAMPLE_COUNT_64_BIT;
        gvk::wsi::Context wsiContext = VK_NULL_HANDLE;
        gvk_result(gvk::wsi::Context::create(device, surface, &wsiContextCreateInfo, nullptr, &wsiContext));

        // Create gvk::gui::Renderer
        gvk::gui::Renderer guiRenderer = VK_NULL_HANDLE;
        gvk_result(gvk::gui::Renderer::create(device, queue, commandBuffer, wsiContext.get<gvk::RenderPass>(), nullptr, &guiRenderer));
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // TODO : Automatically handle ImGui add-ons
        ImPlot::CreateContext();

        // Main loop
        gvk::system::Clock clock;
        while (!(systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::CloseRequested)) {
            gvk::system::Surface::update();
            clock.update();
            auto deltaTime = clock.elapsed<gvk::system::Seconds<float>>();

            // Get window extent and position
            int32_t width = 0;
            int32_t height = 0;
            systemSurface.get_window_extent(&width, &height);
            guiInfo.windowExtent = { (uint32_t)width, (uint32_t)height };
            systemSurface.get_window_position(&guiInfo.windowPosition.x, &guiInfo.windowPosition.y);

            // Acquire next image
            gvk::wsi::AcquiredImageInfo acquiredImageInfo{};
            gvk::RenderTarget acquiredImageRenderTarget = VK_NULL_HANDLE;
            auto wsiStatus = wsiContext.acquire_next_image(UINT64_MAX, VK_NULL_HANDLE, &acquiredImageInfo, &acquiredImageRenderTarget);
            if (wsiStatus == VK_SUCCESS || wsiStatus == VK_SUBOPTIMAL_KHR) {
                const auto& extent = wsiContext.get<gvk::SwapchainKHR>().get<VkSwapchainCreateInfoKHR>().imageExtent;
                const auto& input = systemSurface.get<gvk::system::Input>();

                // Handle ImGui io and events
                auto imguiCursor = ImGui::GetMouseCursor();
                if (imguiCursor == ImGuiMouseCursor_None || ImGui::GetIO().MouseDrawCursor) {
                    systemSurface.set(gvk::system::Surface::CursorMode::Hidden);
                } else {
                    switch (imguiCursor) {
                    case ImGuiMouseCursor_Arrow: { systemSurface.set(gvk::system::Surface::CursorType::Arrow); } break;
                    case ImGuiMouseCursor_TextInput: { systemSurface.set(gvk::system::Surface::CursorType::IBeam); } break;
                    case ImGuiMouseCursor_Hand: { systemSurface.set(gvk::system::Surface::CursorType::Hand); } break;
                    case ImGuiMouseCursor_ResizeNS: { systemSurface.set(gvk::system::Surface::CursorType::ResizeNS); } break;
                    case ImGuiMouseCursor_ResizeEW: { systemSurface.set(gvk::system::Surface::CursorType::ResizeEW); } break;
                    case ImGuiMouseCursor_ResizeAll: { systemSurface.set(gvk::system::Surface::CursorType::ResizeAll); } break;
                    case ImGuiMouseCursor_ResizeNESW: { systemSurface.set(gvk::system::Surface::CursorType::ResizeNESW); } break;
                    case ImGuiMouseCursor_ResizeNWSE: { systemSurface.set(gvk::system::Surface::CursorType::ResizeNWSE); } break;
                    case ImGuiMouseCursor_NotAllowed: { systemSurface.set(gvk::system::Surface::CursorType::NotAllowed); } break;
                    default: { } break;
                    }
                }
                if (systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::GainedFocus) {
                    ImGui::GetIO().AddFocusEvent(true);
                }
                if (systemSurface.get<gvk::system::Surface::StatusFlags>() & gvk::system::Surface::LostFocus) {
                    ImGui::GetIO().AddFocusEvent(false);
                }

                // Process incoming messages from backend
                gvk::pipeline_explorer::gui::process_incoming_messages(guiInfo);

                // Prepare a gvk::gui::Renderer::BeginInfo
                const auto& textStream = systemSurface.get<gvk::system::Surface::TextStream>();
                const auto& droppedPaths = systemSurface.get<gvk::system::Surface::DroppedPaths>();
                auto guiRendererBeginInfo = gvk::get_default<gvk::gui::Renderer::BeginInfo>();
                guiRendererBeginInfo.deltaTime = deltaTime;
                guiRendererBeginInfo.extent = { (float)extent.width, (float)extent.height };
                guiRendererBeginInfo.pInput = &input;
                guiRendererBeginInfo.textStreamCodePointCount = (uint32_t)textStream.size();
                guiRendererBeginInfo.pTextStreamCodePoints = !textStream.empty() ? textStream.data() : nullptr;
                guiRendererBeginInfo.pDragDropPath = !droppedPaths.empty() ? droppedPaths[0].c_str() : nullptr;
                guiRenderer.begin_gui(guiRendererBeginInfo);
                windowManager.on_gui(guiInfo);
                gvk_result(guiRenderer.end_gui(acquiredImageInfo.index));

                // Process outgoing messages to backend
                process_outgoing_messages(guiInfo);

                // Reset on workload close
                if (guiInfo.applicationInfo.closed) {
                    ///////////////////////////////////////////////////////////////////////////////
                    // TODO : Unify application shutdown and stream shutdown
                    guiInfo.requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
                    guiInfo.requestInfo.warmupRangeCount = 4;
                    guiInfo.requestInfo.queryRangeCount = 16;
                    guiInfo.activePipelines.clear();
                    guiInfo.sortedPipelines.clear();
                    guiInfo.pipelineInfos.clear();
                    guiInfo.availableMetrics.clear();
                    guiInfo.filteredMetrics.clear();
                    guiInfo.metricsFilters.clear();
                    guiInfo.metricsAnyOfFilter.clear();
                    guiInfo.metricsAllOfFilter.clear();
                    guiInfo.performanceCountersInfo.reset();
                    guiInfo.pipelineStatisticsQueryInfo.reset();
                    guiInfo.pluginPerformanceCounterInfo.reset();
                    guiInfo.selectedPipeline = { };
                    guiInfo.enabledMetricsGroup = 0;
                    guiInfo.applicationInfo = { };
                    guiInfo.apiCallInfo.reset();
                    guiInfo.resultPending = false;
                    windowManager.clear();
                    ///////////////////////////////////////////////////////////////////////////////
                }

                // Render GUI
                auto renderPassBeginInfo = acquiredImageRenderTarget.get<VkRenderPassBeginInfo>();
                gvk::CommandBuffer acquiredImageCommandBuffer = acquiredImageInfo.commandBuffer;
                gvk_result(acquiredImageCommandBuffer.BeginCommandBuffer(&gvk::get_default<VkCommandBufferBeginInfo>()));
                {
                    acquiredImageCommandBuffer.CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        VkRect2D scissor{ { }, renderPassBeginInfo.renderArea.extent };
                        acquiredImageCommandBuffer.CmdSetScissor(0, 1, &scissor);
                        VkViewport viewport{ 0, 0, (float)scissor.extent.width, (float)scissor.extent.height, 0, 1 };
                        acquiredImageCommandBuffer.CmdSetViewport(0, 1, &viewport);
                        guiRenderer.record_cmds(acquiredImageCommandBuffer, acquiredImageInfo.index);
                    }
                    acquiredImageCommandBuffer.CmdEndRenderPass();
                }
                gvk_result(acquiredImageCommandBuffer.EndCommandBuffer());

                // Submit and present
                gvk_result(queue.QueueSubmit(1, &wsiContext.get<VkSubmitInfo>(acquiredImageInfo), acquiredImageInfo.fence));
                wsiStatus = wsiContext.queue_present(queue, &acquiredImageInfo);
                gvk_result((wsiStatus == VK_SUBOPTIMAL_KHR || wsiStatus == VK_ERROR_OUT_OF_DATE_KHR) ? VK_SUCCESS : wsiStatus);
            }
            if (guiInfo.saveRequired) {
                save_workspace(guiInfo);
            }
        }

#ifdef VK_USE_PLATFORM_WIN32_KHR
        // TODO : This should probably be managed by the WorkspaceWindow
        if (guiInfo.applicationInfo.processInformation.hProcess) {
            TerminateProcess(guiInfo.applicationInfo.processInformation.hProcess, 0);
        }
#endif

        save_workspace(guiInfo);
        gvk_result(device.DeviceWaitIdle());

        // TODO : Automatically handle ImGui add-ons
        ImPlot::DestroyContext();

    } gvk_result_scope_end;
    return gvkResult;
}

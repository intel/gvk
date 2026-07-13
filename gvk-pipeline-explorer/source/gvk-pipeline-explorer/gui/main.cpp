
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
#include "gvk-runtime.hpp"
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
    // Set report path
    std::string reportPath;
    if (guiInfo.reportEnabled) {
        reportPath = (std::filesystem::path(guiInfo.workspace) / "reports").string();
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
        guiInfo.requestInfo.getApiCalls ||
        guiInfo.requestInfo.getGpuCalls ||
        (guiInfo.requestInfo.decompilePipeline && guiInfo.requestInfo.pDecompilePipelinePath) ||
        (guiInfo.requestInfo.recompilePipeline && guiInfo.requestInfo.pRecompilePipelinePath) ||
        (guiInfo.requestInfo.experimentPipeline && guiInfo.requestInfo.pExperimentPipelinePath) ||
        (guiInfo.requestInfo.highlightPipeline && guiInfo.requestInfo.pHighlightPipelinePath) ||
        (guiInfo.requestInfo.sampleMetricIdCount && guiInfo.requestInfo.pSampleMetricIds)) {
        auto vkResult = gvk::write_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", guiInfo.requestInfo);
        if (vkResult == VK_SUCCESS) {
            guiInfo.resultPending = true;
        }
        guiInfo.messages.clear();
    }

    // Reset request
    guiInfo.requestInfo.pReportPath = nullptr;
    guiInfo.requestInfo.device = VK_NULL_HANDLE;
    guiInfo.requestInfo.refreshActivePipelines = false;
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
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", resultInfo)) {
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
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", pipelineExplorerAvailableMetricsInfo)) {
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
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", pipelineExplorerPerformanceCounterCollection)) {
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
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", "GvkApiCommandCollection", commandCollection)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        guiInfo.commandCollection = std::move(commandCollection);
        std::ofstream commandCollectionFile(std::filesystem::path(guiInfo.workspace) / "GvkApiCommandCollection.json");
        commandCollectionFile << gvk::to_string(guiInfo.commandCollection, pipeline_explorer::PrinterFlags) << std::endl;
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
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", "GvkGpuCommandCollection", commandCollection)) {
    case VK_SUCCESS: {
        guiInfo.resultPending = false;
        guiInfo.commandCollection = std::move(commandCollection);
        std::ofstream commandCollectionFile(std::filesystem::path(guiInfo.workspace) / "GvkGpuCommandCollection.json");
        commandCollectionFile << gvk::to_string(guiInfo.commandCollection, pipeline_explorer::PrinterFlags) << std::endl;
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
    switch (gvk::read_serialized_structure(std::filesystem::path(guiInfo.workspace) / ".data", autoQueryResultInfo)) {
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

inline void process_incoming_messages_ipc(GuiInfo& guiInfo)
{
    (void)guiInfo;
#ifdef GVK_PLATFORM_WINDOWS

    // Handle named pipe connect / disconnect
    if (guiInfo.ipcPipe) {
        if (guiInfo.ipcPipe.is_accepted()) {
            // New layer instance connected — configure the messenger
            auto h = guiInfo.ipcPipe.get_handle();
            guiInfo.ipcMessenger.set_read_pipe(h);
            guiInfo.ipcMessenger.set_write_pipe(h);
            for (const auto& message : guiInfo.startupIpcMessages) {
                guiInfo.ipcMessenger.write(message.text.c_str(), (uint32_t)message.data.size(), message.data.data());
            }
        } else if (guiInfo.ipcMessenger.get_read_pipe()) {
            // Check if the current client has disconnected
            DWORD bytes = 0;
            if (!PeekNamedPipe(guiInfo.ipcMessenger.get_read_pipe(), nullptr, 0, nullptr, &bytes, nullptr)) {
                auto error = GetLastError();
                if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
                    guiInfo.ipcMessenger.reset();
                    guiInfo.ipcPipe.disconnect();
                    guiInfo.ipcPipe.begin_accept();
                }
            }
        }
    }

    for (auto&& message : guiInfo.ipcMessenger.read()) {
        if (message.text == "gvk::pipeline_explorer::IpcMessenger started") {
            // NOTE : This is to handle applications that create/destroy multiple instances
            //  on startup.  This ensures that each instance receives the startup messages.
            for (const auto& startupMessage : guiInfo.startupIpcMessages) {
                guiInfo.ipcMessenger.write(startupMessage.text.c_str(), (uint32_t)startupMessage.data.size(), startupMessage.data.data());
            }
        } else if (gvk::string::starts_with(message.text, "message")) {
            guiInfo.messages += std::string((char*)message.data.data(), message.data.size()) + "\n";
        } else {
            guiInfo.incomingIpcMessages[message.text].push_back(std::move(message));
        }
    }
#endif // GVK_PLATFORM_WINDOWS
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
        guiInfo.workspace = cmdLine["-w"];
        // guiInfo.launch = cmdLine["-a"];
        if (!guiInfo.workspace.empty()) {
            std::filesystem::create_directories(std::filesystem::path(guiInfo.workspace) / ".data");
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
        for (const auto& layerProperties : guiInfo.layerProperties) {
            if (!strcmp(layerProperties.layerName, "VK_LAYER_KHRONOS_validation")) {
                guiInfo.validationLayerAvailable = true;
            }
        }

        // Get gvk::Context objects
        const auto& instance = context.get<gvk::Instance>();
        const auto& device = context.get<gvk::Devices>()[0];
        const auto& queue = gvk::get_queue_family(context.get<gvk::Devices>()[0], 0).queues[0];
        const auto& commandBuffer = context.get<gvk::CommandBuffers>()[0];

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

#if 0
        // Load Font Awesome for icons
        // FROM : https://github.com/juliettef/IconFontCppHeaders
        // TODO : Manage fonts via gvk::gui::Renderer and support dynamic font sizes
        ImGui::GetIO().Fonts->AddFontDefault();
        float baseFontSize = 13.0f; // 13.0f is the size of the default font. TODO : Make font size dynamic
        float iconFontSize = baseFontSize * 2.0f / 3.0f; // Font Awesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
        // Merge in icons from Font Awesome
        static const int ICON_MIN_FA = 0xe005;
        static const int ICON_MAX_16_FA = 0xf8ff;
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = iconFontSize;
        ImGui::GetIO().Fonts->AddFontFromFileTTF("fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges);
#endif

        // Create gvk::gui::Renderer
        gvk::gui::Renderer guiRenderer = VK_NULL_HANDLE;
        gvk_result(gvk::gui::Renderer::create(device, queue, commandBuffer, wsiContext.get<gvk::RenderPass>(), nullptr, &guiRenderer));
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // TODO : Automatically handle ImGui add-ons
        ImPlot::CreateContext();

        // Create WindowManager
        // NOTE : Must be created after gvk::gui::Renderer because ImGui must be initialized
        gvk::pipeline_explorer::gui::Window::Manager windowManager;

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

            // TODO : Documentation
            // TODO : Queue?  Should the messenger itself handle this?
#ifdef GVK_PLATFORM_WINDOWS
            guiInfo.incomingIpcMessages.clear();
            if (guiInfo.workload) {
                gvk::pipeline_explorer::gui::process_outgoing_messages(guiInfo);
                gvk::pipeline_explorer::gui::process_incoming_messages_ipc(guiInfo);
                gvk::pipeline_explorer::gui::process_incoming_messages(guiInfo);
            }
#endif

            // TODO : Documentation
            windowManager.on_update(guiInfo);

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

                // Render GUI
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

#ifdef GVK_PLATFORM_WINDOWS
            // Reset on workload close
            std::lock_guard<std::mutex> lock(guiInfo.workloadMutex);
            if (!guiInfo.workload && guiInfo.onWorkloadShutdown) {
                // TODO : Hook up to on_save()
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                windowManager.on_terminate(guiInfo);
                guiInfo.onWorkloadShutdown();
                guiInfo.onWorkloadShutdown = nullptr;
            }
#endif // GVK_PLATFORM_WINDOWS
        }

#ifdef GVK_PLATFORM_WINDOWS
        // Shutdown workload
        guiInfo.workload.reset();
#endif

        // TODO : Automatically handle ImGui add-ons
        ImPlot::DestroyContext();

        // Destroy gvk::gui::Renderer
        // NOTE : Explicitly destroying guiRenderer so ImGuiSettingsHandler::WriteAllFn
        //  will be called for the last time before any dtors are called
        guiRenderer = gvk::nullref;

        // Make sure Vulkan resources are done being used before tearing everything down
        gvk_result(device.DeviceWaitIdle());
    } gvk_result_scope_end;
    return gvkResult;
}

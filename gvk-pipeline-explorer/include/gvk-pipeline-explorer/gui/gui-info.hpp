
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

#include "gvk-pipeline-explorer/utilities.hpp"
#include "gvk-reference/handle-id.hpp"
#include "gvk-defines.hpp"
#include "gvk-gui.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-structures.hpp"

#include "boost/multiprecision/integer.hpp"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <codecvt>
#include <locale>
#include <Psapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

class ApplicationInfo final
{
public:
#ifdef VK_USE_PLATFORM_WIN32_KHR
    PROCESS_INFORMATION processInformation{ };
    HANDLE waitHandle{ };
#endif
    bool closed{ };
};

class WorkspaceInfo final
{
public:
    WorkspaceInfo() = default;

    WorkspaceInfo(const GvkPipelineExplorerWorkspaceInfo& pipelineExplorerWorkspaceInfo)
        : waitForDebugger{ (bool)pipelineExplorerWorkspaceInfo.waitForDebugger }
        , openTerminal{ (bool)pipelineExplorerWorkspaceInfo.openTerminal }
        , autoWorkingDirectory{ (bool)pipelineExplorerWorkspaceInfo.autoWorkingDirectory }
        , autoWorkspace{ (bool)pipelineExplorerWorkspaceInfo.autoWorkspace }
        , launch{ pipelineExplorerWorkspaceInfo.pLaunch ? pipelineExplorerWorkspaceInfo.pLaunch : std::string() }
        , target{ pipelineExplorerWorkspaceInfo.pTarget ? pipelineExplorerWorkspaceInfo.pTarget : std::string() }
        , args{ pipelineExplorerWorkspaceInfo.pArgs ? pipelineExplorerWorkspaceInfo.pArgs : std::string() }
        , workingDirectory{ pipelineExplorerWorkspaceInfo.pWorkingDirectory ? pipelineExplorerWorkspaceInfo.pWorkingDirectory : std::string() }
        , workspace{ pipelineExplorerWorkspaceInfo.pWorkspace ? pipelineExplorerWorkspaceInfo.pWorkspace : std::string() }
    {
    }

    WorkspaceInfo& operator=(const WorkspaceInfo& other) = default;

    operator GvkPipelineExplorerWorkspaceInfo() const
    {
        auto pipelineExplorerWorkspaceInfo = gvk::get_default<GvkPipelineExplorerWorkspaceInfo>();
        pipelineExplorerWorkspaceInfo.waitForDebugger = waitForDebugger;
        pipelineExplorerWorkspaceInfo.openTerminal = openTerminal;
        pipelineExplorerWorkspaceInfo.autoWorkingDirectory = autoWorkingDirectory;
        pipelineExplorerWorkspaceInfo.autoWorkspace = autoWorkspace;
        pipelineExplorerWorkspaceInfo.pLaunch = !launch.empty() ? launch.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pTarget = !target.empty() ? target.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pArgs = !args.empty() ? args.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pWorkingDirectory = !workingDirectory.empty() ? workingDirectory.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pWorkspace = !workspace.empty() ? workspace.c_str() : nullptr;
        return pipelineExplorerWorkspaceInfo;
    }

    bool operator==(const WorkspaceInfo& other) const
    {
        return (GvkPipelineExplorerWorkspaceInfo)*this == (GvkPipelineExplorerWorkspaceInfo)other;
    }

    bool operator!=(const WorkspaceInfo& other) const
    {
        return !(*this == other);
    }

    bool waitForDebugger{ };
    bool openTerminal{ };
    bool autoWorkingDirectory{ true };
    bool autoWorkspace{ true };
    std::string launch;
    std::string target;
    std::string args;
    std::string workingDirectory;
    std::string workspace;
};

class PipelineInfo final
{
public:
    boost::multiprecision::uint256_t uuid;
    boost::multiprecision::uint256_t driverUUID;
    gvk::HandleId<VkDevice, VkPipeline> pipeline;
    VkPipelineBindPoint bindPoint{ };
    std::string bindPointStr;
    std::string uuidStr;
    std::string driverUUIDStr;
    std::string handleStr;
    std::string name;
    std::unordered_set<std::string> labels;
    bool experimentEnabled{ };
    bool highlightEnabled{ };
    bool sampleMetrics{ };
    ImVec4 highlightColor{ 1, 0, 1, 1 };
    bool infoWriteEnabled{ true };
    std::map<GvkPipelineExplorerMetricId, gvk::Auto<GvkPipelineExplorerMetricResultInfo>> metrics;
};

class GuiInfo final
{
public:
    std::string windowTitle;
    GvkPipelineExplorerRequestInfo requestInfo{ gvk::get_default<GvkPipelineExplorerRequestInfo>() };
    std::unordered_set<gvk::HandleId<VkDevice, VkPipeline>> activePipelines;
    std::vector<gvk::HandleId<VkDevice, VkPipeline>> sortedPipelines;
    std::vector<ImGuiTableColumnSortSpecs> pipelineSortSpecs;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, PipelineInfo> pipelineInfos;
    std::map<uint32_t, std::vector<gvk::Auto<GvkPipelineExplorerMetricInfo>>> availableMetrics;
    std::map<uint32_t, std::vector<gvk::Auto<GvkPipelineExplorerMetricInfo>>> filteredMetrics;
    std::vector<std::pair<std::string, uint32_t>> metricsFilters;
    std::string metricsAnyOfFilter;
    std::string metricsAllOfFilter;
    gvk::HandleId<VkDevice, VkPipeline> selectedPipeline;
    uint32_t enabledMetricsGroup{ };
    bool buildDefaultDockSpace{ true };
    bool reportEnabled{ false };
    bool cliProvidedWorkspace{ };
    bool saveRequired{ };
    std::string messages;
    VkExtent2D windowExtent{ };
    VkOffset2D windowPosition{ };
    float fontScale{ 1.0f };
    ApplicationInfo applicationInfo{ };
    WorkspaceInfo workspaceInfo{ };
    std::vector<WorkspaceInfo> recentWorkspaceInfos;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
inline VOID CALLBACK process_wait_callback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
    (void)TimerOrWaitFired;
    if (lpParameter) {
        auto& guiInfo = *(GuiInfo*)lpParameter;
        guiInfo.messages += "INFO : " + guiInfo.workspaceInfo.launch + " closed\n";
        guiInfo.applicationInfo.closed = true;
    }
}

inline std::string get_target(const WorkspaceInfo& workspaceInfo)
{
    std::string target;
    if (!workspaceInfo.launch.empty()) {
        auto applicationName = std::filesystem::path(workspaceInfo.launch).filename().replace_extension().string();
        target = !workspaceInfo.target.empty() ? workspaceInfo.target : applicationName;
    }
    return target;
}

inline std::filesystem::path get_default_workspace_path(const WorkspaceInfo& workspaceInfo)
{
    std::filesystem::path workspacePath;
    if (!workspaceInfo.launch.empty()) {
        PWSTR pDocumentsPath = NULL;
        auto hResult = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pDocumentsPath);
        auto target = get_target(workspaceInfo) + "-pipeline-explorer";
        workspacePath = (SUCCEEDED(hResult) && pDocumentsPath) ? std::filesystem::path(pDocumentsPath) / "GPA" / target : target;
        CoTaskMemFree(pDocumentsPath);
    }
    return workspacePath;
}

inline std::filesystem::path get_default_working_directory(const WorkspaceInfo& workspaceInfo)
{
    std::filesystem::path workingDirectory;
    if (!workspaceInfo.launch.empty()) {
        workingDirectory = std::filesystem::path(workspaceInfo.launch).parent_path();
    }
    return workingDirectory;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

inline const std::string& get_bind_point_label(VkPipelineBindPoint bindPoint)
{
    static std::unordered_map<VkPipelineBindPoint, std::string> sBindPointLabels;
    auto itr = sBindPointLabels.find(bindPoint);
    if (itr == sBindPointLabels.end()) {
        auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
        auto label = gvk::string::remove(gvk::to_string(bindPoint, printerFlags), "\"");
        itr = sBindPointLabels.insert({ bindPoint, label }).first;
    }
    return itr->second;
}

inline const std::vector<std::string>& get_validation_layer_setting_names()
{
    static const std::vector<std::string> sValidationLayerSettingNames{
        /* BOOL      : true                                            */ "VK_LAYER_FINE_GRAINED_LOCKING",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VALIDATE_CORE",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_IMAGE_LAYOUT",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_COMMAND_BUFFER",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_OBJECT_IN_USE",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_QUERY",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_SHADERS",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_CHECK_SHADERS_CACHING",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_UNIQUE_HANDLES",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_OBJECT_LIFETIME",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_STATELESS_PARAM",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_THREAD_SAFETY",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VALIDATE_SYNC",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_SYNC_QUEUE_SUBMIT",
        /* ENUM      : GPU_BASED_NONE                                  */  // "VK_KHRONOS_VALIDATION_VALIDATE_GPU_BASED",
        /* BOOL      : true                                            */  // "VK_KHRONOS_VALIDATION_PRINTF_TO_STDOUT",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_PRINTF_VERBOSE",
        /* INT       : 1024                                            */  // "VK_KHRONOS_VALIDATION_PRINTF_BUFFER_SIZE",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_RESERVE_BINDING_SLOT",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VMA_LINEAR_OUTPUT",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_GPUAV_DESCRIPTOR_CHECKS",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_WARN_ON_ROBUST_OOB",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_VALIDATE_INDIRECT_BUFFER",
        /* BOOL      : true                                            */ "VK_KHRONOS_VALIDATION_USE_INSTRUMENTED_SHADER_CACHE",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_SELECT_INSTRUMENTED_SHADERS",
        /* INT       : 10000                                           */  // "VK_KHRONOS_VALIDATION_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_ARM",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_AMD",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_IMG",
        /* BOOL      : false                                           */  // "VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_NVIDIA",
        /* FLAGS     : VK_DBG_LAYER_ACTION_LOG_MSG                     */  // "VK_KHRONOS_VALIDATION_DEBUG_ACTION",
        /* SAVE_FILE : stdout                                          */  // "VK_KHRONOS_VALIDATION_LOG_FILENAME",
        /* FLAGS     : error                                           */  // "VK_KHRONOS_VALIDATION_REPORT_FLAGS",
        /* BOOL      : true                                            */  // "VK_KHRONOS_VALIDATION_ENABLE_MESSAGE_LIMIT",
        /* INT       : 10                                              */  // "VK_LAYER_DUPLICATE_MESSAGE_LIMIT",
        /* LIST      :                                                 */  // "VK_LAYER_MESSAGE_ID_FILTER",
        /* FLAGS     : VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT */  // "VK_LAYER_DISABLES",
        /* FLAGS     :                                                 */  // "VK_LAYER_ENABLES",
    };
    return sValidationLayerSettingNames;
}

inline void sort_pipelines(GuiInfo& guiInfo)
{
    std::sort(
        guiInfo.sortedPipelines.begin(),
        guiInfo.sortedPipelines.end(),
        [&](const gvk::HandleId<VkDevice, VkPipeline>& lhs, const gvk::HandleId<VkDevice, VkPipeline>& rhs)
        {
            const auto& lhsPipelineInfo = guiInfo.pipelineInfos[lhs];
            const auto& rhsPipelineInfo = guiInfo.pipelineInfos[rhs];
            for (const auto& pipelineSortSpec : guiInfo.pipelineSortSpecs) {
                auto ascending = pipelineSortSpec.SortDirection == ImGuiSortDirection_Ascending;
                switch (pipelineSortSpec.ColumnUserID) {
                case 0: { return ascending ? lhsPipelineInfo.uuid        < rhsPipelineInfo.uuid        : lhsPipelineInfo.uuid        > rhsPipelineInfo.uuid; } break;
                case 1: { return ascending ? lhsPipelineInfo.driverUUID  < rhsPipelineInfo.driverUUID  : lhsPipelineInfo.driverUUID  > rhsPipelineInfo.driverUUID; } break;
                case 2: { return ascending ? lhsPipelineInfo.pipeline    < rhsPipelineInfo.pipeline    : lhsPipelineInfo.pipeline    > rhsPipelineInfo.pipeline; } break;
                case 3: { return ascending ? lhsPipelineInfo.name        < rhsPipelineInfo.name        : lhsPipelineInfo.name        > rhsPipelineInfo.name; } break;
                case 4: { return ascending ? lhsPipelineInfo.bindPoint   < rhsPipelineInfo.bindPoint   : lhsPipelineInfo.bindPoint   > rhsPipelineInfo.bindPoint; } break;
                case 5: {
                    // TODO : Automate metrics columns/sorting
                    double lhsExecutionCount = 0;
                    for (const auto& metrics : lhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                            lhsExecutionCount = metrics.second->average;
                            break;
                        }
                    }
                    double rhsExecutionCount = 0;
                    for (const auto& metrics : rhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                            rhsExecutionCount = metrics.second->average;
                            break;
                        }
                    }
                    return ascending ? lhsExecutionCount < rhsExecutionCount : lhsExecutionCount > rhsExecutionCount;
                } break;
                case 6: {
                    // TODO : Automate metrics columns/sorting
                    double lhsTime = 0;
                    for (const auto& metrics : lhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                            lhsTime = metrics.second->average;
                            break;
                        }
                    }
                    double rhsTime = 0;
                    for (const auto& metrics : rhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                            rhsTime = metrics.second->average;
                            break;
                        }
                    }
                    return ascending ? lhsTime < rhsTime : lhsTime > rhsTime;
                } break;
                case 7: {
                    std::array<float, 4> lhsColor{ };
                    memcpy(lhsColor.data(), &lhsPipelineInfo.highlightColor, sizeof(lhsColor));
                    std::array<float, 4> rhsColor{ };
                    memcpy(rhsColor.data(), &rhsPipelineInfo.highlightColor, sizeof(rhsColor));
                    return ascending ? lhsColor < rhsColor : lhsColor > rhsColor;
                } break;
                case 8: {
                    return ascending ? lhsPipelineInfo.experimentEnabled < rhsPipelineInfo.experimentEnabled : lhsPipelineInfo.experimentEnabled > rhsPipelineInfo.experimentEnabled;
                } break;
                case 9: {
                    return ascending ? lhsPipelineInfo.sampleMetrics < rhsPipelineInfo.sampleMetrics : lhsPipelineInfo.sampleMetrics > rhsPipelineInfo.sampleMetrics;
                } break;
                default: {
                } break;
                }
            }
            return lhsPipelineInfo.uuid < rhsPipelineInfo.uuid;
        }
    );
}

inline std::string get_pipeline_path(GuiInfo& guiInfo, VkDevice device, VkPipeline pipeline)
{
#if 0
    auto itr = guiInfo.pipelineInfos.find({ device, pipeline });
    if (itr != guiInfo.pipelineInfos.end() && itr->second.path.empty()) {
        itr->second.path = (guiInfo.workspacePath / ("VkPipeline-UUID-" + itr->second.uuidStr)).string();
    }
    return itr != guiInfo.pipelineInfos.end() ? itr->second.path.c_str() : nullptr;
#else
    std::string pipelinePath;
    auto itr = guiInfo.pipelineInfos.find({ device, pipeline });
    if (itr != guiInfo.pipelineInfos.end()) {
        pipelinePath = (std::filesystem::path(guiInfo.workspaceInfo.workspace) / ("VkPipeline-UUID-" + itr->second.uuidStr)).string();
    }
    return pipelinePath;
#endif
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

namespace GvkGui {

inline bool InputPath(const char* label, std::string* str)
{
    auto result = ImGui::InputText(label, str);
    if (ImGui::BeginDragDropTarget()) {
        auto pPayload = ImGui::AcceptDragDropPayload("external");
        if (pPayload) {
            *str = std::string((const char*)pPayload->Data, pPayload->DataSize);
            result = true;
        }
        ImGui::EndDragDropTarget();
    }
    if (result) {
        *str = gvk::string::scrub_path(*str);
    }
    return result;
}

} // namespace GvkGui

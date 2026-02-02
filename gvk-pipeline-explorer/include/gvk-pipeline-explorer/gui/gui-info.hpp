
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

#include "gvk-pipeline-explorer/backend/utilities.hpp"
#include "gvk-reference/handle-id.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-defines.hpp"
#include "gvk-gui.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-structures.hpp"

#include "boost/multiprecision/integer.hpp"
#if GVK_GITS_ENABLED
#include "libGits.h"
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "implot.h"
#include "implot_internal.h"

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

template <typename RequestType, typename ResultType>
class RequestResult final
{
public:
    RequestResult() = default;

    const gvk::Auto<RequestType>& get_request() const
    {
        return mRequest;
    }

    const gvk::Auto<ResultType>& get_result() const
    {
        return mResult;
    }

    void reset()
    {
        mRequest.reset();
        mResult.reset();
        mResultName.clear();
    }

    VkResult check_result(const std::filesystem::path& workspace)
    {
        return ready() ? VK_SUCCESS : (pending() ? read_serialized_structure(workspace / ".data", mResultName, mResult) : VK_NOT_READY);
    }

    VkResult submit_request(const std::filesystem::path& workspace, const RequestType& request, const std::string& requestName = "", const std::string& resultName = "")
    {
        // TODO : Timeout parameter
        // TODO : Queue?
        // TODO : Need to be able to check if any RequestResult is pending
        // TODO : Message ID
        // TODO : Maybe it should send over the thing it wants back to fill out?
        if (idle() || ready()) {
            reset();
            auto result = write_serialized_structure(workspace / ".data", requestName, request);
            if (result == VK_SUCCESS) {
                mRequest = request;
                mResultName = resultName;
            }
            return result;
        }
        return VK_NOT_READY;
    }

    bool idle() const
    {
        return mRequest->sType != gvk::get_stype<RequestType>() && mResult->sType != gvk::get_stype<ResultType>();
    }

    bool pending() const
    {
        return mRequest->sType == gvk::get_stype<RequestType>() && mResult->sType != gvk::get_stype<ResultType>();
    }

    bool ready() const
    {
        return mRequest->sType == gvk::get_stype<RequestType>() && mResult->sType == gvk::get_stype<ResultType>();
    }

private:
    gvk::Auto<RequestType> mRequest;
    gvk::Auto<ResultType> mResult;
    std::string mResultName;

    RequestResult(const RequestResult&) = delete;
    RequestResult& operator=(const RequestResult&) = delete;
};

class ApplicationInfo final
{
public:
#ifdef VK_USE_PLATFORM_WIN32_KHR
    struct PipePair
    {
        static constexpr int INHERIT_READ = 1 << 0;
        static constexpr int INHERIT_WRITE = 1 << 1;
        static BOOL create(DWORD inheritFlags, ApplicationInfo::PipePair* pPipePair)
        {
            (void)inheritFlags;
            if (pPipePair) {
                ApplicationInfo::PipePair::close(pPipePair);
                SECURITY_ATTRIBUTES securityAtributes{ };
                securityAtributes.nLength = sizeof(securityAtributes);
                securityAtributes.bInheritHandle = TRUE;
                auto success = CreatePipe(&pPipePair->read, &pPipePair->write, &securityAtributes, 0);
                // if (success && pPipePair->read && !(inheritFlags & INHERIT_READ)) {
                //     success &= SetHandleInformation(pPipePair->read, HANDLE_FLAG_INHERIT, 0);
                // }
                // if (success && pPipePair->write && !(inheritFlags & INHERIT_WRITE)) {
                //     success &= SetHandleInformation(pPipePair->write, HANDLE_FLAG_INHERIT, 0);
                // }
                if (!success) {
                    ApplicationInfo::PipePair::close(pPipePair);
                }
            }
            return pPipePair && pPipePair->read && pPipePair->write;
        }

        static void close(ApplicationInfo::PipePair* pPipePair)
        {
            if (pPipePair) {
                if (pPipePair->read) {
                    CloseHandle(pPipePair->read);
                }
                if (pPipePair->write) {
                    CloseHandle(pPipePair->write);
                }
                *pPipePair = { };
            }
        }

        HANDLE read{ };
        HANDLE write{ };
    };

    PROCESS_INFORMATION processInformation{ };
    HANDLE waitHandle{ };
    PipePair stdIn{ };
    PipePair stdOut{ };
    PipePair stdErr{ };
    std::pair<HANDLE, DWORD> stdInThread{ };
    std::pair<HANDLE, DWORD> stdOutThread{ };
    std::pair<HANDLE, DWORD> stdErrThread{ };
    HANDLE ioThread{ };
#endif
    bool running{ };
    bool closed{ };
};

class StreamInfo final
{
public:
    class Benchmark
    {
    public:
        double value{ };
        std::string str;
    };

    std::string path;
    std::unordered_map<int, double> selectedCalls;
    std::map<std::string, std::vector<Benchmark>> benchmarks;
    std::string selectedBenchmark;
    std::vector<double> selectedBenchmarkValues;
    uint64_t loopCount{ };
#if GVK_GITS_ENABLED
    GitsInstance gitsInstance{ };
    GitsPlayer gitsPlayer{ };
    GitsDispatchTable gitsDispatchTable{ };
#endif // GVK_GITS_ENABLED
    bool running{ };
    bool stop{ };
    bool runFrame{ };
    gvk::Auto<GvkCommandCollection> commandCollection;
};

class ApiCallInfo final
{
public:
    void reset()
    {
        commandCollection.reset();
        requestResult.reset();
    }

    gvk::Auto<GvkCommandCollection> commandCollection;
    std::vector<double> commandDurations;
    RequestResult<GvkPipelineExplorerCommandCollectionRequestInfo, GvkPipelineExplorerCommandCollectionResultInfo> requestResult;
#if 0
    std::vector<double> commandCollectionTimings; 
#endif
    std::vector<std::pair<std::string, double>> gpuCallInfos; //contains call stype + timings
    bool populateGpuCallPairs{}; //TODO AUSTIN: REMOVE ME this will be removed when commandCollectionTimings contains the actual gpu call timings

    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::unordered_set<uint64_t>> mPipelineToCallIndex;
    std::unordered_map<uint64_t, gvk::HandleId<VkDevice, VkPipeline>> mCallIndexToPipeline;
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
        , autoLogPath{ (bool)pipelineExplorerWorkspaceInfo.autoLogPath }
        , logToStdOut{ (bool)pipelineExplorerWorkspaceInfo.logToStdOut }
        , logToFile{ (bool)pipelineExplorerWorkspaceInfo.logToFile }
        , record{ (bool)pipelineExplorerWorkspaceInfo.record }
        , launch{ pipelineExplorerWorkspaceInfo.pLaunch ? pipelineExplorerWorkspaceInfo.pLaunch : std::string() }
        , target{ pipelineExplorerWorkspaceInfo.pTarget ? pipelineExplorerWorkspaceInfo.pTarget : std::string() }
        , args{ pipelineExplorerWorkspaceInfo.pArgs ? pipelineExplorerWorkspaceInfo.pArgs : std::string() }
        , workingDirectory{ pipelineExplorerWorkspaceInfo.pWorkingDirectory ? pipelineExplorerWorkspaceInfo.pWorkingDirectory : std::string() }
        , workspace{ pipelineExplorerWorkspaceInfo.pWorkspace ? pipelineExplorerWorkspaceInfo.pWorkspace : std::string() }
        , logPath{ pipelineExplorerWorkspaceInfo.pLogPath ? pipelineExplorerWorkspaceInfo.pLogPath : std::string() }
        , gits{ pipelineExplorerWorkspaceInfo.pGits ? pipelineExplorerWorkspaceInfo.pGits : std::string() }
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
        pipelineExplorerWorkspaceInfo.autoLogPath = autoLogPath;
        pipelineExplorerWorkspaceInfo.logToStdOut = logToStdOut;
        pipelineExplorerWorkspaceInfo.logToFile = logToFile;
        pipelineExplorerWorkspaceInfo.pLaunch = !launch.empty() ? launch.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pTarget = !target.empty() ? target.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pArgs = !args.empty() ? args.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pWorkingDirectory = !workingDirectory.empty() ? workingDirectory.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pWorkspace = !workspace.empty() ? workspace.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pLogPath = !logPath.empty() ? logPath.c_str() : nullptr;
        pipelineExplorerWorkspaceInfo.pGits = !gits.empty() ? gits.c_str() : nullptr;
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
    bool autoLogPath{ true };
    bool logToStdOut{ true };
    bool logToFile{ false };
    bool record{ false };
    std::string launch;
    std::string target;
    std::string args;
    std::string workingDirectory;
    std::string workspace;
    std::string logPath;
    std::string gits;

    ////////

    bool gitsStream{ };
    StreamInfo streamInfo{ };
};

struct PerformanceCounterResult
{
    double total{ };
    double average{ };
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
    std::map<std::array<uint8_t, VK_UUID_SIZE>, PerformanceCounterResult> performanceCounterResults;
    std::map<VkQueryPipelineStatisticFlagBits, PerformanceCounterResult> pipelineStatisticsQueryResults;
};

class PerformanceCountersInfo final
{
public:
    void reset()
    {
        available.reset();
        filters.clear();
        scopes.clear();
        categories.clear();
        anyOfFilter.clear();
        allOfFilter.clear();
        sortSpecs.clear();
        active.clear();
        enabled.clear();
        activeEnabledCount = 0;
        requestResult.reset();
    }

    gvk::Auto<GvkPipelineExplorerPerformanceCounterCollection> available;
    std::vector<std::pair<std::string, uint32_t>> filters;
    std::map<VkPerformanceCounterScopeKHR, bool> scopes;
    std::map<std::string, bool> categories;
    std::string anyOfFilter;
    std::string allOfFilter;
    std::vector<ImGuiTableColumnSortSpecs> sortSpecs;
    std::vector<uint32_t> active;
    std::vector<bool> enabled;
    uint32_t activeEnabledCount{ };
    RequestResult<GvkPipelineExplorerPerformanceQueryRequestInfo, GvkPipelineExplorerPerformanceQueryResultInfo> requestResult;
};

class PluginPerformanceCounterInfo final
{
public:
    void reset()
    {
        available.reset();
        requestResult.reset();
    }

    gvk::Auto<GvkPipelineExplorerPluginCounterInfo> available;
    RequestResult<GvkPipelineExplorerPerformanceQueryRequestInfo, GvkPipelineExplorerPerformanceQueryResultInfo> requestResult;
};

class TimestampInfo final
{
public:
    void reset()
    {
        requestResult.reset();
    }

    gvk::Auto<GvkPipelineExplorerTimestampQueryResultInfo> available;
    RequestResult<GvkPipelineExplorerPerformanceQueryRequestInfo, GvkPipelineExplorerTimestampQueryResultInfo> requestResult;
};

class PipelineStatisticsQueryInfo final
{
public:
    void reset()
    {
        available.reset();
        requestResult.reset();
    }

    gvk::Auto<GvkPipelineExplorerPerformanceCounterCollection> available;
    RequestResult<GvkPipelineExplorerPipelineStatisticsQueryRequestInfo, GvkPipelineExplorerPipelineStatisticsQueryResultInfo> requestResult;
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
    PerformanceCountersInfo performanceCountersInfo;
    PluginPerformanceCounterInfo pluginPerformanceCounterInfo;
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
    ApiCallInfo apiCallInfo{ };
    TimestampInfo timestampInfo{ };
    PipelineStatisticsQueryInfo pipelineStatisticsQueryInfo{ };
    std::vector<WorkspaceInfo> recentWorkspaceInfos;
    std::set<std::string> screenshots;
    std::vector<VkLayerProperties> layerProperties;
    bool resultPending{ };
    std::ofstream logFile;
    bool autoQuery{ };
};

inline std::string ConvertToStringAndRound(double input, int decimalPercision)
{
    std::string inputStr = std::to_string(input);
    return inputStr.substr(0, inputStr.find(".") + decimalPercision);
}

inline ImU32 LightenColor(ImU32 color, float inAmount)
{
    ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color);
    ;    ImVec4 lightenedColor = ImVec4(
        std::min((float)1, colorVec.x + 1 * inAmount),
        std::min((float)1, colorVec.y + 1 * inAmount),
        std::min((float)1, colorVec.z + 1 * inAmount),
        colorVec.w);
    return ImGui::ColorConvertFloat4ToU32(lightenedColor);
}

//TODO: This should be under GvkGui, but had namespace errors when I tried to use it from there
inline void SetToolTip(const char* pToolTip)
{
    //Making a long delay using the IsItemHovered flags disables tooltips when disabled textbox is used. 
    //So we are using the ImGuiHoveredFlags_AllowWhenDisabled flag (github.com/ocornut/imgui/issues/1940)
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(pToolTip, ImGui::GetStyle().HoverDelayNormal);
    }
}

enum struct BarChartFlags
{
    CONTINUOUS_SELECTION = 1 << 0, //Select a range of interest, selecting all bars within this frame
    NON_CONTINUOUS_SELECTION = 1 << 1,
    TIMING_BRACKET = 1 << 2, //invalid if no selection flag is enabled
    DYNAMIC_BARS = 1 << 3,
    TOOL_TIPS_FOR_GPU_CALLS = 1 << 4, //It is invalid behavior for both of the tooltips flags to be enabled
    TOOL_TIPS_FOR_FRAMES = 1 << 5,
    DRAG_RECT_PREVIEW = 1 << 6,
    BORDER = 1 << 7, //replace bool border class member with this
    WIP = 1 << 8 //Just Under Active Development text
};

class BarChart final
{
public:

    //Check whether a flag is set to true
    bool has_flag(BarChartFlags flag)
    {
        return (flagValue & (uint32_t)flag) == (uint32_t)flag;
    }

    struct PlotInfo
    {
        const char* pLabelName{};
        const double* pXData{}; //User can provide this. If user does not, we will generate x_data iteratively from 0 to y_data_count.
        const double* pYData{};
        int yDataCount{};
        double yValuesSum{}; //Only used by dynamic bar chart (total accumulated time). Seems to fit here more than in BarChart class itself
        std::vector<std::pair<std::string, double>> gpuCallInfos; //TODO: guiInfo class has this... this is literally a copy just so barchart can access it. Would like to not need this
    };

    void plot_bar_chart(const PlotInfo& plotInfo) 
    {
        //Ensure flag set is valid
        assert(!has_flag(BarChartFlags::TIMING_BRACKET) || (has_flag(BarChartFlags::TIMING_BRACKET) && (has_flag(BarChartFlags::CONTINUOUS_SELECTION) || has_flag(BarChartFlags::NON_CONTINUOUS_SELECTION))));
        assert(!has_flag(BarChartFlags::TOOL_TIPS_FOR_GPU_CALLS) || !has_flag(BarChartFlags::TOOL_TIPS_FOR_FRAMES));

        if (has_flag(BarChartFlags::WIP)) {
            //Disable chart functionality for now until more it is polished more
            ImPlot::BeginPlot(chartName.c_str(), ImVec2(-1, 0), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoMouseText);
            ImPlot::PlotText("Under Development", 4.0f, 6.0f);
        } else if (plotInfo.pYData && ImPlot::BeginPlot(chartName.c_str(), ImVec2(-1, 0), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoMouseText)) { // | ImPlotDragToolFlags_Delayed)) {
        
            // get ImGui window DrawList
            ImDrawList* drawList = ImPlot::GetPlotDrawList();
            auto thisPlot = ImPlot::GetPlot(chartName.c_str());

            if (has_flag(BarChartFlags::DYNAMIC_BARS)) {
                setup_plot_and_initial_fit(thisPlot, plotInfo, 0, plotInfo.yValuesSum);
            } else {//Standard barchart
                //Setup integer-only ticks
                //std::vector<int> labels = compute_labels( //investigate AddTicksDefaultS
                //ImPlot::SetupAxisTicks(ImAxis_X1, 0, (double)(plotInfo.yDataCount - 1), plotInfo.yDataCount, labels);

                //Compute desired barwidth
                setup_plot_and_initial_fit(thisPlot, plotInfo, 0 - barWidth / 2, plotInfo.yDataCount - 1 + barWidth / 2);
            }
            if (ImPlot::BeginItem(plotInfo.pLabelName ? plotInfo.pLabelName : "Bars")) {
                //Override legend color
                ImPlot::GetCurrentItem()->Color = IM_COL32(64, 64, 64, 255);
                if (has_flag(BarChartFlags::DYNAMIC_BARS)) {
                    bar_render_and_selection_logic(thisPlot, plotInfo, drawList);
                } else { 
                    bar_render_and_selection_logic(thisPlot, plotInfo, drawList);
                }
                ImPlot::EndItem();
            }
        }
        ImPlot::EndPlot();
    }

    void setup_plot_and_initial_fit(ImPlotPlot* thisPlot, const PlotInfo& plotInfo, double xRangeMin, double xRangeMax)
    {
        ImPlot::SetupAxis(ImAxis_X1, xAxisName.c_str());
        ImPlot::SetupAxis(ImAxis_Y1, yAxisName.c_str(), ImPlotAxisFlags_AutoFit);

        //Keep X-Axis within stream duration
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, xRangeMin, xRangeMax);
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, xRangeMin, xRangeMax);

        //Initial Fit for  X and Y-Axis
        if (!startupCompleted) {
            thisPlot->Flags &= ~ImPlotAxisFlags_PanStretch;
            thisPlot->Flags |= ImPlotAxisFlags_Lock;

            //NOTE: How to access internal data if needed: thisPlot->XAxis(0).Range.Min
            thisPlot->XAxis(0).FitExtents.Min = xRangeMin;
            thisPlot->XAxis(0).FitExtents.Max = xRangeMin;

            for (int i = 0; i < plotInfo.yDataCount; ++i) {
                ImPlot::FitPointY(plotInfo.pYData[i] * 1.3);
            }
            startupCompleted = true;
        }

        thisPlot->XAxis(0).Range.Max = xRangeMax;
    }

    //Draws bar at plotInfo.pYData[i] with width thisBarwidth.  
    void bar_render_and_selection_logic(ImPlotPlot* thisPlot, const PlotInfo& plotInfo, ImDrawList* drawList)
    {
        double accumFrameTime = 0;
        double localMax = 0;
        double xValue = 0;
        double thisBarWidth = 0;
        bool buttonHovered = false;
        int singleBarSelection = -1; //Used for dynamic barchart single bar selection
        std::pair<float, float> singleBarSelectionBorders{}; //Used for dynamic barchart single bar selection

        for (int i = 0; i < plotInfo.yDataCount; ++i) { //TODO: Change this loop to be more performant by only looping through visible bars

            if (has_flag(BarChartFlags::DYNAMIC_BARS)) {
                thisBarWidth = plotInfo.pYData[i];
                xValue = accumFrameTime;
            } else {
                // calc real value width
                thisBarWidth = barWidth;
                double x = i; //Make synthetic x data
                if (plotInfo.pXData != nullptr) {
                    x = plotInfo.pXData[i]; //Use user provided x_data if exists
                }
                xValue = x - thisBarWidth / 2;
            }

            //Find local maximum for Y-Axis fitting 
            auto xLimits = ImPlot::GetPlotLimits().X;
            if (xLimits.Min <= xValue + plotInfo.pYData[i] && xValue <= xLimits.Max) {
                localMax = std::max(plotInfo.pYData[i], localMax);
            }

            //Y-axis fitting logic
            thisPlot->YAxis(0).Range.Max = localMax * 1.3;
            thisPlot->YAxis(0).Range.Min = 0;

            //Important: PlotToPixels locks setup. Do NOT call Setup code after you using these non-Setup API calls
            ImVec2 topLeft = ImPlot::PlotToPixels(xValue, plotInfo.pYData[i]);
            ImVec2 bottomRight = ImPlot::PlotToPixels(xValue + thisBarWidth, 0);
            ImU32 currentBarColor = barColor;

            if (has_flag(BarChartFlags::CONTINUOUS_SELECTION)) {
                //Selection recording logic
                //double tempBarWidth = has_flag(BarChartFlags::DYNAMIC_BARS) ? plotInfo.pYData[i] : thisBarWidth;
                if (xValue + thisBarWidth >= std::min(selectStart, selectEnd) && xValue < std::max(selectStart, selectEnd)) {
                    currentBarColor = IM_COL32(255, 0, 0, 255);
                    if (selecting && ImGui::IsMouseDown(ImGuiMouseButton_Right) && !selectedBars[i]) {//Would be nice to not do this per-frame... at least hash lookup is O(1)
                        //we don't use this frameLength data, its just filler for the hash table
                        selectedBars[i] = plotInfo.pYData[i];
                    }
                } else {
                    if (selecting && ImGui::IsMouseDown(ImGuiMouseButton_Right) && (selectedBars.find(i) != selectedBars.end())) {
                        selectedBars.erase(i);
                        //TODO: If we want to keep selected noncontinuous bars in selectedBars when we start a continuous selection,
                        //We need to ensure they don't get erased here.
                    }
                }

            }

            if (has_flag(BarChartFlags::NON_CONTINUOUS_SELECTION) && selectedBars.find(i) != selectedBars.end()) {
                currentBarColor = IM_COL32(255, 0, 0, 255);
            }

            ImVec2 currentCursorPos = ImGui::GetCursorPos();
            ImVec2 size = ImVec2(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);

            //Set cursor pos to topLeft of Bar
            ImGui::SetCursorScreenPos(topLeft);
            ImGui::PushStyleColor(ImGuiCol_Button, currentBarColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, LightenColor(currentBarColor, (float)0.3));
            std::string id = "##" + std::to_string(i);
            //Draw Bar (button)
            ImGui::Button(id.c_str(), size);

            if (ImGui::IsItemHovered()) {
                buttonHovered = true;
                //For selecting on the bar itself in non-continuous selection
                if (has_flag(BarChartFlags::NON_CONTINUOUS_SELECTION) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    if (selectedBars.find(i) == selectedBars.end()) {//Would be nice to not do this per-frame... at least hash lookup is O(1))
                        // if single bar selected somewhere else, clear it
                        if (!ImGui::GetIO().KeyCtrl) {
                            selectedBars.clear();
                            selecting = false; //these 3 vars are used by continuous selection 
                            selectStart = -1;
                            selectEnd = -1;
                        }

                        //Add to selection
                        selectedBars[i] = plotInfo.pYData[i];
                    }
                    else {//current bar is selected
                        //Remove from selection
                        if (selectedBars.size() == 1 || ImGui::GetIO().KeyCtrl) {
                            selectedBars.erase(i);
                        //OR if others bars selected, clear them but keep this bar selected
                        } else if (selectedBars.size() > 1) {
                                selectedBars.clear();
                                selecting = false; //used by continuous selection 
                                selectedBars[i] = plotInfo.pYData[i];
                        }
                    }
                }
            }

            //Add tooltips if requested
            if (has_flag(BarChartFlags::TOOL_TIPS_FOR_GPU_CALLS) || has_flag(BarChartFlags::TOOL_TIPS_FOR_FRAMES)) {
                std::string toolTip = "";
                if (has_flag(BarChartFlags::TOOL_TIPS_FOR_FRAMES)) {
                    toolTip = "Frame " + std::to_string(i) + ": " + std::to_string(plotInfo.pYData[i]);
                } else if (has_flag(BarChartFlags::TOOL_TIPS_FOR_GPU_CALLS)) {
                    //TODO: Add assert for gpuCallInfos
                    toolTip = plotInfo.gpuCallInfos[i].first + " #";
                    toolTip += std::to_string(i) + "\nGPU Duration: " + std::to_string(plotInfo.gpuCallInfos[i].second) + "ms";
                }
                SetToolTip(toolTip.c_str());
            }

            //Add timing bracket if requested
            if (has_flag(BarChartFlags::TIMING_BRACKET)) {
                ImPlot::EndItem();
                if (ImPlot::BeginItem("Timing Brackets")) {
                    ImU32 bracketColor = barColor;
                    float thickness = 2.5f;
                    //no timing bracket started and current bar selected
                    if (timingBracketStartBar == -1 && (selectedBars.find(i) != selectedBars.end())) {
                        timingBracketStartBar = i;
                        if (has_flag(BarChartFlags::DYNAMIC_BARS)) {
                            timingBracketStartX = xValue;
                        }
                    }
                    //timing bracket started
                    if (timingBracketStartBar != -1) {
                        //and next bar unselected or on the last bar
                        if (i + 1 == plotInfo.yDataCount || selectedBars.find(i + 1) == selectedBars.end()) {
                            //Don't continue if this is the only bar selected in the sequence
                            if (i - timingBracketStartBar == 0) {
                                timingBracketStartBar = -1;
                            } else {
                                //draw timing bracket from timing_bracket_start_bar to i-1
                                double firstTimingBracketX = has_flag(BarChartFlags::DYNAMIC_BARS) ? timingBracketStartX : timingBracketStartBar;
                                double bracketStartX = firstTimingBracketX - thisBarWidth / 2;
                                double bracketEndX = xValue + thisBarWidth;
                                ImVec2 coord1 = ImPlot::PlotToPixels(bracketStartX, plotInfo.pYData[timingBracketStartBar] * 1.1);
                                ImVec2 coord2 = ImPlot::PlotToPixels(bracketStartX, localMax * 1.15);
                                drawList->AddLine(coord1, coord2, bracketColor, thickness);
                                coord1 = ImPlot::PlotToPixels(bracketStartX, localMax * 1.15);
                                coord2 = ImPlot::PlotToPixels(bracketEndX, localMax * 1.15);
                                drawList->AddLine(coord1, coord2, bracketColor, thickness);
                                coord1 = ImPlot::PlotToPixels(bracketEndX, plotInfo.pYData[i] * 1.1);
                                coord2 = ImPlot::PlotToPixels(bracketEndX, localMax * 1.15);
                                drawList->AddLine(coord1, coord2, bracketColor, thickness);

                                //Draw annotation for duration
                                std::string duration_string = ConvertToStringAndRound(timingBracketDuration, 6);
                                auto annotation_start_x = (bracketEndX - bracketStartX) / 2 + bracketStartX; //ImPlot::PlotToPixels((bracketEndX - bracketStartX) / 2,0);
                                ImPlot::Annotation(annotation_start_x, localMax * 1.2, ImVec4(0.0f, 0.45f, 1.0f, 1.0f), ImVec2(0, 0), true, "%s", duration_string.c_str());
                                timingBracketStartBar = -1;
                                timingBracketDuration = 0;
                            }
                        } else {
                            //add to timing_bracket_duration
                            timingBracketDuration += plotInfo.pYData[i];
                        }
                    }
                    ImPlot::EndItem();
                }
                ImPlot::BeginItem(plotInfo.pLabelName ? plotInfo.pLabelName : "Bars");
            }
            //single bar selection caching for continuous selection
            if (has_flag(BarChartFlags::CONTINUOUS_SELECTION) && selectedBars.size() == 1 && (selectedBars.find(i) != selectedBars.end())) {
                singleBarSelection = i;
                singleBarSelectionBorders.first = topLeft.x;
                singleBarSelectionBorders.second = bottomRight.x;
            }

            //Restore color and cursor pos
            ImGui::SetCursorPos(currentCursorPos);
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();

            //Legacy custom bars:
            //draw_list->AddRectFilled(topLeft, bottomRight, current_bar_color);

            if (has_flag(BarChartFlags::BORDER)) {
                drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x, bottomRight.y), borderColor, 2.0f);
            }

            if (has_flag(BarChartFlags::DYNAMIC_BARS)) {
                accumFrameTime += plotInfo.pYData[i];
            }
        }

        //For Performance reasons, the selection state logic was moved outside of the above for loop.
        if (has_flag(BarChartFlags::CONTINUOUS_SELECTION))
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && (buttonHovered || ImPlot::IsPlotHovered())) {
                if (selecting) {
                    selecting = false;
                    selectStart = -1;
                    selectEnd = -1;
                } else {
                    ImVec2 mousePos = ImGui::GetMousePos();
                    selectStart = ImPlot::PixelsToPlot(mousePos).x;
                    selectEnd = selectStart;
                    selecting = true;
                    //TODO: If keeping non-continuous selection active when starting continuous selectiong, this might be neccesary:
                    //if (!(has_flag(BarChartFlags::NON_CONTINUOUS_SELECTION) && ImGui::GetIO().KeyCtrl)) {
                    selectedBars.clear();
                }
            }

            if (selecting) {
                std::string selectText = "";
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    ImVec2 mousePos = ImGui::GetMousePos();
                    selectEnd = ImPlot::PixelsToPlot(mousePos).x;
                    selectText = "Selecting: ";

                } else if (!has_flag(BarChartFlags::DYNAMIC_BARS)) {
                    selecting = false;
                    selectStart = -1;
                    selectEnd = -1;
                }
                double selectionDuration = selectEnd - selectStart;
                ImVec2 selectTopLeft = ImPlot::PlotToPixels(selectStart, ImPlot::GetPlotLimits().Y.Max);
                ImVec2 selectBottomRight = ImPlot::PlotToPixels(selectEnd, 0);
                if (has_flag(BarChartFlags::DYNAMIC_BARS)) {
                    double annotationStartX = selectStart + selectionDuration / 2;
                    if (selectionDuration == 0 && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                        //Single bar selection
                        selectionDuration = plotInfo.pYData[singleBarSelection];
                        selectTopLeft.x = singleBarSelectionBorders.first;
                        selectBottomRight.x = singleBarSelectionBorders.second;
                        annotationStartX = ImPlot::PixelsToPlot(selectTopLeft).x + selectionDuration / 2;
                    }
                    selectText = selectText + ConvertToStringAndRound(std::abs(selectionDuration), 6) + "ms";
                    ImPlot::Annotation(annotationStartX, ImPlot::GetPlotLimits().Y.Max, ImVec4(214, 164, 0, 255), ImVec2(0, 0), true, "%s", selectText.c_str());
                }
                drawList->AddRectFilled(selectTopLeft, selectBottomRight, IM_COL32(255, 205, 65, 120));
                drawList->AddRect(selectTopLeft, selectBottomRight, IM_COL32(255, 205, 65, 255));
            }
        }
        if (has_flag(BarChartFlags::NON_CONTINUOUS_SELECTION)) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && buttonHovered) {
                //Disallow exiting multi-selection mode for ease of use.
                if (!ImGui::GetIO().KeyCtrl) {
                    if (selectedBars.size() > 1) {
                        selectedBars.clear();
                        selecting = false; //used by continuous selection 
                    }
                }
            }
        }
    }

    bool startupCompleted = false;

    std::string chartName {};
    std::string xAxisName {};
    std::string yAxisName {};

    //Selection helpers
    bool selecting = false;
    double selectStart = -1;
    double selectEnd = -1;
    std::unordered_map<int, double> selectedBars = {}; //using hash table to get the O(1) find time as we do it per-frame. We don't really need the double value.

    //Timing bracket helpers (only used when selection flag on)
    int32_t timingBracketStartBar = -1;
    double timingBracketStartX = 0; //For dynamic bars
    double timingBracketDuration = 0;

    float barWidth = 20.0f;
    ImU32 barColor {ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.45f, 1.0f, 1.0f))};
    bool border = false;
    ImU32 borderColor {ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f))};

    uint32_t flagValue = 0;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
inline BOOL redirect_io(GuiInfo& guiInfo, HANDLE read, HANDLE write)
{
    DWORD count = 0;
    std::array<char, 1024> buffer{ };
    auto success = ReadFile(read, buffer.data(), (DWORD)buffer.size() - 1, &count, NULL);
    if (success && count) {
        if (guiInfo.workspaceInfo.logToStdOut) {
            success = WriteFile(write, buffer.data(), count, NULL, NULL);
        }
        if (guiInfo.workspaceInfo.logToFile && guiInfo.logFile.is_open()) {
            guiInfo.logFile.write(buffer.data(), count);
            guiInfo.logFile.flush();
        }
    }
    return success;
}

inline DWORD CALLBACK process_io_callback(_In_ LPVOID lpParameter)
{
    if (lpParameter) {
        auto& guiInfo = *(GuiInfo*)lpParameter;
        auto currentThreadId = GetCurrentThreadId();
        if (currentThreadId == guiInfo.applicationInfo.stdInThread.second) {
            while (redirect_io(guiInfo, GetStdHandle(STD_INPUT_HANDLE), guiInfo.applicationInfo.stdIn.write)) {
            }
        } else if (currentThreadId == guiInfo.applicationInfo.stdOutThread.second) {
            while (redirect_io(guiInfo, guiInfo.applicationInfo.stdOut.read, GetStdHandle(STD_OUTPUT_HANDLE))) {
            }
        } else if (currentThreadId == guiInfo.applicationInfo.stdErrThread.second) {
            while (redirect_io(guiInfo, guiInfo.applicationInfo.stdErr.read, GetStdHandle(STD_ERROR_HANDLE))) {
            }
        }
    }
    return 0;
}

inline VOID CALLBACK process_wait_callback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
    (void)TimerOrWaitFired;
    if (lpParameter) {
        auto& guiInfo = *(GuiInfo*)lpParameter;
        guiInfo.messages += "INFO : " + guiInfo.workspaceInfo.launch + " closed\n";
        guiInfo.applicationInfo.closed = true;
        // TODO : Double check that all handles/resources associated with child process
        //  are correctly closed/cleaned up
        guiInfo.applicationInfo.running = false;
        ApplicationInfo::PipePair::close(&guiInfo.applicationInfo.stdIn);
        ApplicationInfo::PipePair::close(&guiInfo.applicationInfo.stdOut);
        ApplicationInfo::PipePair::close(&guiInfo.applicationInfo.stdErr);
        if (guiInfo.applicationInfo.ioThread) {
            WaitForSingleObject(guiInfo.applicationInfo.ioThread, INFINITE);
            CloseHandle(guiInfo.applicationInfo.ioThread);
            guiInfo.applicationInfo.ioThread = NULL;
        }
        guiInfo.logFile.close();
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
    std::string pipelinePath;
    auto itr = guiInfo.pipelineInfos.find({ device, pipeline });
    if (itr != guiInfo.pipelineInfos.end()) {
        pipelinePath = (std::filesystem::path(guiInfo.workspaceInfo.workspace) / ("VkPipeline-UUID-" + itr->second.uuidStr)).string();
    }
    return pipelinePath;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

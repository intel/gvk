
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

#include "gvk-defines.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-pipeline-explorer/backend/ipc-messenger.hpp"
#include "gvk-pipeline-explorer/backend/utilities.hpp"
#include "gvk-pipeline-explorer/gui/pipeline-info.hpp"
#include "gvk-pipeline-explorer/gui/range-info.hpp"
#include "gvk-reference/handle-id.hpp"
#include "gvk-runtime/child-process.hpp"
#include "gvk-runtime/io-pipe.hpp"
#include "gvk-gui.hpp"
#include "gvk-runtime.hpp"
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
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// FROM : https://github.com/juliettef/IconFontCppHeaders/blob/main/IconsFontAwesome6.h
#define ICON_FA_CHART_AREA             "\xef\x87\xbe" // U+f1fe
#define ICON_FA_CHART_BAR              "\xef\x82\x80" // U+f080
#define ICON_FA_CHART_COLUMN           "\xee\x83\xa3" // U+e0e3
#define ICON_FA_CHART_DIAGRAM          "\xee\x9a\x95" // U+e695
#define ICON_FA_CHART_GANTT            "\xee\x83\xa4" // U+e0e4
#define ICON_FA_CHART_LINE             "\xef\x88\x81" // U+f201
#define ICON_FA_CHART_PIE              "\xef\x88\x80" // U+f200
#define ICON_FA_CHART_SIMPLE           "\xee\x91\xb3" // U+e473
#define ICON_FA_MAGNIFYING_GLASS_CHART "\xee\x94\xa2" // U+e522

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
    void reset()
    {
        workspace.clear();

        ///////////////////////////////////////////////////////////////////////////////
        // TODO : Unify application shutdown and stream shutdown

#ifdef GVK_PLATFORM_WINDOWS
        startupIpcMessages.clear();
        incomingIpcMessages.clear();
#endif // GVK_PLATFORM_WINDOWS

        messages.clear();
        requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
        requestInfo.warmupRangeCount = 4;
        requestInfo.queryRangeCount = 16;
        activePipelines.clear();
        sortedPipelines.clear();
        pipelineInfos.clear();
        availableMetrics.clear();
        filteredMetrics.clear();
        metricsFilters.clear();
        metricsAnyOfFilter.clear();
        metricsAllOfFilter.clear();
        performanceCountersInfo.reset();
        pipelineStatisticsQueryInfo.reset();
        pluginPerformanceCounterInfo.reset();
        selectedPipeline = { };
        enabledMetricsGroup = 0;
        apiCallInfo.reset();
        resultPending = false;

        frameDurations = { };
        frameIndex = 0;
        frameCount = 0;
        frameDurationAccumulator = 0;
        frameRate = 0;
        ///////////////////////////////////////////////////////////////////////////////
    }

    std::string workspace;
#ifdef GVK_PLATFORM_WINDOWS
    gvk::ChildProcess workload;
    std::function<void()> onWorkloadShutdown;
    gvk::NamedPipe ipcPipe;
    gvk::pipeline_explorer::IpcMessenger ipcMessenger;
    std::vector<gvk::IpcMessenger::Message> startupIpcMessages;
    std::unordered_map<std::string, std::vector<gvk::IpcMessenger::Message>> incomingIpcMessages;
#endif // GVK_PLATFORM_WINDOWS
    std::ofstream logFile;
    std::mutex workloadMutex;

    // TODO : Documentation
    RangeInfo rangeInfo{ };

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

    // TODO : Documentation
    std::array<double, 60> frameDurations{ };
    uint32_t frameIndex{ };
    uint32_t frameCount{ };
    double frameDurationAccumulator{ };
    double frameRate{ };
    
    // TODO : Documentation
    GvkPipelineExplorerTimelineQueryInterval queryInterval { gvk::get_default<GvkPipelineExplorerTimelineQueryInterval>() };

    // TODO : Documentation
    std::vector<VkLayerProperties> layerProperties;
    bool validationLayerAvailable{ };

    ApiCallInfo apiCallInfo{ };
    TimestampInfo timestampInfo{ };
    PipelineStatisticsQueryInfo pipelineStatisticsQueryInfo{ };
    std::set<std::string> screenshots;
    gvk::Auto<GvkCommandCollection> commandCollection;
    bool resultPending{ };
    bool autoQuery{ };
};

inline std::string ConvertToStringAndRound(double input, int decimalPrecision)
{
    std::string inputStr = std::to_string(input);
    return inputStr.substr(0, inputStr.find(".") + decimalPrecision);
}

inline ImU32 LightenColor(ImU32 color, float inAmount)
{
    ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color);
    ImVec4 lightenedColor = ImVec4(
        std::min(1.0f, colorVec.x + 1.0f * inAmount),
        std::min(1.0f, colorVec.y + 1.0f * inAmount),
        std::min(1.0f, colorVec.z + 1.0f * inAmount),
        colorVec.w
    );
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

template <typename T>
inline bool compare_pipeline_value(ImGuiSortDirection sortDirection, const T& lhs, const T& rhs)
{
    auto ascending = sortDirection == ImGuiSortDirection_Ascending;
    return ascending ? lhs < rhs : lhs > rhs;
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
            const auto& lhsPipelineExecutionInfoItr = guiInfo.rangeInfo.pipelineExecutionInfos.find(lhs);
            const auto& rhsPipelineExecutionInfoItr = guiInfo.rangeInfo.pipelineExecutionInfos.find(rhs);
            const auto& lhsPipelineExecutionInfo = (lhsPipelineExecutionInfoItr != guiInfo.rangeInfo.pipelineExecutionInfos.end()) ? lhsPipelineExecutionInfoItr->second : PipelineExecutionInfo { };
            const auto& rhsPipelineExecutionInfo = (rhsPipelineExecutionInfoItr != guiInfo.rangeInfo.pipelineExecutionInfos.end()) ? rhsPipelineExecutionInfoItr->second : PipelineExecutionInfo { };
            for (const auto& pipelineSortSpec : guiInfo.pipelineSortSpecs) {
                switch (pipelineSortSpec.ColumnUserID) {
                case 0: {
                    if (lhsPipelineInfo.uuid != rhsPipelineInfo.uuid) {
                         return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsPipelineInfo.uuid, rhsPipelineInfo.uuid);
                    }
                } break;
                case 1: {
                    if (lhsPipelineInfo.driverUUID != rhsPipelineInfo.driverUUID) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsPipelineInfo.driverUUID, rhsPipelineInfo.driverUUID);
                    }
                } break;
                case 2: {
                    if (lhsPipelineInfo.pipeline != rhsPipelineInfo.pipeline) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsPipelineInfo.pipeline, rhsPipelineInfo.pipeline);
                    }
                } break;
                case 3: {
                    if (lhsPipelineInfo.name != rhsPipelineInfo.name) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsPipelineInfo.name, rhsPipelineInfo.name);
                    }
                } break;
                case 4: {
                    if (lhsPipelineInfo.bindPoint != rhsPipelineInfo.bindPoint) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsPipelineInfo.bindPoint, rhsPipelineInfo.bindPoint);
                    }
                } break;
                case 5: {
                    // TODO : Automate metrics columns/sorting
#if 0
                    double lhsExecutionCount = 0;
                    for (const auto& metrics : lhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                            #if 0
                            lhsExecutionCount = metrics.second->average;
                            #else
                            lhsExecutionCount = metrics.second->total;
                            #endif
                            break;
                        }
                    }
                    double rhsExecutionCount = 0;
                    for (const auto& metrics : rhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_EXECUTION_COUNT) {
                            #if 0
                            rhsExecutionCount = metrics.second->average;
                            #else
                            rhsExecutionCount = metrics.second->total;
                            #endif
                            break;
                        }
                    }
                    if (lhsExecutionCount != rhsExecutionCount) {
                         return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsExecutionCount, rhsExecutionCount);
                    }
#else
                    auto lhsExecutionCount = lhsPipelineExecutionInfo.executionCount;
                    auto rhsExecutionCount = rhsPipelineExecutionInfo.executionCount;
                    if (lhsExecutionCount != rhsExecutionCount) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsExecutionCount, rhsExecutionCount);
                    }
#endif
                } break;
                case 6: {
                    // TODO : Automate metrics columns/sorting
#if 0
                    double lhsTime = 0;
                    for (const auto& metrics : lhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                            lhsTime = metrics.second->total;
                            break;
                        }
                    }
                    double rhsTime = 0;
                    for (const auto& metrics : rhsPipelineInfo.metrics) {
                        if (metrics.first.x == GVK_PIPELINE_EXPLORER_METRIC_ID_TIMESTAMP_QUERY) {
                            rhsTime = metrics.second->total;
                            break;
                        }
                    }
                    if (lhsTime != rhsTime) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsTime, rhsTime);
                    }
#else
                    auto lhsTotalDuration = lhsPipelineExecutionInfo.totalDurationNs;
                    auto rhsTotalDuration = rhsPipelineExecutionInfo.totalDurationNs;
                    if (lhsTotalDuration != rhsTotalDuration) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsTotalDuration, rhsTotalDuration);
                    }
#endif
                } break;
                case 7: {
                    // TODO : Automate metrics columns/sorting
#if 0
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
                    if (lhsTime != rhsTime) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsTime, rhsTime);
                    }
#else
                    auto lhsAvgDuration = lhsPipelineExecutionInfo.executionCount ? (double)lhsPipelineExecutionInfo.totalDurationNs / (double)lhsPipelineExecutionInfo.executionCount : 0;
                    auto rhsAvgDuration = rhsPipelineExecutionInfo.executionCount ? (double)rhsPipelineExecutionInfo.totalDurationNs / (double)rhsPipelineExecutionInfo.executionCount : 0;
                    if (lhsAvgDuration != rhsAvgDuration) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsAvgDuration, rhsAvgDuration);
                    }
#endif
                } break;
                case 8: {
                    if (lhsPipelineInfo.experimentEnabled != rhsPipelineInfo.experimentEnabled) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsPipelineInfo.experimentEnabled, rhsPipelineInfo.experimentEnabled);
                    }
                } break;
                case 9: {
                    std::array<float, 4> lhsColor{ };
                    memcpy(lhsColor.data(), &lhsPipelineInfo.highlightColor, sizeof(lhsColor));
                    std::array<float, 4> rhsColor{ };
                    memcpy(rhsColor.data(), &rhsPipelineInfo.highlightColor, sizeof(rhsColor));
                    if (lhsColor != rhsColor) {
                        return compare_pipeline_value(pipelineSortSpec.SortDirection, lhsColor, rhsColor);
                    }
                } break;
                default: {
                } break;
                }
            }
            return lhsPipelineInfo.uuid < rhsPipelineInfo.uuid;
        }
    );
}

// TODO : Unify with backend
inline std::string get_pipeline_path(GuiInfo& guiInfo, VkDevice device, VkPipeline pipeline)
{
    std::string pipelinePath;
    auto itr = guiInfo.pipelineInfos.find({ device, pipeline });
    if (itr != guiInfo.pipelineInfos.end()) {
        pipelinePath = (std::filesystem::path(guiInfo.workspace) / ("VkPipeline-UUID-" + itr->second.uuidStr)).string();
    }
    return pipelinePath;
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

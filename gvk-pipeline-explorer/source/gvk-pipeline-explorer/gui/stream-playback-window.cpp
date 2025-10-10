
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

#include "gvk-pipeline-explorer/gui/stream-playback-window.hpp"
#include "gvk-pipeline-explorer/gui/gui-info.hpp"
#include "gvk-environment.hpp"

#if GVK_GITS_ENABLED
#include "libGits.h"
#endif

#include <fstream>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

StreamPlaybackWindow::StreamPlaybackWindow(Window::Manager& windowManager, const std::string& name)
    : Window(windowManager, name)
{
}

StreamPlaybackWindow::~StreamPlaybackWindow()
{
    if (mStreamPlaybackThread.joinable()) {
        mStreamPlaybackThread.join();
    }
}

void StreamPlaybackWindow::on_gui(GuiInfo& guiInfo)
{
    if (!guiInfo.workspaceInfo.launch.empty() && guiInfo.workspaceInfo.gitsStream) {

        // TODO : Documentation
        ImGui::BeginDisabled(guiInfo.workspaceInfo.streamInfo.running);
        {
            // TODO : Documentation
            if (guiInfo.workspaceInfo.streamInfo.benchmarks.empty()) {
                std::filesystem::path streamPath = guiInfo.workspaceInfo.launch;
                auto benchmarkFilePath = streamPath / "benchmark.csv";
                if (std::filesystem::exists(benchmarkFilePath)) {

                    // TODO : Documentation
                    std::string line;
                    std::vector<std::string> lines;
                    std::ifstream file(benchmarkFilePath);
                    while (std::getline(file, line)) {
                        lines.push_back(line);
                    }

                    // Extract benchmark values (expecting 4) to guiInfo.workspaceInfo.streamInfo.benchmarks
                    std::vector<std::string> columnHeaders;
                    if (!lines.empty()) {
                        auto itr = lines.begin();
                        columnHeaders = gvk::string::split(gvk::string::replace(*itr, ", ", ","), ",");
                        while (++itr != lines.end()) {
                            auto values = gvk::string::split(gvk::string::replace(*itr, ", ", ","), ",");
                            assert(columnHeaders.size() == values.size());
                            for (size_t i = 0; i < columnHeaders.size() && i < values.size(); ++i) {
                                StreamInfo::Benchmark benchmark{ };
                                benchmark.value = gvk::string::to_number<double>(values[i]);
                                benchmark.str = values[i];
                                guiInfo.workspaceInfo.streamInfo.benchmarks[columnHeaders[i]].push_back(benchmark);
                            }
                        }
                    }

                    // Only extract stamp (Frame Time) and make a graph based off of that
                    auto itr = guiInfo.workspaceInfo.streamInfo.benchmarks.find("stamp");
                    if (itr != guiInfo.workspaceInfo.streamInfo.benchmarks.end()) {
                        guiInfo.workspaceInfo.streamInfo.selectedBenchmark = "Frame Time";
                        guiInfo.workspaceInfo.streamInfo.selectedBenchmarkValues.clear();
                        auto previousValue = !itr->second.empty() ? itr->second.front().value : 0.0;
                        for (size_t i = 1; i < itr->second.size(); ++i) {
                            auto value = itr->second[i].value;
                            guiInfo.workspaceInfo.streamInfo.selectedBenchmarkValues.push_back(value - previousValue);
                            previousValue = value;
                        }
                    }
                }
            }

            // TODO : Documentation
            if (!guiInfo.workspaceInfo.streamInfo.selectedBenchmarkValues.empty()) {

                multiFrameChart.chartName = "Multi-Frame View";
                multiFrameChart.xAxisName = "Total Time (ms)";
                multiFrameChart.yAxisName = "Frame Time (ms)";
                multiFrameChart.border = true;

                BarChart::PlotInfo plotInfo;
                plotInfo.pLabelName = "CPU Frame Duration";
                plotInfo.pYData = guiInfo.workspaceInfo.streamInfo.selectedBenchmarkValues.data();
                plotInfo.yDataCount = (int)guiInfo.workspaceInfo.streamInfo.selectedBenchmarkValues.size();

                //We will get streamDuration from a report later on. For now we calculate here.
                double streamDuration = 0;
                for (auto& n : guiInfo.workspaceInfo.streamInfo.selectedBenchmarkValues) {
                    streamDuration += n;
                }
                plotInfo.yValuesSum = streamDuration;
#if 0
                multiFrameChart.flagValue |= static_cast<int>(BarChartFlags::DYNAMIC_BARS) |
                    static_cast<int>(BarChartFlags::CONTINUOUS_SELECTION) |
                    static_cast<int>(BarChartFlags::BORDER) |
                    static_cast<int>(BarChartFlags::TOOL_TIPS_FOR_FRAMES);
#endif
                multiFrameChart.flagValue |= static_cast<int>(BarChartFlags::WIP);
                multiFrameChart.plot_bar_chart(plotInfo);

                ImGui::SeparatorText("Trace Information");
                std::string totalFrames = "Total Frames: " + std::to_string(plotInfo.yDataCount);
                std::string debugSelectedStreams = "";
                if (multiFrameChart.selectedBars.empty()) {
                    ImGui::Text("%s", totalFrames.c_str());
                } else {
                    totalFrames += "      Selected Frames: " + std::to_string(multiFrameChart.selectedBars.size());
                    ImGui::Text("%s", totalFrames.c_str());
                    for (auto& it : multiFrameChart.selectedBars) {
                        debugSelectedStreams += " " + std::to_string(it.first);
                    }
                    ImGui::Text("%s", debugSelectedStreams.c_str());
                }
            }

            // TODO : Documentation
            ImGui::SeparatorText("Frame Tools");


            #if GVK_GITS_ENABLED
            // TODO : Documentation
            if (ImGui::Button("Launch")) {
                guiInfo.workspaceInfo.streamInfo.running = true;
                guiInfo.workspaceInfo.streamInfo.loopCount = 0;
                if (mStreamPlaybackThread.joinable()) {
                    mStreamPlaybackThread.join();
                }
                mStreamPlaybackThread = std::thread(stream_playback_thread_proc, &guiInfo);
            }
            #else
            ImGui::BeginDisabled();
            ImGui::Button("Launch");
            ImGui::EndDisabled();
            #endif // GVK_GITS_ENABLED
        }
        ImGui::EndDisabled();

        // TODO : Documentation
        ImGui::BeginDisabled(!guiInfo.workspaceInfo.streamInfo.running);
        {
            if (ImGui::Button("Stop")) {
                guiInfo.workspaceInfo.streamInfo.stop = true;
            }
            if (ImGui::Button("Run Frame") || guiInfo.resultPending) {
                guiInfo.workspaceInfo.streamInfo.runFrame = true;
            }
            if (ImGui::Button("Get Frame API Calls")) {
                guiInfo.requestInfo.getApiCalls = true;
            }
            if (ImGui::Button("Get Frame GPU Calls")) {
                guiInfo.requestInfo.getGpuCalls = true;
            }
            ImGui::Text("Loop Iterations : %zu", guiInfo.workspaceInfo.streamInfo.loopCount);
        }
        ImGui::EndDisabled();
    }
}

void StreamPlaybackWindow::stream_playback_thread_proc(GuiInfo* pGuiInfo)
{
    (void)pGuiInfo;
#if GVK_GITS_ENABLED
    // NOTE : Doesn't work here...probably because the loader is already loaded
    gvk::set_env_var("VK_LOADER_DEBUG", "all");

    ///////////////////////////////////////////////////////////////////////////////
    // TODO : DRY
    // Get the path to this gui .exe, the layer .dll should be next to it
    HMODULE hModule = NULL;
    const size_t CharBufferSize = 16384;
    static std::array<wchar_t, CharBufferSize> wcharBuffer;
    wcharBuffer = { };
    if (get_this_module_handle(&hModule)) {
        GetModuleFileNameW(hModule, wcharBuffer.data(), (DWORD)wcharBuffer.size());
    }
    std::filesystem::path guiPath = wcharBuffer[0] ? wcharBuffer.data() : std::filesystem::path();
    std::filesystem::path layerPath = std::filesystem::path(guiPath).remove_filename();
    gvk::append_value_to_env_var("VK_ADD_LAYER_PATH", layerPath.string());
    gvk::append_value_to_env_var("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_INTEL_gvk_pipeline_explorer,VK_LAYER_INTEL_gvk_virtual_swapchain");

    // TODO : Documentation
    gvk::set_env_var("GVK_PIPELINE_EXPLORER_WORKSPACE", pGuiInfo->workspaceInfo.workspace);
    gvk::set_env_var("GVK_PIPELINE_EXPLORER_TARGET", get_target(pGuiInfo->workspaceInfo));
    if (pGuiInfo->workspaceInfo.waitForDebugger) {
        gvk::set_env_var("GVK_PIPELINE_EXPLORER_WAIT_FOR_DEBUGGER", "1");
    }
    ///////////////////////////////////////////////////////////////////////////////

    // TODO : Documentation
    auto gitsPlayerPath = std::filesystem::path(pGuiInfo->workspaceInfo.gits) / "Player";
    auto libGitsPath = gitsPlayerPath / "libGits.dll";

    // Load GITS
    auto pLibGits = gvk_dlopen(libGitsPath.string().c_str());
    auto gitsResult = pLibGits ? GITS_SUCCESS : GITS_ERROR_INITIALIZATION_FAILED;
    assert(gitsResult == GITS_SUCCESS);
    if (gitsResult != GITS_SUCCESS) {
        // TODO : Error handling
    }

    // Get GITS dispatch table
    auto pfnGitsGetDispatchTable = (PFN_gitsGetDispatchTable)gvk_dlsym(pLibGits, "gitsGetDispatchTable");
    gitsResult = pfnGitsGetDispatchTable ? GITS_SUCCESS : GITS_ERROR_INITIALIZATION_FAILED;
    assert(gitsResult == GITS_SUCCESS);
    if (gitsResult != GITS_SUCCESS) {
        // TODO : Error handling
    }
    gitsResult = pfnGitsGetDispatchTable(&pGuiInfo->workspaceInfo.streamInfo.gitsDispatchTable);
    assert(gitsResult == GITS_SUCCESS);
    if (gitsResult != GITS_SUCCESS) {
        // TODO : Error handling
    }

    // Create GITS instance
    GitsInstanceCreateInfo gitsInstanceCreateInfo{ };
    gitsInstanceCreateInfo.logLevel = GITS_LOG_LEVEL_INFO;
    gitsResult = pGuiInfo->workspaceInfo.streamInfo.gitsDispatchTable.pfnCreateInstance(&gitsInstanceCreateInfo, &pGuiInfo->workspaceInfo.streamInfo.gitsInstance);
    assert(gitsResult == GITS_SUCCESS);
    if (gitsResult != GITS_SUCCESS) {
        // TODO : Error handling
    }

    // Create GITS player
    GitsPlayerCreateInfo gitsPlayerCreateInfo{ };
    gitsPlayerCreateInfo.pStreamPath = pGuiInfo->workspaceInfo.launch.c_str();
    gitsPlayerCreateInfo.loopFrame = 2; //  64;
    gitsPlayerCreateInfo.pfnLoopEndEvent = stream_playback_on_loop_end;
    gitsPlayerCreateInfo.pfnTickEvent = stream_playback_on_tick;
    gitsPlayerCreateInfo.pEventUserData = pGuiInfo;
    gitsResult = pGuiInfo->workspaceInfo.streamInfo.gitsDispatchTable.pfnCreatePlayer(pGuiInfo->workspaceInfo.streamInfo.gitsInstance, &gitsPlayerCreateInfo, &pGuiInfo->workspaceInfo.streamInfo.gitsPlayer);
    assert(gitsResult == GITS_SUCCESS);
    if (gitsResult != GITS_SUCCESS) {
        // TODO : Error handling
    }

    // Unload GITS
    pGuiInfo->workspaceInfo.streamInfo.gitsDispatchTable.pfnDestroyPlayer(pGuiInfo->workspaceInfo.streamInfo.gitsInstance, pGuiInfo->workspaceInfo.streamInfo.gitsPlayer);
    pGuiInfo->workspaceInfo.streamInfo.gitsDispatchTable.pfnDestroyInstance(pGuiInfo->workspaceInfo.streamInfo.gitsInstance);
    gvk_dlclose(pLibGits);
    pLibGits = nullptr;

    // TODO : Documentation
    pGuiInfo->workspaceInfo.streamInfo.running = false;
    pGuiInfo->workspaceInfo.streamInfo.stop = false;
    pGuiInfo->workspaceInfo.streamInfo.runFrame = false;
    pGuiInfo->workspaceInfo.streamInfo.gitsInstance = nullptr;
    pGuiInfo->workspaceInfo.streamInfo.gitsPlayer = nullptr;
    pGuiInfo->workspaceInfo.streamInfo.loopCount = 0;
    pGuiInfo->workspaceInfo.streamInfo.gitsDispatchTable = { };

    ///////////////////////////////////////////////////////////////////////////////
    // TODO : Unify application shutdown and stream shutdown
    pGuiInfo->requestInfo = gvk::get_default<GvkPipelineExplorerRequestInfo>();
    pGuiInfo->requestInfo.warmupRangeCount = 4;
    pGuiInfo->requestInfo.queryRangeCount = 16;
    pGuiInfo->activePipelines.clear();
    pGuiInfo->sortedPipelines.clear();
    pGuiInfo->pipelineInfos.clear();
    pGuiInfo->availableMetrics.clear();
    pGuiInfo->filteredMetrics.clear();
    pGuiInfo->metricsFilters.clear();
    pGuiInfo->metricsAnyOfFilter.clear();
    pGuiInfo->metricsAllOfFilter.clear();
    pGuiInfo->selectedPipeline = { };
    pGuiInfo->enabledMetricsGroup = 0;
    pGuiInfo->applicationInfo = { };
    pGuiInfo->resultPending = false;
    // TODO : Clear WindowManager
    ///////////////////////////////////////////////////////////////////////////////
#endif // GVK_GITS_ENABLED
}

void StreamPlaybackWindow::stream_playback_on_tick(void* pUserData)
{
    (void)pUserData;
#if GVK_GITS_ENABLED
    auto& guiInfo = *(GuiInfo*)pUserData;
    if (guiInfo.workspaceInfo.streamInfo.runFrame || guiInfo.resultPending) {
        guiInfo.workspaceInfo.streamInfo.runFrame = false;
        guiInfo.workspaceInfo.streamInfo.gitsDispatchTable.pfnPlayStream(guiInfo.workspaceInfo.streamInfo.gitsPlayer, nullptr);
    }
    if (guiInfo.workspaceInfo.streamInfo.stop) {
        guiInfo.workspaceInfo.streamInfo.gitsDispatchTable.pfnStopStream(guiInfo.workspaceInfo.streamInfo.gitsPlayer);
    }
#endif // GVK_GITS_ENABLED
}

void StreamPlaybackWindow::stream_playback_on_loop_end(void* pUserData)
{
    (void)pUserData;
#if GVK_GITS_ENABLED
    auto& guiInfo = *(GuiInfo*)pUserData;
    guiInfo.workspaceInfo.streamInfo.loopCount++;
    if (2000 <= guiInfo.workspaceInfo.streamInfo.loopCount) {
        guiInfo.workspaceInfo.streamInfo.gitsDispatchTable.pfnStopStream(guiInfo.workspaceInfo.streamInfo.gitsPlayer);
    }
    if (!guiInfo.resultPending) {
        guiInfo.workspaceInfo.streamInfo.gitsDispatchTable.pfnPauseStream(guiInfo.workspaceInfo.streamInfo.gitsPlayer);
    }
#endif // GVK_GITS_ENABLED
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

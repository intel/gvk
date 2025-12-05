
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

#include "gvk-pipeline-explorer/gui/metrics/metrics-tab.hpp"

#if GVK_AUTOCORR_ENABLED
#include "gvk-autocorr/autocorr.hpp"
#endif

#include <map>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

class MetricSetResults final
{
public:
    std::set<uint32_t, PerformanceCounterResult> metricResults;
};

class MetricGroupResults final
{
public:
    std::map<uint32_t, MetricSetResults> metricSetResults;
};

class PluginMetricsTab final
    : public MetricsTab
{
public:
    PluginMetricsTab(MetricsWindow& metricsWindow);
    uint32_t get_selected_group();
    uint32_t get_selected_set();
    bool idle(GuiInfo& guiInfo) const override final;
    bool enabled(GuiInfo& guiInfo) const override final;
    void submit_metrics_query_request(GuiInfo& guiInfo) override final;
    void on_update(GuiInfo& guiInfo) override final;
    void on_gui(GuiInfo& guiInfo) override final;

private:
    void filter_counters(GuiInfo& guiInfo) override final;
    void draw_gui(GuiInfo& guiInfo);
    void draw_metric_group(const std::pair<uint32_t, std::set<uint32_t>>& groupItr, GuiInfo& guiInfo);
    void draw_metric_set(const std::pair<uint32_t, std::set<uint32_t>>& groupItr, GuiInfo& guiInfo);

    uint32_t mSelectedGroup{ };
    std::map<uint32_t, uint32_t> mSelectedSets;
    std::map<uint32_t, std::set<uint32_t>> mFiltered;
    std::map<uint32_t, std::vector<uint32_t>> mFilteredEx;
    RequestResult<GvkPipelineExplorerPerformanceQueryRequestInfo, GvkPipelineExplorerPerformanceQueryResultInfo> mRequestResult;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, PerformanceCounterResult>>>> mResults;
#if GVK_AUTOCORR_ENABLED
    friend class AutocorrWindow;
#endif
};

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

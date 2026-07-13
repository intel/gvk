
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

#include "gvk-pipeline-explorer/gui/window.hpp"

#include <limits>
#include <map>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

class MetricsWindow;

class MetricsTab
{
public:
    class PlotResults final
    {
    public:
        double minTimestamp{ std::numeric_limits<double>::max() };
        double maxTimestamp{ std::numeric_limits<double>::min() };
        double minValue{ std::numeric_limits<double>::max() };
        double maxValue{ std::numeric_limits<double>::min() };
        std::vector<double> timestamps;
        std::vector<double> values;
    };

    MetricsTab(MetricsWindow& metricsWindow);
    virtual ~MetricsTab() = 0;
    virtual void reset();
    MetricsWindow& get_metrics_window();
    virtual const std::string& get_name() const;
    virtual bool idle(GuiInfo& guiInfo) const;
    virtual bool enabled(GuiInfo& guiInfo) const;
    virtual void submit_metrics_query_request(GuiInfo& guiInfo);
    virtual void reset_query_requests(GuiInfo& guiInfo);
    virtual void on_update(GuiInfo& guiInfo);
    virtual void on_plot(GuiInfo& guiInfo);
    virtual void on_gui(GuiInfo& guiInfo);

protected:
    virtual void filter_counters(GuiInfo& guiInfo);
    virtual void sort_counters(GuiInfo& guiInfo);
    virtual void draw_plot(GuiInfo& guiInfo, const char* pLabel);

    std::string mName;
    bool mAutoQuery{ true };
    std::map<std::string, bool> mCategories;
    double mMinTimestamp{ std::numeric_limits<double>::max() };
    double mMaxTimestamp{ std::numeric_limits<double>::min() };
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::map<std::string, PlotResults>> mPlotResults;

private:
    MetricsWindow& mMetricsWindow;
    MetricsTab(const MetricsTab&) = delete;
    MetricsTab& operator=(const MetricsTab&) = delete;
};

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

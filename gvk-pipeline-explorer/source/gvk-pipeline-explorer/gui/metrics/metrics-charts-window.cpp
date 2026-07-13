
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

#include "gvk-pipeline-explorer/gui/metrics/metrics-charts-window.hpp"
#include "gvk-pipeline-explorer/gui/metrics/metrics-window.hpp"

#include <functional>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

MetricsChartsWindow::MetricsChartsWindow(Window::Manager& windowManager, MetricsWindow& metricsWindow)
    : Window(windowManager, "Metrics Charts")
    , mMetricsWindow{ metricsWindow }
{
}

void MetricsChartsWindow::on_gui(GuiInfo& guiInfo)
{
    auto pActiveMetricsTab = mMetricsWindow.get_active_metrics_tab();
    if (pActiveMetricsTab) {
        pActiveMetricsTab->on_plot(guiInfo);
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

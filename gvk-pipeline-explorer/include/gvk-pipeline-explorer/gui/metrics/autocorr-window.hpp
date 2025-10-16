
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
#ifndef GVK_AUTOCORR_ENABLED
#define GVK_AUTOCORR_ENABLED 0
#endif
#ifndef GVK_MDAPI_ENABLED
#define GVK_MDAPI_ENABLED 0
#endif

#include "gvk-pipeline-explorer/gui/window.hpp"
#if GVK_AUTOCORR_ENABLED
#include "gvk-autocorr/autocorr.hpp"
#endif
#if GVK_MDAPI_ENABLED
#include "gvk-mdapi/mdapi.hpp"
#endif
#include "gvk-structures.hpp"
#include "gvk-python/context.hpp"


namespace gvk {
namespace pipeline_explorer {
namespace gui {
class PluginMetricsTab;

class AutocorrWindow final
    : public Window
{
public:
    AutocorrWindow(Window::Manager& windowManager, const std::string&, PluginMetricsTab& pluginMetricsTab);

protected:
    void on_gui(GuiInfo& guiInfo) override final;
private:  
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> create_request(GuiInfo& guiInfo, uint32_t group_i, uint32_t set_i);
    std::string detect_similar_metrics(std::string expectedMetricName);

private:
    PluginMetricsTab& mPluginMetricsTab;
    std::vector<gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo>> mRequests;
    std::vector<gvk::Auto<GvkPipelineExplorerPerformanceQueryResultInfo>> mResults;
    RequestResult<GvkPipelineExplorerPerformanceQueryRequestInfo, GvkPipelineExplorerPerformanceQueryResultInfo> mRequestResult;
#if GVK_MDAPI_ENABLED
    gvk::Auto<GvkMdapiGlobalSymbolCollection> mGlobalSymbolCollection;
    gvk::Auto<MetricsDiscovery::SMetricsDeviceParams_1_2> mMetricsDeviceParams;
    gvk::Auto<MetricsDiscovery::TAdapterParams_1_9> mAdapterParams;
#endif
    static gvk::python::Context mPythonContext;
};

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk


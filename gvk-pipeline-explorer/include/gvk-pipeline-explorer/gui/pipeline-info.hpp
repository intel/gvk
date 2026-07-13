
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

#include "gvk-command-structures.hpp"
#include "gvk-defines.hpp"
#include "gvk-gui.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-reference.hpp"

#include <map>
#include <unordered_set>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

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

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

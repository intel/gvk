
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
#include "gvk-gui.hpp"
#include "gvk-reference.hpp"
#include "gvk-system/time.hpp"

#include <filesystem>
#include <map>
#include <unordered_map>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

class GuiInfo;

class CmdInfo final
{
public:
    std::string label;
    std::string content;
    uint64_t threadId{ };
    VkDevice device{ };
    VkQueue queue{ };
    VkPipeline pipeline{ };
    uint64_t beginNs{ };
    uint64_t endNs{ };
};

class PipelineExecutionInfo final
{
public:
    uint64_t executionCount{ };
    uint64_t totalDurationNs{ };
};

class RangeInfo final
{
public:
    RangeInfo();
    void add_timeline_query_result_info(gvk::Auto<GvkPipelineExplorerTimelineQueryResultInfo>&& timelineQueryResultInfo);
    void save(const GuiInfo& guiInfo, const std::filesystem::path& path);
    void load(GuiInfo& guiInfo, const std::filesystem::path& path);

    std::string name;
    std::vector<CmdInfo> cmdInfos;
    uint64_t selectedCmd{ std::numeric_limits<uint64_t>::max() };
    uint64_t minCmdTimestamp{ std::numeric_limits<uint64_t>::max() };
    uint64_t maxCmdTimestamp{ std::numeric_limits<uint64_t>::min() };
    uint64_t maxCmdDuration{ std::numeric_limits<uint64_t>::min() };
    uint64_t submitCount{ };
    uint64_t presentCount{ };
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, PipelineExecutionInfo> pipelineExecutionInfos;
    std::vector<gvk::Auto<GvkPipelineExplorerTimelineQueryResultInfo>> timelineQueryResultInfos;
};

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

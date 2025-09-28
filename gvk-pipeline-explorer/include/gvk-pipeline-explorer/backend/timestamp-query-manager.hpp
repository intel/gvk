
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

#include "gvk-pipeline-explorer/backend/query-manager.hpp"

#include <filesystem>
#include <unordered_map>

namespace gvk {
namespace pipeline_explorer {

class CmdSequence final
{
public:
    uint64_t beginTimestamp{ };
    uint64_t endTimestamp{ };
    uint64_t firstCmdIndex{ };
    std::vector<GvkCommandStructureType> cmdTypes;
};

class TimestampQueryManager final
    : public gvk::pipeline_explorer::QueryManager
{
public:
    TimestampQueryManager() = default;
    const GvkPipelineExplorerPerformanceQueryRequestInfo& get_request() const;
    VkResult initialize_auto_query();
    VkResult submit_request(const std::filesystem::path& workspace, gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo>&& request);
    bool collect_metrics(VkDevice device, VkPipeline pipeline) const;
    void extract_results(std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::vector<std::vector<CmdSequence>>>& extractResults);

protected:
    uint32_t get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const override final;
    VkResult validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_range() override final;
    VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_range() override final;

private:
    VkResult write_timestamp(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo, VkPipelineStageFlagBits pipelineStage);
    VkResult publish_result() const;

    bool mAutoQuery{ };
    std::filesystem::path mWorkspace;
    gvk::Auto<GvkPipelineExplorerPerformanceQueryRequestInfo> mRequest;
    uint32_t resultIndex{ };
    uint32_t warmupRangeCount{ };
    uint32_t queryRangeCount{ };
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::vector<std::vector<CmdSequence>>> results;

    TimestampQueryManager(const TimestampQueryManager&) = delete;
    TimestampQueryManager& operator=(const TimestampQueryManager&) = delete;
};

} // namespace pipeline_explorer
} // namespace gvk

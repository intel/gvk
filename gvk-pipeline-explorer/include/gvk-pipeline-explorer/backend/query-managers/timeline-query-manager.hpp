
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

#include "gvk-pipeline-explorer/backend/query-managers/query-manager.hpp"
#include "gvk-pipeline-explorer/backend/ipc-messenger.hpp"
#include "gvk-command-structures.hpp"

#include <array>
#include <filesystem>
#include <memory>
#include <string>

namespace gvk {
namespace pipeline_explorer {

class TimelineQueryManager final
    : public gvk::pipeline_explorer::QueryManager
{
public:
    TimelineQueryManager() = default;
    ~TimelineQueryManager() override final;
    uint64_t get_type_id() const override final;
    VkResult enable(const std::filesystem::path& reportPath, gvk::pipeline_explorer::IpcMessenger* pIpcMessenger);
    VkResult disable();
    bool collect_metrics(VkDevice device, VkPipeline pipeline) const;

protected:
    bool tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const override final;
    uint32_t get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const override final;
    VkResult validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_range() override final;
    VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult pre_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_range() override final;

private:
    VkResult write_timestamp(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo, VkPipelineStageFlagBits pipelineStage);

    IpcMessenger* mpIpcMessenger{ };
    std::array<uint64_t, 2> mCalibrationTimestamps{ };
    GvkCommandStructureQueueSubmit mQueueSubmit{ };
    GvkPipelineExplorerTimelineCommandInfo mQueueSubmitInfo{ };
    uint64_t mCalibrationUnixEpochNs{ };
};

} // namespace pipeline_explorer
} // namespace gvk

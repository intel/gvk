
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

#include "gvk-pipeline-explorer/backend/tool.hpp"
#include "gvk-command-structures.hpp"

#include <filesystem>

namespace gvk {
namespace pipeline_explorer {

class CommandCollectionRequestManager final
    : public gvk::pipeline_explorer::Tool
{
public:
    CommandCollectionRequestManager() = default;
    uint64_t get_type_id() const  override final;
    const GvkPipelineExplorerCommandCollectionRequestInfo& get_request() const;
    void process_incoming_requests(const std::filesystem::path& workspace);

protected:
    virtual bool tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const override final;
    VkResult pre_process_range() override final;
    VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override final;
    VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override final;
    VkResult post_process_range() override final;

private:
    VkResult publish_result() const;

    std::filesystem::path mWorkspace;
    gvk::Auto<GvkPipelineExplorerCommandCollectionRequestInfo> mRequest;
    gvk::BasicCommandRecorder mCommandRecorder;

    CommandCollectionRequestManager(const CommandCollectionRequestManager&) = delete;
    CommandCollectionRequestManager& operator=(const CommandCollectionRequestManager&) = delete;
};

} // namespace pipeline_explorer
} // namespace gvk

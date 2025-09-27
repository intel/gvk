
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

namespace gvk {
namespace pipeline_explorer {

class QueryManager
    : public gvk::pipeline_explorer::Tool
{
public:
    QueryManager() = default;
    virtual ~QueryManager() = 0;
    virtual void reset() override;

protected:
    virtual uint32_t get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const;
    virtual uint32_t get_query_count(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const;
    virtual VkResult validate_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) = 0;
    virtual VkResult reset_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult pre_process_range() override;
    virtual VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override;
    virtual VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override;
    virtual VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override;
    virtual VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) override;
    virtual VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override;
    virtual VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) override;
    virtual VkResult post_process_range() override;
    virtual VkResult generate_report();

    gvk::QueryPool mQueryPool;
    uint32_t mQueryIndex{ };
    uint32_t mResultIndex{ };

private:
    QueryManager(const QueryManager&) = delete;
    QueryManager& operator=(const QueryManager&) = delete;
};

} // namespace pipeline_explorer
} // namespace gvk

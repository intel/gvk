
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

////////////////////////////////////////////////////////////////////////////////
// TODO : Very annoying that Windows and Linux need different include orders for
//  these...that's a very good indicator that these utilities need a rework
#include "gvk-pipeline-explorer/generated/pipeline-explorer.h"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-enumerations-to-string.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-comparison-operators.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-create-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-deserialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-destroy-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-get-stype.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-serialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-to-string.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "gvk-pipeline-explorer.hpp"
#include "gvk-handles.hpp"

#include <vector>

namespace gvk {
namespace pipeline_explorer {

class Tool
{
public:
    class DispatchManager;

    Tool() = default;
    virtual ~Tool() = 0;
    virtual void reset();
    virtual uint64_t get_type_id() const = 0;

    template <typename Type>
    static uint64_t get_type_id()
    {
        static uint8_t sId;
        return (uint64_t)&sId;
    }

protected:
    virtual bool tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const;
    virtual VkResult pre_process_range();
    virtual VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    virtual VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    virtual VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    virtual VkResult pre_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    virtual VkResult post_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    virtual VkResult post_process_range();
    bool mEnabled{ false };

private:
    Tool(const Tool&) = delete;
    Tool& operator=(const Tool&) = delete;
};

} // namespace pipeline_explorer
} // namespace gvk

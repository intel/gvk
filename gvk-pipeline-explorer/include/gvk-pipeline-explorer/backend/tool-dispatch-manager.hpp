
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

#include "gvk-pipeline-explorer/backend/tool.hpp"
#include "gvk-pipeline-explorer/backend/ipc-messenger.hpp"
#include "gvk-system/time.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-handles.hpp"

#include <vector>

namespace gvk {
namespace pipeline_explorer {

class Tool::DispatchManager final
{
public:
    DispatchManager() = default;
    ~DispatchManager();
    void reset();
    void update(GvkCommandStructureType delimiter = { });
    void submit_request(const GvkPipelineExplorerQueryRequestInfo& requestInfo, IpcMessenger& ipcMessenger);
    void enable();
    void disable();
    bool enabled() const;
    bool tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const;

    void push_back(Tool* pTool)
    {
        if (pTool) {
            erase(pTool->get_type_id());
            if (mToolIdIndices.insert({ pTool->get_type_id(), mTools.size() }).second) {
                mTools.push_back(pTool);
            }
        }
    }

    template <typename ToolType>
    Tool* get() const
    {
        auto itr = mToolIdIndices.find(Tool::get_type_id<ToolType>());
        return (itr != mToolIdIndices.end() && itr->second < mTools.size()) ? mTools[itr->second] : nullptr;
    }

    template <typename ToolType>
    void erase()
    {
        erase(Tool::get_type_id<ToolType>());
    }

    void erase(uint64_t typeId)
    {
        auto itr = mToolIdIndices.find(typeId);
        if (itr != mToolIdIndices.end()) {
            auto index = itr->second;
            assert(index < mTools.size());
            assert(mTools[index]);
            mToolIdIndices.erase(itr);
            mTools.erase(mTools.begin() + index);
            // NOTE : After erasing a tool, the loop walks the remaining tools to fix up indices.
            //  This is correct but O(n).  Not a concern at typical tool counts, but worth noting.
            for (; index < mTools.size(); ++index) {
                auto pTool = mTools[index];
                assert(pTool);
                itr = mToolIdIndices.find(pTool->get_type_id());
                assert(itr != mToolIdIndices.end());
                itr->second = index;
            }
        }
    }

    VkResult pre_process_range();
    VkResult pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    VkResult pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    VkResult post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    VkResult post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo);
    VkResult pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    VkResult post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    VkResult pre_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    VkResult post_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo);
    VkResult post_process_range();

private:
    IpcMessenger* mpIpcMessenger{ };
    GvkPipelineExplorerTimelineQueryInterval mQueryInterval{ };
    GvkPipelineExplorerTimelineQueryInterval mQueryStatus{ };
    std::unordered_map<uint64_t, size_t> mToolIdIndices;
    std::vector<Tool*> mTools;
    gvk::system::Timer mTimer;
    bool mEnabled{ false };

    DispatchManager(const DispatchManager&) = delete;
    DispatchManager& operator=(const DispatchManager&) = delete;
};

} // namespace pipeline_explorer
} // namespace gvk

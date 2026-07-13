
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

#include "gvk-pipeline-explorer/backend/tool-dispatch-manager.hpp"

namespace gvk {
namespace pipeline_explorer {

Tool::DispatchManager::~DispatchManager()
{
    reset();
}

void Tool::DispatchManager::reset()
{
    mToolIdIndices.clear();
    mTools.clear();
    mEnabled = false;
}

void Tool::DispatchManager::update(GvkCommandStructureType delimiter)
{
    switch (mQueryStatus.mode) {
    case GVK_PIPELINE_EXPLORER_QUERY_MODE_DELIMITER: {
        if (mQueryStatus.count && mQueryStatus.delimiter == delimiter) {
            --mQueryStatus.count;
        }
        if (!mQueryStatus.count) {
            disable();
        }
    } break;
    case GVK_PIPELINE_EXPLORER_QUERY_MODE_DURATION: {
        if (mQueryStatus.durationMS <= mTimer.total<>()) {
            disable();
        }
    } break;
    default: {
        // NOOP :
    } break;
    }
}

void Tool::DispatchManager::submit_request(const GvkPipelineExplorerQueryRequestInfo& requestInfo, IpcMessenger& ipcMessenger)
{
    mpIpcMessenger = &ipcMessenger;
    mQueryInterval = requestInfo.interval;
    mQueryStatus = mQueryInterval;
    switch (mQueryInterval.mode) {
    case GVK_PIPELINE_EXPLORER_QUERY_MODE_DELIMITER: {
        if ((mQueryInterval.delimiter == GVK_COMMAND_STRUCTURE_TYPE_QUEUE_SUBMIT ||
            mQueryInterval.delimiter == GVK_COMMAND_STRUCTURE_TYPE_QUEUE_PRESENT_KHR) &&
            mQueryInterval.count
        ) {
            enable();
        } else {
            disable();
        }
    } break;
    case GVK_PIPELINE_EXPLORER_QUERY_MODE_DURATION: {
        mTimer.reset();
        if (0 < mQueryInterval.durationMS) {
            enable();
        } else {
            disable();
        }
    } break;
    case GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE: {
        if (mQueryInterval.toggle) {
            enable();
        } else {
            disable();
        }
    } break;
    default: {
        disable();
    } break;
    }
}

void Tool::DispatchManager::enable()
{
    mEnabled = true;
    if (mpIpcMessenger) {
        auto pipelineExplorerQueryRequestInfo = gvk::get_default<GvkPipelineExplorerQueryRequestInfo>();
        pipelineExplorerQueryRequestInfo.interval.mode = GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE;
        pipelineExplorerQueryRequestInfo.interval.toggle = mEnabled;
        mpIpcMessenger->write("GvkPipelineExplorerQueryRequestInfo", pipelineExplorerQueryRequestInfo);
    }
}

void Tool::DispatchManager::disable()
{
    mEnabled = false;
    mQueryInterval = { };
    mQueryStatus = { };
    if (mpIpcMessenger) {
        auto pipelineExplorerQueryRequestInfo = gvk::get_default<GvkPipelineExplorerQueryRequestInfo>();
        pipelineExplorerQueryRequestInfo.interval.mode = GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE;
        pipelineExplorerQueryRequestInfo.interval.toggle = mEnabled;
        mpIpcMessenger->write("GvkPipelineExplorerQueryRequestInfo", pipelineExplorerQueryRequestInfo);
    }
}

bool Tool::DispatchManager::enabled() const
{
    for (const auto& pTool : mTools) {
        if (pTool && pTool->mEnabled) {
            return mEnabled;
        }
    }
    return false;
}

bool Tool::DispatchManager::tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const
{
    for (const auto& pTool : mTools) {
        if (pTool && pTool->tool_command(pCommand, vkDevice, vkQueue, vkPipeline)) {
            return mEnabled;
        }
    }
    return false;
}

VkResult Tool::DispatchManager::pre_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto itr = mTools.begin(); itr != mTools.end(); ++itr) {
                gvk_result_assert(*itr);
                if ((*itr)->mEnabled) {
                    gvk_result((*itr)->pre_process_range());
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto itr = mTools.begin(); itr != mTools.end(); ++itr) {
                gvk_result_assert(*itr);
                if ((*itr)->mEnabled) {
                    gvk_result((*itr)->pre_process_command_buffers(toolInfo));
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto itr = mTools.begin(); itr != mTools.end(); ++itr) {
                gvk_result_assert(*itr);
                if ((*itr)->mEnabled) {
                    gvk_result((*itr)->pre_process_cmd(toolInfo));
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto ritr = mTools.rbegin(); ritr != mTools.rend(); ++ritr) {
                gvk_result_assert(*ritr);
                if ((*ritr)->mEnabled) {
                    gvk_result((*ritr)->post_process_cmd(toolInfo));
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto ritr = mTools.rbegin(); ritr != mTools.rend(); ++ritr) {
                gvk_result_assert(*ritr);
                if ((*ritr)->mEnabled) {
                    gvk_result((*ritr)->post_process_command_buffers(toolInfo));
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        update();
        if (mEnabled) {
            for (auto itr = mTools.begin(); itr != mTools.end(); ++itr) {
                gvk_result_assert(*itr);
                if ((*itr)->mEnabled) {
                    gvk_result((*itr)->pre_process_queue_submission(toolInfo));
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto ritr = mTools.rbegin(); ritr != mTools.rend(); ++ritr) {
                gvk_result_assert(*ritr);
                if ((*ritr)->mEnabled) {
                    gvk_result((*ritr)->post_process_queue_submission(toolInfo));
                }
            }
        }
        update(GVK_COMMAND_STRUCTURE_TYPE_QUEUE_SUBMIT);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::pre_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        update();
        if (mEnabled) {
            for (auto itr = mTools.begin(); itr != mTools.end(); ++itr) {
                gvk_result_assert(*itr);
                if ((*itr)->mEnabled) {
                    gvk_result((*itr)->pre_process_queue_present(toolInfo));
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::post_process_queue_present(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto ritr = mTools.rbegin(); ritr != mTools.rend(); ++ritr) {
                gvk_result_assert(*ritr);
                if ((*ritr)->mEnabled) {
                    gvk_result((*ritr)->post_process_queue_present(toolInfo));
                }
            }
        }
        update(GVK_COMMAND_STRUCTURE_TYPE_QUEUE_PRESENT_KHR);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::DispatchManager::post_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mEnabled) {
            for (auto ritr = mTools.rbegin(); ritr != mTools.rend(); ++ritr) {
                gvk_result_assert(*ritr);
                if ((*ritr)->mEnabled) {
                    gvk_result((*ritr)->post_process_range());
                }
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk

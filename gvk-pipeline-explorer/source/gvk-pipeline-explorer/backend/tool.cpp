
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

#include "gvk-pipeline-explorer/backend/tool.hpp"

namespace gvk {
namespace pipeline_explorer {

Tool::~Tool()
{
}

void Tool::reset()
{
}

VkResult Tool::pre_process_range()
{
    return VK_SUCCESS;
}

VkResult Tool::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult Tool::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult Tool::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult Tool::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult Tool::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult Tool::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    (void)toolInfo;
    return VK_SUCCESS;
}

VkResult Tool::post_process_range()
{
    return VK_SUCCESS;
}

VkResult Tool::pre_process_range(void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->pre_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->pre_process_command_buffers(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->pre_process_cmd(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->post_process_cmd(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->post_process_command_buffers(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->pre_process_queue_submission(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx* pToolInfo, void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pToolInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->post_process_queue_submission(*pToolInfo));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Tool::post_process_range(void* pUserData)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(pUserData ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(((gvk::pipeline_explorer::Tool*)pUserData)->post_process_range());
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk

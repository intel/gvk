
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

#include "gvk-pipeline-explorer/backend/query-managers/query-manager.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"

namespace gvk {
namespace pipeline_explorer {

QueryManager::~QueryManager()
{
}

void QueryManager::reset()
{
    Tool::reset();
    mQueryPool.reset();
    mQueryIndex = 0;
    mResultIndex = 0;
}

uint32_t QueryManager::get_query_count(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo) const
{
    return toolInfo.collectionRangeCount;
}

uint32_t QueryManager::get_query_count(const GvkPipelineExplorerToolQueueInfoEx& toolInfo) const
{
    return toolInfo.collectionRangeCount;
}

VkResult QueryManager::reset_query_resources(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk::Device gvkDevice = toolInfo.device;
        gvk_result_assert(gvkDevice);
        gvk::Queue gvkQueue = toolInfo.queue;
        gvk_result_assert(gvkQueue);
        gvk::pipeline_explorer::QueueInfo queueInfo = toolInfo.queue;
        gvk_result_assert(queueInfo);
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        gvk_result(queueInfo->get_command_buffer(&commandBuffer));
        gvk_result_assert(commandBuffer);

        // Record command buffer to reset query pool
        const auto& dispatchTable = gvkDevice.get<gvk::DispatchTable>();
        auto commandBufferBeginInfo = gvk::get_default<VkCommandBufferBeginInfo>();
        gvk_result(dispatchTable.gvkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
        gvkQueue.get<gvk::DispatchTable>().gvkCmdResetQueryPool(commandBuffer, mQueryPool, 0, get_query_count(toolInfo));
        gvk_result(dispatchTable.gvkEndCommandBuffer(commandBuffer));

        // Submit reset query pool command buffer
        auto submitInfo = gvk::get_default<VkSubmitInfo>();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        gvk_result(gvkQueue.QueueSubmit(1, &submitInfo, VK_NULL_HANDLE));

        // Reset query index
        mQueryIndex = 0;

    } gvk_result_scope_end;
    return gvkResult;
}

VkResult QueryManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(Tool::pre_process_command_buffers(toolInfo));
        if (get_query_count(toolInfo)) {
            gvk_result(validate_query_resources(toolInfo));
            gvk_result(reset_query_resources(toolInfo));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult QueryManager::generate_report()
{
    return VK_SUCCESS;
}

} // namespace pipeline_explorer
} // namespace gvk


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

#include "gvk-pipeline-explorer/backend/query-managers/command-collection-request-manager.hpp"
#include "gvk-pipeline-explorer.hpp"
#include "gvk-structures.hpp"
#include "gvk-system.hpp"

namespace gvk {
namespace pipeline_explorer {

uint64_t CommandCollectionRequestManager::get_type_id() const
{
    return Tool::get_type_id<CommandCollectionRequestManager>();
}

const GvkPipelineExplorerCommandCollectionRequestInfo& CommandCollectionRequestManager::get_request() const
{
    return mRequest;
}

void CommandCollectionRequestManager::process_incoming_requests(const std::filesystem::path& workspace)
{
    mWorkspace = workspace;
#ifdef GVK_PLATFORM_WINDOWS
    if (mRequest->sType != gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>()) {
        switch (gvk::read_serialized_structure(mWorkspace / ".data", mRequest)) {
            // NOOP :
        case VK_INCOMPLETE: {
            // assert(false && "TODO : Error handling");
        } break;
        case VK_NOT_READY:
        default: {
            // NOOP : No file to process
        } break;
        }
    }
#else
    // TODO :
#endif // GVK_PLATFORM_WINDOWS
}

bool CommandCollectionRequestManager::tool_command(const GvkCommandBaseStructure* pCommand, VkDevice vkDevice, VkQueue vkQueue, VkPipeline vkPipeline) const
{
    return Tool::tool_command(pCommand, vkDevice, vkQueue, vkPipeline);
}

VkResult CommandCollectionRequestManager::pre_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::pre_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::pre_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::post_process_cmd(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>()) {
            gvk_result_assert(toolInfo.cmdIndex < toolInfo.cmdCount);
            gvk_result_assert(toolInfo.ppCmds);
            const auto& pCmd = toolInfo.ppCmds[toolInfo.cmdIndex];
            if (pCmd->sType != gvk::get_stype<GvkCommandStructureBeginCommandBuffer>() &&
                pCmd->sType != gvk::get_stype<GvkCommandStructureEndCommandBuffer>()) {
                mCommandRecorder.add_command(*reinterpret_cast<const GvkCommandBaseStructure*>(pCmd));
            }
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::post_process_command_buffers(const GvkPipelineExplorerToolCommandBufferInfoEx& toolInfo)
{
    (void)toolInfo;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::pre_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    (void)toolInfo;
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::post_process_queue_submission(const GvkPipelineExplorerToolQueueInfoEx& toolInfo)
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>()) {
            mCommandRecorder.add_command(*reinterpret_cast<const GvkCommandBaseStructure*>(toolInfo.pCommand));
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::post_process_range()
{
    gvk_result_scope_begin(VK_SUCCESS) {
        if (mRequest->sType == gvk::get_stype<GvkPipelineExplorerCommandCollectionRequestInfo>()) {
            gvk_result(publish_result());
            mRequest.reset();
            mWorkspace.clear();
            mCommandRecorder.reset();
        }
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult CommandCollectionRequestManager::publish_result() const
{
    gvk_result_scope_begin(VK_SUCCESS) {
        gvk_result(VK_SUCCESS);

        const auto& commands = mCommandRecorder.get_commands();
        auto commandCollectionResultInfo = gvk::get_default<GvkPipelineExplorerCommandCollectionResultInfo>();
        commandCollectionResultInfo.commands.commandCount = commands.size();
        commandCollectionResultInfo.commands.ppCommands = !commands.empty() ? commands.data() : nullptr;

        // Get date and time strings
        auto dateTime = gvk::system::DateTime::now();
        auto dateStr = dateTime.get_date_str();
        auto timeStr = dateTime.get_time_str();

        // Write report
        if (mRequest->pReportPath) {
            std::filesystem::path reportPath = mRequest->pReportPath;
            std::filesystem::create_directories(reportPath);
            auto dateTimeStr = gvk::string::replace(dateStr, "/", "-") + "_" + gvk::string::replace(timeStr, ":", "-");
            reportPath /= dateTimeStr + ".GvkPipelineExplorerCommandCollectionResultInfo.json";
            std::ofstream file(reportPath);
            file << gvk::to_string(commandCollectionResultInfo, gvk::Printer::Default & ~gvk::Printer::EnumValue) << std::endl;
        }

        // Send message to frontend
        if (!mWorkspace.empty()) {
#ifdef GVK_PLATFORM_WINDOWS
            gvk::write_serialized_structure(mWorkspace / ".data", commandCollectionResultInfo);
#else
            // TODO :
#endif // GVK_PLATFORM_WINDOWS
        }
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace pipeline_explorer
} // namespace gvk

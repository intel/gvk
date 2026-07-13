
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

#include "gvk-pipeline-explorer/gui/range-info.hpp"
#include "gvk-pipeline-explorer/gui/gui-info.hpp"
#include "gvk-pipeline-explorer/gui/pipelines-window.hpp"

namespace gvk {
namespace pipeline_explorer {
namespace gui {

RangeInfo::RangeInfo()
{
    auto dateTime = gvk::system::DateTime::now();
    name = dateTime.get_date_str('-') + "_" + dateTime.get_time_str('-');
}

void RangeInfo::add_timeline_query_result_info(gvk::Auto<GvkPipelineExplorerTimelineQueryResultInfo>&& timelineQueryResultInfo)
{
    // TODO : Use vkQueueSubmit timestamps to create region markers

    // HACK : Backend sends this message on queue submit, so using that to increment
    //  submit count here, but backend really should send a dedicated message for submit
    ++submitCount;

    auto count = std::min(timelineQueryResultInfo->commandInfoCount, timelineQueryResultInfo->commands.commandCount);
    if (timelineQueryResultInfo->commandInfoCount != timelineQueryResultInfo->commands.commandCount) {
        std::cout << "Warning : Timestamp query incomplete" << std::endl;
    }
    for (uint32_t cmd_i = 0; cmd_i < count; ++cmd_i) {

        // TODO : Documentation
        auto pCmd = timelineQueryResultInfo->commands.ppCommands[cmd_i];
        const auto& timelineCmdInfo = timelineQueryResultInfo->pCommandInfos[cmd_i];
        assert(timelineCmdInfo.beginNs <= timelineCmdInfo.endNs);
        auto duration = timelineCmdInfo.endNs - timelineCmdInfo.beginNs;
        CmdInfo cmdInfo{ };
        cmdInfo.label = gvk::to_string(pCmd->sType, gvk::Printer::Default & ~gvk::Printer::EnumValue);
        cmdInfo.content = gvk::to_string(*pCmd, gvk::Printer::Default & ~gvk::Printer::EnumValue);
        cmdInfo.threadId = timelineCmdInfo.threadId;
        cmdInfo.device = timelineQueryResultInfo->device;
        cmdInfo.queue = timelineCmdInfo.queue;
        cmdInfo.pipeline = timelineCmdInfo.pipeline;
        cmdInfo.beginNs = timelineCmdInfo.beginNs;
        cmdInfo.endNs = timelineCmdInfo.endNs;
        cmdInfos.push_back(cmdInfo);
        minCmdTimestamp = std::min(minCmdTimestamp, cmdInfo.beginNs);
        maxCmdTimestamp = std::max(maxCmdTimestamp, cmdInfo.endNs);
        maxCmdDuration = std::max(maxCmdDuration, duration);

        // TODO : Documentation
        auto& pipelineExecutionInfo = pipelineExecutionInfos[{ timelineQueryResultInfo->device, timelineCmdInfo.pipeline }];
        ++pipelineExecutionInfo.executionCount;
        pipelineExecutionInfo.totalDurationNs += duration;
    }

    timelineQueryResultInfos.push_back(std::move(timelineQueryResultInfo));
}

void RangeInfo::save(const GuiInfo& guiInfo, const std::filesystem::path& path)
{
    // TODO : Figure out why saving RangeInfo causes a hitch in the tooled application
    if (!name.empty() && !timelineQueryResultInfos.empty() && !std::filesystem::exists(path / name)) {
        std::filesystem::create_directories(path / name);

        // TODO : Documentation
        auto width = std::to_string(timelineQueryResultInfos.size() - 1).size();
        for (size_t i = 0; i < timelineQueryResultInfos.size(); ++i) {
            auto& timelineQueryResultInfo = timelineQueryResultInfos[i];
            std::stringstream strStrm;
            strStrm << "queue-submit-" << std::setfill('0') << std::setw(width) << i;
            write_serialized_structure(path / name, strStrm.str(), *timelineQueryResultInfo);
        }

        // TODO : Documentation
        std::vector<GvkPipelineExplorerPipelineResultInfo> pipelineResultInfos(pipelineExecutionInfos.size());
        pipelineResultInfos.reserve(pipelineExecutionInfos.size());
        for (const auto& itr : pipelineExecutionInfos) {
            auto pipelineId = itr.first;
            pipelineResultInfos.push_back(gvk::get_default<GvkPipelineExplorerPipelineResultInfo>());
            pipelineResultInfos.back().pipelineInfo = gvk::get_default<GvkPipelineExplorerPipelineInfo>();
            const auto& pipelineInfoItr = guiInfo.pipelineInfos.find(pipelineId);
            if (pipelineId && pipelineInfoItr != guiInfo.pipelineInfos.end()) {
                const auto& pipelineInfo = pipelineInfoItr->second;

                auto& pipelineExplorerPipelineInfo = pipelineResultInfos.back().pipelineInfo;
                boost::multiprecision::export_bits(pipelineInfo.uuid, pipelineExplorerPipelineInfo.uuid, 8);
                boost::multiprecision::export_bits(pipelineInfo.driverUUID, pipelineExplorerPipelineInfo.driverUUID, 8);
                pipelineExplorerPipelineInfo.device = pipelineInfo.pipeline.get_dispatchable_handle();
                pipelineExplorerPipelineInfo.pipeline = pipelineInfo.pipeline.get_handle();
                pipelineExplorerPipelineInfo.bindPoint = pipelineInfo.bindPoint;
                pipelineExplorerPipelineInfo.experimentEnabled = pipelineInfo.experimentEnabled;
                pipelineExplorerPipelineInfo.highlightEnabled = pipelineInfo.highlightEnabled;
                memcpy(pipelineExplorerPipelineInfo.highlightColor, &pipelineInfo.highlightColor, sizeof(pipelineInfo.highlightColor));
            }
        }
        auto pipelineExplorerResultInfo = gvk::get_default<GvkPipelineExplorerResultInfo>();
        pipelineExplorerResultInfo.pipelineResultCount = (uint32_t)pipelineResultInfos.size();
        pipelineExplorerResultInfo.pPipelineResults = !pipelineResultInfos.empty() ? pipelineResultInfos.data() : nullptr;
        write_serialized_structure(path / name, "pipelines", pipelineExplorerResultInfo);
    }
}

void RangeInfo::load(GuiInfo& guiInfo, const std::filesystem::path& path)
{
    *this = { };

    if (!std::filesystem::exists(path)) {
        return;
    }

    // Extract the range name from the path
    name = path.filename().string();

    // Read timeline query result info files
    std::vector<std::filesystem::path> timelineFiles;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().stem().string().find("queue-submit-") == 0) {
            timelineFiles.push_back(entry.path());
        }
    }

    // Sort files to maintain order
    std::sort(timelineFiles.begin(), timelineFiles.end());

    // Load each timeline query result and populate RangeInfo
    for (const auto& file : timelineFiles) {
        gvk::Auto<GvkPipelineExplorerTimelineQueryResultInfo> timelineQueryResultInfo;
        if (read_serialized_structure(path, file.stem().string(), timelineQueryResultInfo, false) == VK_SUCCESS) {
            add_timeline_query_result_info(std::move(timelineQueryResultInfo));
        }
    }

    // Load pipeline result info
    guiInfo.activePipelines.clear();
    PipelinesWindow::set_selected_pipeline(guiInfo, { });
    gvk::Auto<GvkPipelineExplorerResultInfo> pipelineExplorerResultInfo;
    if (read_serialized_structure(path, "pipelines", pipelineExplorerResultInfo, false) == VK_SUCCESS) {
        for (uint32_t i = 0; i < pipelineExplorerResultInfo->pipelineResultCount; ++i) {
            const auto& pipelineResultInfo = pipelineExplorerResultInfo->pPipelineResults[i];
            const auto& pipelineExplorerPipelineInfo = pipelineResultInfo.pipelineInfo;

            auto device = pipelineExplorerPipelineInfo.device;
            auto pipeline = pipelineExplorerPipelineInfo.pipeline;

            // Skip pipelines with no handle ID
            if (!pipeline) {
                continue;
            }

            guiInfo.activePipelines.insert({ device, pipeline });
            auto& pipelineInfo = guiInfo.pipelineInfos[{ device, pipeline }];

            // Import UUIDs
            boost::multiprecision::import_bits(pipelineInfo.uuid, pipelineExplorerPipelineInfo.uuid, pipelineExplorerPipelineInfo.uuid + GVK_PIPELINE_EXPLORER_UUID_SIZE);
            boost::multiprecision::import_bits(pipelineInfo.driverUUID, pipelineExplorerPipelineInfo.driverUUID, pipelineExplorerPipelineInfo.driverUUID + GVK_PIPELINE_EXPLORER_UUID_SIZE);

            pipelineInfo.pipeline = { device, pipeline };
            pipelineInfo.bindPoint = pipelineExplorerPipelineInfo.bindPoint;

            auto printerFlags = gvk::Printer::Default & ~gvk::Printer::EnumValue;
            pipelineInfo.bindPointStr = gvk::string::remove(gvk::to_string(pipelineExplorerPipelineInfo.bindPoint, printerFlags), "\"");
            pipelineInfo.uuidStr = uuid_to_string(pipelineInfo.uuid, 18);
            pipelineInfo.driverUUIDStr = uuid_to_string(pipelineInfo.driverUUID, 18);
            pipelineInfo.handleStr = gvk::to_hex_string(pipeline);

            // Use name from file if available (pName is omitted in save, so this will be empty)
            if (pipelineExplorerPipelineInfo.pName) {
                pipelineInfo.name = pipelineExplorerPipelineInfo.pName;
            } else {
                pipelineInfo.name = "VkPipeline " + pipelineInfo.handleStr;
            }

            // Load experiment and highlight settings
            pipelineInfo.experimentEnabled = pipelineExplorerPipelineInfo.experimentEnabled;
            pipelineInfo.highlightEnabled = pipelineExplorerPipelineInfo.highlightEnabled;

            // Use the color that was saved in the file
            memcpy(&pipelineInfo.highlightColor, pipelineExplorerPipelineInfo.highlightColor, sizeof(pipelineInfo.highlightColor));
        }

        // Update sorted pipelines list
        guiInfo.sortedPipelines.clear();
        for (auto activePipeline : guiInfo.activePipelines) {
            guiInfo.sortedPipelines.push_back(activePipeline);
        }
        sort_pipelines(guiInfo);
    }
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk

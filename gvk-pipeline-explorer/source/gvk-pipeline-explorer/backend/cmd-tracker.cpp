
/******************************************************************************
© Intel Corporation.
 
This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.


 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#include "gvk-pipeline-explorer/backend/cmd-tracker.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"

namespace gvk {
namespace pipeline_explorer {

void CmdTracker::reset()
{
    BasicCommandRecorder::reset();
}

void CmdTracker::record_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    (void)commandBuffer;
    (void)flags;
    reset();
}

void CmdTracker::record_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    reset();
    auto command = gvk::get_default<GvkCommandStructureBeginCommandBuffer>();
    command.commandBuffer = commandBuffer;
    command.pBeginInfo = pBeginInfo;
    mCommands.push_back((const GvkCommandBaseStructure*)gvk::detail::create_dynamic_array_copy(1, &command, nullptr));
}

void CmdTracker::record_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
    auto command = gvk::get_default<GvkCommandStructureEndCommandBuffer>();
    command.commandBuffer = commandBuffer;
    mCommands.push_back((const GvkCommandBaseStructure*)gvk::detail::create_dynamic_array_copy(1, &command, nullptr));
}

const std::vector<const GvkCommandBaseStructure*>& CmdTracker::get_commands() const
{
    return mCommands;
}

} // namespace pipeline_explorer
} // namespace gvk


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

#include "gvk/mesh.hpp"

namespace gvk {

void Mesh::reset()
{
    mCpuBuffer.reset();
    mGpuBuffer.reset();
    mIndexDataOffset = 0;
    mIndexType = VK_INDEX_TYPE_NONE_KHR;
    mIndexCount = 0;
}

void Mesh::record_cmds(const gvk::CommandBuffer& commandBuffer) const
{
    VkDeviceSize vertexDataOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(const VkBuffer&)mGpuBuffer, &vertexDataOffset);
    vkCmdBindIndexBuffer(commandBuffer, mGpuBuffer, mIndexDataOffset, mIndexType);
    vkCmdDrawIndexed(commandBuffer, (uint32_t)mIndexCount, 1, 0, 0, 0);
}

} // namespace gvk

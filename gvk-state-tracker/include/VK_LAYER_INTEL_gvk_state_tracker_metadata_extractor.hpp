
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
#include "gvk-binding-info.hpp"
#include "gvk-command-structures.hpp"
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

namespace gvk {

class MetadataExtractor final
{
public:
    MetadataExtractor() = default;
    void reset();
    const std::vector<const GvkCommandBaseStructure*>& get_commands() const;
    void add_command(const GvkCommandBaseStructure& command);
    gvk::Auto<GvkBindingInfo> get_binding_info(uint64_t commandIndex) const;

private:
    ////////////////////////////////////////////////////////////////////////////////
    // Command buffer state
    void process_command(const GvkCommandStructureBeginCommandBuffer& command);
    void process_command(const GvkCommandStructureCmdExecuteCommands& command);
    void process_command(const GvkCommandStructureEndCommandBuffer& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Render pass cmds
    void process_command(const GvkCommandStructureCmdBeginRenderPass& command);
    void process_command(const GvkCommandStructureCmdBeginRenderPass2& command);
    void process_command(const GvkCommandStructureCmdBeginRenderPass2KHR& command);
    void process_command(const GvkCommandStructureCmdNextSubpass& command);
    void process_command(const GvkCommandStructureCmdNextSubpass2& command);
    void process_command(const GvkCommandStructureCmdNextSubpass2KHR& command);
    void process_command(const GvkCommandStructureCmdEndRenderPass& command);
    void process_command(const GvkCommandStructureCmdEndRenderPass2& command);
    void process_command(const GvkCommandStructureCmdEndRenderPass2KHR& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Dynamic rendering cmds
    void process_command(const GvkCommandStructureCmdBeginRendering& command);
    void process_command(const GvkCommandStructureCmdBeginRenderingKHR& command);
    void process_command(const GvkCommandStructureCmdEndRendering& command);
    void process_command(const GvkCommandStructureCmdEndRenderingKHR& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Pipeline cmds
    void process_command(const GvkCommandStructureCmdBindPipeline& command);
    void process_command(const GvkCommandStructureCmdPipelineBarrier& command);
    void process_command(const GvkCommandStructureCmdPipelineBarrier2& command);
    void process_command(const GvkCommandStructureCmdPipelineBarrier2KHR& command);
    void process_command(const GvkCommandStructureCmdPushConstants& command);
    void process_command(const GvkCommandStructureCmdPushConstants2& command);
    void process_command(const GvkCommandStructureCmdPushConstants2KHR& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Update/fill/clear cmds
    void process_command(const GvkCommandStructureCmdUpdateBuffer& command);
    void process_command(const GvkCommandStructureCmdFillBuffer& command);
    void process_command(const GvkCommandStructureCmdClearColorImage& command);
    void process_command(const GvkCommandStructureCmdClearDepthStencilImage& command);
    void process_command(const GvkCommandStructureCmdClearAttachments& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Copy/resolve cmds
    void process_command(const GvkCommandStructureCmdCopyBuffer& command);
    void process_command(const GvkCommandStructureCmdCopyBufferToImage& command);
    void process_command(const GvkCommandStructureCmdCopyImage& command);
    void process_command(const GvkCommandStructureCmdCopyImageToBuffer& command);
    void process_command(const GvkCommandStructureCmdBlitImage& command);
    void process_command(const GvkCommandStructureCmdResolveImage& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Index/vertex buffer cmds
    void process_command(const GvkCommandStructureCmdBindIndexBuffer& command);
    void process_command(const GvkCommandStructureCmdBindVertexBuffers& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Bind descriptor cmds
    void process_command(const GvkCommandStructureCmdBindDescriptorSets& command);
    void process_command(const GvkCommandStructureCmdBindDescriptorSets2& command);
    void process_command(const GvkCommandStructureCmdBindDescriptorSets2KHR& command);
    void process_command(const GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplers2EXT& command);
    void process_command(const GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplersEXT& command);
    void process_command(const GvkCommandStructureCmdBindDescriptorBuffersEXT& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Push descriptor cmds
    void process_command(const GvkCommandStructureCmdPushDescriptorSet& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSet2& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSet2KHR& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSetKHR& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplate& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplate2& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplate2KHR& command);
    void process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplateKHR& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Dispatch cmds
    void process_command(const GvkCommandStructureCmdDispatch& command);
    void process_command(const GvkCommandStructureCmdDispatchBase& command);
    void process_command(const GvkCommandStructureCmdDispatchBaseKHR& command);
    void process_command(const GvkCommandStructureCmdDispatchIndirect& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Draw cmds
    void process_command(const GvkCommandStructureCmdDraw& command);
    void process_command(const GvkCommandStructureCmdDrawIndexed& command);
    void process_command(const GvkCommandStructureCmdDrawIndexedIndirect& command);
    void process_command(const GvkCommandStructureCmdDrawIndirect& command);
    void process_command(const GvkCommandStructureCmdDrawIndexedIndirectCountAMD& command);
    void process_command(const GvkCommandStructureCmdDrawIndexedIndirectCountKHR& command);
    void process_command(const GvkCommandStructureCmdDrawIndirectCountAMD& command);
    void process_command(const GvkCommandStructureCmdDrawIndirectCountKHR& command);
    void process_command(const GvkCommandStructureCmdDrawIndirectByteCountEXT& command);
    ////////////////////////////////////////////////////////////////////////////////
    // Ray tracing cmds
    void process_command(const GvkCommandStructureCmdTraceRaysKHR& command);
    ////////////////////////////////////////////////////////////////////////////////
    // GvkResourceInfo registration
    template <typename CreateInfoType, typename ObjectType>
    const GvkResourceInfo* register_resource_info(VkDevice device, ObjectType object);
    const GvkResourceInfo* register_render_pass_resource_info(VkDevice device, VkRenderPass renderPass);
    const GvkResourceInfo* register_descriptor_set_layout_resource_info(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);
    const GvkResourceInfo* register_pipeline_layout_resource_info(VkDevice device, VkPipelineLayout pipelineLayout);
    const GvkResourceInfo* register_pipeline_resource_info(VkDevice device, VkPipeline pipeline);
    ////////////////////////////////////////////////////////////////////////////////
    // GvkDescriptorBindingInfo registration
    const GvkDescriptorBindingInfo* register_buffer_descriptor_binding_info(VkDevice device, uint32_t setIndex, uint32_t bindIndex, uint32_t arrayIndex, const VkDescriptorBufferInfo& descriptor);
    const GvkDescriptorBindingInfo* register_buffer_descriptor_binding_info(VkDevice device, uint32_t setIndex, uint32_t bindIndex, uint32_t arrayIndex, VkBufferView texelBufferView);
    const GvkDescriptorBindingInfo* register_image_descriptor_binding_info(VkDevice device, uint32_t setIndex, uint32_t bindIndex, uint32_t arrayIndex, const VkDescriptorImageInfo& descriptor);
    void bind_descriptor_set(VkDevice device, VkPipelineBindPoint bindPoint, VkDescriptorSet descriptorSet, uint32_t setIndex, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets, uint32_t* pDynamicOffsetIndex);

    gvk::BasicCommandRecorder mCommandRecorder;
    gvk::detail::BindingRegistry mBindingRegistry;
    gvk::detail::ComputeBindingMonitorCollection mComputeBindings;
    gvk::detail::GraphicsBindingMonitorCollection mGraphicsBindings;
    gvk::detail::RayTracingBindingMonitorCollection mRayTracingBindings;
};

} // namespace gvk

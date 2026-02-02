
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

#include "VK_LAYER_INTEL_gvk_state_tracker_metadata_extractor.hpp"

#ifndef VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION
#define VK_LAYER_INTEL_gvk_state_tracker_hpp_IMPLEMENTATION
#endif
#include "VK_LAYER_INTEL_gvk_state_tracker.hpp"

#include "gvk-format-info.hpp"

namespace gvk {

static VkDevice get_device(VkCommandBuffer commandBuffer)
{

    VkDevice device = VK_NULL_HANDLE;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pUserData = &device;
    enumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure*, void* pUserData)
    {
        assert(pStateTrackedObject);
        if (pStateTrackedObject->type == VK_OBJECT_TYPE_DEVICE) {
            assert(pUserData);
            auto pDevice = (VkDevice*)pUserData;
            *pDevice = (VkDevice)pStateTrackedObject->handle;
        }
    };

    // Enumerate VkCommandBuffer dependencies to get its VkDevice
    GvkStateTrackedObject stateTrackedCommandBuffer{ };
    stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
    stateTrackedCommandBuffer.handle = (uint64_t)commandBuffer;
    stateTrackedCommandBuffer.dispatchableHandle = stateTrackedCommandBuffer.handle;
    gvkEnumerateStateTrackedObjectDependencies(&stateTrackedCommandBuffer, &enumerateInfo);
    return device;
}

void MetadataExtractor::reset()
{
    mCommandRecorder.reset();
    mBindingRegistry.reset();
    mComputeBindings.reset();
    mGraphicsBindings.reset();
    mRayTracingBindings.reset();
}

const std::vector<const GvkCommandBaseStructure*>& MetadataExtractor::get_commands() const
{
    return mCommandRecorder.get_commands();
}

template <typename BindingInfoCollectionType>
BindingInfoCollectionType* validate_binding_info_collection(const BindingInfoCollectionType*& pBindingInfoCollection)
{
    if (!pBindingInfoCollection) {
        pBindingInfoCollection = gvk::detail::create_dynamic_array<BindingInfoCollectionType>(1, nullptr);
        *const_cast<BindingInfoCollectionType*>(pBindingInfoCollection) = { };
    }
    return const_cast<BindingInfoCollectionType*>(pBindingInfoCollection);
}

template <typename BindingInfoCollectionType, typename BindingInfoType, typename CopyBindingInfosFunctionType>
void copy_binding_infos(const std::vector<BindingInfoType>& bindingInfos, const BindingInfoCollectionType*& pBindingInfoCollection, CopyBindingInfosFunctionType setBindingInfos)
{
    if (!bindingInfos.empty()) {
        auto pBindingInfos = gvk::detail::create_dynamic_array_copy((uint32_t)bindingInfos.size(), !bindingInfos.empty() ? bindingInfos.data() : nullptr, nullptr);
        setBindingInfos((uint32_t)bindingInfos.size(), pBindingInfos, validate_binding_info_collection(pBindingInfoCollection));
    }
}

gvk::Auto<GvkBindingInfo> MetadataExtractor::get_binding_info(uint64_t command_i) const
{
    // Get bindings at a specified cmd index
    // NOTE : Resource and binding info objects are deep-copied here.  This means that
    //  info objects that are referenced multiple times at a single cmd index will be
    //  fully copied (including create info) for every reference.  This differs from
    //  the GPA FW metadata extractor where info objects were always unique.  This is
    //  due to the fact that the binding registry isn't being serialized because
    //  gvk::Auto<> assumes it owns all memory it manages, which results in double free
    //  when gvk::Auto<> cleans up.
    // TODO : These objects need a custom allocator and it seems it's finally time to
    //  properly hook VkAllocationCallbacks up to gvk::Auto<>.

    // Resource and binding info collections populated via lambda
    gvk::Auto<GvkBindingInfo> bindingInfo;
    auto pBindingInfo = (GvkBindingInfo*)&*bindingInfo;
    std::vector<GvkPipelineBindingInfo> pipelineBindingInfos;
    std::vector<GvkShaderBindingInfo> shaderBindingInfos;
    std::vector<GvkDescriptorBindingInfo> descriptorBindingInfos;
    std::vector<GvkIndexBufferBindingInfo> indexBufferBindingInfos;
    std::vector<GvkVertexBufferBindingInfo> vertexBufferBindingInfos;
    std::vector<GvkRenderTargetBindingInfo> renderTargetBindingInfos;
    auto getBindingInfos = [&](VkPipelineBindPoint bindPoint)
    {
        pipelineBindingInfos.clear();
        shaderBindingInfos.clear();
        descriptorBindingInfos.clear();
        indexBufferBindingInfos.clear();
        vertexBufferBindingInfos.clear();
        for (const auto& bindingInfoBaseStructure : mBindingRegistry.get_binding_infos(bindPoint, command_i)) {
            switch (bindingInfoBaseStructure.sType) {
            case GVK_STRUCTURE_TYPE_PIPELINE_BINDING_INFO: {
                pipelineBindingInfos.push_back((const GvkPipelineBindingInfo&)bindingInfoBaseStructure);
            } break;
            case GVK_STRUCTURE_TYPE_SHADER_BINDING_INFO: {
                shaderBindingInfos.push_back((const GvkShaderBindingInfo&)bindingInfoBaseStructure);
            } break;
            case GVK_STRUCTURE_TYPE_DESCRIPTOR_BINDING_INFO: {
                descriptorBindingInfos.push_back((const GvkDescriptorBindingInfo&)bindingInfoBaseStructure);
            } break;
            case GVK_STRUCTURE_TYPE_INDEX_BUFFER_BINDING_INFO: {
                indexBufferBindingInfos.push_back((const GvkIndexBufferBindingInfo&)bindingInfoBaseStructure);
            } break;
            case GVK_STRUCTURE_TYPE_VERTEX_BUFFER_BINDING_INFO: {
                vertexBufferBindingInfos.push_back((const GvkVertexBufferBindingInfo&)bindingInfoBaseStructure);
            } break;
            case GVK_STRUCTURE_TYPE_RENDER_TARGET_BINDING_INFO: {
                renderTargetBindingInfos.push_back((const GvkRenderTargetBindingInfo&)bindingInfoBaseStructure);
            } break;
            default: {
            } break;
            }
        }
    };

    // Lambdas to populate resource and binding info objects
    auto setPipelineBindingInfo = [](uint32_t bindingInfoCount, const auto* pBindingInfo, auto* pBindingInfoCollection)
    {
        (void)bindingInfoCount;
        assert(!bindingInfoCount || bindingInfoCount == 1);
        assert(!bindingInfoCount == !pBindingInfo);
        pBindingInfoCollection->pPipelineBindingInfo = pBindingInfo;
    };
    auto setShaderBindingInfos = [](uint32_t bindingInfoCount, const auto* pBindingInfos, auto* pBindingInfoCollection)
    {
        assert(!bindingInfoCount == !pBindingInfos);
        pBindingInfoCollection->shaderCount = bindingInfoCount;
        pBindingInfoCollection->pShaderBindingInfos = pBindingInfos;
    };
    auto setDescriptorBindingInfos = [](uint32_t bindingInfoCount, const auto* pBindingInfos, auto* pBindingInfoCollection)
    {
        assert(!bindingInfoCount == !pBindingInfos);
        pBindingInfoCollection->descriptorCount = bindingInfoCount;
        pBindingInfoCollection->pDescriptorBindingInfos = pBindingInfos;
    };
    auto setIndexBufferBindingInfo = [](uint32_t bindingInfoCount, const auto* pBindingInfo, auto* pBindingInfoCollection)
    {
        (void)bindingInfoCount;
        assert(!bindingInfoCount || bindingInfoCount == 1);
        assert(!bindingInfoCount == !pBindingInfo);
        pBindingInfoCollection->pIndexBufferBindingInfo = pBindingInfo;
    };
    auto setVertexBufferBindingInfos = [](uint32_t bindingInfoCount, const auto* pBindingInfos, auto* pBindingInfoCollection)
    {
        assert(!bindingInfoCount == !pBindingInfos);
        pBindingInfoCollection->vertexBufferCount = bindingInfoCount;
        pBindingInfoCollection->pVertexBufferBindingInfos = pBindingInfos;
    };
    auto setRenderTargetBindingInfos = [](uint32_t bindingInfoCount, const auto* pBindingInfos, auto* pBindingInfoCollection)
    {
        assert(!bindingInfoCount == !pBindingInfos);
        pBindingInfoCollection->renderTargetCount = bindingInfoCount;
        pBindingInfoCollection->pRenderTargetBindingInfos = pBindingInfos;
    };

    // Process compute resource and binding infos
    getBindingInfos(VK_PIPELINE_BIND_POINT_COMPUTE);
    copy_binding_infos(pipelineBindingInfos, pBindingInfo->pComputeBindingInfo, setPipelineBindingInfo);
    copy_binding_infos(shaderBindingInfos, pBindingInfo->pComputeBindingInfo,
        [](uint32_t bindingInfoCount, const auto* pBindingInfo, auto* pBindingInfoCollection)
        {
            (void)bindingInfoCount;
            assert(!bindingInfoCount || bindingInfoCount == 1);
            assert(!bindingInfoCount == !pBindingInfo);
            pBindingInfoCollection->pShaderBindingInfo = pBindingInfo;
        }
    );
    copy_binding_infos(descriptorBindingInfos, pBindingInfo->pComputeBindingInfo, setDescriptorBindingInfos);

    // Process graphics resource and binding infos
    getBindingInfos(VK_PIPELINE_BIND_POINT_GRAPHICS);
    copy_binding_infos(pipelineBindingInfos, pBindingInfo->pGraphicsBindingInfo, setPipelineBindingInfo);
    copy_binding_infos(shaderBindingInfos, pBindingInfo->pGraphicsBindingInfo, setShaderBindingInfos);
    copy_binding_infos(descriptorBindingInfos, pBindingInfo->pGraphicsBindingInfo, setDescriptorBindingInfos);
    copy_binding_infos(indexBufferBindingInfos, pBindingInfo->pGraphicsBindingInfo, setIndexBufferBindingInfo);
    copy_binding_infos(vertexBufferBindingInfos, pBindingInfo->pGraphicsBindingInfo, setVertexBufferBindingInfos);
    copy_binding_infos(renderTargetBindingInfos, pBindingInfo->pGraphicsBindingInfo, setRenderTargetBindingInfos);

    // Process ray tracing resource and binding infos
    getBindingInfos(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    copy_binding_infos(pipelineBindingInfos, pBindingInfo->pRayTracingBindingInfo, setPipelineBindingInfo);
    copy_binding_infos(shaderBindingInfos, pBindingInfo->pRayTracingBindingInfo, setShaderBindingInfos);
    copy_binding_infos(descriptorBindingInfos, pBindingInfo->pRayTracingBindingInfo, setDescriptorBindingInfos);

    return bindingInfo;
}

void MetadataExtractor::add_command(const GvkCommandBaseStructure& command)
{
    mCommandRecorder.add_command(command);
    switch (command.sType) {

    ////////////////////////////////////////////////////////////////////////////////
    // Command buffer state
    case gvk::get_stype<GvkCommandStructureBeginCommandBuffer>(): {
        process_command((const GvkCommandStructureBeginCommandBuffer&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdExecuteCommands>(): {
        process_command((const GvkCommandStructureCmdExecuteCommands&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureEndCommandBuffer>(): {
        process_command((const GvkCommandStructureEndCommandBuffer&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Render pass cmds
    case gvk::get_stype<GvkCommandStructureCmdBeginRenderPass>(): {
        process_command((const GvkCommandStructureCmdBeginRenderPass&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBeginRenderPass2>(): {
        process_command((const GvkCommandStructureCmdBeginRenderPass2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBeginRenderPass2KHR>(): {
        process_command((const GvkCommandStructureCmdBeginRenderPass2KHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdNextSubpass>(): {
        process_command((const GvkCommandStructureCmdNextSubpass&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdNextSubpass2>(): {
        process_command((const GvkCommandStructureCmdNextSubpass2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdNextSubpass2KHR>(): {
        process_command((const GvkCommandStructureCmdNextSubpass2KHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdEndRenderPass>(): {
        process_command((const GvkCommandStructureCmdEndRenderPass&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdEndRenderPass2>(): {
        process_command((const GvkCommandStructureCmdEndRenderPass2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdEndRenderPass2KHR>(): {
        process_command((const GvkCommandStructureCmdEndRenderPass2KHR&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Dynamic rendering cmds
    case gvk::get_stype<GvkCommandStructureCmdBeginRendering>(): {
        process_command((const GvkCommandStructureCmdBeginRendering&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBeginRenderingKHR>(): {
        process_command((const GvkCommandStructureCmdBeginRenderingKHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdEndRendering>(): {
        process_command((const GvkCommandStructureCmdEndRendering&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdEndRenderingKHR>(): {
        process_command((const GvkCommandStructureCmdEndRenderingKHR&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Pipeline cmds
    case gvk::get_stype<GvkCommandStructureCmdBindPipeline>(): {
        process_command((const GvkCommandStructureCmdBindPipeline&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPipelineBarrier>(): {
        process_command((const GvkCommandStructureCmdPipelineBarrier&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPipelineBarrier2>(): {
        process_command((const GvkCommandStructureCmdPipelineBarrier2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPipelineBarrier2KHR>(): {
        process_command((const GvkCommandStructureCmdPipelineBarrier2KHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushConstants>(): {
        process_command((const GvkCommandStructureCmdPushConstants&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushConstants2>(): {
        process_command((const GvkCommandStructureCmdPushConstants2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushConstants2KHR>(): {
        process_command((const GvkCommandStructureCmdPushConstants2KHR&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Update/fill/clear cmds
    case gvk::get_stype<GvkCommandStructureCmdUpdateBuffer>(): {
        process_command((const GvkCommandStructureCmdUpdateBuffer&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdFillBuffer>(): {
        process_command((const GvkCommandStructureCmdFillBuffer&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdClearColorImage>(): {
        process_command((const GvkCommandStructureCmdClearColorImage&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdClearDepthStencilImage>(): {
        process_command((const GvkCommandStructureCmdClearDepthStencilImage&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdClearAttachments>(): {
        process_command((const GvkCommandStructureCmdClearAttachments&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Copy/resolve cmds
    case gvk::get_stype<GvkCommandStructureCmdCopyBuffer>(): {
        process_command((const GvkCommandStructureCmdCopyBuffer&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdCopyBufferToImage>(): {
        process_command((const GvkCommandStructureCmdCopyBufferToImage&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdCopyImage>(): {
        process_command((const GvkCommandStructureCmdCopyImage&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdCopyImageToBuffer>(): {
        process_command((const GvkCommandStructureCmdCopyImageToBuffer&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBlitImage>(): {
        process_command((const GvkCommandStructureCmdBlitImage&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdResolveImage>(): {
        process_command((const GvkCommandStructureCmdResolveImage&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Index/vertex buffer cmds
    case gvk::get_stype<GvkCommandStructureCmdBindIndexBuffer>(): {
        process_command((const GvkCommandStructureCmdBindIndexBuffer&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBindVertexBuffers>(): {
        process_command((const GvkCommandStructureCmdBindVertexBuffers&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Bind descriptor cmds
    case gvk::get_stype<GvkCommandStructureCmdBindDescriptorSets>(): {
        process_command((const GvkCommandStructureCmdBindDescriptorSets&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBindDescriptorSets2>(): {
        process_command((const GvkCommandStructureCmdBindDescriptorSets2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBindDescriptorSets2KHR>(): {
        process_command((const GvkCommandStructureCmdBindDescriptorSets2KHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplers2EXT>(): {
        process_command((const GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplers2EXT&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplersEXT>(): {
        process_command((const GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplersEXT&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdBindDescriptorBuffersEXT>(): {
        process_command((const GvkCommandStructureCmdBindDescriptorBuffersEXT&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Push descriptor cmds
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSet>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSet&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSet2>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSet2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSet2KHR>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSet2KHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSetKHR>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSetKHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSetWithTemplate>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSetWithTemplate&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSetWithTemplate2>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSetWithTemplate2&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSetWithTemplate2KHR>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSetWithTemplate2KHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdPushDescriptorSetWithTemplateKHR>(): {
        process_command((const GvkCommandStructureCmdPushDescriptorSetWithTemplateKHR&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Dispatch cmds
    case gvk::get_stype<GvkCommandStructureCmdDispatch>(): {
        process_command((const GvkCommandStructureCmdDispatch&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDispatchBase>(): {
        process_command((const GvkCommandStructureCmdDispatchBase&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDispatchBaseKHR>(): {
        process_command((const GvkCommandStructureCmdDispatchBaseKHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDispatchIndirect>(): {
        process_command((const GvkCommandStructureCmdDispatchIndirect&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Draw cmds
    case gvk::get_stype<GvkCommandStructureCmdDraw>(): {
        process_command((const GvkCommandStructureCmdDraw&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndexed>(): {
        process_command((const GvkCommandStructureCmdDrawIndexed&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirect>(): {
        process_command((const GvkCommandStructureCmdDrawIndexedIndirect&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndirect>(): {
        process_command((const GvkCommandStructureCmdDrawIndirect&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirectCountAMD>(): {
        process_command((const GvkCommandStructureCmdDrawIndexedIndirectCountAMD&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndexedIndirectCountKHR>(): {
        process_command((const GvkCommandStructureCmdDrawIndexedIndirectCountKHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndirectCountAMD>(): {
        process_command((const GvkCommandStructureCmdDrawIndirectCountAMD&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndirectCountKHR>(): {
        process_command((const GvkCommandStructureCmdDrawIndirectCountKHR&)command);
    } break;
    case gvk::get_stype<GvkCommandStructureCmdDrawIndirectByteCountEXT>(): {
        process_command((const GvkCommandStructureCmdDrawIndirectByteCountEXT&)command);
    } break;

    ////////////////////////////////////////////////////////////////////////////////
    // Ray tracing cmds
    case gvk::get_stype<GvkCommandStructureCmdTraceRaysKHR>(): {
        process_command((const GvkCommandStructureCmdTraceRaysKHR&)command);
    } break;

    default: {
    } break;
    }
    mBindingRegistry.increment_command_count();
}

////////////////////////////////////////////////////////////////////////////////
// Command buffer state
void MetadataExtractor::process_command(const GvkCommandStructureBeginCommandBuffer& command)
{
    (void)command;
    mComputeBindings.reset();
    mGraphicsBindings.reset();
    mRayTracingBindings.reset();
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdExecuteCommands& command)
{
    // TODO : Handle resource bindings across secondary command buffers

    for (uint32_t commandBuffer_i = 0; commandBuffer_i < command.commandBufferCount; ++commandBuffer_i) {
        auto commandBuffer = command.pCommandBuffers[commandBuffer_i];

        // Setup GvkStateTrackedObject
        auto stateTrackedCommandBuffer = gvk::get_default<GvkStateTrackedObject>();
        stateTrackedCommandBuffer.type = VK_OBJECT_TYPE_COMMAND_BUFFER;
        stateTrackedCommandBuffer.handle = (uint64_t)commandBuffer;
        stateTrackedCommandBuffer.dispatchableHandle = (uint64_t)commandBuffer;

        // Setup GvkStateTrackedObjectEnumerateInfo
        auto stateTrackedObjectEnumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
        stateTrackedObjectEnumerateInfo.pfnCallback = [](const GvkStateTrackedObject* pStateTrackedObject, const VkBaseInStructure* pInfo, void* pUserData)
            {
                (void)pStateTrackedObject;
                assert(pInfo);
                const auto* pCmd = (GvkCommandBaseStructure*)pInfo;
                assert(pUserData);
                auto pMetadataExtractor = (MetadataExtractor*)pUserData;
                switch (pCmd->sType) {
                case gvk::get_stype<GvkCommandStructureBeginCommandBuffer>(): {
                    pMetadataExtractor->mCommandRecorder.add_command(*pCmd);
                    pMetadataExtractor->mBindingRegistry.increment_command_count();
                } break;
                case gvk::get_stype<GvkCommandStructureEndCommandBuffer>(): {
                    pMetadataExtractor->mCommandRecorder.add_command(*pCmd);
                    pMetadataExtractor->mBindingRegistry.increment_command_count();
                } break;
                default: {
                    pMetadataExtractor->add_command(*pCmd);
                } break;
                }
            };
        stateTrackedObjectEnumerateInfo.pUserData = this;
        gvkEnumerateStateTrackedCommandBufferCmds(&stateTrackedCommandBuffer, &stateTrackedObjectEnumerateInfo);
    }
}

void MetadataExtractor::process_command(const GvkCommandStructureEndCommandBuffer& command)
{
    (void)command;
    mComputeBindings.reset();
    mGraphicsBindings.reset();
    mRayTracingBindings.reset();
}

////////////////////////////////////////////////////////////////////////////////
// Render pass cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdBeginRenderPass& command)
{
    mGraphicsBindings.renderPassBindingMonitors.begin_render_pass(get_device(command.commandBuffer), *command.pRenderPassBegin,
        [](uint64_t dispatchableHandle, VkObjectType objectType, uint64_t handle, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo)
        {
            auto stateTrackedObject = gvk::get_default<GvkStateTrackedObject>();
            stateTrackedObject.type = objectType;
            stateTrackedObject.handle = handle;
            stateTrackedObject.dispatchableHandle = dispatchableHandle;
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, pCreateInfoType, pCreateInfo);
        }
    );
    mGraphicsBindings.renderPassBindingMonitors.bind_load_and_clear_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_framebuffer_attachments(mBindingRegistry);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBeginRenderPass2& command)
{
    mGraphicsBindings.renderPassBindingMonitors.begin_render_pass(get_device(command.commandBuffer), *command.pRenderPassBegin,
        [](uint64_t dispatchableHandle, VkObjectType objectType, uint64_t handle, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo)
        {
            auto stateTrackedObject = gvk::get_default<GvkStateTrackedObject>();
            stateTrackedObject.type = objectType;
            stateTrackedObject.handle = handle;
            stateTrackedObject.dispatchableHandle = dispatchableHandle;
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, pCreateInfoType, pCreateInfo);
        }
    );
    mGraphicsBindings.renderPassBindingMonitors.bind_load_and_clear_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_framebuffer_attachments(mBindingRegistry);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBeginRenderPass2KHR& command)
{
    mGraphicsBindings.renderPassBindingMonitors.begin_render_pass(get_device(command.commandBuffer), *command.pRenderPassBegin,
        [](uint64_t dispatchableHandle, VkObjectType objectType, uint64_t handle, VkStructureType* pCreateInfoType, VkBaseOutStructure* pCreateInfo)
        {
            auto stateTrackedObject = gvk::get_default<GvkStateTrackedObject>();
            stateTrackedObject.type = objectType;
            stateTrackedObject.handle = handle;
            stateTrackedObject.dispatchableHandle = dispatchableHandle;
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, pCreateInfoType, pCreateInfo);
        }
    );
    mGraphicsBindings.renderPassBindingMonitors.bind_load_and_clear_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_framebuffer_attachments(mBindingRegistry);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdNextSubpass& command)
{
    (void)command;
    mGraphicsBindings.renderPassBindingMonitors.next_subpass();
    mGraphicsBindings.renderPassBindingMonitors.bind_load_and_clear_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_resolve_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_framebuffer_attachments(mBindingRegistry);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdNextSubpass2& command)
{
    (void)command;
    mGraphicsBindings.renderPassBindingMonitors.next_subpass();
    mGraphicsBindings.renderPassBindingMonitors.bind_load_and_clear_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_resolve_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_framebuffer_attachments(mBindingRegistry);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdNextSubpass2KHR& command)
{
    (void)command;
    mGraphicsBindings.renderPassBindingMonitors.next_subpass();
    mGraphicsBindings.renderPassBindingMonitors.bind_load_and_clear_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_resolve_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.bind_framebuffer_attachments(mBindingRegistry);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdEndRenderPass& command)
{
    (void)command;
    mGraphicsBindings.renderPassBindingMonitors.bind_resolve_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.end_render_pass();
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdEndRenderPass2& command)
{
    (void)command;
    mGraphicsBindings.renderPassBindingMonitors.bind_resolve_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.end_render_pass();
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdEndRenderPass2KHR& command)
{
    (void)command;
    mGraphicsBindings.renderPassBindingMonitors.bind_resolve_attachments(mBindingRegistry);
    mGraphicsBindings.renderPassBindingMonitors.end_render_pass();
}

////////////////////////////////////////////////////////////////////////////////
// Dynamic rendering cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdBeginRendering& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBeginRenderingKHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdEndRendering& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdEndRenderingKHR& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Pipeline cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdBindPipeline& command)
{
    // Get VkDevice from VkCommandBuffer
    auto device = get_device(command.commandBuffer);

    // Register pipeline resource info
    auto pPipelineResourceInfo = register_pipeline_resource_info(device, command.pipeline);
    assert(pPipelineResourceInfo);
    assert(pPipelineResourceInfo->pCreateInfo);

    // Get shader stages and binding monitors for pipeline bind point
    uint32_t stageCount = 0;
    const VkPipelineShaderStageCreateInfo* pStages = nullptr;
    gvk::detail::BindingMonitorCollection* pBindingMonitorCollection = nullptr;
    switch (command.pipelineBindPoint) {
    case VK_PIPELINE_BIND_POINT_COMPUTE: {
        pBindingMonitorCollection = &mComputeBindings;
        assert(pPipelineResourceInfo->pCreateInfo->sType == gvk::get_stype<VkComputePipelineCreateInfo>());
        stageCount = 1;
        pStages = &((const VkComputePipelineCreateInfo*)pPipelineResourceInfo->pCreateInfo)->stage;
    } break;
    case VK_PIPELINE_BIND_POINT_GRAPHICS: {
        pBindingMonitorCollection = &mGraphicsBindings;
        assert(pPipelineResourceInfo->pCreateInfo->sType == gvk::get_stype<VkGraphicsPipelineCreateInfo>());
        stageCount = ((const VkGraphicsPipelineCreateInfo*)pPipelineResourceInfo->pCreateInfo)->stageCount;
        pStages = ((const VkGraphicsPipelineCreateInfo*)pPipelineResourceInfo->pCreateInfo)->pStages;
    } break;
    case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
        pBindingMonitorCollection = &mRayTracingBindings;
        assert(pPipelineResourceInfo->pCreateInfo->sType == gvk::get_stype<VkRayTracingPipelineCreateInfoKHR>());
        stageCount = ((const VkRayTracingPipelineCreateInfoKHR*)pPipelineResourceInfo->pCreateInfo)->stageCount;
        pStages = ((const VkRayTracingPipelineCreateInfoKHR*)pPipelineResourceInfo->pCreateInfo)->pStages;
    } break;
    default: {
        assert(false && "Unservied VkPipelineBindPoint encountered; gvk maintenance required");
    } break;
    }

    if (pBindingMonitorCollection) {

        // Register and bind pipeline binding info
        auto pipelineBindingInfo = gvk::get_default<GvkPipelineBindingInfo>();
        pipelineBindingInfo.pResourceInfo = pPipelineResourceInfo;
        pipelineBindingInfo.bindPoint = command.pipelineBindPoint;
        auto pPipelineBindingInfo = mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&pipelineBindingInfo);
        mBindingRegistry.bind(command.pipelineBindPoint, pipelineBindingInfo.pResourceInfo, pPipelineBindingInfo, &pBindingMonitorCollection->pipelineBindingMonitor);

        // Clear current shader binding monitors to commit any existing shader binding
        //  infos to the timeline, then register and bind updated shader binding infos
        pBindingMonitorCollection->shaderBindingMonitors.clear();
        if (stageCount && pStages) {
            pBindingMonitorCollection->shaderBindingMonitors.resize(stageCount);
            for (uint32_t stage_i = 0; stage_i < stageCount; ++stage_i) {
                auto shaderBindingInfo = gvk::get_default<GvkShaderBindingInfo>();
                shaderBindingInfo.pResourceInfo = register_resource_info<VkShaderModuleCreateInfo>(device, pStages[stage_i].module);
                shaderBindingInfo.stage = pStages[stage_i].stage;
                auto pShaderBindingInfo = mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&shaderBindingInfo);
                mBindingRegistry.bind(command.pipelineBindPoint, shaderBindingInfo.pResourceInfo, pShaderBindingInfo, &pBindingMonitorCollection->shaderBindingMonitors[stage_i]);
            }
        }
    }
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPipelineBarrier& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPipelineBarrier2& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPipelineBarrier2KHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushConstants& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushConstants2& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushConstants2KHR& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Update/fill/clear cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdUpdateBuffer& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdFillBuffer& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdClearColorImage& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdClearDepthStencilImage& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdClearAttachments& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Copy/resolve cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdCopyBuffer& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdCopyBufferToImage& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdCopyImage& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdCopyImageToBuffer& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBlitImage& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdResolveImage& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Index/vertex buffer cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdBindIndexBuffer& command)
{
    auto indexBufferBindingInfo = gvk::get_default<GvkIndexBufferBindingInfo>();
    indexBufferBindingInfo.pResourceInfo = register_resource_info<VkBufferCreateInfo>(get_device(command.commandBuffer), command.buffer);
    indexBufferBindingInfo.offset = command.offset;
    indexBufferBindingInfo.indexType = command.indexType;
    auto pIndexBufferBindingInfo = mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&indexBufferBindingInfo);
    mBindingRegistry.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, indexBufferBindingInfo.pResourceInfo, pIndexBufferBindingInfo, &mGraphicsBindings.indexBufferBindingMonitor);
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBindVertexBuffers& command)
{
    // TODO : Double check count logic
    validate_size_for_index(command.firstBinding + command.bindingCount, mGraphicsBindings.vertexBufferBindingMonitors);
    for (uint32_t buffer_i = 0; buffer_i < command.bindingCount; ++buffer_i) {
        auto& bindingMonitor = mGraphicsBindings.vertexBufferBindingMonitors[command.firstBinding + buffer_i];
        auto buffer = command.pBuffers ? command.pBuffers[buffer_i] : VK_NULL_HANDLE;
        if (buffer) {
            auto vertexBufferBindingInfo = gvk::get_default<GvkVertexBufferBindingInfo>();
            vertexBufferBindingInfo.pResourceInfo = register_resource_info<VkBufferCreateInfo>(get_device(command.commandBuffer), buffer);
            vertexBufferBindingInfo.offset = command.pOffsets ? command.pOffsets[buffer_i] : 0;
            // TODO : Vertex input state info
            auto pVertexBufferBindingInfo = mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&vertexBufferBindingInfo);
            mBindingRegistry.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, vertexBufferBindingInfo.pResourceInfo, pVertexBufferBindingInfo, &bindingMonitor);
        } else {
            bindingMonitor.reset();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Bind descriptor cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdBindDescriptorSets& command)
{
    uint32_t dynamicOffset_i = 0;
    auto setIndex = command.firstSet;
    for (uint32_t descriptorSet_i = 0; descriptorSet_i < command.descriptorSetCount; ++descriptorSet_i) {
        bind_descriptor_set(get_device(command.commandBuffer), command.pipelineBindPoint, command.pDescriptorSets[descriptorSet_i], setIndex, command.dynamicOffsetCount, command.pDynamicOffsets, &dynamicOffset_i);
        ++setIndex;
    }
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBindDescriptorSets2& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBindDescriptorSets2KHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplers2EXT& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBindDescriptorBufferEmbeddedSamplersEXT& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdBindDescriptorBuffersEXT& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Push descriptor cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSet& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSet2& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSet2KHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSetKHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplate& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplate2& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplate2KHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdPushDescriptorSetWithTemplateKHR& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Dispatch cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdDispatch& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDispatchBase& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDispatchBaseKHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDispatchIndirect& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Draw cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdDraw& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndexed& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndexedIndirect& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndirect& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndexedIndirectCountAMD& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndexedIndirectCountKHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndirectCountAMD& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndirectCountKHR& command)
{
    (void)command;
    // TODO :
}

void MetadataExtractor::process_command(const GvkCommandStructureCmdDrawIndirectByteCountEXT& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// Ray tracing cmds
void MetadataExtractor::process_command(const GvkCommandStructureCmdTraceRaysKHR& command)
{
    (void)command;
    // TODO :
}

////////////////////////////////////////////////////////////////////////////////
// GvkResourceInfo registration
template <typename CreateInfoType, typename ObjectType>
const GvkResourceInfo* MetadataExtractor::register_resource_info(VkDevice device, ObjectType object)
{
    if (device && object) {

        // Setup GvkResourceInfo
        GvkStateTrackedObject stateTrackedObject{ };
        stateTrackedObject.type = gvk::detail::get_object_type<ObjectType>();
        stateTrackedObject.handle = (uint64_t)object;
        stateTrackedObject.dispatchableHandle = (uint64_t)device;

        // Register GvkResourceInfo
        GvkResourceInfo resourceInfo{ };
        resourceInfo.type = stateTrackedObject.type;
        resourceInfo.handle = stateTrackedObject.handle;
        resourceInfo.dispatchableHandle = stateTrackedObject.dispatchableHandle;

        // Get create info
        CreateInfoType createInfo{ };
        VkStructureType createInfoType{ };
        gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, nullptr);
        // TODO : It may be correct to omit registration for any resource lacking a create
        //  info (mostly applies to descriptors).  Need to investigate more workloads.
        if (createInfoType == gvk::get_stype<CreateInfoType>()) {
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&createInfo);
            resourceInfo.pCreateInfo = (VkBaseOutStructure*)&createInfo;
        }

        // Register resource info
        return mBindingRegistry.register_resource_info(&resourceInfo);
    }
    return nullptr;
}

const GvkResourceInfo* MetadataExtractor::register_render_pass_resource_info(VkDevice device, VkRenderPass renderPass)
{
    // Setup GvkResourceInfo
    GvkResourceInfo resourceInfo{ };
    resourceInfo.type = VK_OBJECT_TYPE_RENDER_PASS;
    resourceInfo.handle = (uint64_t)renderPass;
    resourceInfo.dispatchableHandle = (uint64_t)device;
    const GvkResourceInfo* pResourceInfo = nullptr;

    // Setup GvkStateTrackedObject
    // TODO : Unify GvkStateTrackedObject, GvkStateTrackedObject, and other type erased
    //  handle structures
    GvkStateTrackedObject stateTrackedObject{ };
    stateTrackedObject.type = resourceInfo.type;
    stateTrackedObject.handle = resourceInfo.handle;
    stateTrackedObject.dispatchableHandle = resourceInfo.dispatchableHandle;

    // Get create info type, then switch on type
    // NOTE : Using mBindingRegistry.register_resource_info() directly instead of the
    //  register_resource_info<>() utility function because different logic is needed
    //  based on create info type.
    VkStructureType createInfoType{ };
    gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, nullptr);
    switch (createInfoType) {
    case gvk::get_stype<VkRenderPassCreateInfo>(): {
        VkRenderPassCreateInfo createInfo{ };
        gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&createInfo);
        assert(createInfo.sType == gvk::get_stype<VkRenderPassCreateInfo>());
        resourceInfo.pCreateInfo = (VkBaseOutStructure*)&createInfo;
        pResourceInfo = mBindingRegistry.register_resource_info(&resourceInfo);
    } break;
    case gvk::get_stype<VkRenderPassCreateInfo2>(): {
        VkRenderPassCreateInfo2 createInfo{ };
        gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&createInfo);
        assert(createInfo.sType == gvk::get_stype<VkRenderPassCreateInfo2>());
        resourceInfo.pCreateInfo = (VkBaseOutStructure*)&createInfo;
        pResourceInfo = mBindingRegistry.register_resource_info(&resourceInfo);
    } break;
    default: {
        assert(false && "gvk::MetadataExtractor unserviced VkRenderPass create info type encountered; gvk maintenance required");
    } break;
    }
    return pResourceInfo;
}

const GvkResourceInfo* MetadataExtractor::register_descriptor_set_layout_resource_info(VkDevice device, VkDescriptorSetLayout descriptorSetLayout)
{
    auto pResourceInfo = register_resource_info<VkDescriptorSetLayoutCreateInfo>(device, descriptorSetLayout);
    assert(pResourceInfo);
    assert(pResourceInfo->pCreateInfo);
    assert(pResourceInfo->pCreateInfo->sType == gvk::get_stype<VkDescriptorSetLayoutCreateInfo>());
    const auto& createInfo = *(const VkDescriptorSetLayoutCreateInfo*)pResourceInfo->pCreateInfo;
    for (uint32_t binding_i = 0; binding_i < createInfo.bindingCount; ++binding_i) {
        const auto& binding = createInfo.pBindings[binding_i];
        if (binding.pImmutableSamplers) {
            for (uint32_t sampler_i = 0; sampler_i < binding.descriptorCount; ++sampler_i) {
                register_resource_info<VkSampler>(device, binding.pImmutableSamplers[sampler_i]);
            }
        }
    }
    return pResourceInfo;
}

const GvkResourceInfo* MetadataExtractor::register_pipeline_layout_resource_info(VkDevice device, VkPipelineLayout pipelineLayout)
{
    auto pResourceInfo = register_resource_info<VkPipelineLayoutCreateInfo>(device, pipelineLayout);
    assert(pResourceInfo);
    assert(pResourceInfo->pCreateInfo);
    assert(pResourceInfo->pCreateInfo->sType == gvk::get_stype<VkPipelineLayoutCreateInfo>());
    const auto& createInfo = *(const VkPipelineLayoutCreateInfo*)pResourceInfo->pCreateInfo;
    for (uint32_t setLayout_i = 0; setLayout_i < createInfo.setLayoutCount; ++setLayout_i) {
        register_descriptor_set_layout_resource_info(device, createInfo.pSetLayouts[setLayout_i]);
    }
    return pResourceInfo;
}

const GvkResourceInfo* MetadataExtractor::register_pipeline_resource_info(VkDevice device, VkPipeline pipeline)
{
    const GvkResourceInfo* pResourceInfo = nullptr;
    if (pipeline) {

        // Setup GvkResourceInfo
        GvkResourceInfo resourceInfo{ };
        resourceInfo.type = VK_OBJECT_TYPE_PIPELINE;
        resourceInfo.handle = (uint64_t)pipeline;
        resourceInfo.dispatchableHandle = (uint64_t)device;

        // Setup GvkStateTrackedObject
        // TODO : Unify GvkStateTrackedObject, GvkStateTrackedObject, and other type erased
        //  handle structures
        GvkStateTrackedObject stateTrackedObject{ };
        stateTrackedObject.type = resourceInfo.type;
        stateTrackedObject.handle = resourceInfo.handle;
        stateTrackedObject.dispatchableHandle = resourceInfo.dispatchableHandle;

        // Get create info type, then switch on type
        // NOTE : Using mBindingRegistry.register_resource_info() directly instead of the
        //  register_resource_info<>() utility function because different logic is needed
        //  based on create info type.
        VkStructureType createInfoType{ };
        gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, nullptr);
        switch (createInfoType) {
        case gvk::get_stype<VkComputePipelineCreateInfo>(): {
            VkComputePipelineCreateInfo createInfo{ };
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&createInfo);
            assert(createInfo.sType == gvk::get_stype<VkComputePipelineCreateInfo>());
            resourceInfo.pCreateInfo = (VkBaseOutStructure*)&createInfo;
            pResourceInfo = mBindingRegistry.register_resource_info(&resourceInfo);
            register_pipeline_resource_info(device, createInfo.basePipelineHandle);
            register_pipeline_layout_resource_info(device, createInfo.layout);
            register_resource_info<VkShaderModuleCreateInfo>(device, createInfo.stage.module);
        } break;
        case gvk::get_stype<VkGraphicsPipelineCreateInfo>(): {
            VkGraphicsPipelineCreateInfo createInfo{ };
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&createInfo);
            assert(createInfo.sType == gvk::get_stype<VkGraphicsPipelineCreateInfo>());
            resourceInfo.pCreateInfo = (VkBaseOutStructure*)&createInfo;
            pResourceInfo = mBindingRegistry.register_resource_info(&resourceInfo);
            register_pipeline_resource_info(device, createInfo.basePipelineHandle);
            register_pipeline_layout_resource_info(device, createInfo.layout);
            register_render_pass_resource_info(device, createInfo.renderPass);
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                register_resource_info<VkShaderModuleCreateInfo>(device, createInfo.pStages[stage_i].module);
            }
        } break;
        case gvk::get_stype<VkRayTracingPipelineCreateInfoKHR>(): {
            VkRayTracingPipelineCreateInfoKHR createInfo{ };
            gvkGetStateTrackedObjectCreateInfo(&stateTrackedObject, &createInfoType, (VkBaseOutStructure*)&createInfo);
            assert(createInfo.sType == gvk::get_stype<VkRayTracingPipelineCreateInfoKHR>());
            resourceInfo.pCreateInfo = (VkBaseOutStructure*)&createInfo;
            pResourceInfo = mBindingRegistry.register_resource_info(&resourceInfo);
            register_pipeline_resource_info(device, createInfo.basePipelineHandle);
            register_pipeline_layout_resource_info(device, createInfo.layout);
            for (uint32_t stage_i = 0; stage_i < createInfo.stageCount; ++stage_i) {
                register_resource_info<VkShaderModuleCreateInfo>(device, createInfo.pStages[stage_i].module);
            }
        } break;
        default: {
            assert(false && "gvk::MetadataExtractor unserviced VkPipeline create info type encountered; gvk maintenance required");
        } break;
        }
    }
    return pResourceInfo;
}

////////////////////////////////////////////////////////////////////////////////
// GvkDescriptorBindingInfo registration
const GvkDescriptorBindingInfo* MetadataExtractor::register_buffer_descriptor_binding_info(VkDevice device, uint32_t setIndex, uint32_t bindIndex, uint32_t arrayIndex, const VkDescriptorBufferInfo& descriptor)
{
    auto descriptorBufferBindingInfo = gvk::get_default<GvkDescriptorBufferInfo>();
    descriptorBufferBindingInfo.offset = descriptor.offset; // TODO : Dynamic offset
    descriptorBufferBindingInfo.size = descriptor.range;

    auto descriptorBindingInfo = gvk::get_default<GvkDescriptorBindingInfo>();
    descriptorBindingInfo.pResourceInfo = register_resource_info<VkBufferCreateInfo>(device, descriptor.buffer);
    descriptorBindingInfo.setIndex = setIndex;
    descriptorBindingInfo.bindIndex = bindIndex;
    descriptorBindingInfo.arrayIndex = arrayIndex;
    descriptorBindingInfo.stage = VK_SHADER_STAGE_ALL; // TODO : Get stage from layout
    descriptorBindingInfo.pBufferInfo = &descriptorBufferBindingInfo;
    return (const GvkDescriptorBindingInfo*)mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&descriptorBindingInfo);
}

const GvkDescriptorBindingInfo* MetadataExtractor::register_buffer_descriptor_binding_info(VkDevice device, uint32_t setIndex, uint32_t bindIndex, uint32_t arrayIndex, VkBufferView texelBufferView)
{
    // Register buffer view
    auto pBufferViewResourceInfo = register_resource_info<VkBufferViewCreateInfo>(device, texelBufferView);
    if (pBufferViewResourceInfo) {
        auto bufferViewCreateInfo = gvk::get_default<VkBufferViewCreateInfo>();
        if (pBufferViewResourceInfo->pCreateInfo && pBufferViewResourceInfo->pCreateInfo->sType == gvk::get_stype<VkBufferViewCreateInfo>()) {
            bufferViewCreateInfo = *(const VkBufferViewCreateInfo*)pBufferViewResourceInfo->pCreateInfo;
        }

        // Register buffer
        auto pBufferResourceInfo = register_resource_info<VkBufferCreateInfo>(device, bufferViewCreateInfo.buffer);
        if (pBufferResourceInfo) {
            auto bufferCreateInfo = gvk::get_default<VkBufferCreateInfo>();
            if (pBufferResourceInfo->pCreateInfo && pBufferResourceInfo->pCreateInfo->sType == gvk::get_stype<VkBufferCreateInfo>()) {
                bufferCreateInfo = *(const VkBufferCreateInfo*)pBufferResourceInfo->pCreateInfo;
            }

            // Setup descriptor buffer binding info
            auto descriptorBufferBindingInfo = gvk::get_default<GvkDescriptorBufferInfo>();
            descriptorBufferBindingInfo.pBufferViewResourceInfo = pBufferViewResourceInfo;
            descriptorBufferBindingInfo.offset = bufferViewCreateInfo.offset; // TODO : Dynamic offset
            descriptorBufferBindingInfo.size = bufferViewCreateInfo.range;

            // NOTE : VK_WHOLE_SIZE is uint64_t max (which is necessarily larger than buffer
            //  size), so clamp to actual buffer size
            if (bufferCreateInfo.size < descriptorBufferBindingInfo.size) {
                descriptorBufferBindingInfo.size = bufferCreateInfo.size - bufferViewCreateInfo.offset;
                auto formatInfo = gvk::get_default<GvkFormatInfo>();
                gvk::get_format_info(bufferViewCreateInfo.format, &formatInfo);
                while (descriptorBufferBindingInfo.size % formatInfo.blockSize) {
                    --descriptorBufferBindingInfo.size;
                }
            }

            // Setup descriptor binding info
            auto descriptorBindingInfo = gvk::get_default<GvkDescriptorBindingInfo>();
            descriptorBindingInfo.pResourceInfo = pBufferResourceInfo;
            descriptorBindingInfo.setIndex = setIndex;
            descriptorBindingInfo.bindIndex = bindIndex;
            descriptorBindingInfo.arrayIndex = arrayIndex;
            descriptorBindingInfo.stage = VK_SHADER_STAGE_ALL; // TODO : Get stage from layout
            descriptorBindingInfo.pBufferInfo = &descriptorBufferBindingInfo;
            return (const GvkDescriptorBindingInfo*)mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&descriptorBindingInfo);
        }
    }
    return nullptr;
}

const GvkDescriptorBindingInfo* MetadataExtractor::register_image_descriptor_binding_info(VkDevice device, uint32_t setIndex, uint32_t bindIndex, uint32_t arrayIndex, const VkDescriptorImageInfo& descriptor)
{
    // Register image view
    auto pImageViewResourceInfo = register_resource_info<VkImageViewCreateInfo>(device, descriptor.imageView);
    if (pImageViewResourceInfo) {
        auto imageViewCreateInfo = gvk::get_default<VkImageViewCreateInfo>();
        if (pImageViewResourceInfo->pCreateInfo && pImageViewResourceInfo->pCreateInfo->sType == gvk::get_stype<VkImageViewCreateInfo>()) {
            imageViewCreateInfo = *(const VkImageViewCreateInfo*)pImageViewResourceInfo->pCreateInfo;
        }

        // Setup descriptor image binding info
        auto descriptorImageBindingInfo = gvk::get_default<GvkDescriptorImageInfo>();
        descriptorImageBindingInfo.pImageViewResourceInfo = pImageViewResourceInfo;
        // Register sampler
        descriptorImageBindingInfo.pSamplerResourceInfo = register_resource_info<VkSamplerCreateInfo>(device, descriptor.sampler);

        // Setup descriptor binding info
        auto descriptorBindingInfo = gvk::get_default<GvkDescriptorBindingInfo>();
        // Register image
        descriptorBindingInfo.pResourceInfo = register_resource_info<VkImageCreateInfo>(device, imageViewCreateInfo.image);
        if (!descriptorBindingInfo.pResourceInfo) {
            descriptorBindingInfo.pResourceInfo = descriptorImageBindingInfo.pSamplerResourceInfo;
        }
        if (!descriptorBindingInfo.pResourceInfo) {
            descriptorBindingInfo.pResourceInfo = descriptorImageBindingInfo.pImageViewResourceInfo;
        }
        descriptorBindingInfo.setIndex = setIndex;
        descriptorBindingInfo.bindIndex = bindIndex;
        descriptorBindingInfo.arrayIndex = arrayIndex;
        descriptorBindingInfo.stage = VK_SHADER_STAGE_ALL; // TODO : Get stage from layout
        descriptorBindingInfo.pImageInfo = &descriptorImageBindingInfo;
        return (const GvkDescriptorBindingInfo*)mBindingRegistry.register_binding_info((GvkBindingInfoBaseStructure*)&descriptorBindingInfo);
    }
    return nullptr;
}

void MetadataExtractor::bind_descriptor_set(VkDevice device, VkPipelineBindPoint bindPoint, VkDescriptorSet descriptorSet, uint32_t setIndex, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets, uint32_t* pDynamicOffsetIndex)
{
    // Select binding monitors based on pipeline bind point
    gvk::detail::DescriptorBindingMonitorCollection* pBindingMonitors = nullptr;
    switch (bindPoint) {
    case VK_PIPELINE_BIND_POINT_COMPUTE: {
        pBindingMonitors = &mComputeBindings.descriptorBindingMonitors;
    } break;
    case VK_PIPELINE_BIND_POINT_GRAPHICS: {
        pBindingMonitors = &mGraphicsBindings.descriptorBindingMonitors;
    } break;
    case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
        pBindingMonitors = &mRayTracingBindings.descriptorBindingMonitors;
    } break;
    default: {
        assert(false && "gvk::MetadataExtractor unserviced VkPipelineBindPoint; gvk maintenance required");
    } break;
    }
    assert(pBindingMonitors);
    pBindingMonitors->reset(setIndex);

    // Enumerate descriptor bindings
    GvkStateTrackedObject stateTrackedObject{ };
    stateTrackedObject.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    stateTrackedObject.handle = (uint64_t)descriptorSet;
    stateTrackedObject.dispatchableHandle = (uint64_t)device;
    std::vector<gvk::Auto<VkWriteDescriptorSet>> writeDescriptorSets;
    auto enumerateInfo = gvk::get_default<GvkStateTrackedObjectEnumerateInfo>();
    enumerateInfo.pUserData = &writeDescriptorSets;
    enumerateInfo.pfnCallback = [](const GvkStateTrackedObject*, const VkBaseInStructure* pInfo, void* pUserData)
    {
        assert(pInfo);
        assert(pInfo->sType == gvk::get_stype<VkWriteDescriptorSet>());
        assert(pUserData);
        auto pWriteDescriptorSets = (std::vector<gvk::Auto<VkWriteDescriptorSet>>*)pUserData;
        pWriteDescriptorSets->push_back(*(VkWriteDescriptorSet*)pInfo);
    };
    gvkEnumerateStateTrackedObjectBindings(&stateTrackedObject, &enumerateInfo);

    // Process descriptors based on type
    for (uint32_t write_i = 0; write_i < writeDescriptorSets.size(); ++write_i) {
        const auto& writeDescriptorSet = writeDescriptorSets[write_i];
        switch (writeDescriptorSet->descriptorType) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            assert(writeDescriptorSet->pBufferInfo);
            for (uint32_t descriptor_i = 0; descriptor_i < writeDescriptorSet->descriptorCount; ++descriptor_i) {
                const auto& descriptorBufferInfo = writeDescriptorSet->pBufferInfo[descriptor_i];
                if (descriptorBufferInfo.buffer) {
                    auto pDescriptorBindingInfo = register_buffer_descriptor_binding_info(device, setIndex, writeDescriptorSet->dstBinding, descriptor_i, descriptorBufferInfo);
                    if (pDescriptorBindingInfo && pDescriptorBindingInfo->pResourceInfo) {
                        assert(pDescriptorBindingInfo->pResourceInfo->pCreateInfo);
                        assert(pDescriptorBindingInfo->pResourceInfo->pCreateInfo->sType == gvk::get_stype<VkBufferCreateInfo>());

                        // Process dynamic offsets
                        auto dynamicDescriptor =
                            writeDescriptorSet->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                            writeDescriptorSet->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        if (dynamicDescriptor && pDynamicOffsets && pDynamicOffsetIndex && *pDynamicOffsetIndex < dynamicOffsetCount) {
                            // TODO : Handling these offsets will be crucial when it's time to read/write data
                            //  eg. restore point, data/resource visualization, experiments, etc.  For the short
                            //  near-term, nothing is relying on these offsets, so implmentation will be in
                            //  later pull-request.
                        }

                        // Handle VK_WHOLE_SIZE
                        auto pBufferResourceInfo = pDescriptorBindingInfo->pResourceInfo;
                        if (descriptorBufferInfo.range == VK_WHOLE_SIZE) {
                            // TODO : See comment above
                        }

                        // Bind the descriptor to the timeline
                        auto pBindingMonitor = &(*pBindingMonitors)[{ setIndex, writeDescriptorSet->dstBinding, write_i }];
                        mBindingRegistry.bind(bindPoint, pBufferResourceInfo, (const GvkBindingInfoBaseStructure*)pDescriptorBindingInfo, pBindingMonitor);
                    }
                }
            }
        } break;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            assert(writeDescriptorSet->pImageInfo);
            for (uint32_t descriptor_i = 0; descriptor_i < writeDescriptorSet->descriptorCount; ++descriptor_i) {
                const auto& descriptorImageInfo = writeDescriptorSet->pImageInfo[descriptor_i];
                auto pDescriptorBindingInfo = register_image_descriptor_binding_info(device, setIndex, writeDescriptorSet->dstBinding, descriptor_i, descriptorImageInfo);
                if (pDescriptorBindingInfo && pDescriptorBindingInfo->pResourceInfo) {
                    auto pBindingMonitor = &(*pBindingMonitors)[{ setIndex, writeDescriptorSet->dstBinding, write_i }];
                    mBindingRegistry.bind(bindPoint, pDescriptorBindingInfo->pResourceInfo, (const GvkBindingInfoBaseStructure*)pDescriptorBindingInfo, pBindingMonitor);
                }
            }
        } break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            assert(writeDescriptorSet->pTexelBufferView);
            for (uint32_t descriptor_i = 0; descriptor_i < writeDescriptorSet->descriptorCount; ++descriptor_i) {
                auto pDescriptorBindingInfo = register_buffer_descriptor_binding_info(device, setIndex, writeDescriptorSet->dstBinding, descriptor_i, writeDescriptorSet->pTexelBufferView[descriptor_i]);
                if (pDescriptorBindingInfo && pDescriptorBindingInfo->pResourceInfo) {
                    auto pBindingMonitor = &(*pBindingMonitors)[{ setIndex, writeDescriptorSet->dstBinding, write_i }];
                    mBindingRegistry.bind(bindPoint, pDescriptorBindingInfo->pResourceInfo, (const GvkBindingInfoBaseStructure*)pDescriptorBindingInfo, pBindingMonitor);
                }
            }
        } break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
            // TODO :
            assert(false && "gvk::MetadataExtractor unserviced VkDescriptorType; gvk maintenance required");
        } break;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
            // TODO :
            assert(false && "gvk::MetadataExtractor unserviced VkDescriptorType; gvk maintenance required");
        } break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        default: {
            assert(false && "gvk::MetadataExtractor unserviced VkDescriptorType; gvk maintenance required");
        } break;
        }
    }
}

} // namespace gvk

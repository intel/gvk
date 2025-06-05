
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

#include "gvk-pipeline-explorer/generated/basic-pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/handle-info.hpp"
#include "gvk-pipeline-explorer.hpp"

#include "gvk-command-structures.hpp"
#include "gvk-defines.hpp"
#include "gvk-handles.hpp"
#include "gvk-layer.hpp"
#include "gvk-spirv.hpp"
#include "gvk-structures.hpp"

#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef struct GvkPipelineExplorerToolCommandBufferInfo {
    VkDevice device;
    VkQueue queue;
    uint32_t cmdIndex;
    uint32_t cmdCount;
    const GvkCommandCmdBaseStructure* const* ppCmds;
    uint32_t collectionRangeIndex;
    uint32_t collectionRangeCount;
    const GvkPipelineExplorerCollectionRange* pCollectionRanges;
    uint32_t metricsRequestIdCount;
    const GvkPipelineExplorerMetricId* pMetricsRequestIds;
} GvkPipelineExplorerToolCommandBufferInfo;

typedef struct GvkPipelineExplorerToolQueueInfo {
    VkDevice device;
    VkQueue queue;
    const GvkCommandBaseStructure* pCommand;
    uint32_t collectionRangeCount;
    const GvkPipelineExplorerCollectionRange* pCollectionRanges;
    uint32_t metricsRequestIdCount;
    const GvkPipelineExplorerMetricId* pMetricsRequestIds;
} GvkPipelineExplorerToolQueueInfo;

typedef void(VKAPI_PTR* PFN_gvkPipelineExplorerToolCommandBufferCallback)(const GvkPipelineExplorerToolCommandBufferInfo* pToolCommandBufferInfo, void* pUserData);
typedef void(VKAPI_PTR* PFN_gvkPipelineExplorerToolQueueCallback)(const GvkPipelineExplorerToolQueueInfo* pToolQueueInfo, void* pUserData);

struct GvkPipelineExplorerToolCommandBufferCallbackInfo
{
    PFN_gvkPipelineExplorerToolCommandBufferCallback pfnPreProcessCommandBuffer;
    PFN_gvkPipelineExplorerToolCommandBufferCallback pfnPreProcessCmd;
    PFN_gvkPipelineExplorerToolCommandBufferCallback pfnPostProcessCmd;
    PFN_gvkPipelineExplorerToolCommandBufferCallback pfnPostProcessCommandBuffer;
    PFN_gvkPipelineExplorerToolQueueCallback pfnPreProcessQueueSubmission;
    PFN_gvkPipelineExplorerToolQueueCallback pfnPostProcessQueueSubmission;
    void* pUserData;
};

namespace gvk {

class PipelineExplorer final
    : public pipeline_explorer::BasicPipelineExplorer
{
public:
    ////////////////////////////////////////////////////////////////////////////////
    // VkInstance
    VkResult execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) override final;
    VkResult post_execute_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) override final;
    void execute_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // VkDevice
    VkResult execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) override final;
    VkResult post_execute_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) override final;
    void execute_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // VkQueue
    VkResult execute_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override final;
    VkResult execute_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override final;
    VkResult execute_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override final;
    VkResult execute_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) override final;
    VkResult reset_timestamp_query_pool(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, uint32_t queryCount);
    void write_timestamp(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage);
    VkResult get_timestamp_results(const pipeline_explorer::QueueInfo& queueInfo, std::vector<uint64_t>& results);
    VkResult reset_pipeline_statistics_query_pool(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, VkPipeline pipeline, uint32_t queryCount);
    void begin_pipeline_statistics_query(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer);
    void end_pipeline_statistics_query(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer);
    VkResult get_pipeline_statistics_results(const pipeline_explorer::QueueInfo& queueInfo, std::vector<uint64_t>& results);

    ////////////////////////////////////////////////////////////////////////////////
    // VkCommandPool and VkCommandBuffer
    VkResult execute_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) override final;
    void execute_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) override final;
    VkResult execute_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) override final;
    VkResult execute_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) override final;
    void execute_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) override final;
    VkResult execute_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) override final;
    VkResult execute_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) override final;
    VkResult execute_vkEndCommandBuffer(VkCommandBuffer commandBuffer) override final;
    VkResult inspect_command_buffer(pipeline_explorer::QueueInfo queueInfo, pipeline_explorer::CommandBufferInfo commandBufferInfo, std::vector<const GvkCommandBaseStructure*>& cmds, std::vector<GvkPipelineExplorerCollectionRange>& collectionRanges);
    VkResult tool_command_buffer(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo);

    ////////////////////////////////////////////////////////////////////////////////
    // VkDescriptorSetLayout
    VkResult execute_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) override final;
    void execute_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) override final;
    VkResult create_replacement_descriptor_set_layout(VkDevice vkDevice, VkDescriptorSetLayout vkDescriptorSetLayout, gvk::DescriptorSetLayout* pGvkDescriptorSetLayout);

    ////////////////////////////////////////////////////////////////////////////////
    // VkPipeline
    VkResult execute_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) override final;
    VkResult execute_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) override final;
    VkResult execute_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) override final;
    void execute_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) override final;

    VkResult get_pipeline_executable_properties(pipeline_explorer::PipelineInfo pipelineInfo);
    void decompile_pipeline(VkDevice device, VkPipeline pipeline);
    void write_pipeline_info(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path);
    void read_pipeline_info(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, std::unordered_map<pipeline_explorer::UUID, std::string>* pShaderSource);
    VkResult create_replacement_pipeline(VkDevice device, VkPipeline pipeline, const std::unordered_map<pipeline_explorer::UUID, std::string>& shaderSource, gvk::Pipeline* pPipeline);
    VkResult create_replacement_shader_binding_table(const gvk::Device& gvkDevice, pipeline_explorer::QueueInfo queueInfo, VkCommandBuffer vkCommandBuffer, pipeline_explorer::PipelineInfo pipelineInfo, VkStridedDeviceAddressRegionKHR* pShaderBindingTable);
    VkResult create_replacement_shader_binding_tables(pipeline_explorer::QueueInfo queueInfo, pipeline_explorer::PipelineInfo pipelineInfo, GvkCommandStructureCmdTraceRaysKHR* pCmd);
    VkResult create_experiment_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path);
    std::vector<std::string> enable_experiment_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, VkBool32 enabled);
    VkResult create_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4]);
    std::vector<std::string> enable_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, VkBool32 enabled, const float color[4]);

    ////////////////////////////////////////////////////////////////////////////////
    // VkPipelineLayout
    VkResult execute_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) override final;
    void execute_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) override final;
    VkResult create_replacement_pipeline_layout(VkDevice vkDevice, VkPipelineLayout vkPipelineLayout, gvk::PipelineLayout* pGvkPipelineLayout);

    ////////////////////////////////////////////////////////////////////////////////
    // VkRenderPass
    VkResult execute_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) override final;
    VkResult execute_vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) override final;
    VkResult execute_vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) override final;
    void execute_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) override final;
    VkResult create_replacement_render_pass(VkDevice vkDevice, VkRenderPass vkRenderPass, gvk::RenderPass* pGvkRenderPass);

    ////////////////////////////////////////////////////////////////////////////////
    // VkSampler
    VkResult execute_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) override final;
    void execute_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) override final;
    VkResult create_replacement_sampler(VkDevice vkDevice, VkSampler vkSampler, gvk::Sampler* pGvkSampler);

    ////////////////////////////////////////////////////////////////////////////////
    // VkShaderModule and VkShaderEXT
    VkResult execute_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) override final;
    void execute_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) override final;
    VkResult execute_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) override final;
    void execute_vkDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // VkBuffer
    VkResult execute_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) override final;
    void execute_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) override final;

    ////////////////////////////////////////////////////////////////////////////////
    // VkDeviceMemory
    VkResult execute_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) override final;
    VkResult execute_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) override final;
    VkResult execute_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) override final;
    VkResult execute_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) override final;
    void execute_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) override final;

    void process_end_of_frame_and_outgoing_messages();
    void process_beginning_of_frame_and_incoming_messages();
#if 0
    std::string get_pipeline_report_name(VkDevice device, VkPipeline pipeline);
#endif
    std::vector<std::string> add_metric_result_to_report(VkDevice device, VkPipeline pipeline, GvkPipelineExplorerMetricId metricId, double value);
    std::vector<std::string> publish_metrics_report();
    VkResult handle_pre_process_command_buffer_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo);
    VkResult handle_pre_process_cmd_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo);
    VkResult handle_post_process_cmd_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo);
    VkResult handle_post_process_command_buffer_callback(GvkPipelineExplorerToolCommandBufferInfo toolCommandBufferInfo);
    VkResult handle_pre_process_queue_submission_callback(GvkPipelineExplorerToolQueueInfo toolQueueInfo);
    VkResult handle_post_process_queue_submission_callback(GvkPipelineExplorerToolQueueInfo toolQueueInfo);
    void reset();

    bool vkLayer{ };
    gvk::Instance gvkInstance;
    std::set<gvk::Device> gvkDevices;
    gvk::spirv::Context spirvContext;

    std::mutex queueSubmissionMutex;
    std::string applicationName;
    std::string targetApplicationName;
    std::filesystem::path workspacePath;
    std::vector<std::string> messages;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    PROCESS_INFORMATION guiProcessInformation{ };
#endif
    gvk::Auto<GvkPipelineExplorerRequestInfo> requestInfo;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, uint32_t> pipelineExecutionCounts;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, double> pipelineTimestampQueryResults;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::map<GvkPipelineExplorerMetricId, uint64_t>> pipelineStatisticsQueryResults;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::map<GvkPipelineExplorerMetricId, std::vector<double>>> metricsReport;
    std::map<GvkPipelineExplorerMetricId, gvk::Auto<GvkPipelineExplorerMetricInfo>> availableMetrics;
    GvkPipelineExplorerToolCommandBufferCallbackInfo toolCommandBufferCallbackInfo{ };

    pipeline_explorer::InstanceInfo instanceInfo;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::DeviceInfo::HandleId, pipeline_explorer::DeviceInfo> deviceInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::CommandPoolInfo::HandleId, pipeline_explorer::CommandPoolInfo> commandPoolInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::CommandBufferInfo::HandleId, pipeline_explorer::CommandBufferInfo> commandBufferInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::SamplerInfo::HandleId, pipeline_explorer::SamplerInfo> samplerInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::DescriptorSetLayoutInfo::HandleId, pipeline_explorer::DescriptorSetLayoutInfo> descriptorSetLayoutInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::PipelineLayoutInfo::HandleId, pipeline_explorer::PipelineLayoutInfo> pipelineLayoutInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::ShaderModuleInfo::HandleId, pipeline_explorer::ShaderModuleInfo> shaderModuleInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::RenderPassInfo::HandleId, pipeline_explorer::RenderPassInfo> renderPassInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::PipelineInfo::HandleId, pipeline_explorer::PipelineInfo> pipelineInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::BufferInfo::HandleId, pipeline_explorer::BufferInfo> bufferInfos;
    pipeline_explorer::ThreadSafeUnorderedMap<pipeline_explorer::DeviceMemoryInfo::HandleId, pipeline_explorer::DeviceMemoryInfo> deviceMemoryInfos;

    PipelineExplorer() = default;
    PipelineExplorer(const PipelineExplorer&) = delete;
    PipelineExplorer& operator=(const PipelineExplorer&) = delete;
};

} // namespace gvk

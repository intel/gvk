
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

////////////////////////////////////////////////////////////////////////////////
// TODO : Very annoying that Windows and Linux need different include orders for
//  these...that's a very good indicator that these utilities need a rework
#include "gvk-defines.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer.h"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-enumerations-to-string.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-comparison-operators.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-create-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-deserialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-destroy-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-get-stype.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-serialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-to-string.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "gvk-pipeline-explorer/backend/query-managers/command-collection-request-manager.hpp"
#include "gvk-pipeline-explorer/backend/query-managers/performance-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/query-managers/pipeline-statistics-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/query-managers/timeline-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/query-managers/timestamp-query-manager.hpp"
#include "gvk-pipeline-explorer/backend/handle-info.hpp"
#include "gvk-pipeline-explorer/backend/ipc-messenger.hpp"
#include "gvk-pipeline-explorer/backend/tool-dispatch-manager.hpp"
#include "gvk-pipeline-explorer/generated/basic-pipeline-explorer.hpp"
#include "gvk-pipeline-explorer/plugin-factory/plugin-manager.hpp"
#include "gvk-pipeline-explorer.hpp"

#include "gvk-containers/streambuf.hpp"
#include "gvk-containers/thread-safe-unordered-map.hpp"
#include "gvk-system/time.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-defines.hpp"
#include "gvk-handles.hpp"
#include "gvk-layer.hpp"
#include "gvk-runtime.hpp"
#include "gvk-spirv.hpp"
#include "gvk-structures.hpp"

#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"

#include <array>
#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
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
    VkResult post_execute_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) override final;

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
#if 0
    VkResult reset_timestamp_query_pool(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, uint32_t queryCount);
    void write_timestamp(pipeline_explorer::QueueInfo& queueInfo, VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage);
    VkResult get_timestamp_results(const pipeline_explorer::QueueInfo& queueInfo, std::vector<uint64_t>& results);
#endif

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
    VkResult tool_command_buffers(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo);

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

    void report_pipeline_creation(const pipeline_explorer::PipelineInfo& pipelineInfo);
    void report_pipeline_destruction(const pipeline_explorer::PipelineInfo& pipelineInfo);
    VkResult get_pipeline_executable_properties(pipeline_explorer::PipelineInfo pipelineInfo);
    void decompile_pipeline(VkDevice device, VkPipeline pipeline);
    void write_pipeline_info(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path);
    void read_pipeline_info(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, std::unordered_map<pipeline_explorer::UUID, std::string>* pShaderSource);
    void get_unmodified_pipeline_info(VkDevice device, VkPipeline pipeline, std::unordered_map<pipeline_explorer::UUID, std::string>* pShaderSource);
    VkResult create_replacement_pipeline(VkDevice device, VkPipeline pipeline, const std::unordered_map<pipeline_explorer::UUID, std::string>& shaderSource, gvk::Pipeline* pPipeline);
    VkResult create_replacement_shader_binding_table(const gvk::Device& gvkDevice, pipeline_explorer::QueueInfo queueInfo, VkCommandBuffer vkCommandBuffer, pipeline_explorer::PipelineInfo pipelineInfo, const gvk::ShaderGroupHandleMap& shaderGroupHandleMap, VkStridedDeviceAddressRegionKHR* pShaderBindingTable);
    VkResult create_replacement_shader_binding_tables(pipeline_explorer::QueueInfo queueInfo, pipeline_explorer::PipelineInfo pipelineInfo, const gvk::ShaderGroupHandleMap& shaderGroupHandleMap, GvkCommandStructureCmdTraceRaysKHR* pCmd);
    VkResult create_experiment_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path);
    std::vector<std::string> enable_experiment_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, VkBool32 enabled);
    VkResult create_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4]);
    VkResult create_graphics_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4]);
    VkResult create_ray_tracing_highlight_pipeline(VkDevice device, VkPipeline pipeline, const std::filesystem::path& path, const float color[4]);
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

    VkResult launch_gui(const std::filesystem::path& layerPath);
    void start_ipc_thread();
    void stop_ipc_thread();
    void enable_timeline_query();
    void disable_timeline_query();
    void process_incoming_messages();
    void process_end_of_frame_and_outgoing_messages();
    void process_beginning_of_frame_and_incoming_messages();
    std::vector<std::string> add_metric_result_to_report(VkDevice device, VkPipeline pipeline, GvkPipelineExplorerMetricId metricId, double value);
    std::vector<std::string> publish_metrics_report();
    ////////////////////////////////////////////////////////////////////////////////
    VkResult pre_process_range();
    VkResult pre_process_command_buffers(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo);
    VkResult pre_process_cmd(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo);
    VkResult post_process_cmd(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo);
    VkResult post_process_command_buffers(GvkPipelineExplorerToolCommandBufferInfoEx toolInfo);
    VkResult pre_process_queue_submission(GvkPipelineExplorerToolQueueInfoEx toolInfo);
    VkResult post_process_queue_submission(GvkPipelineExplorerToolQueueInfoEx toolInfo);
    VkResult pre_process_queue_present(GvkPipelineExplorerToolQueueInfoEx toolInfo);
    VkResult post_process_queue_present(GvkPipelineExplorerToolQueueInfoEx toolInfo);
    VkResult post_process_range();
    ////////////////////////////////////////////////////////////////////////////////
    void reset();

    bool vkLayer{ };
    gvk::Instance mGvkInstance;
    std::set<gvk::Device> mGvkDevices;
    gvk::spirv::Context mSpirvContext;
#ifdef GVK_PLATFORM_WINDOWS
    gvk::NamedPipe mIpcPipe;
#endif
    gvk::pipeline_explorer::IpcMessenger mIpcMessenger;
    bool mTimelineQuery{ };

    asio::io_context mIpcContext;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> mupIpcWorkGuard;
    std::unique_ptr<asio::steady_timer> mupIpcTimer;
    std::thread mIpcThread;

    gvk::system::Timer mFrameTimer;

    bool mHeadless{ };
    std::filesystem::path mReportPath;
#if 0
    bool autoQuery{ };
#endif
    bool requestQuery{ };
    std::mutex queueSubmissionMutex;
    std::string applicationName;
    std::string targetApplicationName;
    std::filesystem::path workspacePath;
    std::vector<std::string> messages;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    PROCESS_INFORMATION guiProcessInformation{ };
#endif
    gvk::Auto<GvkPipelineExplorerRequestInfo> requestInfo;
    gvk::pipeline_explorer::PerformanceQueryManager mPerformanceQueryManager;
    gvk::pipeline_explorer::PipelineStatisticsQueryManager mPipelineStatisticsQueryManager;
    gvk::pipeline_explorer::TimelineQueryManager mTimelineQueryManager;
    gvk::pipeline_explorer::TimestampQueryManager mTimestampQueryManager;
    gvk::pipeline_explorer::Tool::DispatchManager mToolDispatchManager;
    gvk::pipeline_explorer::CommandCollectionRequestManager mCommandCollectionRequestManager;
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, uint32_t> pipelineExecutionCounts;
#if 0
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, double> pipelineTimestampQueryResults;
#endif
    std::unordered_map<gvk::HandleId<VkDevice, VkPipeline>, std::map<GvkPipelineExplorerMetricId, std::vector<double>>> metricsReport;
    std::map<GvkPipelineExplorerMetricId, gvk::Auto<GvkPipelineExplorerMetricInfo>> availableMetrics;
    GvkPipelineExplorerToolCommandBufferCallbackInfo toolCommandBufferCallbackInfo{ };
#if 0
    GvkPipelineExplorerToolCallbackInfoEx toolCallbackInfo{ };
#endif
    pipeline_explorer::PluginManager pluginManager;

    pipeline_explorer::InstanceInfo instanceInfo;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::DeviceInfo::HandleId, pipeline_explorer::DeviceInfo> deviceInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::CommandPoolInfo::HandleId, pipeline_explorer::CommandPoolInfo> commandPoolInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::CommandBufferInfo::HandleId, pipeline_explorer::CommandBufferInfo> commandBufferInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::SamplerInfo::HandleId, pipeline_explorer::SamplerInfo> samplerInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::DescriptorSetLayoutInfo::HandleId, pipeline_explorer::DescriptorSetLayoutInfo> descriptorSetLayoutInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::PipelineLayoutInfo::HandleId, pipeline_explorer::PipelineLayoutInfo> pipelineLayoutInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::ShaderModuleInfo::HandleId, pipeline_explorer::ShaderModuleInfo> shaderModuleInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::RenderPassInfo::HandleId, pipeline_explorer::RenderPassInfo> renderPassInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::PipelineInfo::HandleId, pipeline_explorer::PipelineInfo> pipelineInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::BufferInfo::HandleId, pipeline_explorer::BufferInfo> bufferInfos;
    gvk::ThreadSafeUnorderedMap<pipeline_explorer::DeviceMemoryInfo::HandleId, pipeline_explorer::DeviceMemoryInfo> deviceMemoryInfos;

    bool TODO_shouldBeControlledByRequestInfo_getGpuCalls{ };
    BasicCommandRecorder mGpuCalls;

    PipelineExplorer() = default;
    PipelineExplorer(const PipelineExplorer&) = delete;
    PipelineExplorer& operator=(const PipelineExplorer&) = delete;
};

} // namespace gvk

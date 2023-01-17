
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

#include "gvk-sample-entry-points.hpp"

#ifdef VK_NO_PROTOTYPES
PFN_vkCreateInstance vkCreateInstance;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
PFN_vkQueueBindSparse vkQueueBindSparse;
PFN_vkCreateFence vkCreateFence;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkResetFences vkResetFences;
PFN_vkGetFenceStatus vkGetFenceStatus;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkCreateEvent vkCreateEvent;
PFN_vkDestroyEvent vkDestroyEvent;
PFN_vkGetEventStatus vkGetEventStatus;
PFN_vkSetEvent vkSetEvent;
PFN_vkResetEvent vkResetEvent;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkCreateBufferView vkCreateBufferView;
PFN_vkDestroyBufferView vkDestroyBufferView;
PFN_vkCreateImage vkCreateImage;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
PFN_vkMergePipelineCaches vkMergePipelineCaches;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkResetCommandPool vkResetCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
PFN_vkCmdFillBuffer vkCmdFillBuffer;
PFN_vkCmdClearColorImage vkCmdClearColorImage;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCmdResolveImage vkCmdResolveImage;
PFN_vkCmdSetEvent vkCmdSetEvent;
PFN_vkCmdResetEvent vkCmdResetEvent;
PFN_vkCmdWaitEvents vkCmdWaitEvents;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;

void gvk_sample_set_entry_points(const gvk::Device& device)
{
    const auto& instanceDispatchTable = device.get<gvk::Instance>().get<gvk::DispatchTable>();
    const auto& deviceDispatchTable = device.get<gvk::DispatchTable>();
    vkCreateInstance = instanceDispatchTable.gvkCreateInstance;
    vkDestroyInstance = instanceDispatchTable.gvkDestroyInstance;
    vkEnumeratePhysicalDevices = instanceDispatchTable.gvkEnumeratePhysicalDevices;
    vkGetPhysicalDeviceFeatures = instanceDispatchTable.gvkGetPhysicalDeviceFeatures;
    vkGetPhysicalDeviceFormatProperties = instanceDispatchTable.gvkGetPhysicalDeviceFormatProperties;
    vkGetPhysicalDeviceImageFormatProperties = instanceDispatchTable.gvkGetPhysicalDeviceImageFormatProperties;
    vkGetPhysicalDeviceProperties = instanceDispatchTable.gvkGetPhysicalDeviceProperties;
    vkGetPhysicalDeviceQueueFamilyProperties = instanceDispatchTable.gvkGetPhysicalDeviceQueueFamilyProperties;
    vkGetPhysicalDeviceMemoryProperties = instanceDispatchTable.gvkGetPhysicalDeviceMemoryProperties;
    vkGetInstanceProcAddr = instanceDispatchTable.gvkGetInstanceProcAddr;
    vkGetDeviceProcAddr = deviceDispatchTable.gvkGetDeviceProcAddr;
    vkCreateDevice = instanceDispatchTable.gvkCreateDevice;
    vkDestroyDevice = instanceDispatchTable.gvkDestroyDevice;
    vkEnumerateInstanceExtensionProperties = instanceDispatchTable.gvkEnumerateInstanceExtensionProperties;
    vkEnumerateDeviceExtensionProperties = instanceDispatchTable.gvkEnumerateDeviceExtensionProperties;
    vkEnumerateInstanceLayerProperties = instanceDispatchTable.gvkEnumerateInstanceLayerProperties;
    vkEnumerateDeviceLayerProperties = instanceDispatchTable.gvkEnumerateDeviceLayerProperties;
    vkGetDeviceQueue = deviceDispatchTable.gvkGetDeviceQueue;
    vkQueueSubmit = deviceDispatchTable.gvkQueueSubmit;
    vkQueueWaitIdle = deviceDispatchTable.gvkQueueWaitIdle;
    vkDeviceWaitIdle = deviceDispatchTable.gvkDeviceWaitIdle;
    vkAllocateMemory = deviceDispatchTable.gvkAllocateMemory;
    vkFreeMemory = deviceDispatchTable.gvkFreeMemory;
    vkMapMemory = deviceDispatchTable.gvkMapMemory;
    vkUnmapMemory = deviceDispatchTable.gvkUnmapMemory;
    vkFlushMappedMemoryRanges = deviceDispatchTable.gvkFlushMappedMemoryRanges;
    vkInvalidateMappedMemoryRanges = deviceDispatchTable.gvkInvalidateMappedMemoryRanges;
    vkGetDeviceMemoryCommitment = deviceDispatchTable.gvkGetDeviceMemoryCommitment;
    vkBindBufferMemory = deviceDispatchTable.gvkBindBufferMemory;
    vkBindImageMemory = deviceDispatchTable.gvkBindImageMemory;
    vkGetBufferMemoryRequirements = deviceDispatchTable.gvkGetBufferMemoryRequirements;
    vkGetImageMemoryRequirements = deviceDispatchTable.gvkGetImageMemoryRequirements;
    vkGetImageSparseMemoryRequirements = deviceDispatchTable.gvkGetImageSparseMemoryRequirements;
    vkGetPhysicalDeviceSparseImageFormatProperties = deviceDispatchTable.gvkGetPhysicalDeviceSparseImageFormatProperties;
    vkQueueBindSparse = deviceDispatchTable.gvkQueueBindSparse;
    vkCreateFence = deviceDispatchTable.gvkCreateFence;
    vkDestroyFence = deviceDispatchTable.gvkDestroyFence;
    vkResetFences = deviceDispatchTable.gvkResetFences;
    vkGetFenceStatus = deviceDispatchTable.gvkGetFenceStatus;
    vkWaitForFences = deviceDispatchTable.gvkWaitForFences;
    vkCreateSemaphore = deviceDispatchTable.gvkCreateSemaphore;
    vkDestroySemaphore = deviceDispatchTable.gvkDestroySemaphore;
    vkCreateEvent = deviceDispatchTable.gvkCreateEvent;
    vkDestroyEvent = deviceDispatchTable.gvkDestroyEvent;
    vkGetEventStatus = deviceDispatchTable.gvkGetEventStatus;
    vkSetEvent = deviceDispatchTable.gvkSetEvent;
    vkResetEvent = deviceDispatchTable.gvkResetEvent;
    vkCreateQueryPool = deviceDispatchTable.gvkCreateQueryPool;
    vkDestroyQueryPool = deviceDispatchTable.gvkDestroyQueryPool;
    vkGetQueryPoolResults = deviceDispatchTable.gvkGetQueryPoolResults;
    vkCreateBuffer = deviceDispatchTable.gvkCreateBuffer;
    vkDestroyBuffer = deviceDispatchTable.gvkDestroyBuffer;
    vkCreateBufferView = deviceDispatchTable.gvkCreateBufferView;
    vkDestroyBufferView = deviceDispatchTable.gvkDestroyBufferView;
    vkCreateImage = deviceDispatchTable.gvkCreateImage;
    vkDestroyImage = deviceDispatchTable.gvkDestroyImage;
    vkGetImageSubresourceLayout = deviceDispatchTable.gvkGetImageSubresourceLayout;
    vkCreateImageView = deviceDispatchTable.gvkCreateImageView;
    vkDestroyImageView = deviceDispatchTable.gvkDestroyImageView;
    vkCreateShaderModule = deviceDispatchTable.gvkCreateShaderModule;
    vkDestroyShaderModule = deviceDispatchTable.gvkDestroyShaderModule;
    vkCreatePipelineCache = deviceDispatchTable.gvkCreatePipelineCache;
    vkDestroyPipelineCache = deviceDispatchTable.gvkDestroyPipelineCache;
    vkGetPipelineCacheData = deviceDispatchTable.gvkGetPipelineCacheData;
    vkMergePipelineCaches = deviceDispatchTable.gvkMergePipelineCaches;
    vkCreateGraphicsPipelines = deviceDispatchTable.gvkCreateGraphicsPipelines;
    vkCreateComputePipelines = deviceDispatchTable.gvkCreateComputePipelines;
    vkDestroyPipeline = deviceDispatchTable.gvkDestroyPipeline;
    vkCreatePipelineLayout = deviceDispatchTable.gvkCreatePipelineLayout;
    vkDestroyPipelineLayout = deviceDispatchTable.gvkDestroyPipelineLayout;
    vkCreateSampler = deviceDispatchTable.gvkCreateSampler;
    vkDestroySampler = deviceDispatchTable.gvkDestroySampler;
    vkCreateDescriptorSetLayout = deviceDispatchTable.gvkCreateDescriptorSetLayout;
    vkDestroyDescriptorSetLayout = deviceDispatchTable.gvkDestroyDescriptorSetLayout;
    vkCreateDescriptorPool = deviceDispatchTable.gvkCreateDescriptorPool;
    vkDestroyDescriptorPool = deviceDispatchTable.gvkDestroyDescriptorPool;
    vkResetDescriptorPool = deviceDispatchTable.gvkResetDescriptorPool;
    vkAllocateDescriptorSets = deviceDispatchTable.gvkAllocateDescriptorSets;
    vkFreeDescriptorSets = deviceDispatchTable.gvkFreeDescriptorSets;
    vkUpdateDescriptorSets = deviceDispatchTable.gvkUpdateDescriptorSets;
    vkCreateFramebuffer = deviceDispatchTable.gvkCreateFramebuffer;
    vkDestroyFramebuffer = deviceDispatchTable.gvkDestroyFramebuffer;
    vkCreateRenderPass = deviceDispatchTable.gvkCreateRenderPass;
    vkDestroyRenderPass = deviceDispatchTable.gvkDestroyRenderPass;
    vkGetRenderAreaGranularity = deviceDispatchTable.gvkGetRenderAreaGranularity;
    vkCreateCommandPool = deviceDispatchTable.gvkCreateCommandPool;
    vkDestroyCommandPool = deviceDispatchTable.gvkDestroyCommandPool;
    vkResetCommandPool = deviceDispatchTable.gvkResetCommandPool;
    vkAllocateCommandBuffers = deviceDispatchTable.gvkAllocateCommandBuffers;
    vkFreeCommandBuffers = deviceDispatchTable.gvkFreeCommandBuffers;
    vkBeginCommandBuffer = deviceDispatchTable.gvkBeginCommandBuffer;
    vkEndCommandBuffer = deviceDispatchTable.gvkEndCommandBuffer;
    vkResetCommandBuffer = deviceDispatchTable.gvkResetCommandBuffer;
    vkCmdBindPipeline = deviceDispatchTable.gvkCmdBindPipeline;
    vkCmdSetViewport = deviceDispatchTable.gvkCmdSetViewport;
    vkCmdSetScissor = deviceDispatchTable.gvkCmdSetScissor;
    vkCmdSetLineWidth = deviceDispatchTable.gvkCmdSetLineWidth;
    vkCmdSetDepthBias = deviceDispatchTable.gvkCmdSetDepthBias;
    vkCmdSetBlendConstants = deviceDispatchTable.gvkCmdSetBlendConstants;
    vkCmdSetDepthBounds = deviceDispatchTable.gvkCmdSetDepthBounds;
    vkCmdSetStencilCompareMask = deviceDispatchTable.gvkCmdSetStencilCompareMask;
    vkCmdSetStencilWriteMask = deviceDispatchTable.gvkCmdSetStencilWriteMask;
    vkCmdSetStencilReference = deviceDispatchTable.gvkCmdSetStencilReference;
    vkCmdBindDescriptorSets = deviceDispatchTable.gvkCmdBindDescriptorSets;
    vkCmdBindIndexBuffer = deviceDispatchTable.gvkCmdBindIndexBuffer;
    vkCmdBindVertexBuffers = deviceDispatchTable.gvkCmdBindVertexBuffers;
    vkCmdDraw = deviceDispatchTable.gvkCmdDraw;
    vkCmdDrawIndexed = deviceDispatchTable.gvkCmdDrawIndexed;
    vkCmdDrawIndirect = deviceDispatchTable.gvkCmdDrawIndirect;
    vkCmdDrawIndexedIndirect = deviceDispatchTable.gvkCmdDrawIndexedIndirect;
    vkCmdDispatch = deviceDispatchTable.gvkCmdDispatch;
    vkCmdDispatchIndirect = deviceDispatchTable.gvkCmdDispatchIndirect;
    vkCmdCopyBuffer = deviceDispatchTable.gvkCmdCopyBuffer;
    vkCmdCopyImage = deviceDispatchTable.gvkCmdCopyImage;
    vkCmdBlitImage = deviceDispatchTable.gvkCmdBlitImage;
    vkCmdCopyBufferToImage = deviceDispatchTable.gvkCmdCopyBufferToImage;
    vkCmdCopyImageToBuffer = deviceDispatchTable.gvkCmdCopyImageToBuffer;
    vkCmdUpdateBuffer = deviceDispatchTable.gvkCmdUpdateBuffer;
    vkCmdFillBuffer = deviceDispatchTable.gvkCmdFillBuffer;
    vkCmdClearColorImage = deviceDispatchTable.gvkCmdClearColorImage;
    vkCmdClearDepthStencilImage = deviceDispatchTable.gvkCmdClearDepthStencilImage;
    vkCmdClearAttachments = deviceDispatchTable.gvkCmdClearAttachments;
    vkCmdResolveImage = deviceDispatchTable.gvkCmdResolveImage;
    vkCmdSetEvent = deviceDispatchTable.gvkCmdSetEvent;
    vkCmdResetEvent = deviceDispatchTable.gvkCmdResetEvent;
    vkCmdWaitEvents = deviceDispatchTable.gvkCmdWaitEvents;
    vkCmdPipelineBarrier = deviceDispatchTable.gvkCmdPipelineBarrier;
    vkCmdBeginQuery = deviceDispatchTable.gvkCmdBeginQuery;
    vkCmdEndQuery = deviceDispatchTable.gvkCmdEndQuery;
    vkCmdResetQueryPool = deviceDispatchTable.gvkCmdResetQueryPool;
    vkCmdWriteTimestamp = deviceDispatchTable.gvkCmdWriteTimestamp;
    vkCmdCopyQueryPoolResults = deviceDispatchTable.gvkCmdCopyQueryPoolResults;
    vkCmdPushConstants = deviceDispatchTable.gvkCmdPushConstants;
    vkCmdBeginRenderPass = deviceDispatchTable.gvkCmdBeginRenderPass;
    vkCmdNextSubpass = deviceDispatchTable.gvkCmdNextSubpass;
    vkCmdEndRenderPass = deviceDispatchTable.gvkCmdEndRenderPass;
    vkCmdExecuteCommands = deviceDispatchTable.gvkCmdExecuteCommands;
    vkCreateSwapchainKHR = deviceDispatchTable.gvkCreateSwapchainKHR;
    vkDestroySwapchainKHR = deviceDispatchTable.gvkDestroySwapchainKHR;
    vkGetSwapchainImagesKHR = deviceDispatchTable.gvkGetSwapchainImagesKHR;
    vkAcquireNextImageKHR = deviceDispatchTable.gvkAcquireNextImageKHR;
    vkQueuePresentKHR = deviceDispatchTable.gvkQueuePresentKHR;
}
#endif // VK_NO_PROTOTYPES

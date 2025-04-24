
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

#include "gvk-handles.hpp"

namespace gvk {

struct GpuMemcpyInfo
{
    VkDeviceAddress dst{ };
    VkDeviceAddress src{ };
    VkDeviceSize size{ };
};

/**
Creates a GPU memory copy pipeline.
@param [in] gvkDevice The Vulkan device used to create the pipeline.
@param [out] pGvkPipeline A pointer to a `gvk::Pipeline` object where the created pipeline will be stored.
    @note The following Vulkan features must be enabled to use this functionality:
    - `bufferDeviceAddress`
    - `storageBuffer8BitAccess`
    - `shaderInt64`
@return The `VkResult` indicating success or failure of the pipeline creation.
*/
VkResult create_gpu_memcpy_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pGvkPipeline);

/**
Executes a GPU memory copy operation.
@param [in] gvkDevice The Vulkan device used to execute the memory copy operation.
@param [in] vkQueue The Vulkan queue on which the memory copy operation will be submitted.
@param [in] vkCommandBuffer The Vulkan command buffer used to record the memory copy commands.
@param [in](optional) vkFence A Vulkan fence used to synchronize the memory copy operation. The fence will be signaled once the operation is complete.
@param [in] pGpuMemcpyInfo A pointer to a `gvk::GpuMemcpyInfo` structure containing the source and destination buffer addresses and the size of the data to copy.
@param [in] gpuMemcpyPipeline The `gvk::Pipeline` object representing the GPU memory copy pipeline.
    @note The following Vulkan features must be enabled to use this functionality:
    - `bufferDeviceAddress`
    - `storageBuffer8BitAccess`
    - `shaderInt64`
@return The `VkResult` indicating success or failure of the memory copy operation.
*/
VkResult execute_gpu_memcpy(const gvk::Device& gvkDevice, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, VkFence vkFence, const GpuMemcpyInfo* pGpuMemcpyInfo, const gvk::Pipeline& gpuMemcpyPipeline);

} // namespace gvk

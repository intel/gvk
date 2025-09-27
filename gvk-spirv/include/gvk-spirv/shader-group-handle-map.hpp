
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

struct ShaderGroupHandleMapCreateInfo
{
    VkPipeline keysPipeline{ };
    const uint8_t* pShaderGroupHandleKeys{ };
    VkPipeline valuesPipeline{ };
    const uint8_t* pShaderGroupHandleValues{ };
    uint32_t shaderGroupHandleCount{ };
};

class ShaderGroupHandleMap
{
public:
    gvk::Buffer gvkBuffer;
    VkDeviceAddress keys{ };
    VkDeviceAddress values{ };
    VkDeviceSize kvpCount{ };
    VkDeviceSize handleSize{ };

    inline operator bool() const
    {
        return gvkBuffer && keys && values && kvpCount && handleSize;
    }
};

/**
Creates a shader group handle map buffer and initializes its device addresses.
@param [in] gvkDevice The Vulkan device used to create the buffer.
@param [in] pShaderGroupHandleMapCreateInfo A pointer to a `gvk::ShaderGroupHandleMapCreateInfo` structure containing the pipelines and shader group handle data for keys and values.
@param [out] pShaderGroupHandleMap A pointer to a `gvk::ShaderGroupHandleMap` object where the created buffer and device addresses will be stored.
    @note The following Vulkan features must be enabled to use this functionality:
    - `bufferDeviceAddress`
    - `storageBuffer8BitAccess`
    - `shaderInt64`
@return The `VkResult` indicating success or failure of the shader group handle map creation.
*/
VkResult create_shader_group_handle_map(const gvk::Device& gvkDevice, const gvk::ShaderGroupHandleMapCreateInfo* pShaderGroupHandleMapCreateInfo, gvk::ShaderGroupHandleMap* pShaderGroupHandleMap);

/**
Creates a pipeline for mapping shader group handles.
@param [in] gvkDevice The Vulkan device used to create the pipeline.
@param [out] pShaderGroupHandleMapPipeline A pointer to a `gvk::Pipeline` object where the created pipeline will be stored.
    @note The following Vulkan features must be enabled to use this functionality:
    - `bufferDeviceAddress`
    - `storageBuffer8BitAccess`
    - `shaderInt64`
@return The `VkResult` indicating success or failure of the pipeline creation.
*/
VkResult create_shader_group_handle_map_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pShaderGroupHandleMapPipeline);

struct GpuAddressMapInfo
{
    VkDeviceAddress dst{ };
    VkDeviceAddress src{ };
    VkDeviceSize stride{ };
    VkDeviceSize count{ };
    VkDeviceAddress keys{ };
    VkDeviceAddress values{ };
    VkDeviceSize kvpCount{ };
};

/**
Executes a GPU operation to map shader group handles from source to destination using the provided pipeline.
@param [in] gvkDevice The Vulkan device used to execute the mapping operation.
@param [in] vkQueue The Vulkan queue on which the mapping operation will be submitted.
@param [in] vkCommandBuffer The Vulkan command buffer used to record the mapping commands.
@param [in](optional) vkFence A Vulkan fence used to synchronize the mapping operation. The fence will be signaled once the operation is complete.
@param [in] pGpuAddressMapInfo A pointer to a `gvk::GpuAddressMapInfo` structure containing the source and destination addresses, stride, count, and key-value mapping information.
@param [in] shaderGroupHandleMapPipeline The `gvk::Pipeline` object representing the shader group handle map pipeline.
    @note The following Vulkan features must be enabled to use this functionality:
    - `bufferDeviceAddress`
    - `storageBuffer8BitAccess`
    - `shaderInt64`
@return The `VkResult` indicating success or failure of the mapping operation.
*/
VkResult execute_shader_group_handle_map(const gvk::Device& gvkDevice, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, VkFence vkFence, const GpuAddressMapInfo* pGpuAddressMapInfo, const gvk::Pipeline& shaderGroupHandleMapPipeline);

} // namespace gvk

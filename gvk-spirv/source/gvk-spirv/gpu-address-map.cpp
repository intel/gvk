
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

#include "gvk-spirv/gpu-address-map.hpp"
#include "gvk-spirv/context.hpp"

namespace gvk {

VkResult create_gpu_address_map_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pGpuAddressMapPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGpuAddressMapPipeline ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // Compile GLSL to SPIR-V
        gvk::spirv::Context spirvContext;
        gvk_result(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext));
        auto shaderInfo = gvk::get_default<gvk::spirv::ShaderInfo>();
        shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderInfo.lineOffset = __LINE__;
        shaderInfo.source = R"(

            #version 450

            #extension GL_EXT_buffer_reference : require
            #extension GL_EXT_scalar_block_layout : require
            #extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
            #extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

            #define VkDeviceAddress uint64_t
            #define VkDeviceSize uint64_t
            #define SHADER_GROUP_HANDLE_SIZE 32

            struct ShaderGroupHandle
            {
                uint8_t data[SHADER_GROUP_HANDLE_SIZE];
            };

            // TODO : Query GPU to create shader with ideal workgroup size
            layout(local_size_x = 16) in;
            layout(buffer_reference, scalar) writeonly buffer Dst { uint8_t data[]; };
            layout(buffer_reference, scalar) readonly buffer Src { uint8_t data[]; };
            layout(buffer_reference, scalar) readonly buffer Keys { ShaderGroupHandle data[]; };
            layout(buffer_reference, scalar) readonly buffer Values { ShaderGroupHandle data[]; };

            layout(push_constant, scalar) uniform PushConstants {
                VkDeviceAddress dst;
                VkDeviceAddress src;
                VkDeviceSize stride;
                VkDeviceSize count;
                VkDeviceAddress keys;
                VkDeviceAddress values;
                VkDeviceSize kvpCount;
            } pc;

            bool less(ShaderGroupHandle lhs, ShaderGroupHandle rhs)
            {
                #if 0
                for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                    if (lhs.data[i] < rhs.data[i]) {
                        return true;
                    } else if (rhs.data[i] < lhs.data[i]) {
                        return false;
                    }
                }
                #else
                for (uint i = 32; 0 < i--;) {
                    if (lhs.data[i] < rhs.data[i]) {
                        return true;
                    } else if (rhs.data[i] < lhs.data[i]) {
                        return false;
                    }
                }
                #endif
                return false;
            }

            bool equal(ShaderGroupHandle lhs, ShaderGroupHandle rhs)
            {
                for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                    if (lhs.data[i] != rhs.data[i]) {
                        return false;
                    }
                }
                return true;
            }

            ShaderGroupHandle get_shader_group_handle_map_value(ShaderGroupHandle key)
            {
                #if 0
                // Perform a binary search to find the mapped shader group handle.
                uint left = 0;
                uint right = uint(pc.kvpCount);
                while (left < right) {
                    uint mid = left + (right - left) / 2;
                    ShaderGroupHandle midKey = Keys(pc.keys).data[mid];
                    if (less(midKey, key)) {
                        left = mid + 1;
                    } else if (less(key, midKey)) {
                        right = mid;
                    } else {
                        return Values(pc.values).data[mid];
                    }
                }
                #else
                // Linear search, useful for debugging
                for (uint i = 0; i < uint(pc.kvpCount); ++i) {
                    ShaderGroupHandle shaderGroupHandle = Keys(pc.keys).data[i];
                    if (equal(key, shaderGroupHandle)) {
                        return Values(pc.values).data[i];
                    }
                }
                #endif

                #if 0
                // If the key isn't found, then something went wrong CPU side and there's not
                //  really anything that can be done to recover from here...just return the
                //  given handle.
                ShaderGroupHandle nullShaderGroupHandle;
                for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                    nullShaderGroupHandle.data[i] = uint8_t(i);
                }
                return nullShaderGroupHandle;
                #else
                return key;
                #endif
            }

            void main()
            {
                if (gl_GlobalInvocationID.x < pc.count) {

                    // Get the offset for the current entry
                    uint offset = gl_GlobalInvocationID.x * uint(pc.stride);

                    // Read the shader group handle at the beginning of the current Src entry
                    ShaderGroupHandle key;
                    for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                        key.data[i] = Src(pc.src).data[offset + i];
                    }

                    // Get the mapped value
                    ShaderGroupHandle value = get_shader_group_handle_map_value(key);

                    // Write the shader group handle to the beginning of the current Dst entry
                    for (uint i = 0; i < SHADER_GROUP_HANDLE_SIZE; ++i) {
                        Dst(pc.dst).data[offset + i] = value.data[i];
                    }
                }
            }

        )";
        gvk_result(spirvContext.compile(&shaderInfo));

        // Create shader module
        auto shaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        shaderModuleCreateInfo.codeSize = shaderInfo.bytecode.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode = !shaderInfo.bytecode.empty() ? shaderInfo.bytecode.data() : nullptr;
        gvk::ShaderModule gvkShaderModule;
        gvk_result(gvk::ShaderModule::create(gvkDevice, &shaderModuleCreateInfo, nullptr, &gvkShaderModule));

        // Create pipeline layout
        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(shaderInfo);
        gvk::PipelineLayout gvkPipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(gvkDevice, spirvBindingInfo, nullptr, &gvkPipelineLayout));

        // Create pipeline
        auto computePipelineCreateInfo = gvk::get_default<VkComputePipelineCreateInfo>();
        computePipelineCreateInfo.stage.module = gvkShaderModule;
        computePipelineCreateInfo.layout = gvkPipelineLayout;
        gvk_result(gvk::Pipeline::create(gvkDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pGpuAddressMapPipeline));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult execute_gpu_address_map(const gvk::Device& gvkDevice, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, VkFence vkFence, const GpuAddressMapInfo* pGpuAddressMapInfo, const gvk::Pipeline& gpuAddressMapPipeline)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(vkQueue ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(vkCommandBuffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pGpuAddressMapInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(gpuAddressMapPipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(gvk::execute_immediately(gvkDevice, vkQueue, vkCommandBuffer, vkFence,
            [&](auto)
            {
                gvkDevice.get<DispatchTable>().gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpuAddressMapPipeline);
                gvkDevice.get<DispatchTable>().gvkCmdPushConstants(vkCommandBuffer, gpuAddressMapPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GpuAddressMapInfo), pGpuAddressMapInfo);
                // TODO : Query GPU to create shader with ideal workgroup size
                static const uint32_t local_size_x = 16;
                auto workgroupSize = (uint32_t)((pGpuAddressMapInfo->count + local_size_x - 1) / (uint64_t)local_size_x);
                gvkDevice.get<DispatchTable>().gvkCmdDispatch(vkCommandBuffer, workgroupSize, 1, 1);
            }
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk

#if 0

// The size of each SBT entry is:
entrySize = Align(shaderGroupHandleSize + sizeof(CustomData), shaderGroupBaseAlignment);

// Copy the shader group handles and custom data into the buffer. Ensure each entry is properly aligned.
uint8_t* sbtData = new uint8_t[numShaderGroups * entrySize];
for (size_t i = 0; i < numShaderGroups; ++i) {
    // Copy the shader group handle
    memcpy(sbtData + i * entrySize, shaderGroupHandles.data() + i * shaderGroupHandleSize, shaderGroupHandleSize);
    // Append custom data
    CustomData customData = { /* Initialize your custom data */ };
    memcpy(sbtData + i * entrySize + shaderGroupHandleSize, &customData, sizeof(CustomData));
}
// Copy the SBT data to the Vulkan buffer
void* mappedMemory;
vkMapMemory(device, sbtBufferMemory, 0, VK_WHOLE_SIZE, 0, &mappedMemory);
memcpy(mappedMemory, sbtData, numShaderGroups * entrySize);
vkUnmapMemory(device, sbtBufferMemory);

// Set up the VkStridedDeviceAddressRegionKHR structure for each shader group type. The stride must match the entrySize calculated earlier.
VkStridedDeviceAddressRegionKHR raygenSBT = {};
raygenSBT.deviceAddress = raygenBufferDeviceAddress;
raygenSBT.stride = entrySize; // Includes shader group handle + custom data
raygenSBT.size = entrySize * numRaygenShaders;

#endif

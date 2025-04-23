
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

#include "gvk-spirv/gpu-memcpy.hpp"
#include "gvk-spirv/context.hpp"

namespace gvk {

VkResult create_gpu_memcpy_pipeline(const gvk::Device& gvkDevice, gvk::Pipeline* pGvkPipeline)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pGvkPipeline ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

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

            // TODO : Query GPU to create shader with ideal workgroup size
            layout(local_size_x = 16) in;
            layout(buffer_reference, scalar) writeonly buffer Dst { uint8_t data[]; };
            layout(buffer_reference, scalar) readonly buffer Src { uint8_t data[]; };

            layout(push_constant) uniform PushConstants {
                uint64_t dst;
                uint64_t src;
                uint64_t size;
            } pc;

            void main()
            {
                if (gl_GlobalInvocationID.x < pc.size) {
                    Dst(pc.dst).data[gl_GlobalInvocationID.x] = Src(pc.src).data[gl_GlobalInvocationID.x];
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
        gvk_result(gvk::Pipeline::create(gvkDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, pGvkPipeline));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult execute_gpu_memcpy(const gvk::Device& gvkDevice, VkQueue vkQueue, VkCommandBuffer vkCommandBuffer, VkFence vkFence, const GpuMemcpyInfo* pGpuMemcpyInfo, const gvk::Pipeline& gpuMemcpyPipeline)
{
    gvk_result_scope_begin(VK_ERROR_UNKNOWN) {
        gvk_result(gvkDevice ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(vkQueue ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(vkCommandBuffer ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(pGpuMemcpyInfo ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(gpuMemcpyPipeline ? VK_SUCCESS : VK_ERROR_UNKNOWN);
        gvk_result(gvk::execute_immediately(gvkDevice, vkQueue, vkCommandBuffer, vkFence,
            [&](auto)
            {
                gvkDevice.get<DispatchTable>().gvkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpuMemcpyPipeline);
                gvkDevice.get<DispatchTable>().gvkCmdPushConstants(vkCommandBuffer, gpuMemcpyPipeline.get<gvk::PipelineLayout>(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GpuMemcpyInfo), pGpuMemcpyInfo);
                // TODO : Query GPU to create shader with ideal workgroup size
                static const uint32_t local_size_x = 16;
                auto workgroupSize = (uint32_t)((pGpuMemcpyInfo->size + local_size_x - 1) / (uint64_t)local_size_x);
                gvkDevice.get<DispatchTable>().gvkCmdDispatch(vkCommandBuffer, workgroupSize, 1, 1);
            }
        ));
    } gvk_result_scope_end;
    return gvkResult;
}

} // namespace gvk
